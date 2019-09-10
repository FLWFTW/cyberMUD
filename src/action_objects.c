/*
 * This file handles non-fighting player actions.
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"


void cmd_inventory( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *obj, *item;
   ITERATOR Iter;
   json_t *json = json_object();
   json_t *data = json_object();

   if( dMob->hold_right )
   {
      item = dMob->hold_right;
      json_t *held = json_object();
      json_object_set_new( held, "object", object_to_json_cli( item ) );
      if( SizeOfList( item->contents ) > 0 )
      {
         json_t *conts = json_array();
         AttachIterator( &Iter, item->contents );
         while( ( obj = NextInList( &Iter ) ) != NULL )
            json_array_append_new( conts, object_to_json_cli( obj ) );
         DetachIterator( &Iter );
         json_object_set_new( held, "contents", conts );
      }
      json_object_set_new( data, "right", held );
   }

   if( dMob->hold_left )
   {
      item = dMob->hold_left;
      json_t *held = json_object();
      json_object_set_new( held, "object", object_to_json_cli( item ) );
      if( SizeOfList( item->contents ) > 0 )
      {
         json_t *conts = json_array();
         AttachIterator( &Iter, item->contents );
         while( ( obj = NextInList( &Iter ) ) != NULL )
            json_array_append_new( conts, object_to_json_cli( obj ) );
         DetachIterator( &Iter );
         json_object_set_new( held, "contents", conts );
      }
      json_object_set_new( data, "left", held );
   }

   json_t *containers = json_array();
   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      if( dMob->equipment[i]->worn[0] != NULL && ( SizeOfList( dMob->equipment[i]->worn[0]->contents ) > 0 ) )
      {
         json_t *container = object_to_json_cli( dMob->equipment[i]->worn[0] );
         json_t *contents = json_array();
         
         D_OBJECT *content;

         AttachIterator( &Iter, dMob->equipment[i]->worn[0]->contents );
         while( ( content = NextInList( &Iter ) ) != NULL )
            json_array_append_new( contents, object_to_json_cli( content ) );
         DetachIterator( &Iter );
         json_object_set_new( container, "contents", contents );
         json_array_append_new( containers, container );
      }
      if( dMob->equipment[i]->worn[1] != NULL && ( SizeOfList( dMob->equipment[i]->worn[1]->contents ) > 0 ) )
      {
         json_t *container = object_to_json_cli( dMob->equipment[i]->worn[1] );
         json_t *contents = json_array();
         
         D_OBJECT *content;

         AttachIterator( &Iter, dMob->equipment[i]->worn[1]->contents );
         while( ( content = NextInList( &Iter ) ) != NULL )
            json_array_append_new( contents, object_to_json_cli( content ) );
         DetachIterator( &Iter );
         json_object_set_new( container, "contents", contents );
         json_array_append_new( containers, container );
      }
   }

   if( json_array_size( containers ) > 0 )
      json_object_set_new( data, "containers", containers );

   json_object_set_new( json, "type", json_string( "inventory" ) );
   json_object_set_new( json, "data", data );

   char *dump = json_dumps( json, 0 );
   send_json_m( dMob, "%s", dump );
   free( dump );
   json_decref( json );
   return;

}

void cmd_remove( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *obj = NULL;
   
   if( dMob->hold_right && dMob->hold_left )
   {
      text_to_mobile_j( dMob, "error", "Your hands are full!\r\n" );
      return;
   }

   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      if( dMob->equipment[i]->worn[0] != NULL && ( is_name( arg, dMob->equipment[i]->worn[0]->name ) || !strcmp( arg, dMob->equipment[i]->worn[0]->guid ) ) )
      {
         if( dMob->equipment[i]->worn[1] != NULL )
         {
            text_to_mobile_j( dMob, "error", "You have to remove your %s first.", dMob->equipment[i]->worn[1]->sdesc );
            return;
         }
         obj = dMob->equipment[i]->worn[0];
         dMob->equipment[i]->worn[0] = NULL;
         break;
      }
      if( dMob->equipment[i]->worn[1] != NULL && ( is_name( arg, dMob->equipment[i]->worn[1]->name ) || !strcmp( arg, dMob->equipment[i]->worn[1]->guid ) ) )
      {
         obj = dMob->equipment[i]->worn[1];
         dMob->equipment[i]->worn[1] = NULL;
         break;
      }
   }
   if( obj == NULL )
   {
      text_to_mobile_j( dMob, "error", "You aren't wearing that.\r\n" );
      return;
   }

   if( !dMob->hold_right )
   {
      if( obj->type == ITEM_FIREARM && obj->ivar4 == 2 )
      {
         text_to_mobile_j( dMob, "text", "You unsling your %s and ready it in your right hand.", obj->sdesc );
         echo_around( dMob, "text", "%s unslings %s and readies it in %s right hand.", MOBNAME(dMob), obj->sdesc, POSSESSIVE(dMob ) );
      }
      else
      {

         text_to_mobile_j( dMob, "text", "You remove %s and hold it in your right hand.",
               obj->sdesc );
         echo_around( dMob, "text", "%s removes %s and holds it in %s right hand.",
               MOBNAME(dMob), obj->sdesc,
               POSSESSIVE( dMob ) );
      }
      dMob->hold_right = obj;
   }
   else if( !dMob->hold_left )
   {
      if( obj->type == ITEM_FIREARM && obj->ivar4 == 2 )//ivar4 for weapons is how many hands is needed to operate it. You can sling 2 handed weapons but must sheath or holster 1 handed.
      {
         text_to_mobile_j( dMob, "text", "You unsling your %s and ready it in your left hand.", obj->sdesc );
         echo_around( dMob, "text", "%s unslings %s and readies it in %s left hand.", MOBNAME(dMob), obj->sdesc, POSSESSIVE(dMob ) );
      }
      else
      {

         text_to_mobile_j( dMob, "text", "You remove %s and hold it in your left hand.",
               obj->sdesc );
         echo_around( dMob, "text", "%s removes %s and holds it in %s left hand.",
               MOBNAME(dMob), obj->sdesc,
               POSSESSIVE( dMob ) );
      }
      dMob->hold_left = obj;
   }
}

void cmd_give( D_MOBILE *dMob, char *arg )
{
   char objName[MAX_BUFFER];
   char toName[MAX_BUFFER];
   D_OBJECT *what;
   D_MOBILE *to;

   if( !arg || arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Give what, to whom?" );
      return;
   }

   arg = one_arg( arg, objName ); //we don't check for 'my' because you can only give something you're holding (And I'm too lazy to implement it...)
   arg = one_arg( arg, toName );

   if( !strcasecmp( toName, "to" ) && arg[0] != '\0' )
      arg = one_arg( arg, toName );

   if( dMob->hold_right && is_name( objName, dMob->hold_right->name ) )
      what = dMob->hold_right;
   else if( dMob->hold_left && is_name( objName, dMob->hold_left->name ) )
      what = dMob->hold_left;
   else
   {
      text_to_mobile_j( dMob, "error", "You aren't holding that." );
      return;
   }

   if( ( to = get_mobile_list( toName, dMob->room->mobiles ) ) == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find anyone who looks like that." );
      return;
   }

   if( dMob == to )
   {
      text_to_mobile_j( dMob, "error", "You can't give something to yourself." );
      return;
   }


   if( dMob->hold_right && dMob->hold_right == what )
   {
      text_to_mobile_j( dMob, "text", "You offer %s in your right hand to %s.", what->sdesc, MOBNAME(to) );
      text_to_mobile_j( to, "text", "%s offers you %s.", MOBNAME(dMob),  what->sdesc );
      dMob->offer_right->to = to;
      dMob->offer_right->what = what;
      dMob->offer_right->when = time( NULL );
   }
   else if( dMob->hold_left && dMob->hold_left == what )
   {
      text_to_mobile_j( dMob, "text", "You offer %s in your left hand to %s.", what->sdesc, MOBNAME(to) );
      text_to_mobile_j( to, "text", "%s offers you %s.", MOBNAME(dMob), what->sdesc );
      dMob->offer_left->to = to;
      dMob->offer_left->what = what;
      dMob->offer_left->when = time( NULL );
   }
   else
   {
      bug( "Should not have gotten here...( %s %s %i )", __FILE__, __func__, __LINE__ );
   }
   return;
}

void cmd_draw( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *con, *obj;
   bool drawn = FALSE;
   if( dMob->hold_right && dMob->hold_left )
   {
      text_to_mobile_j( dMob, "error", "Your hands are full." );
      return;
   }

   if( ( dMob->hold_right != NULL && ( is_crippled( dMob, BODY_LARM ) || is_crippled( dMob, BODY_LHAND ) ) )
    || ( dMob->hold_left  != NULL && ( is_crippled( dMob, BODY_RARM ) || is_crippled( dMob, BODY_RHAND ) ) ) )
   {
      text_to_mobile_j( dMob, "error", "Your crippled limb prevents you from holding anything." );
      return;
   }


   for( size_t i = 0; i < WEAR_NONE; i++ )
   {
      con = dMob->equipment[i]->worn[0];
      if( con 
          && ( con->type == ITEM_SHEATH || con->type == ITEM_HOLSTER )
          && ( ( obj = NthFromList( con->contents, 0 ) ) != NULL )
          && ( arg[0] == '\0' || is_name( arg, obj->name ) || !strcmp( arg, obj->guid ) ) )
      {
         if( dMob->hold_right && dMob->hold_left ) //If they type draw with no argument and fill their hands
            return;
         text_to_mobile_j( dMob, "text", "You draw %s from %s and wield it in your %s hand.",
               obj->sdesc, con->sdesc, dMob->hold_right == NULL ? "right" : "left" );
         echo_around( dMob, "text", "%s draws %s from %s and wields it in %s %s hand.",
               MOBNAME(dMob), obj->sdesc, con->sdesc, POSSESSIVE( dMob ), dMob->hold_right == NULL ? "right" : "left" );
         object_from_object( obj, con );
         object_to_mobile( obj, dMob );
         drawn = TRUE;
         if( arg[0] != '\0' )
            return;
      }
      con = dMob->equipment[i]->worn[1];
      if( con 
          && ( con->type == ITEM_SHEATH || con->type == ITEM_HOLSTER )
          && ( ( obj = NthFromList( con->contents, 0 ) ) != NULL )
          && ( arg[0] == '\0' || is_name( arg, obj->name ) || !strcmp( arg, obj->guid ) ) )
      {
         if( dMob->hold_right && dMob->hold_left ) //If they type draw with no argument and fill their hands
            return;
         text_to_mobile_j( dMob, "text", "You draw %s from %s and wield it in your %s hand.",
               obj->sdesc, con->sdesc, dMob->hold_right == NULL ? "right" : "left" );
         echo_around( dMob, "text", "%s draws %s from %s and wields it in %s %s hand.",
               MOBNAME(dMob), obj->sdesc, con->sdesc, POSSESSIVE( dMob ), dMob->hold_right == NULL ? "right" : "left" );
         object_from_object( obj, con );
         object_to_mobile( obj, dMob );
         drawn = TRUE;
         if( arg[0] == '\0' )
            continue;
         else
            return;
      }
   }
   if( drawn )
      return;

   if( arg[0] == '\0' )
      text_to_mobile_j( dMob, "error", "You don't have anything to draw." );
   else
      text_to_mobile_j( dMob, "error", "You can't find that to draw." );

}

void cmd_unsling( D_MOBILE *dMob, char *arg )
{
   if( dMob->equipment[WEAR_SLUNG]->worn[0] == NULL )
   {
      text_to_mobile_j( dMob, "error", "You don't have anything slung." );
      return;
   }

   if( ( dMob->hold_right != NULL && ( is_crippled( dMob, BODY_LARM ) || is_crippled( dMob, BODY_LHAND ) ) )
    || ( dMob->hold_left  != NULL && ( is_crippled( dMob, BODY_RARM ) || is_crippled( dMob, BODY_RHAND ) ) ) )
   {
      text_to_mobile_j( dMob, "error", "Your crippled limb prevents you from holding anything." );
      return;
   }

   if( arg[0] == '\0' )
   {
      if( dMob->equipment[WEAR_SLUNG]->worn[0] )
      {
         cmd_remove( dMob, dMob->equipment[WEAR_SLUNG]->worn[0]->guid );
         return;
      }
   }
   if( is_name( arg, dMob->equipment[WEAR_SLUNG]->worn[0]->name ) )
      cmd_remove( dMob, dMob->equipment[WEAR_SLUNG]->worn[0]->guid );
   else
      text_to_mobile_j( dMob, "error", "You don't have that slung." );
}

/*
 * ungive
 *    usage: ungive [item name]
 *    if item name is blank retracts any outstanding offers
 */
