#include "mud.h"

void reset_area( D_AREA *pArea )
{
   if( pArea == NULL )
      return;

   LIST *rlist = pArea->resets;
   D_RESET *reset;
   D_ROOM  *room;
   D_MOBILE *dMob;
   D_OBJECT *dObj;

   ITERATOR Iter;
   pArea->last_reset = time(NULL);
   if( pArea->reset_script && pArea->reset_script[0] != '\0' &&
       luaL_dostring( globalLuaState, pArea->reset_script ) == 1 )
   {
      log_string( "Error running reset script for area %s.\n%s\n", pArea->name, pArea->reset_script );
   }
   AttachIterator( &Iter, rlist );
   while( ( reset = (D_RESET *)NextInList( &Iter ) ) != NULL )
   {
      if( ( room = get_room_by_vnum( reset->location ) ) == NULL )
      {
         bug( "Reset Error: Reset with null room (vnum: %i)", reset->location );
         continue;
      }
      if( reset->data == NULL )
      {
         bug( "Reset Error: Reset with null data!" );
         continue;
      }
      switch( reset->type )
      {
         case RESET_EXIT:
            {
               break;
            }
         case RESET_OBJ:
            {
               D_OBJECT *pObj = (D_OBJECT *)reset->what;
               if( pObj == NULL || pObj->in_room != room )
               {
                  if( ( dObj = reset_to_object( reset->data ) ) == NULL )
                  {
                     bug( "Reset Error: Bad JSON data." );
                     break;
                  }
                  AppendToList( dObj, dobject_list );
                  obj_to_room( dObj, room );
               }
               break;
            }
         case RESET_MOB:
            {
               if( reset->what == NULL )
               {
                  if( ( dMob = reset_to_mobile( reset->data ) ) == NULL )
                  {
                     bug( "Reset Error: Bad JSON data." );
                     break;
                  }
                  reset->what = dMob;
                  dMob->reset = reset;
                  AppendToList( dMob, dmobile_list );
                  mob_to_room( dMob, room );
                  break;
               }
            }
         case MAX_RESET:
            {
               break;
            }
         default:
            {
               bug( "Reset Error: Unknown reset type (%i)", reset->type );
               break;
            }
      }
   }
   DetachIterator( &Iter );


}

json_t *object_to_reset( D_OBJECT *obj )
{
   json_t *json = json_object();

   json_object_set_new( json, "vnum", json_integer( obj->vnum ) );
   if( SizeOfList( obj->contents ) > 0 )
   {
      json_t *contents = json_array();
      ITERATOR Iter;
      D_OBJECT *pObj;
      AttachIterator( &Iter, obj->contents );
      while( ( pObj = NextInList( &Iter ) ) != NULL )
      {
         json_array_append_new( contents, object_to_reset( pObj ) );
      }
      DetachIterator( &Iter );
      json_object_set_new( json, "contents", contents );
   }
   return json;
}

D_OBJECT *reset_to_object( json_t *data )
{
   D_OBJECT *obj;

   int vnum = json_integer_value( json_object_get( data, "vnum" ) );
   obj = spawn_object( vnum );
   if( obj == NULL )
   {
      bug( "Reset refers to object with vnum %i, which does not exist.", vnum );
      return NULL;
   }
   json_t *contents = json_object_get( data, "contents" );
   json_t *object;
   int index;
   json_array_foreach( contents, index, object )
   {
      object_to_object( reset_to_object( object ), obj );
   }

   return obj;

}

json_t *mobile_to_reset( D_MOBILE *dMob )
{
   if( dMob == NULL )
      return NULL;

   json_t *json = json_object();
   json_object_set_new( json, "vnum", json_integer( dMob->vnum ) );
   if( dMob->hold_right )
      json_object_set_new( json, "hold_right", object_to_reset( dMob->hold_right ) );
   if( dMob->hold_left )
      json_object_set_new( json, "hold_left", object_to_reset( dMob->hold_left ) );
  
   json_t *equipment = json_object();
   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      json_t *position = json_object();
      if( dMob->equipment[i]->worn[0] != NULL )
         json_object_set_new( position, "worn0", object_to_reset( dMob->equipment[i]->worn[0] ) );
      if( dMob->equipment[i]->worn[1] != NULL )
         json_object_set_new( position, "worn1", object_to_reset( dMob->equipment[i]->worn[1] ) );
      if( dMob->equipment[i]->worn[0] || dMob->equipment[i]->worn[1] )
         json_object_set_new( equipment, wear_pos[i], position );
   }
   json_object_set_new( json, "equipment", equipment);

   return json;
}

D_MOBILE *reset_to_mobile( json_t *data )
{
   D_MOBILE *dMob;

   int vnum = json_integer_value( json_object_get( data, "vnum" ) );
   dMob = spawn_mobile( vnum );
   if( dMob == NULL )
   {
      bug( "Reset refers to mobile with vnum %i, which does not exist.", vnum );
      return NULL;
   }

   json_t *equipment = NULL;
   json_t *equipped = json_object_get( data, "equipment" );
   if( equipped == NULL )
      return dMob;

   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      equipment = json_object_get( equipped, wear_pos[i] );
      if( equipment == NULL )
         continue;
      dMob->equipment[i]->worn[0] = reset_to_object( json_object_get( equipment, "worn0" ) );
      dMob->equipment[i]->worn[1] = reset_to_object( json_object_get( equipment, "worn1" ) );
      if( dMob->equipment[i]->worn[0] != NULL )
         dMob->equipment[i]->worn[0]->carried_by = dMob;
      if( dMob->equipment[i]->worn[1] != NULL )
         dMob->equipment[i]->worn[1]->carried_by = dMob;
   }

   return dMob;
}

