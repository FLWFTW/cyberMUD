/**
 * @file handler_json.c
 * @author Will Sayin
 * @version 1.0
 *
 * @section DESCRIPTION
 * JSON handler. Lots of support functions for save.c and world.c,
 * serializes objects into JSON and turns JSON into objects.
 */
#include "mud.h"

json_t *adjectives_to_json( D_ADJECTIVES *a );
D_ADJECTIVES *json_to_adjectives( json_t *json );

D_ACCOUNT *json_to_account( json_t *json )
{
   D_ACCOUNT *account = new_account();
   json_t *value;
   const char *key;
   
   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "email" ) )
      {
         if( account->email ) free( account->email );
         account->email = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "name" ) )
      {
         if( account->name ) free( account->name );
         account->name = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "password" ) )
      {
         if( account->password ) free( account->password );
         account->password = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "characters" ) )
      {
         if( json_array_size( value ) > 0 )
         {
            size_t charIndex;
            json_t *charValue;
            json_array_foreach( value, charIndex, charValue )
            {
               AppendToList( strdup( json_string_value( charValue ) ), account->characters );
            }
         }
      }
      else
      {
         bug( "Unknown JSON key '%s'.", key );
      }
   }

   return account;
}

D_EXIT *json_to_exit( json_t *json )
{
   D_EXIT *exit = new_exit();
   json_t *value;
   const char *key;

   json_object_foreach( json, key, value )
   {
     if( !strcmp( key, "to_vnum" ) )
     {
        exit->to_vnum = json_integer_value( value );
     }
     else if( !strcmp( key, "lock_level" ) )
     {
        exit->lock_level = json_integer_value( value );
     }
     else if( !strcmp( key, "name" ) )
     {
        if( exit->name ) free( exit->name );
        exit->name = strdup( json_string_value( value ) );
     }
     else if( !strcmp( key, "farside_name" ) )
     {
        if( exit->farside_name ) free( exit->farside_name );
        exit->farside_name = strdup( json_string_value( value ) );
     }
     else if( !strcmp( key, "lock_state" ) )
      {
         exit->lock = (enum lock_state_tb)json_integer_value( value );
      }
      else if( !strcmp( key, "exit_state" ) )
      {
         exit->exit = (enum exit_state_tb)json_integer_value( value );
      }

   }

   return exit;
}

HELP_DATA *json_to_help( json_t *json )
{
   if( !json )
   return NULL;

   HELP_DATA *hdata = new_help();
   json_t *value;
   const char *key;

   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "keyword" ) )
      {
         if( hdata->keyword ) free( hdata->keyword );
         hdata->keyword = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "text" ) )
      {
         if( hdata->text ) free( hdata->text );
         hdata->text = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "level" ) )
      {
         hdata->level = json_integer_value( value );
      }
      else
      {
         bug( "Unknown help_data key '%s'.", key );
      }
   }
   hdata->load_time = time(NULL);

   return hdata;
}

json_t *help_to_json( HELP_DATA *help )
{
   if( !help )
      return NULL;

   json_t *json = json_object();

   json_object_set_new( json, "keyword", json_string( help->keyword ) );
   json_object_set_new( json, "text", json_string( help->text ) );
   json_object_set_new( json, "level", json_integer( help->level ) );

   return json;
}



D_ROOM *json_to_room( json_t *json )
{
   if( !json )
      return NULL;

   D_ROOM *room = new_room();
   json_t *value;
   const char *key;

   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "name" ) )
      {
         if( room->name ) free( room->name );
         room->name = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "description" ) )
      {
         if( room->description ) free( room->description );
         room->description = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "vnum" ) )
      {
         room->vnum = json_integer_value( value );
      }
      else if( !strcmp( key, "sector" ) )
      {
         int i;
         char s[MAX_STRING_LENGTH];
         strncpy( s, json_string_value( value ), MAX_STRING_LENGTH );
         for( i = 0; i < MAX_SECTOR; i++ )
         {
            if( !strcasecmp( s, sector_types[i] ) )
               break;
         }
         if( i == MAX_SECTOR )
         {
            bug( "Room with invalid sector %s.", s );
            i = 0;
         }
         room->sectortype = i;
      }
      else if( !strcmp( key, "exits" ) )
      {
         if( json_array_size( value ) > 0 )
         {
            size_t index;
            json_t *exit;
            json_array_foreach( value, index, exit )
            {
               AppendToList( json_to_exit( exit ), room->exits );
            }
         }

      }
   }

   return room;
}