void cmd_ungive( D_MOBILE *dMob, char *arg )
{
   if( dMob->offer_right->what == NULL && dMob->offer_left->what == NULL )
   {
      text_to_mobile_j( dMob, "error", "You aren't offering anyone anything." );
      return;
   }

   if( dMob->offer_right->what && ( is_name( arg, dMob->offer_right->what->name ) || arg[0] == '\0' ) )
   {
      text_to_mobile_j( dMob, "text", "You retract your offer of %s to %s.",
            dMob->offer_right->what->sdesc,
            MOBNAME(dMob->offer_right->to) );
      text_to_mobile_j( dMob->offer_right->to, "text", "%s retracts %s offer of %s.",
            MOBNAME(dMob), POSSESSIVE( dMob ), 
            dMob->offer_right->what->sdesc );
      dMob->offer_right->what = NULL;
      dMob->offer_right->to   = NULL;
      dMob->offer_right->when = 0;
   }
   else if( dMob->offer_left->what && ( is_name( arg, dMob->offer_left->what->name ) || arg[0] == '\0' ) )
   {
      text_to_mobile_j( dMob, "text", "You retract your offer of %s to %s.",
            dMob->offer_left->what->sdesc,
            MOBNAME(dMob->offer_left->to) );
      text_to_mobile_j( dMob->offer_left->to, "text", "%s retracts %s offer of %s.",
            MOBNAME(dMob), POSSESSIVE( dMob ),
            dMob->offer_left->what->sdesc );
      dMob->offer_left->what = NULL;
      dMob->offer_left->to   = NULL;
      dMob->offer_left->when = 0;
   }

   return;
}

