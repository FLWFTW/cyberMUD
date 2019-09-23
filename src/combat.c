/**
 * @file combat.c
 * @author Will Sayin
 * @version 1.0
 *
 * @section DESCRIPTION
 * Functions supporting MUD combat
 */
#include "mud.h"

static enum bodyparts_tb random_part()
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

void cripple( D_MOBILE *target, enum bodyparts_tb location )
{

   text_to_mobile_j( target, "combat", "Your %s %s crippled!", body_parts[location], location == BODY_EYES ? "are" : "is" );
   if( ( location == BODY_RARM || location == BODY_RHAND ) && target->hold_right != NULL )
   {
      text_to_mobile_j( target, "combat", "You lose your grip on %s and it falls to the ground!", target->hold_right->sdesc );
      echo_around( target, "combat", "%s loses %s grip on %s and it clatters to the ground!", MOBNAME( target ), POSSESSIVE( target ), target->hold_right->sdesc );
      object_from_mobile( target->hold_right, target );
      object_to_room( target->hold_right, target->room );
      target->hold_right = NULL;
   }
   else if( ( location == BODY_LARM || location == BODY_LHAND ) && target->hold_left != NULL )
   {
      text_to_mobile_j( target, "combat", "You lose your grip on %s and it falls to the ground!", target->hold_left->sdesc );
      echo_around( target, "combat", "%s loses %s grip on %s and it clatters to the ground!", MOBNAME( target ), POSSESSIVE( target ), target->hold_left->sdesc );
      object_from_mobile( target->hold_left, target );
      object_to_room( target->hold_left, target->room );
      target->hold_left = NULL;
   }
   else if( location == BODY_EYES )
   {
      text_to_mobile_j( target, "combat", "You have been blinded!" );
   }

   return;
}

//critical can be either 0 (no critical damange bonus), 1 (1.5x damage), or 2 (2x damage, from natural 100 roll)
int bullet_damage( D_MOBILE *target, D_OBJECT *bullet, enum bodyparts_tb location, int critical )
{
   size_t dam = 0, res = 0;
   D_OBJECT *armor = get_armor_pos( target, b_to_e( location ) );
   enum damage_type_tb type = DAMAGE_PROJECTILE;

   if( armor )
   {
      res = armor->ivar1;
   }

   dam = dice( (char*)ammo_dice[ bullet->ivar1 % MAX_AMMO ] );

   /*If HP we double the damage but also double the target's armor resistance*/
   if( bullet->ivar1 == AMMO_JHP )
   {
      res *= 2;
      dam *= 2;
   }
   else if( bullet->ivar1 == AMMO_AP ) /* AP does half damage but armor has half strength */
   {
      res /= 2;
      dam /= 2;
   }
   else if( bullet->ivar1 == AMMO_API ) /* Same as API but damage is burning */
   {
      res /= 2;
      dam /= 2;
      type = DAMAGE_BURN;
   }
   else if( bullet->ivar2 == AMMO_EXP ) /* Damage is 1/3 but damage type is explosive, which does double damage to armor */
   {
      dam /= 3;
      type = DAMAGE_EXPLOSIVE;
   }

   /* Check for critical damage */
   if( critical == 1 )
   {
      dam *= 15;
      dam /= 10;
   }
   else if( critical == 2 )
   {
      dam *= 2;
   }

   if( armor )
   {
      if( armor->ivar1 > 0 )
      {
         if( type == DAMAGE_EXPLOSIVE )
            armor->ivar1 -= 3;
         else
            armor->ivar1--;
      }
   }

   dam -= res;

   return damage( target, dam, location, type );
}