json_t *room_to_json( D_ROOM *room )
{
   json_t *json = json_object();
   json_object_set_new( json, "name", json_string( room->name ) );
   json_object_set_new( json, "description", json_string( room->description ) );
   json_object_set_new( json, "vnum", json_integer( room->vnum ) );
   json_object_set_new( json, "sector", json_string( sector_types[room->sectortype] ) );
   json_t *exits = json_array();
   ITERATOR Iter;
   D_EXIT *exit = NULL;
   AttachIterator( &Iter, room->exits );
   while( ( exit = (D_EXIT *)NextInList( &Iter ) ) != NULL )
   {
      json_array_append_new( exits, exit_to_json( exit ) );
   }
   DetachIterator( &Iter );
   if( SizeOfList( room->scripts ) > 0 )
   {
      json_t *scripts = json_array();
      char *script;
      AttachIterator( &Iter, room->scripts );
      while( ( script = (char *)NextInList( &Iter ) ) != NULL )
      {
         json_array_append_new( scripts, json_string( script ) );
      }
      DetachIterator( &Iter );
      json_object_set_new( json, "scripts", scripts );
   }

   json_object_set_new( json, "exits", exits );
   return json;
}

D_OBJECT *json_to_object( json_t *json )
{
   if( json == NULL )
      return NULL;
   D_OBJECT *obj = new_object();
   json_t *value;
   const char *key;

   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "name" ) )
      {
         if( obj->name ) free( obj->name );
         obj->name = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "vnum" ) )
      {
         obj->vnum = json_integer_value( value );
      }
      else if( !strcmp( key, "sdesc" ) )
      {
         if( obj->sdesc ) free( obj->sdesc );
         obj->sdesc = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "ldesc" ) )
      {
         if( obj->ldesc ) free( obj->ldesc );
         obj->ldesc = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "wear_pos" ) )
      {
         char tmp[256];
         strncpy( tmp, json_string_value( value ), 256 );
         obj->wear_pos = WEAR_NONE;
         for( size_t i = WEAR_HEAD; i < MAX_WEAR; i++ )
         {
            if( !strcasecmp( tmp, wear_pos[i] ) )
            {
               obj->wear_pos = i;
               break;
            }
         }
      }
      else if( !strcmp( key, "capacity" ) )
      {
         obj->capacity_cm3 = json_integer_value( value );
      }
      else if( !strcmp( key, "volume" ) )
      {
         obj->volume_cm3 = json_integer_value( value );
      }
      else if( !strcmp( key, "type" ) )
      {
         obj->type = ITEM_UNKNOWN;
         for( size_t i = ITEM_CLOTHING; i < MAX_ITEM; i++ )
         {
            if( !strcmp( json_string_value( value ), item_type[i] ) )
            {
               obj->type = i;
               break;
            }
         }
      }               
      else if( !strcmp( key, "contents" ) )
      {
         json_t *arr;
         size_t index;
         json_array_foreach( value, index, arr )
         {
            AppendToList( json_to_object( arr ), obj->contents );
         }
      }
      else if( !strcmp( key, "guid" ) )
      {
         //These are keys we want to ignore, they are re-calculated elsewhere.
      }
      else if( !strcmp( key, "ivar1" ) )
      {
         obj->ivar1 = json_integer_value( value );
      }
      else if( !strcmp( key, "ivar2" ) )
      {
         obj->ivar2 = json_integer_value( value );
      }
      else if( !strcmp( key, "ivar3" ) )
      {
         obj->ivar3 = json_integer_value( value );
      }
      else if( !strcmp( key, "ivar4" ) )
      {
         obj->ivar4 = json_integer_value( value );
      }
      else if( !strcmp( key, "ivar5" ) )
      {
         obj->ivar5 = json_integer_value( value );
      }
      else if( !strcmp( key, "ivar6" ) )
      {
         obj->ivar6 = json_integer_value( value );
      }
      else if( !strcmp( key, "svar1" ) )
      {
         if( obj->svar1 ) free( obj->svar1 );
         obj->svar1 = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "svar2" ) )
      {
         if( obj->svar2 ) free( obj->svar2 );
         obj->svar2 = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "svar3" ) )
      {
         if( obj->svar3 ) free( obj->svar3 );
         obj->svar3 = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "svar4" ) )
      {
         if( obj->svar4 ) free( obj->svar4 );
         obj->svar4 = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "svar5" ) )
      {
         if( obj->svar5 ) free( obj->svar5 );
         obj->svar5 = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "svar6" ) )
      {
         if( obj->svar6 ) free( obj->svar6 );
         obj->svar6 = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "repair" ) )
      {
         obj->repair = json_integer_value( value );
         if( obj->repair > 100 ) obj->repair = 100;
      }
      else if( !strcmp( key, "adjectives" ) )
      {
         obj->adjectives = json_to_adjectives( value );
      }
      else
      {
         bug( "Unknown object JSON key \'%s\'.", key );
      }
   }
   return obj;
}