/*
 * accept
 * usage:
 *    accept [offered object name or offerer name]
 *    if argument is blank it will accept the first offer found
 */
void cmd_accept( D_MOBILE *dMob, char *arg )
{
   ITERATOR Iter;
   D_MOBILE *pMob;

   if( dMob->hold_right != NULL && dMob->hold_left != NULL )
   {
      text_to_mobile_j( dMob, "error", "Your hands are full." );
      return;
   }

   if( ( dMob->hold_right != NULL && ( is_crippled( dMob, BODY_LARM ) || is_crippled( dMob, BODY_LHAND ) ) )
    || ( dMob->hold_left  != NULL && ( is_crippled( dMob, BODY_RARM ) || is_crippled( dMob, BODY_RHAND ) ) ) )
   {
      text_to_mobile_j( dMob, "error", "Your crippled limb prevents you from holding anything." );
      return;
   }

   AttachIterator( &Iter, dMob->room->mobiles );
   while( ( pMob = NextInList( &Iter ) ) != NULL )
   {
      if( pMob->offer_right->to && pMob->offer_right->to == dMob )
      {
         if( arg[0] == '\0' || ( is_prefix( arg, pMob->name ) || is_name( arg, pMob->offer_right->what->name ) ) )
         {
            if( dMob->hold_right == NULL )
            {
               dMob->hold_right = pMob->hold_right;
               pMob->hold_right = NULL;
               pMob->offer_right->what = NULL;
               pMob->offer_right->to = NULL;
               pMob->offer_right->when = 0;
               text_to_mobile_j( dMob, "text", "You accept %s's offer of %s and take it in your right hand.",
                     MOBNAME(pMob), dMob->hold_right->sdesc );
               text_to_mobile_j( pMob, "text", "%s accepts your offer and you give %s your %s.",
                     MOBNAME(dMob), OBJECTIVE( dMob ), dMob->hold_right->sdesc );
               echo_around_two( pMob, dMob, "text", "%s gives %s to %s.", MOBNAME(pMob), 
                     dMob->hold_right->sdesc, MOBNAME(dMob) );
               return;
            }
            else if( dMob->hold_left == NULL )
            {
               dMob->hold_left = pMob->hold_right;
               pMob->hold_right = NULL;
               pMob->offer_right->what = NULL;
               pMob->offer_right->to = NULL;
               pMob->offer_right->when = 0;
               text_to_mobile_j( dMob, "text", "You accept %s's offer of %s and take it in your left hand.",
                     MOBNAME(pMob), dMob->hold_left->sdesc );
               text_to_mobile_j( pMob, "text", "%s accepts your offer and you give %s your %s.",
                     MOBNAME(dMob), OBJECTIVE( dMob ), dMob->hold_left->sdesc );
               echo_around_two( pMob, dMob, "text", "%s gives %s to %s.", MOBNAME(pMob), 
                     dMob->hold_left->sdesc, MOBNAME(dMob));
               return;
            }
         }
      }
      else if( pMob->offer_left->to && pMob->offer_left->to == dMob )
      {
         if( arg[0] == '\0' || ( is_prefix( arg, pMob->name ) || is_name( arg, pMob->offer_left->what->name ) ) )
         {
            if( dMob->hold_right == NULL )
            {
               dMob->hold_right = pMob->hold_left;
               pMob->hold_left = NULL;
               pMob->offer_left->what = NULL;
               pMob->offer_left->to = NULL;
               pMob->offer_left->when = 0;
               text_to_mobile_j( dMob, "text", "You accept %s's offer of %s and take it in your right hand.",
                     MOBNAME(pMob), dMob->hold_right->sdesc );
               text_to_mobile_j( pMob, "text", "%s accepts your offer and you give %s your %s.",
                     MOBNAME(dMob), OBJECTIVE( dMob ), dMob->hold_right->sdesc );
               echo_around_two( pMob, dMob, "text", "%s gives %s to %s.", MOBNAME(pMob), 
                     dMob->hold_right->sdesc, MOBNAME(dMob) );
               return;
            }
            else if( dMob->hold_left == NULL )
            {
               dMob->hold_left = pMob->hold_left;
               pMob->hold_left = NULL;
               pMob->offer_left->what = NULL;
               pMob->offer_left->to = NULL;
               pMob->offer_left->when = 0;
               text_to_mobile_j( dMob, "text", "You accept %s's offer of %s and take it in your left hand.",
                     MOBNAME(pMob), dMob->hold_left->sdesc );
               text_to_mobile_j( pMob, "text", "%s accepts your offer and you give %s your %s.",
                     MOBNAME(dMob), OBJECTIVE( dMob ), dMob->hold_left->sdesc );
               echo_around_two( pMob, dMob, "text", "%s gives %s to %s.", MOBNAME(pMob), 
                     dMob->hold_left->sdesc, MOBNAME(dMob) );
               return;
            }
         }
      }
   }
   DetachIterator( &Iter );
   if( arg[0] == '\0' )
      text_to_mobile_j( dMob, "error", "You're not being offered anything." );
   else
      text_to_mobile_j( dMob, "error", "You're not being offered that." );
   return;
}

