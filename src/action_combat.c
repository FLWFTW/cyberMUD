/**
 * @file action_combat.c
 * @author Will Sayin
 * @version 1.0
 *
 * @section DESCRIPTION
 * This file handles player combat actions. Combat support code
 * is in combat.c
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
      for( enum bodyparts_tb i = BODY_HEAD; i < MAX_BODY; i++ )
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
}

void cmd_unload( D_MOBILE *dMob, char *arg )
{

   return;
}

/** load and reload use the same code
 */
void cmd_load( D_MOBILE *dMob, char *arg )
{
   ITERATOR Iter;
   D_OBJECT *mag = NULL, *ammo = NULL, *gun = NULL;

   if( !dMob->hold_right && !dMob->hold_left )
   {
      text_to_mobile_j( dMob, "error", "You're not holding anything.", arg );
      return;
   }

   if( arg[0] == '\0' ) /* Player just typed 'reload' */
   {
      if( dMob->hold_right )
      {
         if( dMob->hold_right->type == ITEM_FIREARM )
            gun = dMob->hold_right;
         else if( dMob->hold_right->type == ITEM_MAGAZINE )
            mag = dMob->hold_right;
         else if( dMob->hold_right->type == ITEM_BULLET )
            ammo = dMob->hold_right;
      }
      if( dMob->hold_left )
      {
         if( dMob->hold_left->type == ITEM_FIREARM )
            gun = dMob->hold_left;
         else if( dMob->hold_left->type == ITEM_MAGAZINE )
            mag = dMob->hold_left;
         else if( dMob->hold_left->type == ITEM_BULLET )
            ammo = dMob->hold_left;
      }

      if( gun == NULL && mag == NULL )
      {
         text_to_mobile_j( dMob, "error", "You're not holding something you can load." );
         return;
      }
   }
   else /* Player either typed reload <firearm> or reload <firearm> <ammo source/magazine> */
   {    /* Or reload <magazine> <ammo source> */
      char gunName[MAX_STRING_LENGTH];
      arg = one_arg( arg, gunName );

      if( dMob->hold_right && is_name( gunName, dMob->hold_right->name ) )
         gun = dMob->hold_right;
      else if( dMob->hold_left && is_name( gunName, dMob->hold_left->name ) )
         gun = dMob->hold_left;
      else
         gun = get_object_list( gunName, dMob->room->objects );
      if( gun == NULL )
      {
         text_to_mobile_j( dMob, "error", "You can't find that here." );
         return;
      }
      if( gun->type != ITEM_FIREARM && gun->type != ITEM_MAGAZINE )
      {
         text_to_mobile_j( dMob, "error", "That's not something you can load." );
         return;
      }
      if( gun->type == ITEM_MAGAZINE )
      {
         mag = gun;
         gun = NULL;
      }

      if( arg[0] != '\0' ) /* player typed load <firearm/magazine> <ammo source> */
      {
         if( dMob->hold_right && is_name( arg, dMob->hold_right->name ) )
            ammo = dMob->hold_right;
         else if( dMob->hold_left && is_name( arg, dMob->hold_left->name ) )
            ammo = dMob->hold_left;
         else
            ammo = get_object_list( arg, dMob->room->objects );
      }
   }

   /* We've identified what's getting loaded, and possibly our ammo source */
   if( gun == NULL && mag == NULL )
   {
      text_to_mobile_j( dMob, "error", "That's not something you can load." );
      return;
   }

   if( gun && mag )
   {
      if( gun->ivar3 != 0 )
      {
         text_to_mobile_j( dMob, "error", "This weapon does not accept magazines." );
         return;
      }
      if( gun->ivar1 != mag->ivar1 ) /* Wrong caliber */
      {
         text_to_mobile_j( dMob, "error", "That's not the right type of magazine for this weapon." );
         return;
      }

      AttachIterator( &Iter, gun->contents );
      D_OBJECT *oldMag;
      if( ( oldMag = NextInList( &Iter ) ) != NULL )
      {
         text_to_mobile_j( dMob, "text", "You pull the old magazine from %s and slide %s in.", mag->sdesc, gun->sdesc );
         object_from_object( oldMag, gun );
         /*If they were holding the magazine then the old magazine goes to that hand. Otherwise it goes to the ground*/
         if( mag == dMob->hold_right )
         {
            object_from_mobile( mag, dMob );
            dMob->hold_right = oldMag;
         }
         else if( mag == dMob->hold_left )
         {
            dMob->hold_left = oldMag;
            object_from_mobile( mag, dMob );
         }
         else
            object_to_room( oldMag, dMob->room );
         object_to_object( mag, gun );
      }
      else
      {
         text_to_mobile_j( dMob, "text", "You slide %s into %s.", mag->sdesc, gun->sdesc );
         if( mag == dMob->hold_right )
         {
            object_from_mobile( mag, dMob );
            dMob->hold_right = NULL;
         }
         else if( mag == dMob->hold_left )
         {
            object_from_mobile( mag, dMob );
            dMob->hold_left = NULL;
         }
         object_to_object( mag, gun );
      }
      DetachIterator( &Iter );
      return;
   }
   else if( gun && ammo )
   {
      if( gun->ivar3 == 0 )
      {
         text_to_mobile_j( dMob, "error", "%s needs to be loaded with a magazine.", gun->sdesc );
         return;
      }
      if( ammo->type == ITEM_CONTAINER ) /* Are they loading from a box of ammo or bandolier? */
      {
         if( SizeOfList( ammo->contents ) > 0 )
         {
            
            D_OBJECT *pObj = NthFromList( ammo->contents, 0 );
            if( pObj->type != ITEM_BULLET || pObj->ivar1 != gun->ivar1 )
            {
               text_to_mobile_j( dMob, "error", "%s doesn't have the right kind of ammo for %s.", ammo->sdesc, gun->sdesc );
               return;
            }
            size_t count = 0;
            AttachIterator( &Iter, ammo->contents );
            while( ( ( pObj = NextInList( &Iter ) ) != NULL ) && pObj->ivar1 == gun->ivar1 && SizeOfList( pObj->contents ) != gun->ivar3 )
            {
               object_from_object( pObj, ammo );
               object_to_object( pObj, gun );
               count++;
            }
            DetachIterator( &Iter );
            text_to_mobile_j( dMob, "text", "You load %s with %u rounds of %s from %s.", gun->sdesc, count, ammo_caliber[gun->ivar1], ammo->sdesc );
            echo_around( dMob, "text", "%s loads %s from %s.", MOBNAME( dMob ), gun->sdesc, ammo->sdesc );
            return;
         }
      }
      else if( ammo->type != ITEM_BULLET )
      {
         text_to_mobile_j( dMob, "error", "You can't load %s into %s.", ammo->sdesc, gun->sdesc );
         return;
      }
      else if( gun->ivar1 != ammo->ivar1 )
      {
         text_to_mobile_j( dMob, "error", "%s uses %s ammunition.", gun->sdesc, ammo_caliber[gun->ivar1] );
         return;
      }
      if( SizeOfList( gun->contents ) == gun->ivar3 )
      {
         text_to_mobile_j( dMob, "error", "%s is loaded to capacity.", gun->sdesc );
         return;
      }
      if( dMob->hold_right == ammo )
      {
         dMob->hold_right = NULL;
         object_from_mobile( ammo, dMob );
      }
      else if( dMob->hold_left == ammo )
      {
         dMob->hold_left = NULL;
         object_from_mobile( ammo, dMob );
      }
      object_to_object( ammo, gun );
      text_to_mobile_j( dMob, "text", "You load %s into %s.", ammo->sdesc, gun->sdesc );
      return;
   }
   else if( mag && ammo )
   {
      if( ammo->type == ITEM_CONTAINER ) /* Are they loading from a box of ammo or bandolier? */
      {
         if( SizeOfList( ammo->contents ) > 0 )
         {
            
            D_OBJECT *pObj = NthFromList( ammo->contents, 0 );
            /* We're just going to say that a box of bullets will only have bullets. If it has anything other than bullets it will
             * be up to the player to fish them out individually. */
            if( pObj->type != ITEM_BULLET || pObj->ivar1 != mag->ivar1 )
            {
               text_to_mobile_j( dMob, "error", "%s doesn't have the right kind of ammo for %s.", ammo->sdesc, mag->sdesc );
               return;
            }
            if( SizeOfList( mag->contents ) == mag->ivar2 )
            {
               text_to_mobile_j( dMob, "error", "%s is loaded to capacity.", mag->sdesc );
               return;
            }
            size_t count = 0;
            AttachIterator( &Iter, ammo->contents );
            while( ( ( pObj = NextInList( &Iter ) ) != NULL ) && pObj->ivar1 == mag->ivar1 && SizeOfList( mag->contents ) != mag->ivar2 )
            {
               object_from_object( pObj, ammo );
               object_to_object( pObj, mag );
               count++;
            }
            DetachIterator( &Iter );
            text_to_mobile_j( dMob, "text", "You load %s with %u rounds of %s from %s.", mag->sdesc, count, ammo_caliber[mag->ivar1], ammo->sdesc );
            echo_around( dMob, "text", "%s loads %s fromm %s.", MOBNAME( dMob ), mag->sdesc, ammo->sdesc );
            return;
         }
      }
      else if( mag->ivar1 != ammo->ivar1 )
      {
         text_to_mobile_j( dMob, "error", "%s uses %s ammunition.", mag->sdesc, ammo_caliber[mag->ivar1] );
         return;
      }
      if( SizeOfList( mag->contents ) == mag->ivar2 )
      {
         text_to_mobile_j( dMob, "error", "%s is loaded to capacity.", mag->sdesc );
         return;
      }
      if( dMob->hold_right == ammo )
      {
         object_from_mobile( ammo, dMob );
         dMob->hold_right = NULL;
      }
      else if( dMob->hold_left == ammo )
      {
         object_from_mobile( ammo, dMob );
         dMob->hold_left = NULL;
      }
      else
         object_from_room( ammo, dMob->room );
      object_to_object( ammo, mag );
      text_to_mobile_j( dMob, "text", "You load %s into %s.", ammo->sdesc, mag->sdesc );
      return;
   }
   else if( gun && gun->ivar3 == 0 ) /* We need to find a magazine */
   {
      D_OBJECT *pObj, *bandolier;

      /* Look through their equipment for containers holding a magazine */
      for( size_t i = WEAR_HEAD; i < WEAR_NONE && mag == NULL; i++ )
      {
         if( dMob->equipment[i]->worn[0] && ( dMob->equipment[i]->worn[0]->type == ITEM_CONTAINER || dMob->equipment[i]->worn[0]->type == ITEM_BANDOLIER ) )
         {
            bandolier = dMob->equipment[i]->worn[0];
            AttachIterator( &Iter, bandolier->contents );
            while( ( pObj = NextInList( &Iter ) ) != NULL )
            {
               if( pObj->type == ITEM_MAGAZINE && pObj->ivar1 == gun->ivar1 )
               {
                  mag = pObj;
                  object_from_object( pObj, bandolier );
                  break;
               }
            }
            DetachIterator( &Iter );
            if( mag )
            {
               /* We'll be nice and if they have a magazine in their weapon we'll do a 1-for-1 swap in their container */
               if( SizeOfList( gun->contents ) > 0 ) /* Swapping mags */
               {
                  AttachIterator( &Iter, gun->contents );
                  pObj = NextInList( &Iter );
                  object_from_object( pObj, gun );
                  object_to_object( pObj, bandolier );
                  DetachIterator( &Iter );
                  text_to_mobile_j( dMob, "text", "You draw %s from %s and reload %d, replacing %s in %s.", mag->sdesc, bandolier->sdesc, gun->sdesc, pObj->sdesc, bandolier->sdesc );
               }
               else
               {
                  text_to_mobile_j( dMob, "text", "You draw %s from %s and load it into %s.", mag->sdesc, bandolier->sdesc, gun->sdesc );
               }
               object_to_object( mag, gun );
               return;
            }
         }
         if( dMob->equipment[i]->worn[1] && ( dMob->equipment[i]->worn[1]->type == ITEM_CONTAINER || dMob->equipment[i]->worn[1]->type == ITEM_BANDOLIER ) )
         {
            bandolier = dMob->equipment[i]->worn[1];
            AttachIterator( &Iter, bandolier->contents );
            while( ( pObj = NextInList( &Iter ) ) != NULL )
            {
               if( pObj->type == ITEM_MAGAZINE && pObj->ivar1 == gun->ivar1 )
               {
                  mag = pObj;
                  object_from_object( pObj, bandolier );
                  break;
               }
            }
            DetachIterator( &Iter );
            if( mag )
            {
               /* We'll be nice and if they have a magazine in their weapon we'll do a 1-for-1 swap in their container */
               if( SizeOfList( gun->contents ) > 0 ) /* Swapping mags */
               {
                  AttachIterator( &Iter, gun->contents );
                  pObj = NextInList( &Iter );
                  object_from_object( pObj, gun );
                  object_to_object( pObj, bandolier );
                  DetachIterator( &Iter );
                  text_to_mobile_j( dMob, "text", "You draw %s from %s and reload %d, replacing %s in %s.", mag->sdesc, bandolier->sdesc, gun->sdesc, pObj->sdesc, bandolier->sdesc );
               }
               else
               {
                  text_to_mobile_j( dMob, "text", "You draw %s from %s and load it into %s.", mag->sdesc, bandolier->sdesc, gun->sdesc );
               }
               object_to_object( mag, gun );
               return;
            }
         }
      }
   }

   text_to_mobile_j( dMob, "error", "Reload what, with what?" );

   return;
}

