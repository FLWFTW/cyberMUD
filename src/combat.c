/**
 * Functions supporting MUD combat
 */
#include "mud.h"

static enum bodyparts_t random_part()
{
   size_t part = roll( 1, 100 );

   if( part < 3 ) //2% chance
      return BODY_HEAD;
   else if( part == 3 ) //1% chance
      return BODY_EYES;
   else if( part < 7 )  //3% chance
      return BODY_FACE;
   else if( part == 7 ) //1% chance
      return BODY_NECK;
   else if( part < 15 )  //7% chance
      return BODY_LARM;
   else if( part == 15 ) //1% chance
      return BODY_LHAND;
   else if( part < 23 ) //7% chance
      return BODY_RARM;
   else if( part == 23 ) //1% chance
      return BODY_RHAND;
   else if( part < 74 ) //50% chance
      return BODY_TORSO;
   else if( part < 79 ) //5% chance
      return BODY_GROIN;
   else if( part < 89 ) //10% chance
      return BODY_LLEG;
   else if( part == 89 ) //1% chance
      return BODY_LFOOT;
   else if( part < 100 ) //10% chance
      return BODY_RLEG;
   else if( part == 100 ) //1% chance
      return BODY_RFOOT;
   else
      bug( "How the fuck did this happen?(%i)", part );
   
   return BODY_TORSO;
}

int damage( D_MOBILE *target, int amount, enum bodyparts_t location, enum damage_type_t type )
{
   if( target == NULL )
   {
      bug( "damage called with NULL target" );
      return 0;
   }

   D_OBJECT *armor = get_armor_pos( target, b_to_e(location) );

   //do damage
   if( armor ) //damage the armor (if present)
   {
      //ivar1 stores armor's material, (leather = 0, steel=1, alloy=2, kevlar=3, composite=4)
      amount -= armor->ivar1;
      if( amount > 0 && armor->ivar1 > 0 )
         armor->ivar1--;
   }

   //damage the bodypart hit
   target->body[location]->health -= amount;
   if( type == DAMAGE_BURN && target->body[location]->burn_trauma < MAX_TRAUMA )
   {
      target->body[location]->burn_trauma++;
   }
   else if( type == DAMAGE_BLUNT && target->body[location]->blunt_trauma < MAX_TRAUMA )
   {
      target->body[location]->blunt_trauma++;
   }
   else if( target->body[location]->wound_trauma < MAX_TRAUMA )
   {
      target->body[location]->wound_trauma++;
   }

   return amount;
}

void init_combat( D_MOBILE *aggressor, D_MOBILE *target )
{
   if( aggressor == target )
      return;

   if( IS_PC( aggressor ) && IS_PC( target ) && aggressor->fighting != target )
      log_string( "[COMBAT] %s initiated combat against %s", MOBNAME( aggressor ), MOBNAME( target ) );

   aggressor->fighting = target;
   target->fighting = aggressor;

   return;
}

void fire( D_MOBILE *shooter, D_MOBILE *target, D_OBJECT *firearm, enum bodyparts_t aim )
{
   int aimmod = 100, chance = 0, dam = 0, spMod = 0, tpMod = 0, handMod = 0, stopping_power = 0;
   int shooting_skill = 70; //temporary until we actually look up skills

   if( shooter == NULL || target == NULL || firearm == NULL )
   {
      bug( "fire called with NULL arguments." );
      return;
   }

   init_combat( shooter, target );

   if( aim == MAX_BODY ) //lets find a place to hit
   {
      aim = random_part();
      aimmod = 100;
   }
   else
   {
      aimmod = body_aim_mod[aim];
   }

   if( shooter == target ) //100% chance of success
   {
      chance = 100;
   }
   else
   {
      chance = ( shooting_skill * aimmod ) / 100;
      //position based modifiers
      if( target->position == POS_KNEELING )
         tpMod -=10;
      if( target->position == POS_PRONE )
         tpMod -= 15;
      if( target->position < POS_RESTING )
         tpMod = 95;
      if( target->position == POS_RESTING )
         tpMod += 15;
      if( target->position == POS_SITTING )
         tpMod += 10;

      if( shooter->position == POS_PRONE )
         spMod += 15;
      if( shooter->position == POS_KNEELING )
         spMod += 10;
      //Penalty for shooting one handed, greater penalty if the firearm is a
      //2 handed firearm (rifle, shotgun, etc)
      if( shooter->hold_left != NULL && shooter->hold_right != NULL ) //<-- are both their hands full?
         handMod -= 10 * firearm->ivar4; //ivar4 is how many hands are required to fire the weapon
   }

   chance = chance + tpMod + spMod + handMod;

   size_t check = roll( 1, 100 );
   dam = dice( (char*)ammo_dice[ firearm->ivar1 % MAX_AMMO ] );

   if( check <= chance ) //hit!
   {
      stopping_power = dam - damage( target, dam, aim, DAMAGE_PROJECTILE ); //return the actual amount of damage done after checking for armor resistance
      text_to_mobile_j( shooter, "combat", "Skill (%i) * AimMod (%i) = Base (%i) + spMod (%i) + tpMod (%i) + handMod (%i) =  Chance (%i) > Roll (%i) HIT %s for %s(%i-%i=%i) damage!",
            shooting_skill, aimmod/100, shooting_skill*aimmod/100, spMod, tpMod, handMod, chance, check, body_parts[aim], ammo_dice[firearm->ivar1 % MAX_AMMO], dam+stopping_power, stopping_power, dam );
   }
   else
   {
      text_to_mobile_j( shooter, "combat", "Skill (%i) * AimMod (%i) = Base (%i) + spMod (%i) + tpMod (%i) + handMod (%i) =  Chance (%i) > Roll (%i) MISS %s!",
            shooting_skill, aimmod/100, shooting_skill*aimmod/100, spMod, tpMod, handMod, chance, check, body_parts[aim] );
   }
}