void cmd_wear( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *obj;

   if( dMob->hold_right && ( is_name( arg, dMob->hold_right->name ) || !strcmp( dMob->hold_right->guid, arg ) ) )
      obj = dMob->hold_right;
   else if( dMob->hold_left && ( is_name( arg, dMob->hold_left->name ) || !strcmp( dMob->hold_left->guid, arg ) ) )
      obj = dMob->hold_left;
   else
   {
      text_to_mobile_j( dMob, "error", "You're not holding that." );
      return;
   }

   if( obj->wear_pos > WEAR_NONE || obj->wear_pos < WEAR_HEAD )
   {
      bug( "Object with wear position %d in cmd_wear", obj->wear_pos );
      obj->wear_pos = WEAR_NONE;
   }
   if( obj->wear_pos == WEAR_NONE )
   {
      text_to_mobile_j( dMob, "error", "You can't wear %s.\r\n", 
            dMob->hold_right->sdesc );
      return;
   }

   if( dMob->equipment[obj->wear_pos]->worn[1] == NULL )
   {
      //They are wearing nothing in that wear location
      if( dMob->equipment[obj->wear_pos]->worn[0] == NULL )
      {
         dMob->equipment[obj->wear_pos]->worn[0] = obj;
         if( dMob->hold_right == obj )
            dMob->hold_right = NULL;
         else
            dMob->hold_left = NULL;
         if( obj->wear_pos == WEAR_SLUNG )
         {
            text_to_mobile_j( dMob, "text", "You sling %s.", obj->sdesc );
            echo_around( dMob, "text", "%s slings %s %s.", MOBNAME(dMob), POSSESSIVE(dMob), obj->sdesc );
         }
         else
         {
            text_to_mobile_j( dMob, "text", "You wear %s on your %s.", obj->sdesc, wear_pos[obj->wear_pos] );
            echo_around( dMob, "text", "%s wears %s on %s %s.", MOBNAME(dMob), obj->sdesc, POSSESSIVE(dMob), wear_pos[obj->wear_pos] );
         }
         return;
      }

      //They have an item in their base layer, ensure they're not double stacking armor
      if( dMob->equipment[obj->wear_pos]->worn[0] != NULL && ( dMob->equipment[obj->wear_pos]->worn[0]->type != ITEM_ARMOR || obj->type != ITEM_ARMOR ) )
      {
         if( obj->wear_pos == WEAR_SLUNG )//or slinging more than one weapon at a time
         {
            text_to_mobile_j( dMob, "error", "You already have a weapon slung." );
            return;
         }
         dMob->equipment[obj->wear_pos]->worn[1] = obj;
         if( dMob->hold_right == obj )
            dMob->hold_right = NULL;
         else
            dMob->hold_left = NULL;

         text_to_mobile_j( dMob, "text", "You wear %s over %s on your %s.", obj->sdesc, 
               dMob->equipment[obj->wear_pos]->worn[0]->sdesc,
               wear_pos[obj->wear_pos] );
         echo_around( dMob, "text", "%s wears %s on %s %s.", MOBNAME(dMob), obj->sdesc, POSSESSIVE(dMob), wear_pos[obj->wear_pos] );
         return;
      }

      text_to_mobile_j( dMob, "error", "You're already wearing armor on your %s.", wear_pos[obj->wear_pos] );
      return;
   }
   else
   {
      text_to_mobile_j( dMob, "error", "You can't fit anything else on your %s.", wear_pos[obj->wear_pos] );
      return;
   }
}

