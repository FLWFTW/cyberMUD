/**
 * @file objects.c
 * @author Will Sayin
 * @version 1.0
 *
 * @section DESCRIPTION
 * object support functions
 */
#include "mud.h"

D_OBJECT *new_object()
{
   D_OBJECT *obj = calloc( 1, sizeof( D_OBJECT ) );
   obj->adjectives = calloc( 1, sizeof( D_ADJECTIVES ) );

   obj->name = NULL;
   obj->sdesc = NULL;
   obj->ldesc = NULL;
   obj->vnum  = 0;
   obj->wear_pos = WEAR_NONE;
   obj->capacity_cm3 = 0;
   obj->volume_cm3 = 0;
   obj->weight_g = 0;
   obj->guid = gen_guid();
   obj->in_room = NULL;
   obj->type = ITEM_UNKNOWN;

   obj->ivar1 = 0;
   obj->ivar2 = 0;
   obj->ivar3 = 0;
   obj->ivar4 = 0;
   obj->ivar5 = 0;
   obj->ivar6 = 0;

   obj->svar1 = NULL;
   obj->svar2 = NULL;
   obj->svar3 = NULL;
   obj->svar4 = NULL;
   obj->svar5 = NULL;
   obj->svar6 = NULL;

   obj->contents = AllocList();
   obj->scripts  = AllocList();

   return obj;
}

D_OBJECT *get_object_mob( D_MOBILE *dMob, char *name )
{

   if( dMob->hold_right && ( is_name( name, dMob->hold_right->name ) || !strcmp( name, dMob->hold_right->guid ) ) )
      return dMob->hold_right;
   else if( dMob->hold_left && ( is_name( name, dMob->hold_left->name ) || !strcmp( name, dMob->hold_left->guid ) ) )
      return dMob->hold_left;

   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      if( dMob->equipment[i]->worn[1] != NULL && ( is_name( name, dMob->equipment[i]->worn[1]->name ) || !strcmp( name, dMob->equipment[i]->worn[1]->guid ) ) )
         return dMob->equipment[i]->worn[1];
      else if( dMob->equipment[i]->worn[0] != NULL && ( is_name( name, dMob->equipment[i]->worn[0]->name ) || !strcmp( name, dMob->equipment[i]->worn[0]->guid ) ) )
         return dMob->equipment[i]->worn[0];
   }

   return NULL;
}