D_OBJECT *make_corpse( D_MOBILE *dMob )
{
   if( !dMob )
      return NULL;
   char buf[MAX_STRING_LENGTH];

   D_OBJECT *corpse = new_object();
   snprintf( buf, MAX_STRING_LENGTH, "corpse %s", MOBNAME( dMob ) );
   corpse->name = strdup( buf );
   snprintf( buf, MAX_STRING_LENGTH, "corpse of %s", MOBNAME( dMob ) );
   corpse->sdesc = strdup( buf );
   snprintf( buf, MAX_STRING_LENGTH, "The corpse of %s lays here.", MOBNAME( dMob ) );
   corpse->ldesc = strdup( buf );

   corpse->type = ITEM_CORPSE;
   corpse->weight_g = 900;//grams
   corpse->volume_cm3 = 67960;
   corpse->capacity_cm3 = 1;

   corpse->in_room = dMob->room;

   for( int i = 0; i < WEAR_NONE; i++ )
   {
      if( dMob->equipment[i]->worn[0] )
      {
         object_to_object( dMob->equipment[i]->worn[0], corpse );
         dMob->equipment[i]->worn[0] = NULL;
      }
      if( dMob->equipment[i]->worn[1] )
      {
         object_to_object( dMob->equipment[i]->worn[1], corpse );
         dMob->equipment[i]->worn[1] = NULL;
      }
   }

   return corpse;
}

void kill( D_MOBILE *dMob )
{
   if( dMob->fighting && dMob->fighting->fighting == dMob )
      dMob->fighting->fighting = NULL;
   dMob->fighting = NULL;
   
   echo_around( dMob, "combat", "%s crumples to the ground, dead.", MOBNAME( dMob ) );
   D_OBJECT *corpse = make_corpse( dMob );
   if( dMob->hold_right )
   {
      echo_around( dMob, "combat", "%s skitters across the ground as it falls from %s's grasp.",
            dMob->hold_right->sdesc, MOBNAME( dMob ) );
      object_from_mobile( dMob->hold_right, dMob );
      object_to_room( dMob->hold_right, dMob->room );
      dMob->hold_right = NULL;
   }
   if( dMob->hold_left )
   {
      echo_around( dMob, "combat", "%s skitters across the ground as it falls from %s's grasp.",
            dMob->hold_left->sdesc, MOBNAME( dMob ) );
      object_from_mobile( dMob->hold_left, dMob );
      object_to_room( dMob->hold_left, dMob->room );
      dMob->hold_left = NULL;
   }

   object_to_room( corpse, dMob->room );

   //@todo: remove any affects from poisons, stims, chems, etc.
   if( IS_PC( dMob ) )
   {
      mob_to_room( dMob, get_room_by_vnum( FIRST_ROOM ) );
      text_to_mobile_j( dMob, "combat", "You have been killed!" );
      text_to_mobile_j( dMob, "death", "%s", death_message );
      log_string( "%s has been killed!", dMob->name );
      dMob->position = POS_RESTING;
   }
   else
   {
      free_mobile( dMob );
   }
}