void cmd_equipment( D_MOBILE *dMob, char *arg )
{
   json_t *json = json_object();

   json_object_set_new( json, "type", json_string( "equipment" ) );

   json_t *data = json_array();
   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      json_t *jObj = json_object();
      if( dMob->equipment[i]->worn[0] != NULL )
      {
         json_object_set_new( jObj, "sdesc", json_string(dMob->equipment[i]->worn[0]->sdesc ) );
         json_object_set_new( jObj, "covers", json_string( wear_pos[dMob->equipment[i]->worn[0]->wear_pos ] ) );
         json_array_append_new( data, jObj );
         jObj = json_object(); //json_array_append_new steals the reference to jObj, so we have to recreate it for the second layer
      }

      if( dMob->equipment[i]->worn[1] != NULL )
      {
         json_object_set_new( jObj, "sdesc", json_string(dMob->equipment[i]->worn[1]->sdesc ) );
         json_object_set_new( jObj, "covers", json_string( wear_pos[dMob->equipment[i]->worn[1]->wear_pos ] ) );
         json_array_append_new( data, jObj );
      }
      else//if there is no second layer we need to get rid of jObj to avoid a memory leak
      {
         json_decref( jObj );
      }
   }

   json_object_set_new( json, "data", data );
   char *dump = json_dumps( json, 0 );
   send_json_m( dMob, dump );
   json_decref( json );
   free( dump );
   return;
}

/*
 * stow
 * Usage:
 *    stow [object]
 *    If stow is used without an object it will put any held objects in the first available worn
 *    container that it fits in.
 *
 *    If an object is specified it will first check the player's hands for a match then check in
 *    the player's room for a matching object.
 */
void cmd_stow( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      if( dMob->hold_right == NULL && dMob->hold_left == NULL )
      {
         text_to_mobile_j( dMob, "error", "You aren't holding anything." );
         return;
      }
      if( dMob->hold_right )
         cmd_stow( dMob, dMob->hold_right->guid );
      if( dMob->hold_left )
         cmd_stow( dMob, dMob->hold_left->guid );
      return;
   }
   D_OBJECT *obj = NULL;
   D_OBJECT *con = NULL;

   if( dMob->hold_right && ( is_name( arg, dMob->hold_right->name ) || !strcmp( dMob->hold_right->guid, arg ) ) )
      obj = dMob->hold_right;
   else if( dMob->hold_left && ( is_name( arg, dMob->hold_left->name ) || !strcmp( dMob->hold_left->guid, arg ) ) )
      obj = dMob->hold_left;

   if( obj != NULL )
   {
      for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
      {
         con = dMob->equipment[i]->worn[0];
         if( con && object_can_fit( obj, con ) && con->type == ITEM_CONTAINER )
         {
            text_to_mobile_j( dMob, "text", "You put %s in %s.", obj->sdesc,
                  con->sdesc );
            echo_around( dMob, "text", "%s puts %s in %s", MOBNAME(dMob), obj->sdesc,
                  con->sdesc );
            object_from_mobile( obj, dMob );
            object_to_object( obj, con );
            if( obj == dMob->hold_right )
               dMob->hold_right = NULL;
            else
               dMob->hold_left = NULL;
            return;
         }
         con = dMob->equipment[i]->worn[1];
         if( con && object_can_fit( obj, con ) && con->type == ITEM_CONTAINER )
         {
            text_to_mobile_j( dMob, "text", "You put %s in %s.", obj->sdesc,
                  con->sdesc );
            echo_around( dMob, "text", "%s puts %s in %s", MOBNAME(dMob), obj->sdesc,
                  con->sdesc );
            object_from_mobile( obj, dMob );
            object_to_object( obj, con );
            if( obj == dMob->hold_right )
               dMob->hold_right = NULL;
            else
               dMob->hold_left = NULL;
            return;
         }
      }
      text_to_mobile_j( dMob, "error", "You can't fit %s anywhere.", obj->sdesc );
      return;
   }

   //If we get this far that means we're trying to stow something from the ground.
   if( dMob->hold_right && dMob->hold_left )
   {
      text_to_mobile_j( dMob, "error", "Your hands are full." );
      return;
   }

   if( ( obj = get_object_list( arg, dMob->room->objects ) ) != NULL )
   {
      if( !can_lift( dMob, obj ) )
      {
         text_to_mobile_j( dMob, "error", "You're not strong enough to lift it." );
         return;
      }
      for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
      {
         con = dMob->equipment[i]->worn[0];
         if( con && object_can_fit( obj, con ) )
         {
            text_to_mobile_j( dMob, "text", "You put %s in %s.", obj->sdesc,
                  con->sdesc );
            echo_around( dMob, "text", "%s puts %s in %s", MOBNAME(dMob), obj->sdesc,
                  con->sdesc );
            object_from_room( obj, dMob->room );
            object_to_object( obj, con );
            return;
         }
         con = dMob->equipment[i]->worn[1];
         if( con && object_can_fit( obj, con ) && con->type == ITEM_CONTAINER )
         {
            text_to_mobile_j( dMob, "text", "You put %s in %s.", obj->sdesc,
                  con->sdesc );
            echo_around( dMob, "text", "%s puts %s in %s", MOBNAME(dMob), obj->sdesc,
                  con->sdesc );
            object_from_room( obj, dMob->room );
            object_to_object( obj, con );
            return;
         }
      }

      text_to_mobile_j( dMob, "error", "You can't fit %s anywhere.", obj->sdesc );
      return;
   }

   text_to_mobile_j( dMob, "error", "You can't find that here." );
   return;
}