json_t *object_to_json_cli( D_OBJECT *obj )
{
   if( !obj )
      return NULL;

   json_t *json = json_object();

   json_object_set_new( json, "name", json_string( obj->name ) );
   json_object_set_new( json, "sdesc", json_string( obj->sdesc ) );
   json_object_set_new( json, "ldesc", json_string( obj->ldesc ) );
   json_object_set_new( json, "wear_pos", json_integer( obj->wear_pos ) );
   json_object_set_new( json, "wear_pos_string", json_string( wear_pos[obj->wear_pos] ) );
   json_object_set_new( json, "vnum", json_integer( obj->vnum ) );
   json_object_set_new( json, "capacity", json_integer( obj->capacity_cm3 ) );
   json_object_set_new( json, "volume", json_integer( obj->volume_cm3 ) );
   json_object_set_new( json, "guid", json_string( obj->guid ) );
   json_object_set_new( json, "type", json_string( item_type[obj->type] ) );
   json_object_set_new( json, "type_string", json_string( item_type[obj->type] ) );
   json_object_set_new( json, "repair", json_integer( obj->repair ) );

   json_object_set_new( json, "ivar1", json_integer( obj->ivar1 ) );
   json_object_set_new( json, "ivar2", json_integer( obj->ivar2 ) );
   json_object_set_new( json, "ivar3", json_integer( obj->ivar3 ) );
   json_object_set_new( json, "ivar4", json_integer( obj->ivar4 ) );
   json_object_set_new( json, "ivar5", json_integer( obj->ivar5 ) );
   json_object_set_new( json, "ivar6", json_integer( obj->ivar6 ) );
   if( obj->svar1 )
      json_object_set_new( json, "svar1", json_string( obj->svar1 ) );
   if( obj->svar2 )
      json_object_set_new( json, "svar2", json_string( obj->svar2 ) );
   if( obj->svar3 )
      json_object_set_new( json, "svar3", json_string( obj->svar3 ) );
   if( obj->svar4 )
      json_object_set_new( json, "svar4", json_string( obj->svar4 ) );
   if( obj->svar5 )
      json_object_set_new( json, "svar5", json_string( obj->svar5 ) );
   if( obj->svar6 )
      json_object_set_new( json, "svar6", json_string( obj->svar6 ) );


   return json;
}