int damage( D_MOBILE *target, int amount, enum bodyparts_tb location, enum damage_type_tb type )
{
   if( target == NULL )
   {
      bug( "damage called with NULL target" );
      return 0;
   }
   bool already_crippled = is_crippled( target, location );

   //damage the bodypart hit
   target->body[location]->cur_hp -= amount;
   if( is_crippled( target, location ) && already_crippled == FALSE )
   {
      cripple( target, location );
   }

   if( amount > target->max_hp / 10 ) //If the damage is over 10% of the target's max health we increase the trauma to wherever they're hit.
   {
      if( type == DAMAGE_BURN && target->body[location]->burn_trauma < MAX_TRAUMA )
      {
         target->body[location]->burn_trauma ++;
      }
      else if( type == DAMAGE_BLUNT && target->body[location]->blunt_trauma < MAX_TRAUMA )
      {
         target->body[location]->blunt_trauma ++;
      }
      else if( target->body[location]->wound_trauma < MAX_TRAUMA )
      {
         target->body[location]->wound_trauma ++;
      }
   }

   target->cur_hp -= amount;

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

void remove_combat( D_MOBILE *aggressor )
{
   if( aggressor->fighting == NULL )
      return;
   aggressor->fighting->fighting = NULL;
   aggressor->fighting = NULL;
   return;
}

D_OBJECT *next_round( D_OBJECT *firearm )
{
   ITERATOR Iter;
   D_OBJECT *round, *magazine;
   if( firearm->ivar3 == 0 ) /* magazine */
   {
      AttachIterator( &Iter, firearm->contents );
      magazine = NextInList( &Iter );
      DetachIterator( &Iter );
      if( !magazine ) /*No magazine*/
         return NULL;
      if( magazine->type != ITEM_MAGAZINE )
      {
         bug( "Expected magazine, got %s.", item_type[magazine->type] );
         return NULL;
      }
      AttachIterator( &Iter, magazine->contents );
      round = NextInList( &Iter );
      DetachIterator( &Iter );
      if( !round ) /*Empty magazine*/
         return NULL;
      if( round->type != ITEM_BULLET )
      {
         bug( "Expected bullet, got %s.", item_type[round->type] );
         return NULL;
      }
      DetachFromList( round, magazine->contents );
      return round;
   }
   else if( firearm->ivar3 >= 1 ) /* Single shot, internal magazine (revolver, pump shotgun, etc) */
   {
      if( SizeOfList( firearm->contents ) == 0 )
      {
         return NULL; /*Out of ammo*/
      }
      AttachIterator( &Iter, firearm->contents );
      round = NextInList( &Iter );
      DetachIterator( &Iter );
      if( round->type != ITEM_BULLET )
      {
         bug( "Expected bullet, got %s.", item_type[round->type] );
         return NULL;
      }
      DetachFromList( round, firearm->contents );
      return round;
   }
   else
   {
      bug( "Firearm with invalid feed mechanism %d.", firearm->ivar3 );
      return NULL;
   }

   return NULL;
}


void fire( D_MOBILE *shooter, D_MOBILE *target, D_OBJECT *firearm, enum bodyparts_tb aim )
{
   int aimmod = 100, difficulty= 0, dam = 0, spMod = 0, tpMod = 0, handMod = 0, aimed = 0;
   int shooting_skill = 70; //temporary until we actually look up skills
   D_OBJECT *round;

   if( shooter == NULL || target == NULL || firearm == NULL )
   {
      bug( "fire called with NULL arguments." );
      return;
   }

   init_combat( shooter, target );

   if( ( round = next_round( firearm ) ) == NULL )
   {
      text_to_mobile_j( shooter, "combat", "***CLICK!***" );
      return;
   }

   if( aim == MAX_BODY ) //lets find a place to hit
   {
      aim = random_part();
      aimmod = 100;
   }
   else
   {
      aimmod = body_aim_mod[aim];
      aimed = 1;
   }

   if( shooter == target ) //100% chance of success
   {
      difficulty = 0;
   }
   else
   {
      difficulty = 100-( ( shooting_skill * aimmod ) / 100 );
      //position based modifiers
      if( target->position == POS_KNEELING )
         tpMod +=10;
      else if( target->position == POS_PRONE )
         tpMod += 15;
      else if( target->position < POS_RESTING )
         tpMod -= 95;
      else if( target->position == POS_RESTING )
         tpMod -= 15;
      else if( target->position == POS_SITTING )
         tpMod -= 10;

      if( shooter->position == POS_PRONE )
         spMod -= 15;
      else if( shooter->position == POS_KNEELING )
         spMod -= 10;
      //Penalty for shooting one handed, greater penalty if the firearm is a
      //2 handed firearm (rifle, shotgun, etc)
      if( shooter->hold_left != NULL && shooter->hold_right != NULL ) //<-- are both their hands full?
         handMod += 10 * firearm->ivar4; //ivar4 is how many hands are required to fire the weapon
   }

   difficulty = difficulty + tpMod + spMod + handMod;

   size_t check = roll( 1, 100 );
   log_string( "Shot difficulty: %i. Roll: %i.", difficulty, check );

   if( check > difficulty ) //hit!
   {
      int critical = 0;
      if( check == 100 ) /* 2x damage on natural 100 */
         critical = 2;
      else if( check > ( 100 - shooter->luck ) ) /* 1.5x damage */
         critical = 1;
      dam = bullet_damage( target, round, aim, critical ); //return the actual amount of damage done after checking for armor resistance
      if( dam > 0 ) /* Armor didn't absorb it all */
      {
         if( aimed == 0 )
         {
            text_to_mobile_j( shooter, "combat", "You fire %s at %s, hitting %s in the %s for %u damage! %s", firearm->sdesc, MOBNAME(target), OBJECTIVE( target ), body_parts[aim], dam, ( critical == 1 ) ? "Critical!" : ( critical == 2 ) ? "***CRITICAL***" : "" );
            text_to_mobile_j( target, "combat", "%s fires %s %s at you, striking you in the %s for %u damage! %s", MOBNAME( shooter ), POSSESSIVE( shooter ), firearm->sdesc, body_parts[aim], dam, ( critical == 1 ) ? "Critical!" : ( critical == 2 ) ? "***CRITICAL***" : "" );
            echo_around_two( shooter, target, "combat", "%s fires %s %s at %s, striking %s in the %s!", MOBNAME( shooter ), POSSESSIVE( shooter ), firearm->sdesc, MOBNAME( target ), OBJECTIVE( target ), body_parts[aim] );
         }
         else
         {
            text_to_mobile_j( shooter, "combat", "You aim %s at %s's %s, hitting %s for %u damage! %s", firearm->sdesc, MOBNAME(target), body_parts[aim], OBJECTIVE( target ), dam, critical == 1 ? "Critical!" : critical == 2 ? "***CRITICAL***" : "" );
            text_to_mobile_j( target, "combat", "%s fires %s %s at you, striking you in the %s for %u damage! %s", MOBNAME( shooter ), POSSESSIVE( shooter ), firearm->sdesc, body_parts[aim], dam, critical == 1 ? "Critical!" : critical == 2 ? "***CRITICAL***" : "" );
            echo_around_two( shooter, target, "combat", "%s fires %s %s at %s, striking %s in the %s!", MOBNAME( shooter ), POSSESSIVE( shooter ), firearm->sdesc, MOBNAME( target ), OBJECTIVE( target ), body_parts[aim] );
         }
      }
      else /* Need to come up with a better message for armor absorbing all the damage */
      {
         text_to_mobile_j( shooter, "combat", "You fire your %s at %s, hitting %s in the %s armor.", firearm->sdesc, MOBNAME(target), OBJECTIVE( target ), body_parts[aim] );
         text_to_mobile_j( target, "combat", "%s fires %s %s at you, striking you in the %s armor.", MOBNAME( shooter ), POSSESSIVE( shooter ), firearm->sdesc, body_parts[aim] );
         echo_around_two( shooter, target, "combat", "%s fires %s %s at %s, striking %s in the %s.", MOBNAME( shooter ), POSSESSIVE( shooter ), firearm->sdesc, MOBNAME( target ), OBJECTIVE( target ), body_parts[aim]  );
      }

   }
   else if( check < ( 10 - shooter->luck ) ) //Critical miss
   {
   }
   else
   {
         text_to_mobile_j( shooter, "combat", "You fire your %s at %s, but miss %s completely!", firearm->sdesc, MOBNAME(target), OBJECTIVE( target ));
         text_to_mobile_j( target, "combat", "%s fires %s %s at you, missing you completely!", MOBNAME( shooter ), POSSESSIVE( shooter ), firearm->sdesc, body_parts[aim] );
         echo_around_two( shooter, target, "combat", "%s fires %s %s at %s, missing %s completely!.", MOBNAME( shooter ), POSSESSIVE( shooter ), firearm->sdesc, MOBNAME( target ), OBJECTIVE( target ) );
   }
   if( target->cur_hp < 0 )
      kill( target );

}

D_OBJECT *make_corpse( D_MOBILE *dMob )
{
   if( !dMob )
      return NULL;
   char buf[MAX_STRING_LENGTH];

   D_OBJECT *corpse = new_object();
   snprintf( buf, MAX_STRING_LENGTH, "corpse %s", MOBNAME( dMob ) );
   corpse->name = strdup( buf );
   snprintf( buf, MAX_STRING_LENGTH, "the corpse of %s", MOBNAME( dMob ) );
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
   /* Cease fighting */
   if( dMob->fighting && dMob->fighting->fighting == dMob )
      dMob->fighting->fighting = NULL;
   dMob->fighting = NULL;
   
   echo_around( dMob, "combat", "%s crumples to the ground, dead!", MOBNAME( dMob ) );
   D_OBJECT *corpse = make_corpse( dMob );
   /* If they're holding anything, send it to the ground */
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

bool is_crippled( D_MOBILE *dMob, enum bodyparts_tb loc )
{
   int cur = dMob->body[loc]->cur_hp;
   int max = dMob->body[loc]->max_hp;

   if( max == 0 )
      return TRUE;

   int percent = ( cur * 100 ) / max;

   if( percent < 25 )
      return TRUE;

   return FALSE;
}