void cmd_holster( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *obj;

   if( arg[0] == '\0' )
   {
      if( dMob->hold_right == NULL && dMob->hold_left == NULL )
      {
         text_to_mobile_j( dMob, "error", "You aren't holding anything." );
         return;
      }
      if( dMob->hold_right )
         cmd_holster( dMob, dMob->hold_right->guid );
      if( dMob->hold_left )
         cmd_holster( dMob, dMob->hold_left->guid );
      return;
   }
   
   if( dMob->hold_right && ( is_name( arg, dMob->hold_right->name ) || !strcmp( arg, dMob->hold_right->guid ) ) )
      obj = dMob->hold_right;
   else if( dMob->hold_left && ( is_name( arg, dMob->hold_left->name ) || !strcmp( arg, dMob->hold_left->guid ) ) )
      obj = dMob->hold_left;
   else
   {
      text_to_mobile_j( dMob, "error", "You aren't holding that." );
      return;
   }

   if( obj->type != ITEM_FIREARM )
   {
      if( obj->type == ITEM_BLADE )
      {
         cmd_sheath( dMob, arg );
         return;
      }
      text_to_mobile_j( dMob, "error", "You can't holster that." );
      return;
   }
   if( obj->ivar4 > 1 )
   {
      cmd_sling( dMob, arg ); //We're gonna say you can't holster 2 handed firearms.
      return;
   }

   bool holstered = FALSE;
   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      if( dMob->equipment[i]->worn[0] 
            && dMob->equipment[i]->worn[0]->type == ITEM_HOLSTER 
            && SizeOfList( dMob->equipment[i]->worn[0]->contents ) == 0 )
      {
         object_from_mobile( obj, dMob );
         object_to_object( obj, dMob->equipment[i]->worn[0] );
         holstered = TRUE;
         text_to_mobile_j( dMob, "text", "You holster %s in your %s.", 
               obj->sdesc, dMob->equipment[i]->worn[0]->sdesc );
         echo_around( dMob, "text", "%s holsters %s in %s %s.", MOBNAME(dMob), 
               obj->sdesc, POSSESSIVE(dMob), dMob->equipment[i]->worn[0]->sdesc );
         break;
      }
      else if( dMob->equipment[i]->worn[1] 
            && dMob->equipment[i]->worn[1]->type == ITEM_HOLSTER 
            && SizeOfList( dMob->equipment[i]->worn[1]->contents ) == 0 )
      {
         object_from_mobile( obj, dMob );
         object_to_object( obj, dMob->equipment[i]->worn[1] );
         holstered = TRUE;
         text_to_mobile_j( dMob, "text", "You holster %s in your %s.", 
               obj->sdesc, dMob->equipment[i]->worn[1]->sdesc );
         echo_around( dMob, "text", "%s holsters %s in %s %s.", MOBNAME(dMob),
               obj->sdesc, POSSESSIVE(dMob), dMob->equipment[i]->worn[1]->sdesc );
         break;
      }
   }
   if( holstered == FALSE )
   {
      text_to_mobile_j( dMob, "error", "You don't have anywhere to holster it." );
      return;
   }
   if( dMob->hold_right == obj )
      dMob->hold_right = NULL;
   else
      dMob->hold_left = NULL;
}

void cmd_sheath( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *obj;

   if( arg[0] == '\0' )
   {
      if( dMob->hold_right == NULL && dMob->hold_left == NULL )
      {
         text_to_mobile_j( dMob, "error", "You aren't holding anything." );
         return;
      }
      if( dMob->hold_right )
         cmd_sheath( dMob, dMob->hold_right->guid );
      if( dMob->hold_left )
         cmd_sheath( dMob, dMob->hold_left->guid );
      return;
   }
   
   if( dMob->hold_right && ( is_name( arg, dMob->hold_right->name ) || !strcmp( arg, dMob->hold_right->guid ) ) )
      obj = dMob->hold_right;
   else if( dMob->hold_left && ( is_name( arg, dMob->hold_left->name ) || !strcmp( arg, dMob->hold_left->guid ) ) )
      obj = dMob->hold_left;
   else
   {
      text_to_mobile_j( dMob, "error", "You aren't holding that." );
      return;
   }

   if( obj->type != ITEM_BLADE )
   {
      if( obj->type == ITEM_FIREARM )
      {
         if( obj->ivar4 < 2 )
            cmd_holster( dMob, arg );
         else
            cmd_sling( dMob, arg );
         return;
      }
      text_to_mobile_j( dMob, "error", "You can't sheath that." );
      return;
   }
   bool sheathed = FALSE;
   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      if( dMob->equipment[i]->worn[0] 
            && dMob->equipment[i]->worn[0]->type == ITEM_SHEATH 
            && SizeOfList( dMob->equipment[i]->worn[0]->contents ) == 0 )
      {
         object_from_mobile( obj, dMob );
         object_to_object( obj, dMob->equipment[i]->worn[0] );
         sheathed = TRUE;
         text_to_mobile_j( dMob, "text", "You sheath %s in your %s.", 
               obj->sdesc, dMob->equipment[i]->worn[0]->sdesc );
         echo_around( dMob, "text", "%s sheaths %s in %s %s.", MOBNAME(dMob), 
               obj->sdesc, POSSESSIVE(dMob), dMob->equipment[i]->worn[0]->sdesc );
         break;
      }
      else if( dMob->equipment[i]->worn[1] 
            && dMob->equipment[i]->worn[1]->type == ITEM_SHEATH 
            && SizeOfList( dMob->equipment[i]->worn[1]->contents ) == 0 )
      {
         object_from_mobile( obj, dMob );
         object_to_object( obj, dMob->equipment[i]->worn[1] );
         sheathed = TRUE;
         text_to_mobile_j( dMob, "text", "You sheath %s in your %s.", 
               obj->sdesc, dMob->equipment[i]->worn[1]->sdesc );
         echo_around( dMob, "text", "%s sheaths %s in %s %s.", MOBNAME(dMob), 
               obj->sdesc, POSSESSIVE(dMob), dMob->equipment[i]->worn[1]->sdesc );
         break;
      }
   }
   if( sheathed == FALSE )
   {
      text_to_mobile_j( dMob, "error", "You don't have anywhere to sheath it." );
      return;
   }
   if( dMob->hold_right == obj )
      dMob->hold_right = NULL;
   else
      dMob->hold_left = NULL;
}