json_t *object_to_json( D_OBJECT *obj )
{
   json_t *json = json_object();

   json_object_set_new( json, "name", json_string( obj->name ) );
   json_object_set_new( json, "sdesc", json_string( obj->sdesc ) );
   json_object_set_new( json, "ldesc", json_string( obj->ldesc ) );
   json_object_set_new( json, "wear_pos", json_string( wear_pos[obj->wear_pos] ) );
   json_object_set_new( json, "vnum", json_integer( obj->vnum ) );
   json_object_set_new( json, "capacity", json_integer( obj->capacity_cm3 ) );
   json_object_set_new( json, "volume", json_integer( obj->volume_cm3 ) );
   json_object_set_new( json, "guid", json_string( obj->guid ) );
   json_object_set_new( json, "type", json_string( item_type[obj->type] ) );
   json_object_set_new( json, "repair", json_integer( obj->repair ) );

   json_object_set_new( json, "adjectives", adjectives_to_json( obj->adjectives ) );

   json_object_set_new( json, "ivar1", json_integer( obj->ivar1 ) );
   json_object_set_new( json, "ivar2", json_integer( obj->ivar2 ) );
   json_object_set_new( json, "ivar3", json_integer( obj->ivar3 ) );
   json_object_set_new( json, "ivar4", json_integer( obj->ivar4 ) );
   json_object_set_new( json, "ivar5", json_integer( obj->ivar5 ) );
   json_object_set_new( json, "ivar6", json_integer( obj->ivar6 ) );
   if( obj->svar1 )
      json_object_set_new( json, "svar1", json_string( obj->svar1 ) );
   if( obj->svar2 )
      json_object_set_new( json, "svar2", json_string( obj->svar2 ) );
   if( obj->svar3 )
      json_object_set_new( json, "svar3", json_string( obj->svar3 ) );
   if( obj->svar4 )
      json_object_set_new( json, "svar4", json_string( obj->svar4 ) );
   if( obj->svar5 )
      json_object_set_new( json, "svar5", json_string( obj->svar5 ) );
   if( obj->svar6 )
      json_object_set_new( json, "svar6", json_string( obj->svar6 ) );

   if( SizeOfList( obj->contents ) > 0 )
   {
      json_t *contents = json_array();
      ITERATOR Iter;
      D_OBJECT *pObj;
      AttachIterator( &Iter, obj->contents );
      while( ( pObj = NextInList( &Iter ) ) != NULL )
      {
         json_array_append_new( contents, object_to_json( pObj ) );
      }
      DetachIterator( &Iter );
      json_object_set_new( json, "contents", contents );
   }

   if( SizeOfList( obj->scripts ) > 0 )
   {
      json_t *scripts = json_array();
      ITERATOR Iter;
      char *script;
      AttachIterator( &Iter, obj->scripts );
      while( ( script = (char *)NextInList( &Iter ) ) != NULL )
      {
         json_array_append_new( scripts, json_string( script ) );
      }
      DetachIterator( &Iter );
      json_object_set_new( json, "scripts", scripts );
   }

   return json;
}

json_t *reset_to_json( D_RESET *reset )
{
   json_t *json = json_object();
   json_object_set_new( json, "type", json_integer( reset->type ) );
   json_object_set_new( json, "location", json_integer( reset->location ) );
   json_object_set_new( json, "data", json_deep_copy( reset->data ) );
   return json;
}

D_RESET *json_to_reset( json_t *json )
{
   D_RESET *reset = NULL;
   const char *key;
   json_t *value;

   reset = calloc( 1, sizeof( D_RESET ) );

   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "type" ) )
      {
         reset->type = json_integer_value( value );
      }
      else if( !strcmp( key, "data" ) )
      {
         reset->data = json_deep_copy( value );
      }
      else if( !strcmp( key, "location" ) )
      {
         reset->location = json_integer_value( value );
      }
   }
   return reset;
}

SKILLS *json_to_skills( json_t *json )
{
   SKILLS *skills;
   const char *key;
   json_t *value;

   skills = calloc( 1, sizeof( SKILLS ) );

   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "combat" ) )
         skills->combat = json_integer_value( value );
      else if( !strcmp( key, "engineering" ) )
         skills->engineering = json_integer_value( value );
      else if( !strcmp( key, "subterfuge" ) )
         skills->subterfuge = json_integer_value( value );
      else if( !strcmp( key, "medicine" ) )
         skills->medicine = json_integer_value( value );
      else if( !strcmp( key, "personality" ) )
         skills->medicine = json_integer_value( value );
      else
         bug( "Unknown key %s in json_to_skills.", key );
   }

   return skills;
}



void load_equipment( D_MOBILE *dMob, json_t *json )
{
   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      json_t *position = json_object_get( json, wear_pos[i] );
      if( position == NULL )
         continue;
      dMob->equipment[i]->worn[0] = json_to_object( json_object_get( position, "worn0" ) );
      dMob->equipment[i]->worn[1] = json_to_object( json_object_get( position, "worn1" ) );
      if( dMob->equipment[i]->worn[0] )
         dMob->equipment[i]->worn[0]->carried_by = dMob;
      if( dMob->equipment[i]->worn[0] )
         dMob->equipment[i]->worn[0]->carried_by = dMob;
   }
}

