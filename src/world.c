#include "mud.h"


void load_area_file( D_AREA *area )
{
   if( !area )
      return;

   char filename[MAX_STRING_LENGTH];
   const char *key;
   json_t *value;
   size_t index;
   json_t *array;
   json_t *json;

   snprintf( filename, MAX_STRING_LENGTH, "../areas/%s", area->filename );

   if( ( json = json_load_file( filename, 0, NULL ) ) == NULL )
   {
      bug( "Can't find area file for area '%s' (%s) [%s]", area->name, area->filename, filename );
      return;
   }

   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "rooms" ) )
      {
         json_array_foreach( value, index, array )
         {
            D_ROOM *room = json_to_room( array );
            room->area = area;
            AttachToList( room, area->rooms );
         }
      }
      else if( !strcmp( key, "mobiles" ) )
      {
         D_MOBILE *mob;
         json_array_foreach( value, index, array )
         {
            mob = json_to_mobile( array );
            AttachToList( mob, area->mobiles );
            AttachToList( mob, mobile_protos );
         }
      }
      else if( !strcmp( key, "objects" ) )
      {
         D_OBJECT *obj;
         json_array_foreach( value, index, array )
         {
            obj = json_to_object( array );
            AttachToList( obj, area->objects );
            AttachToList( obj, object_protos );
         }
      }
      else if( !strcmp( key, "resets" ) )
      {
         D_RESET *reset;
         json_array_foreach( value, index, array )
         {
            reset = json_to_reset( array );
            AttachToList( reset, area->resets );
         }
      }
   }
   json_decref( json );
   return;
}



void load_areas()
{
   json_t *json = json_load_file( "../areas/arealist.json", 0, NULL );
   if( json == NULL )
   {
      bug( "Unable to load area list. Aborting boot..." );
      abort();
   }
   
   size_t index;
   json_t *json_area;
   const char *key;
   json_t *value;

   json_array_foreach( json, index, json_area )
   {
      D_AREA *area = new_area();
      json_object_foreach( json_area, key, value )
      {
         if( !strcmp( key, "name" ) )
            area->name = strdup( json_string_value( value ) );
         else if( !strcmp( key, "author" ) )
            area->author = strdup( json_string_value( value ) );
         else if( !strcmp( key, "filename" ) )
            area->filename = strdup( json_string_value( value ) );
         else if( !strcmp( key, "reset_script" ) )
            area->reset_script = strdup( json_string_value( value ) );
         else if( !strcmp( key, "o_low" ) )
            area->o_low = json_integer_value( value );
         else if( !strcmp( key, "o_hi" ) )
            area->o_hi = json_integer_value( value );
         else if( !strcmp( key, "m_low" ) )
            area->m_low = json_integer_value( value );
         else if( !strcmp( key, "m_hi" ) )
            area->m_hi = json_integer_value( value );
         else if( !strcmp( key, "r_low" ) )
            area->r_low = json_integer_value( value );
         else if( !strcmp( key, "r_hi" ) )
            area->r_hi = json_integer_value( value );
      }
      AttachToList( area, darea_list );
      load_area_file( area );
   }
      
   return;
}

void link_exits()
{
   ITERATOR Iter, eIter;
   D_ROOM *room;
   D_EXIT *exit;

   AttachIterator( &Iter, droom_list );
   while( ( room = (D_ROOM *)NextInList( &Iter ) ) != NULL )
   {
      if( SizeOfList( room->exits ) > 0 )
      {
         AttachIterator( &eIter, room->exits );
         while( ( exit = (D_EXIT *)NextInList( &eIter ) ) != NULL )
         {
            exit->to_room = get_room_by_vnum( exit->to_vnum );
            if( exit->to_room == NULL )
            {
               bug( "Exit %s in room vnum %i pointing to room vnum %i. Can not find room vnum %i.", exit->name, room->vnum, exit->to_vnum, exit->to_vnum );
               continue;
            }
            exit->farside_exit = get_exit_by_name( exit->to_room, exit->farside_name );
         }
         DetachIterator( &eIter );
      }
   }
   DetachIterator( &Iter );

   return;
}