void cmd_sling( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *obj;

   if( arg[0] == '\0' )
   {
      if( dMob->hold_right == NULL && dMob->hold_left == NULL )
      {
         text_to_mobile_j( dMob, "error", "You aren't holding anything." );
         return;
      }
      if( dMob->hold_right )
         cmd_sling( dMob, dMob->hold_right->guid );
      if( dMob->hold_left )
         cmd_sling( dMob, dMob->hold_left->guid );
      return;
   }
      
   if( dMob->hold_right && ( is_name( arg, dMob->hold_right->name ) || !strcmp( arg, dMob->hold_right->guid ) ) )
      obj = dMob->hold_right;
   else if( dMob->hold_left && ( is_name( arg, dMob->hold_left->name ) || !strcmp( arg, dMob->hold_left->guid ) ) )
      obj = dMob->hold_left;
   else
   {
      text_to_mobile_j( dMob, "error", "You aren't holding that." );
      return;
   }

   if( obj->type != ITEM_FIREARM )
   {
      if( obj->type == ITEM_BLADE )
      {
         cmd_sheath( dMob, arg );
         return;
      }
      text_to_mobile_j( dMob, "error", "You can't sling that." );
      return;
   }
   if( obj->ivar4 < 2 ) //For firearms-- number of hands needed to use it.
   {
      cmd_holster( dMob, arg );
      return;
   }

   //Cheat a little bit
   cmd_wear( dMob, obj->guid );
}

void cmd_get( D_MOBILE *dMob, char *arg )
{
   bool my = FALSE;
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Get what?" );
      return;
   }

   if( dMob->hold_right != NULL && dMob->hold_left != NULL )
   {
      text_to_mobile_j( dMob, "error", "Your hands are full!" );
      return;
   }

   if( ( dMob->hold_right != NULL && ( is_crippled( dMob, BODY_LARM ) || is_crippled( dMob, BODY_LHAND ) ) )
    || ( dMob->hold_left  != NULL && ( is_crippled( dMob, BODY_RARM ) || is_crippled( dMob, BODY_RHAND ) ) ) )
   {
      text_to_mobile_j( dMob, "error", "Your limb is crippled and you can't pick it up." );
      return;
   }

   char arg1[MAX_BUFFER], arg2[MAX_BUFFER];
   D_OBJECT *obj, *con;

   arg = one_arg( arg, arg1 );
   arg = one_arg( arg, arg2 );
   if( !strcasecmp( arg2, "from" ) && arg[0] != '\0' )
      arg = one_arg( arg, arg2 );
   if( !strcasecmp( arg2, "my" ) && arg[0] != '\0' )
   {
      my = TRUE;
      arg = one_arg( arg, arg2 );
   }

   if( arg2[0] != '\0' ) //get <arg1> [from] [my] <arg2>
   {
      if( ( con = get_object_mob( dMob, arg2 ) ) || ( ( con = get_object_list( arg2, dMob->room->objects ) ) && my == FALSE ))
      {
         if( ( obj = get_object_list( arg1, con->contents ) ) != NULL )
         {
            if( !can_lift( dMob, obj ) )
            {
               text_to_mobile_j( dMob, "error", "You're not strong enough to lift it." );
               return;
            }
            text_to_mobile_j( dMob, "text", "You get %s from %s and hold it in your %s hand.",
                  obj->sdesc, con->sdesc,
                  dMob->hold_right == NULL ? "right" : "left" );
            echo_around( dMob, "text", "%s gets %s from %s and holds it in %s %s hand.",
                  MOBNAME(dMob), obj->sdesc, con->sdesc,
                  POSSESSIVE( dMob ), dMob->hold_right == NULL ? "right" : "left" );
            object_from_object( obj, con );
            object_to_mobile( obj, dMob );
            return;
         }
         else
         {
            text_to_mobile_j( dMob, "error", "You can't find that in %s.", con->sdesc );
            return;
         }
      }
      else
      {
         text_to_mobile_j( dMob, "error", "You can't find that here." );
         return;
      }
      return;
   }

   if( ( obj = get_object_list( arg1, dMob->room->objects ) ) == NULL )
   {
      text_to_mobile_j( dMob, "error", "You don't see that here." );
      return;
   }
   else
   {
      if( !can_lift( dMob, obj ) )
      {
         text_to_mobile_j( dMob, "error", "You're not strong enough to lift it." );
         return;
      }
      text_to_mobile_j( dMob, "text", "You pick up %s and hold it in your %s hand.",
            obj->sdesc, dMob->hold_right == NULL ? "right" : "left" );
      echo_around( dMob, "text", "%s picks up %s and holds it in %s %s hand.",
            MOBNAME(dMob), obj->sdesc, POSSESSIVE( dMob ), dMob->hold_right == NULL ? "right" : "left" );
      object_from_room( obj, dMob->room );
      object_to_mobile( obj, dMob );
   }
   return;
}