void load_body( D_MOBILE *dMob, json_t *json )
{
   for( size_t i = BODY_HEAD; i < MAX_BODY; i++ )
   {
      json_t *part = json_object_get( json, body_parts[i] );
      if( part == NULL )
         continue;
      dMob->body[i]->cur_hp = json_integer_value( json_object_get( part, "cur_hp" ) );
      dMob->body[i]->max_hp = json_integer_value( json_object_get( part, "max_hp" ) );
      int calc_max = ( dMob->max_hp * body_hp_mod[i] ) / 100;
      if( dMob->body[i]->max_hp != calc_max )
      {
         dMob->body[i]->max_hp = calc_max;
      }
      dMob->body[i]->wound_trauma = json_integer_value( json_object_get( part, "wound_trauma" ) );
      dMob->body[i]->blunt_trauma = json_integer_value( json_object_get( part, "blunt_trauma" ) );
      dMob->body[i]->burn_trauma  = json_integer_value( json_object_get( part, "burn_trauma"  ) );
   }
}


D_MOBILE *json_to_mobile( json_t *json )
{
   json_t *value;
   const char *key;
   D_MOBILE *dMob = new_mobile();

   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "name" ) || !strcmp( key, "pcname" ) ) //name is a reserved word in javascript
      {
         if( dMob->name != NULL ) free( dMob->name );
         dMob->name = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "guid" ) )
      {
         //ignore
      }
      else if( !strcmp( key, "prompt" ) )
      {
         if( dMob->prompt != NULL ) free( dMob->prompt );
         dMob->prompt = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "sdesc" ) )
      {
         if( dMob->sdesc != NULL ) free( dMob->sdesc );
         dMob->sdesc = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "ldesc" ) )
      {
         if( dMob->ldesc != NULL ) free( dMob->ldesc );
         dMob->ldesc = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "association" ) )
      {
         if( dMob->association != NULL ) free( dMob->association );
         dMob->association = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "citizenship" ) )
      {
         if( dMob->citizenship != NULL ) free( dMob->citizenship );
         dMob->citizenship = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "race" ) )
      {
         if( dMob->race != NULL ) free( dMob->race );
         dMob->race = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "eyecolor" ) )
      {
         if( dMob->eyecolor != NULL ) free( dMob->eyecolor );
         dMob->eyecolor = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "haircolor" ) )
      {
         if( dMob->haircolor != NULL ) free( dMob->haircolor );
         dMob->haircolor = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "eyeshape" ) )
      {
         if( dMob->eyeshape != NULL ) free( dMob->eyeshape );
         dMob->eyeshape = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "hairstyle" ) )
      {
         if( dMob->hairstyle != NULL ) free( dMob->hairstyle );
         dMob->hairstyle = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "skincolor" ) )
      {
         if( dMob->skincolor != NULL ) free( dMob->skincolor );
         dMob->skincolor = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "build" ) )
      {
         if( dMob->build != NULL ) free( dMob->build );
         dMob->build = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "height" ) )
      {
         if( dMob->height != NULL ) free( dMob->height );
         dMob->height = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "age" ) )
      {
         dMob->age = json_integer_value( value );
      }
      else if( !strcmp( key, "level" ) )
      {
         dMob->level = json_integer_value( value );
      }
      else if( !strcmp( key, "skills" ) )
      {
         if( dMob->skills ) free( dMob->skills );
         dMob->skills = json_to_skills( value );
      }
      else if( !strcmp( key, "room" ) )
      {
         dMob->room = get_room_by_vnum( json_integer_value( value ) );
         if( dMob->room == NULL )
            dMob->room = get_room_by_vnum( 1 );
      }
      else if( !strcmp( key, "vnum" ) )
      {
         dMob->vnum = json_integer_value( value );
      }
      else if( !strcmp( key, "position" ) )
      {
         dMob->position = (enum position_tb)json_integer_value( value );
      }
      else if( !strcmp( key, "position_string" ) )
      {
         //do nothing
      }
      else if( !strcmp( key, "gender" ) )
      {
         if( !strcmp( json_string_value( value ), "female" ) )
            dMob->gender = FEMALE;
         else if( !strcmp( json_string_value( value ), "male" ) )
            dMob->gender = MALE;
         else
            dMob->gender = NONBINARY;
      }
      else if( !strcmp( key, "brains" ) )
      {
         dMob->brains = json_integer_value( value );
      }
      else if( !strcmp( key, "brawn" ) )
      {
         dMob->brawn = json_integer_value( value );
      }
      else if( !strcmp( key, "coordination" ) )
      {
         dMob->coordination = json_integer_value( value );
      }
      else if( !strcmp( key, "senses" ) )
      {
         dMob->senses = json_integer_value( value );
      }
      else if( !strcmp( key, "stamina" ) )
      {
         dMob->stamina = json_integer_value( value );
      }
      else if( !strcmp( key, "luck" ) )
      {
         dMob->luck = json_integer_value( value );
      }
      else if( !strcmp( key, "cool" ) )
      {
         dMob->cool = json_integer_value( value );
      }
      else if( !strcmp( key, "cur_hp" ) )
      {
         dMob->cur_hp = json_integer_value( value );
      }
      else if( !strcmp( key, "max_hp" ) )
      {
         dMob->max_hp = json_integer_value( value );
      }
      else if( !strcmp( key, "equipment" ) )
      {
         load_equipment( dMob, value );
      }
      else if( !strcmp( key, "body" ) )
      {
      }
      else if( !strcmp( key, "btc" ) )
      {
         dMob->btc = json_real_value( value );
      }
      else if( !strcmp( key, "cur_bandwidth" ) )
      {
         dMob->cur_bandwidth = json_integer_value( value );
      }
      else if( !strcmp( key, "max_bandwidth" ) )
      {
         dMob->max_bandwidth = json_integer_value( value );
      }
      else if( !strcmp( key, "signal" ) )
      {
         dMob->signal = json_integer_value( value );
      }
      else if( !strcmp( key, "encumberance" ) )
      {
         dMob->encumberance = json_integer_value( value );
      }
      else if( !strcmp( key, "hold_right" ) )
      {
         dMob->hold_right = json_to_object( value );
         dMob->hold_right->carried_by = dMob;
      }
      else if( !strcmp( key, "hold_left" ) )
      {
         dMob->hold_left = json_to_object( value );
         dMob->hold_left->carried_by = dMob;
      }
      else
      {
         bug( "Unknown JSON key '%s'.", key );
      }
   }

   
   load_body( dMob, json_object_get( json, "body" ) ); //we do this so when we calculate limb max_hp player's HP is definitely loaded and not 0

   return dMob;

}

