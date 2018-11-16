#include "mud.h"

D_OBJECT *new_object()
{
   D_OBJECT *obj = malloc( sizeof( D_OBJECT ) );

   obj->name = NULL;
   obj->sdesc = NULL;
   obj->ldesc = NULL;
   obj->vnum  = 0;
   obj->wear_pos = WEAR_NONE;
   obj->capacity_cm3 = 0;
   obj->volume_cm3 = 0;
   obj->weight_g = 0;
   obj->guid = gen_guid();
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

void free_object( D_OBJECT *obj )
{
   free( obj->name );
   free( obj->sdesc );
   free( obj->ldesc );
   free( obj->guid );

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
   object->carried_by = NULL;
   if( dMob->hold_right == NULL )
      dMob->hold_right = object;
   else
      dMob->hold_left = object;
   return;
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
      text_to_mobile_j( dMob->offer_right->to, "text", "%s rescinds %s offer of %s %s",
            dMob->name, POSSESSIVE(dMob), AORAN( object->sdesc ), object->sdesc );
      dMob->offer_right->what = NULL;
      dMob->offer_right->to = NULL;
      dMob->offer_right->when = 0;
   }
   else if( dMob->offer_left->what && dMob->offer_left->what == object )
   {
      text_to_mobile_j( dMob->offer_right->to, "text", "%s rescinds %s offer of %s %s",
            dMob->name, POSSESSIVE(dMob), AORAN( object->sdesc ), object->sdesc );
      dMob->offer_left->what = NULL;
      dMob->offer_left->to = NULL;
      dMob->offer_left->when = 0;
   }
   object->in_room = NULL;
   object->in_object = NULL;
   object->carried_by = NULL;
}

