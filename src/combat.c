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

static int damage( D_MOBILE *aggressor, D_MOBILE *target, D_OBJECT *weapon, enum bodyparts_t location )
{
   if( aggressor == NULL || target == NULL || weapon == NULL )
   {
      bug( "damage called with NULL arguments" );
      return 0;
   }

   D_OBJECT *armor = NULL;
   if( target->equipment[b_to_e(location)]->worn[0] != NULL
         && target->equipment[b_to_e(location)]->worn[0]->type == ITEM_ARMOR )
   {
      armor = target->equipment[b_to_e(location)]->worn[0];
   }
   else if( target->equipment[b_to_e(location)]->worn[1] != NULL
         && target->equipment[b_to_e(location)]->worn[1]->type == ITEM_ARMOR )
   {
      armor = target->equipment[b_to_e(location)]->worn[1];
   }

   if( armor )
   {

   }

   return 0;
}

void fire( D_MOBILE *shooter, D_MOBILE *target, D_OBJECT *firearm, enum bodyparts_t aim )
{
   int aimmod = 100, chance = 0;

   int shooting_skill = 70; //temporary until we actually look up skills

   if( shooter == NULL || target == NULL || firearm == NULL )
   {
      bug( "fire called with NULL arguments." );
      return;
   }

   if( shooter == target ) //100% chance of success
   {
      chance = 100;
   }
   else
   {
      if( aim == MAX_BODY ) //lets find a place to hit
      {
         aim = random_part();
      }
      else
      {
         aimmod = body_aim_mod[aim];
      }

      chance = ( shooting_skill * aimmod ) / 100;

      /* Put player's shooting skill check here */
   }

   size_t check = roll( 1, 100 );

   //position based modifiers
   if( target->position == POS_KNEELING )
      chance -=10;
   if( target->position == POS_PRONE )
      chance -= 15;
   if( target->position < POS_RESTING )
      chance= 95;
   if( target->position == POS_RESTING )
      chance+= 15;
   if( target->position == POS_SITTING )
      chance+= 10;

   if( shooter->position == POS_PRONE )
      chance+= 15;
   if( shooter->position == POS_KNEELING )
      chance+= 10;
   //Penalty for shooting one handed, greater penalty if the firearm is a
   //2 handed firearm (rifle, shotgun, etc)
   if( shooter->hold_left != NULL && shooter->hold_right != NULL ) //<-- are both their hands full?
      chance -= 10 * firearm->ivar6; //ivar6 is how many hands are required to fire the weapon
      
   if( check <= chance ) //hit!
   {
      damage( shooter, target, firearm, aim );
   }
   text_to_mobile_j( shooter, "combat", "Skill (%i) * AimMod (0.%i) = Chance (%i) Roll (%i) %s %s!",
         shooting_skill, aimmod, chance, check, check <= chance ? "HIT" : "MISS", body_parts[aim] );

}