json_t *exit_to_json( D_EXIT *exit )
{
   json_t *json = json_object();

   json_object_set_new( json, "to_vnum", json_integer( exit->to_vnum ) );
   json_object_set_new( json, "lock_level", json_integer( exit->lock_level ) );
   json_object_set_new( json, "lock_state", json_integer( exit->lock ) );
   json_object_set_new( json, "exit_state", json_integer( exit->exit ) );
   json_object_set_new( json, "name", json_string( exit->name ) );
   json_object_set_new( json, "farside_name", json_string( exit->farside_name ) );

   return json;
}

json_t *skills_to_json( SKILLS *skills )
{
   json_t *json = json_object();

   json_object_set_new( json, "combat", json_integer( skills->combat ) );
   json_object_set_new( json, "engineering", json_integer( skills->engineering ) );
   json_object_set_new( json, "subterfuge", json_integer( skills->subterfuge ) );
   json_object_set_new( json, "medicine", json_integer( skills->medicine ) );
   json_object_set_new( json, "personality", json_integer( skills->personality ) );

   return json;
}

json_t *mobile_to_json_cli( D_MOBILE *dMob )
{
   json_t *json = mobile_to_json( dMob, FALSE );
   json_object_del( json, "scripts" );
   json_object_del( json, "equipment" );

   return json;
}

json_t *adjectives_to_json( D_ADJECTIVES *a )
{
   json_t *json = json_object();
   if( a->quantity )
      json_object_set_new( json, "quantity", json_string( a->quantity ) );
   if( a->opinion )
      json_object_set_new( json, "opinion", json_string( a->opinion ) );
   if( a->size )
      json_object_set_new( json, "size", json_string( a->size ) );
   if( a->quality )
      json_object_set_new( json, "quality", json_string( a->quality ) );
   if( a->age )
      json_object_set_new( json, "age", json_string( a->age ) );
   if( a->shape )
      json_object_set_new( json, "shape", json_string( a->shape ) );
   if( a->color )
      json_object_set_new( json, "color", json_string( a->color ) );
   if( a->origin )
      json_object_set_new( json, "origin", json_string( a->origin ) );
   if( a->material )
      json_object_set_new( json, "material", json_string( a->material ) );
   if( a->type )
      json_object_set_new( json, "type", json_string( a->type ) );
   if( a->purpose )
      json_object_set_new( json, "purpose", json_string( a->purpose ) );

   return json;
}