void cmd_put( D_MOBILE *dMob, char *arg )
{
   char arg1[MAX_BUFFER], arg2[MAX_BUFFER];
   bool my = FALSE;
   arg = one_arg( arg, arg1 );
   arg = one_arg( arg, arg2 );
   D_OBJECT *con = NULL, *obj = NULL;

   if( !strcasecmp( arg2, "in" ) && arg[0] != '\0' )
      arg = one_arg( arg, arg2 );
   if( !strcasecmp( arg2, "my" ) && arg[0] != '\0' )
   {
      my = TRUE;
      arg = one_arg( arg, arg2 );
   }

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Put what in what?" );
      return;
   }

   if( dMob->hold_right == NULL && dMob->hold_left == NULL )
   {
      text_to_mobile_j( dMob, "error", "You aren't holding anything." );
      return;
   }

   //First check if they're holding the container, then check if they're wearing the container, then check if the container
   //is in the same room as them.
   if( ( dMob->hold_right && ( is_name( arg2, dMob->hold_right->name ) || !strcmp( dMob->hold_right->guid, arg2 ) ) ) )
   {
      con = dMob->hold_right;
   }
   else if( ( dMob->hold_left && ( is_name( arg2, dMob->hold_left->name ) || !strcmp( dMob->hold_left->guid, arg2 ) ) ) )
   {
      con = dMob->hold_left;
   }
   else if( ( con = get_object_mob( dMob,  arg2 ) ) != NULL )
   {
   }
   else if( my == FALSE && ( con = get_object_list( arg2, dMob->room->objects ) ) != NULL )
   {
   }
   else
   {
      if( my == TRUE )
         text_to_mobile_j( dMob, "error", "You're not wearing something like that." );
      else
         text_to_mobile_j( dMob, "error", "You don't see that here." );
      return;
   }
   
   /*
   if( con->capacity_cm3 < 1 )
   {
      text_to_mobile_j( dMob, "error", "%s isn't a container.", con->sdesc );
      return;
   }
   */
   if( dMob->hold_right && ( is_name( arg1, dMob->hold_right->name ) || !strcmp( dMob->hold_right->guid, arg1 ) ) )
      obj = dMob->hold_right;
   else if( dMob->hold_left && ( is_name( arg1, dMob->hold_left->name ) || !strcmp( dMob->hold_left->guid, arg1 ) ) )
      obj = dMob->hold_left;
   if( obj )
   {
      if( obj == con )
      {
         text_to_mobile_j( dMob, "error", "You can't put something inside itself." );
         return;
      }

      if( !object_can_fit( obj, con ) )
      {
         text_to_mobile_j( dMob, "error", "%s won't fit in %s.", obj->sdesc,
               con->sdesc );
         return;
      }

      if( (con->type == ITEM_HOLSTER && obj->type != ITEM_FIREARM && obj->ivar4 > 1 )
       || (con->type == ITEM_SHEATH && obj->type != ITEM_BLADE ) )
      {
         text_to_mobile_j( dMob, "error", "You can't put %s in %s.", obj->sdesc, con->sdesc );
         return;
      }
      object_from_mobile( obj, dMob );
      object_to_object( obj, con );
      text_to_mobile_j( dMob, "text", "You put %s in %s.", obj->sdesc,
            con->sdesc );
      echo_around( dMob, "text", "%s puts %s in %s", MOBNAME(dMob), obj->sdesc,
            con->sdesc );
      if( obj == dMob->hold_right && dMob->offer_right->what )
      {
         dMob->offer_right->what = NULL;
         dMob->offer_right->to = NULL;
         dMob->offer_right->when = 0;
      }
      else if( obj == dMob->hold_left && dMob->offer_left->what )
      {
         dMob->offer_left->what = NULL;
         dMob->offer_left->to = NULL;
         dMob->offer_left->when = 0;
      }
      if( obj == dMob->hold_right )
         dMob->hold_right = NULL;
      else
         dMob->hold_left = NULL;
   }
   else
   {
      text_to_mobile_j( dMob, "error", "You aren't holding that." );
      return;
   }
   return;
}

void cmd_drop( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Drop what?" );
      return;
   }
   if( dMob->hold_right && ( is_name( arg, dMob->hold_right->name ) || !strcmp( dMob->hold_right->guid, arg ) ) )
   {
      text_to_mobile_j( dMob, "text", "You drop %s from your right hand.\r\n", dMob->hold_right->sdesc );
      object_from_mobile( dMob->hold_right, dMob );
      object_to_room( dMob->hold_right, dMob->room );
      dMob->hold_right = NULL;
   }
   else if( dMob->hold_left && ( is_name( arg, dMob->hold_left->name ) || !strcmp( dMob->hold_left->guid, arg ) ) )
   {
      text_to_mobile_j( dMob, "text", "You drop %s from your left hand.\r\n", dMob->hold_left->sdesc );
      object_from_mobile( dMob->hold_left, dMob );
      object_to_room( dMob->hold_left, dMob->room );
      dMob->hold_left = NULL;
   }
   else
   {
      text_to_mobile_j( dMob, "error", "You aren't holding that.\r\n" );
   }
}

void cmd_fill( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Fill what from where?" );
      return;
   }

   char what_name[MAX_STRING_LENGTH];
   D_OBJECT *what = NULL;
   D_OBJECT *where = NULL;

   arg = one_arg( arg, what_name );

   if( dMob->hold_right == NULL && dMob->hold_left == NULL )
   {
      text_to_mobile_j( dMob, "error", "You need to be holding the container you wish to fill." );
      return;
   }

   if( dMob->hold_right && is_name( what_name, dMob->hold_right->name ) )
   {
      what = dMob->hold_right;
   }
   else if( dMob->hold_left && is_name( what_name, dMob->hold_left->name ) )
   {
      what = dMob->hold_left;
   }
   else
   {
      text_to_mobile_j( dMob, "error", "You aren't holding that." );
      return;
   }

   if( arg[0] == '\0' ) //filling from the ground
   {
      MergeList( dMob->room->objects, what->contents );
      text_to_mobile_j( dMob, "text", "You fill %s from the ground.", what->sdesc );
      echo_around( dMob, "text", "%s fills %s from the ground.", MOBNAME( dMob ), what->sdesc );
   }
   else //filling from another object
   {
      if( ( where = get_object_list( arg, dMob->room->objects ) ) == NULL )
      {
         text_to_mobile_j( dMob, "error", "You can't find that here." );
         return;
      }
      MergeList( where->contents, what->contents );
      text_to_mobile_j( dMob, "text", "You fill %s from %s", what->sdesc, where->sdesc );
      echo_around( dMob, "text", "%s fills %s from %s.", MOBNAME( dMob ), what->sdesc, where->sdesc );
   }

}