char *get_odesc( D_OBJECT *dObj )
{
   char desc[MAX_STRING_LENGTH] = {0};
   static char ret[MAX_STRING_LENGTH] = {0};
   int count = 0;
   
   ret[0] = '\0';
   /*Count how many adjectives we have to decide if we are going to use commas or not*/
   if( dObj->adjectives->opinion ) count++;
   if( dObj->adjectives->size ) count++;
   if( dObj->adjectives->quality ) count++;
   if( dObj->adjectives->age ) count++;
   if( dObj->adjectives->shape ) count++;
   if( dObj->adjectives->color ) count++;
   if( dObj->adjectives->origin ) count++;
   if( dObj->adjectives->material ) count++;
   if( dObj->adjectives->type ) count++;
   if( dObj->adjectives->purpose ) count++;

   /* We'll use commas for 3 or more adjectives.
    * so we can wear stylish Italian sunglasses, or stylish, steel, Italian sunglasses.*/

   if( dObj->adjectives->opinion )
   {
      strncat( desc, dObj->adjectives->opinion, MAX_STRING_LENGTH - strlen( desc ) -1 );
      count++;
   }
   if( dObj->adjectives->size )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->size, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }
   if( dObj->adjectives->quality )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->quality, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }
   if( dObj->adjectives->age )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->age, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }
   if( dObj->adjectives->shape )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->shape, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }
   if( dObj->adjectives->color )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->color, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }
   if( dObj->adjectives->origin )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->origin, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }
   if( dObj->adjectives->material )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->material, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }
   if( dObj->adjectives->type )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->type, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }
   if( dObj->adjectives->purpose )
   {
      if( count > 2 )
         strncat( desc, ", ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      else if( desc[0] != '\0' )
         strncat( desc, " ", MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( desc, dObj->adjectives->purpose, MAX_STRING_LENGTH - strlen( desc ) -1 );
   }

   if( dObj->adjectives->quantity )
   {
      strncat( ret, dObj->adjectives->quantity, MAX_STRING_LENGTH - strlen( desc ) -1 );
      strncat( ret, " ", MAX_STRING_LENGTH - strlen( ret ) -1 );
   }
   strncat( ret, desc, MAX_STRING_LENGTH - strlen( ret ) -1 );
   strncat( ret,  " ", MAX_STRING_LENGTH - strlen( ret ) -1 );
   strncat( ret,  dObj->name, MAX_STRING_LENGTH - strlen( ret ) -1 );

   return ret;
}

void free_object( D_OBJECT *obj )
{
   free( obj->name );
   free( obj->sdesc );
   free( obj->ldesc );
   free( obj->guid );

   if( obj->adjectives->quantity )
      free( obj->adjectives->quantity );
   if( obj->adjectives->opinion )
      free( obj->adjectives->opinion );
   if( obj->adjectives->size )
      free( obj->adjectives->size );
   if( obj->adjectives->quality )
      free( obj->adjectives->quality );
   if( obj->adjectives->age )
      free( obj->adjectives->age );
   if( obj->adjectives->shape )
      free( obj->adjectives->shape );
   if( obj->adjectives->color )
      free( obj->adjectives->color );
   if( obj->adjectives->origin )
      free( obj->adjectives->origin );
   if( obj->adjectives->material )
      free( obj->adjectives->material );
   if( obj->adjectives->type )
      free( obj->adjectives->type );
   if( obj->adjectives->purpose )
      free( obj->adjectives->purpose );
   free( obj->adjectives );

   if( SizeOfList( obj->contents ) > 0 )
   {
      ITERATOR Iter;
      D_OBJECT *pObj;
      AttachIterator( &Iter, obj->contents );
      while( ( pObj = NextInList( &Iter ) ) != NULL )
         free_object( pObj );
      DetachIterator( &Iter );
   }
   FreeList( obj->contents );

   if( SizeOfList( obj->scripts ) > 0 )
   {
      ITERATOR Iter;
      char *script;
      AttachIterator( &Iter, obj->scripts );
      while( ( script = (char *)NextInList( &Iter ) ) != NULL )
         free( script );
      DetachIterator( &Iter );
   }
   FreeList( obj->scripts );

   DetachFromList( obj, dobject_list );
   DetachFromList( obj, object_protos );
   free( obj );
   
}

D_OBJECT *get_object_by_vnum( unsigned int vnum )
{
   ITERATOR Iter;
   D_OBJECT *obj;

   AttachIterator( &Iter, object_protos );
   while( ( obj = (D_OBJECT *)NextInList( &Iter ) ) != NULL )
      if( obj->vnum == vnum )
         break;
   DetachIterator( &Iter );
   return obj;
}

D_OBJECT *spawn_object( unsigned int vnum )
{
   ITERATOR Iter;
   D_OBJECT *obj;

   AttachIterator( &Iter, object_protos );
   while( ( obj = (D_OBJECT *)NextInList(&Iter) ) != NULL )
      if( obj->vnum == vnum )
         break;
   DetachIterator( &Iter );

   if( obj == NULL )
      return NULL;

   D_OBJECT *ret = new_object();
   ret->vnum = obj->vnum;
   ret->wear_pos = obj->wear_pos;
   ret->capacity_cm3 = obj->capacity_cm3;
   ret->volume_cm3 = obj->volume_cm3;
   ret->name = strdup( obj->name ? obj->name : "(NULL)" );
   ret->sdesc = strdup( obj->sdesc ? obj->sdesc : "(NULL)" );
   ret->ldesc = strdup( obj->ldesc ? obj->ldesc : "(NULL)" );
   ret->type  = obj->type;
   ret->weight_g = obj->weight_g;

   ret->ivar1 = obj->ivar1;
   ret->ivar2 = obj->ivar2;
   ret->ivar3 = obj->ivar3;
   ret->ivar4 = obj->ivar4;
   ret->ivar5 = obj->ivar5;
   ret->ivar6 = obj->ivar6;

   ret->svar1 = obj->svar1 == NULL ? NULL : strdup( obj->svar1 );
   ret->svar2 = obj->svar2 == NULL ? NULL : strdup( obj->svar2 );
   ret->svar3 = obj->svar3 == NULL ? NULL : strdup( obj->svar3 );
   ret->svar4 = obj->svar4 == NULL ? NULL : strdup( obj->svar4 );
   ret->svar5 = obj->svar5 == NULL ? NULL : strdup( obj->svar5 );
   ret->svar6 = obj->svar6 == NULL ? NULL : strdup( obj->svar6 );

   AttachToList( ret, dobject_list );
   return ret;
}

D_OBJECT *get_object_list( const char *name, LIST *list )
{
   ITERATOR Iter;
   D_OBJECT *obj;

   AttachIterator( &Iter, list );
   while( ( obj = NextInList( &Iter ) ) != NULL )
      if( is_name( (char *)name, obj->name ) || !strcmp( obj->guid, name ) )
         break;
   DetachIterator( &Iter );
   return obj;
}

void object_to_room( D_OBJECT *object, D_ROOM *room )
{
   object->in_room = room;
   object->in_object = NULL;
   object->carried_by = NULL;
   AppendToList( object, room->objects );
}

void object_to_object( D_OBJECT *object, D_OBJECT *container )
{
   object->in_room = NULL;
   object->in_object = container;
   object->carried_by = NULL;
   AppendToList( object, container->contents );
}

bool object_can_fit( D_OBJECT *object, D_OBJECT *container )
{
   if( container->capacity_cm3 < 1 )
      return FALSE;
   if( total_volume( object ) + ( total_volume( container ) - container->volume_cm3 ) > container->capacity_cm3 )
      return FALSE;
   return TRUE;
}

void object_to_mobile( D_OBJECT *object, D_MOBILE *dMob )
{
   if( dMob->hold_right != NULL && dMob->hold_left != NULL )
      return;
   object->in_room = NULL;
   object->in_object = NULL;
   object->carried_by = dMob;
   if( dMob->hold_right == NULL )
      dMob->hold_right = object;
   else
      dMob->hold_left = object;
   return;
}

bool can_lift( D_MOBILE *dMob, D_OBJECT *dObj )
{
   if( dMob->level > LEVEL_PLAYER ) //Immortals/admins can carry anything
      return TRUE;
   if( dObj->can_get == FALSE )
      return FALSE;
   if( calc_brawn( dMob ) * 100 > dObj->weight_g )
      return TRUE;
   return FALSE;
}

bool equip_object( D_MOBILE *dMob, D_OBJECT *obj )
{
   if( dMob->equipment[obj->wear_pos]->worn[1] == NULL )
   {
      //They are wearing nothing in that wear location
      if( dMob->equipment[obj->wear_pos]->worn[0] == NULL )
      {
         dMob->equipment[obj->wear_pos]->worn[0] = obj;
         if( dMob->hold_right == obj )
            dMob->hold_right = NULL;
         else if( dMob->hold_left == obj )
            dMob->hold_left = NULL;
         return TRUE;
      }

      //They have an item in their base layer, ensure they're not double stacking armor
      if( dMob->equipment[obj->wear_pos]->worn[0] != NULL && ( dMob->equipment[obj->wear_pos]->worn[0]->type != ITEM_ARMOR || obj->type != ITEM_ARMOR ) )
      {
         if( obj->wear_pos == WEAR_SLUNG )//or slinging more than one weapon at a time
         {
            return FALSE;
         }
         dMob->equipment[obj->wear_pos]->worn[1] = obj;
         if( dMob->hold_right == obj )
            dMob->hold_right = NULL;
         else if( dMob->hold_left == obj )
            dMob->hold_left = NULL;
         return TRUE;
      }
      return FALSE;
   }
   return FALSE;
}

void object_from_object( D_OBJECT *object, D_OBJECT *container )
{
   DetachFromList( object, container->contents );
   object->in_object = NULL;
   return;
}

void object_from_room( D_OBJECT *object, D_ROOM *room )
{
   DetachFromList( object, room->objects );
   object->in_room = NULL;
   object->in_object = NULL;
   object->carried_by = NULL;
}

void object_from_mobile( D_OBJECT *object, D_MOBILE *dMob )
{
   if( dMob->offer_right->what && dMob->offer_right->what == object )
   {
      text_to_mobile_j( dMob->offer_right->to, "text", "%s rescinds %s offer of %s.",
            dMob->name, POSSESSIVE(dMob), object->sdesc );
      dMob->offer_right->what = NULL;
      dMob->offer_right->to = NULL;
      dMob->offer_right->when = 0;
   }
   else if( dMob->offer_left->what && dMob->offer_left->what == object )
   {
      text_to_mobile_j( dMob->offer_right->to, "text", "%s rescinds %s offer of %s.",
            dMob->name, POSSESSIVE(dMob), object->sdesc );
      dMob->offer_left->what = NULL;
      dMob->offer_left->to = NULL;
      dMob->offer_left->when = 0;
   }
   object->in_room = NULL;
   object->in_object = NULL;
   object->carried_by = NULL;
}

size_t total_volume( D_OBJECT *obj )
{
   if( !obj )
      return 0;

   size_t size = 0;
   ITERATOR Iter;
   D_OBJECT *pObj;

   AttachIterator( &Iter, obj->contents );
   while( ( pObj = NextInList( &Iter ) ) != NULL )
      size += total_volume( pObj );
   DetachIterator( &Iter );

   return size + obj->volume_cm3;
}

D_ROOM *obj_to_room( D_OBJECT *dObj, D_ROOM *to )
{
   if( to == NULL )
      return NULL;

   if( dObj->in_room )
      DetachFromList( dObj, dObj->in_room->objects );
   dObj->in_room = to;
   AppendToList( dObj, to->objects );

   return to;
}