D_ADJECTIVES *json_to_adjectives( json_t *json )
{
   D_ADJECTIVES *a = calloc( 1, sizeof( D_ADJECTIVES ) );

   const char *key;
   json_t *value;

   json_object_foreach( json, key, value )
   {
      if( !strcmp( key, "opinion" ) )
      {
         a->opinion = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "quantity" ) )
      {
         a->quantity = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "size" ) )
      {
         a->size = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "quality" ) )
      {
         a->quality = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "age" ) )
      {
         a->age = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "shape" ) )
      {
         a->shape = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "color" ) )
      {
         a->color = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "origin" ) )
      {
         a->origin = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "material" ) )
      {
         a->material = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "type" ) )
      {
         a->type = strdup( json_string_value( value ) );
      }
      else if( !strcmp( key, "purpose" ) )
      {
         a->purpose = strdup( json_string_value( value ) );
      }
      else
      {
         bug( "Unknown adjective type %s.", key );
      }

   }

   return a;
}

json_t *mobile_to_json( D_MOBILE *dMob, bool showEquipment )
{
   json_t *json = json_object();
   
   json_object_set_new( json, "name", json_string( dMob->name ) );
   json_object_set_new( json, "vnum", json_integer( dMob->vnum ) );
   json_object_set_new( json, "race", json_string( dMob->race ) );
   json_object_set_new( json, "citizenship", json_string( dMob->citizenship ) );
   json_object_set_new( json, "association", json_string( dMob->association ) );
   json_object_set_new( json, "prompt", json_string( dMob->prompt ) );
   json_object_set_new( json, "level", json_integer( dMob->level ) );
   json_object_set_new( json, "skills", skills_to_json( dMob->skills ) );
   json_object_set_new( json, "gender", json_string( dMob->gender == FEMALE ? "female" : dMob->gender == MALE ? "male" : "nonbinary" ) );
   if( IS_NPC( dMob ) )
   {
      json_object_set_new( json, "sdesc", json_string( dMob->sdesc ) );
      json_object_set_new( json, "ldesc", json_string( dMob->ldesc ) );
   }
   json_object_set_new( json, "position", json_integer( (int)dMob->position ) );
   json_object_set_new( json, "position_string", json_string( positions[dMob->position] ) );
   json_object_set_new( json, "guid", json_string( dMob->guid ) );
   json_object_set_new( json, "brains", json_integer( dMob->brains ) );
   json_object_set_new( json, "brawn", json_integer( dMob->brawn ) );
   json_object_set_new( json, "coordination", json_integer( dMob->coordination ) );
   json_object_set_new( json, "senses", json_integer( dMob->senses ) );
   json_object_set_new( json, "stamina", json_integer( dMob->stamina ) );
   json_object_set_new( json, "luck", json_integer( dMob->luck ) );
   json_object_set_new( json, "cool", json_integer( dMob->cool ) );
   json_object_set_new( json, "cur_hp", json_integer( dMob->cur_hp ) );
   json_object_set_new( json, "max_hp", json_integer( dMob->max_hp ) );
   json_object_set_new( json, "height", json_string( dMob->height ) );
   json_object_set_new( json, "build", json_string( dMob->build ) );
   json_object_set_new( json, "eyecolor", json_string( dMob->eyecolor ) );
   json_object_set_new( json, "eyeshape", json_string( dMob->eyeshape ) );
   json_object_set_new( json, "haircolor", json_string( dMob->haircolor ) );
   json_object_set_new( json, "hairstyle", json_string( dMob->hairstyle ) );
   json_object_set_new( json, "skincolor", json_string( dMob->skincolor ) );
   json_object_set_new( json, "age", json_integer( dMob->age ) );
   json_object_set_new( json, "signal", json_integer( dMob->signal ) );
   json_object_set_new( json, "cur_bandwidth", json_integer( dMob->cur_bandwidth ) );
   json_object_set_new( json, "max_bandwidth", json_integer( dMob->max_bandwidth ) );
   json_object_set_new( json, "btc", json_real( dMob->btc ) );
   json_object_set_new( json, "encumberance", json_integer( dMob->encumberance ) );

   json_t *body = json_object();
   for( size_t i = BODY_HEAD; i < MAX_BODY; i++ )
   {
      json_t *part = json_object();
      json_object_set_new( part, "cur_hp", json_integer( dMob->body[i]->cur_hp ) );
      json_object_set_new( part, "max_hp", json_integer( dMob->body[i]->max_hp ) );
      json_object_set_new( part, "wound_trauma", json_integer( dMob->body[i]->wound_trauma ) );
      json_object_set_new( part, "blunt_trauma", json_integer( dMob->body[i]->blunt_trauma ) );
      json_object_set_new( part, "burn_trauma",  json_integer( dMob->body[i]->burn_trauma  ) );
      
      json_object_set_new( body, body_parts[i], part );
   }
   json_object_set_new( json, "body", body );
   if( showEquipment )
   {
      if( dMob->hold_right )
         json_object_set_new( json, "hold_right", object_to_json( dMob->hold_right ) );
      if( dMob->hold_left )
         json_object_set_new( json, "hold_left", object_to_json( dMob->hold_left ) );

      json_t *equipment = json_object();
      for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
      {
         json_t *position = json_object();
         if( dMob->equipment[i]->worn[0] != NULL )
            json_object_set_new( position, "worn0", showEquipment == TRUE ? 
                                                      object_to_json( dMob->equipment[i]->worn[0] ) : 
                                                      object_to_json_cli( dMob->equipment[i]->worn[0] ) );
         if( dMob->equipment[i]->worn[1] != NULL )
            json_object_set_new( position, "worn1", showEquipment == TRUE ?
                                                      object_to_json( dMob->equipment[i]->worn[1] ) :
                                                      object_to_json_cli( dMob->equipment[i]->worn[1] ) );
         json_object_set_new( equipment, wear_pos[i], position );
      }
      json_object_set_new( json, "equipment", equipment);

   }

   if( SizeOfList( dMob->scripts ) > 0 )
   {
      json_t *scripts = json_array();
      ITERATOR Iter;
      char *script;
      AttachIterator( &Iter, dMob->scripts );
      while( ( script = (char *)NextInList( &Iter ) ) != NULL )
      {
         json_array_append_new( scripts, json_string( script ) );
      }
      DetachIterator( &Iter );
      json_object_set_new( json, "scripts", scripts );
   }

   return json;
}
json_t *player_to_json( D_MOBILE *dMob, bool showEquipment )
{
   json_t *json = mobile_to_json( dMob, showEquipment );

   //Put PC specific save code in here.
   json_object_set_new( json, "room", json_integer( dMob->room->vnum ) );

   return json;
}

