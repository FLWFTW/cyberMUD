/*
 * This file handles player combat actions.
 */

#include "mud.h"

void cmd_slay( D_MOBILE *dMob, char *arg )
{
   D_MOBILE *victim;
   char name[MAX_STRING_LENGTH];
   
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Slay who?" );
      return;
   }

   arg = one_arg ( arg, name );

   if( ( victim = get_mobile_list( name, dMob->room->mobiles ) ) == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find anyone who looks like that." );
      return;
   }

   if( victim == dMob )
   {
      text_to_mobile_j( dMob, "error", "You should probably see a psychologist." );
      return;
   }

   text_to_mobile_j( dMob, "combat", "You slay %s in cold blood!", victim->name );
   text_to_mobile_j( victim, "combat", "A sudden snap and you collapse to the ground, dead." );

   kill( victim );
}

void cmd_fire( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *gun = NULL;
   D_MOBILE *target = NULL;
   char who[MAX_STRING_LENGTH];
   arg = one_arg( arg, who );

   if( dMob->hold_right && ISGUN( dMob->hold_right ) )
   {
      gun = dMob->hold_right;
   }
   else if( dMob->hold_left && ISGUN( dMob->hold_left ) )
   {
      gun = dMob->hold_left;
   }

   if( gun == NULL )
   {
      text_to_mobile_j( dMob, "error", "You're not wielding a firearm." );
      return;
   }

   if( !IS_FIGHTING (dMob ) && who[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error","Fire at who?" );
      return;
   }

   if( who[0] == '\0' )
      target = dMob->fighting;
   else
   {
      if( !strcasecmp( who, "self" ) )
         target = dMob;
      else
         target = get_mobile_list( who, dMob->room->mobiles );
   }

   if( target == NULL )
   {
      text_to_mobile_j( dMob, "error", "You don't see that here." );
      return;
   }

   if( arg[0] == '\0' )
   {
      fire( dMob, target, gun, MAX_BODY );
   }
   else
   {
      bool valid_aim = FALSE;
      for( enum bodyparts_t i = BODY_HEAD; i < MAX_BODY; i++ )
      {
         if( is_prefix( arg, body_parts[i] ) )
         {
            valid_aim = TRUE;
            fire( dMob, target, gun, i );
            break;
         }
      }
      if( valid_aim == FALSE )
      {
         text_to_mobile_j( dMob, "error", "You can't aim at their %s!", arg );
         return;
      }
   }
   text_to_mobile_j( dMob, "combat", "You fire your %s at %s!", gun->sdesc, MOBNAME(target) );
   text_to_mobile_j( target, "combat", "%s fires %s %s at you!", MOBNAME( dMob ), POSSESSIVE( dMob ), gun->sdesc );
   echo_around_two( dMob, target, "combat", "%s fires %s %s at %s!", MOBNAME( dMob ), POSSESSIVE( dMob ), gun->sdesc, MOBNAME( target ) );
}