json_t *json_from_keys( json_t *parent, int n, ... )
{
   json_t *result = json_object();
   char *key;

   va_list v;
   va_start( v, n );
   for( int i = 0; i < n; i++ )
   {
      key = va_arg( v, char *);
      json_object_set( result, key, json_string( "nada" ) );
   }
   va_end( v );

   json_object_update_existing( result, parent ); //updates only existing keys

   return result;
}


json_t *areaheader_to_json( D_AREA *area )
{
   json_t *json = json_object();
   json_object_set_new( json, "name", json_string( area->name ) );
   json_object_set_new( json, "filename", json_string( area->filename ) );
   json_object_set_new( json, "author", json_string( area->author ) );
   json_object_set_new( json, "reset_interval", json_integer( area->reset_interval ) );
   json_object_set_new( json, "reset_script", json_string( area->reset_script ) );
   json_object_set_new( json, "o_low", json_integer( area->o_low ) );
   json_object_set_new( json, "o_hi",  json_integer( area->o_hi ) );
   json_object_set_new( json, "m_low", json_integer( area->m_low ) );
   json_object_set_new( json, "m_hi",  json_integer( area->m_hi ) );
   json_object_set_new( json, "r_low", json_integer( area->r_low ) );
   json_object_set_new( json, "r_hi",  json_integer( area->r_hi ) );
   return json;
}

