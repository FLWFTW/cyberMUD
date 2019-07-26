#include "mud.h"

void cmd_oset( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "What object do you want to edit?" );
      return;
   }

   D_OBJECT *obj;
   char oname[MAX_STRING_LENGTH] = {0};
   char action[MAX_STRING_LENGTH] = {0};

   arg = one_arg( arg, oname );
   arg = one_arg( arg, action );

   if( is_number( oname ) )
   {
      obj = get_object_by_vnum( atoi( oname ) );
   }
   else
   {
      if( dMob->hold_right && is_name( oname, dMob->hold_right->name ) )
         obj = dMob->hold_right;
      else if( dMob->hold_left && is_name( oname, dMob->hold_left->name ) )
         obj = dMob->hold_left;
      else
         obj = get_object_list( oname, dMob->room->objects );
   }

   if( obj == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find that object here." );
      return;
   }

   if( !strcasecmp( action, "short" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's short description to what?" );
         return;
      }
      if( obj->sdesc ) free( obj->sdesc );
      obj->sdesc = strdup( arg );
   }
   else if( !strcasecmp( action, "long" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's long description to what?" );
         return;
      }
      if( obj->ldesc ) free( obj->ldesc );
      obj->ldesc = strdup( arg );
   }
   else if( !strcasecmp( action, "name" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's name to what?" );
         return;
      }
      if( obj->name ) free( obj->name );
      obj->name = strdup( arg );
   }
   else if( !strcasecmp( action, "type" ) )
   {
      enum item_type_t i;
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's type to what?" );
         return;
      }
      for( i = ITEM_CLOTHING; i < MAX_ITEM; i++ )
      {
         if( is_prefix( arg, item_type[i] ) )
         {
            obj->type = i;
            break;
         }
      }
      if( i == MAX_ITEM )
      {
         text_to_mobile_j( dMob, "error", "Invalid object type." );
         return;
      }
   }
   else if( !strcasecmp( action, "wear" ) )
   {
      enum wear_pos_t i;
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the oject's wear position to what?" );
         return;
      }
      for( i = WEAR_HEAD; i < MAX_WEAR; i++ )
      {
         if( is_prefix( arg, wear_pos[i] ) )
         {
            obj->wear_pos = i;
            break;
         }
      }
      if( !is_prefix( arg, "none" ) && i == WEAR_NONE )
      {
         text_to_mobile_j( dMob, "error", "Invalid wear position." );
         return;
      }
   }
   else
   {
      text_to_mobile_j( dMob, "Error", "Invalid oset command." );
      return;
   }

   text_to_mobile_j( dMob, "text", "Ok." );
   return;

}

void cmd_eset( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Edit which exit?" );
      return;
   }

   char dir[MAX_STRING_LENGTH], what[MAX_STRING_LENGTH];

   arg = one_arg( arg, dir );
   arg = one_arg( arg, what );
   
   D_EXIT *exit = get_exit_by_name( dMob->room, dir );
   if( exit == NULL )
   {
      text_to_mobile_j( dMob, "error", "Can not find exit %s. Create it with 'redit exit %s <vnum>' first.", dir, dir );
      return;
   }

   if( is_prefix( what, "door_level" ) )
   {
      int level = atoi( arg );
      if( level < 0 || level > 10 )
      {
         text_to_mobile_j( dMob, "error", "Door level must be between 0 and 10." );
         return;
      }
      exit->door_level = level;
      text_to_mobile_j( dMob, "text", "Exit %s door level set to %i.", exit->name, level );
   }
   else if( is_prefix( what, "lock_level" ) )
   {
      int level = atoi( arg );
      if( level < 0 || level > 10 )
      {
         text_to_mobile_j( dMob, "error", "Lock level must be between 0 and 10." );
         return;
      }
      exit->lock_level = level;
      text_to_mobile_j( dMob, "text", "Exit %s lock level set to %i.", exit->name, level );
   }
   else if( is_prefix( what, "lock_type" ) )
   {
      enum lock_type_t i = 0;
      for( i = 0; i < MAX_LOCKTYPE; i++ )
      {
         if( is_prefix( arg, lock_type[i] ) )
         {
            exit->lock_type = i;
            break;
         }
      }
      if( i == MAX_LOCKTYPE )
      {
         text_to_mobile_j( dMob, "error", "Invalid lock type %s.", arg );
      }
      else
      {
         text_to_mobile_j( dMob, "text", "Exit %s lock type set to %s.", exit->name, lock_type[i] );
      }
   }
   else if( is_prefix( what, "lock_state" ) )
   {
      enum lock_state_t i = 0;
      for( i = 0; i < MAX_LOCKSTATE; i++ )
      {
         if( is_prefix( arg, lock_state[i] ) )
         {
            exit->lock = i;
            break;
         }
      }
      if( i == MAX_LOCKSTATE )
      {
         text_to_mobile_j( dMob, "error", "Invalid lock state %s.", arg );
      }
      else
      {
         text_to_mobile_j( dMob, "text", "Exit %s lock state set to %s.", exit->name, lock_state[i] );
      }
   }
   else if( is_prefix( what, "exit_state" ) )
   {
      enum exit_state_t i = 0;
      for( i = 0; i < MAX_EXITSTATE; i++ )
      {
         if( is_prefix( arg, exit_state[i] ) )
         {
            exit->exit = i;
            break;
         }
      }
      if( i == MAX_EXITSTATE )
      {
         text_to_mobile_j( dMob, "error", "Invalid exit state %s.", arg );
      }
      else
      {
         text_to_mobile_j( dMob, "text", "Exit %s state set to %s.", exit->name, exit_state[i] );
      }
   }

   return;
}

void cmd_redit( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "See help redit for usage." );
      return;
   }

   char arg1[MAX_STRING_LENGTH] = {0};
   arg = one_arg( arg, arg1 );

   if( !strcasecmp( arg1, "desc" ) || !strcasecmp( arg1, "description" ) )
   {
      dMob->socket->state = STATE_WRITING;
      text_to_mobile_j( dMob, "text", "[------------------------------------------------------------------------------]" );
      dMob->socket->edit_pointer = &dMob->room->description;
      dMob->socket->edit_buffer = strdup( dMob->room->description );
      return;
   }
   else if( !strcasecmp( arg1, "name" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the room name to what?" );
         return;
      }
      if( dMob->room->name )
         free( dMob->room->name );
      dMob->room->name = strdup( arg );
   }
   else if( !strcasecmp( arg1, "rmexit" ) )
   {
      D_EXIT *exit;
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Remove which exit?" );
         return;
      }

      if( ( exit = get_exit_by_name( dMob->room, arg ) ) == NULL )
      {
         text_to_mobile_j( dMob, "error", "That exit does not exist." );
         return;
      }
      DetachFromList( exit, dMob->room->exits );
      free_exit( exit );
   }
   else if( !strcasecmp( arg1, "exit" ) )
   {
      char dir[MAX_STRING_LENGTH];
      int vnum = 0;
      arg = one_arg( arg, dir );
      vnum = atoi( arg );
      D_ROOM *room;
      if( !is_prefix( dir, "north" ) && !is_prefix( dir, "south" ) && !is_prefix( dir, "east" ) && !is_prefix( dir, "west" ) &&!is_prefix( dir, "up" ) && !is_prefix( dir, "down" )
         &&strcasecmp( dir, "ne" ) && strcasecmp( dir, "se" ) && strcasecmp( dir, "sw" ) && strcasecmp( dir, "nw" ) )
      {
         text_to_mobile_j( dMob, "error", "Exit direction must be one of n/s/e/w/ne/nw/se/sw/u/d." );
         return;
      }
      if( vnum < 1 || ( ( room = get_room_by_vnum( vnum ) ) == NULL ) )
      {
         text_to_mobile_j( dMob, "error", "Invalid vnum." );
         return;
      }

      D_EXIT *exit;
      if( ( exit = get_exit_by_name( dMob->room, dir ) ) != NULL )
         free_exit( exit );
      exit = new_exit();

      if( is_prefix( dir,  "north" ) )
      {
         exit->name = strdup( "north" );
         exit->farside_name = strdup( "south" );
      }
      else if( is_prefix( dir, "south" ) )
      {
         exit->name = strdup( "south" );
         exit->farside_name = strdup( "north" );
      }
      else if( is_prefix( dir, "east" ) )
      {
         exit->name = strdup( "east" );
         exit->farside_name = strdup( "west" );
      }
      else if( is_prefix( dir, "west" ) )
      {
         exit->name = strdup( "west" );
         exit->farside_name = strdup( "east" );
      }
      else if( !strcasecmp( dir, "ne" ) || is_prefix( dir, "northeast" ) )
      {
         exit->name = strdup( "northeast" );
         exit->farside_name = strdup( "southwest" );
      }
      else if( !strcasecmp( dir, "nw" ) || is_prefix( dir, "northwest" ) )
      {
         exit->name = strdup( "northwest" );
         exit->farside_name = strdup( "southeast" );
      }
      else if( !strcasecmp( dir, "se" ) || is_prefix( dir, "southeast" ) )
      {
         exit->name = strdup( "southeast" );
         exit->farside_name = strdup( "northwest" );
      }
      else if( !strcasecmp( dir, "sw" ) || is_prefix( dir, "southwest" ) )
      {
         exit->name = strdup( "southwest" );
         exit->farside_name = strdup( "northeast" );
      }
      else if( is_prefix( dir, "up" ) )
      {
         exit->name = strdup( "up" );
         exit->farside_name = strdup( "down" );
      }
      else if( is_prefix( dir, "down" ) )
      {
         exit->name = strdup( "down" );
         exit->farside_name = strdup( "up" );
      }
      exit->to_room = room;
      exit->to_vnum = vnum;

      AppendToList( exit, dMob->room->exits );
   }

   text_to_mobile_j( dMob, "text", "Ok." );

   return;
}

void cmd_savearea( D_MOBILE *dMob, char *arg )
{

   json_t *rooms = json_array();
   json_t *objects = json_array();
   json_t *mobiles = json_array();
   json_t *resets = json_array();
   json_t *area = json_object();
   D_AREA *pArea;
   D_ROOM *pRoom;
   D_OBJECT *pObj;
   D_MOBILE *pMob;
   D_RESET *pReset;
   ITERATOR Iter;

   pArea = dMob->room->area;

   for( int i = pArea->r_low; i <= pArea->r_hi; i++ )
   {
      pRoom = get_room_by_vnum( i );
      if( pRoom == NULL )
         continue;
      json_array_append_new( rooms, room_to_json( pRoom ) );
   }
   json_object_set_new( area, "rooms", rooms );

   for( int i = pArea->o_low; i <= pArea->o_hi; i++ )
   {
      pObj = get_object_by_vnum( i );
      if( pObj == NULL )
         continue;
      json_array_append_new( objects, object_to_json( pObj ) );
   }
   json_object_set_new( area, "objects", objects );

   for( int i = pArea->m_low; i <= pArea->m_hi; i++ )
   {
      pMob = get_mobile_by_vnum( i );
      if( pMob == NULL )
         continue;
      json_array_append_new( mobiles, mobile_to_json( pMob, FALSE ) );
   }
   json_object_set_new( area, "mobiles", mobiles );

   AttachIterator( &Iter, pArea->resets );
   while( ( pReset = (D_RESET *)NextInList( &Iter ) ) != NULL )
   {
      json_array_append_new( resets, reset_to_json( pReset ) );
   }
   DetachIterator( &Iter );
   json_object_set_new( area, "resets", resets );

   char filename[MAX_STRING_LENGTH];
   snprintf( filename, MAX_STRING_LENGTH, "%s.json", pArea->name );
   if( pArea->filename == NULL )
      pArea->filename = strdup( filename );

   if( pArea->filename[0] == '.' )
      pArea->filename[0] = '_';
   if( pArea->filename[1] == '.' )
      pArea->filename[1] = '_';
   for( int i = 0; i < strlen( pArea->filename ); i++ )
   {
      if( pArea->filename[i] == '/' || pArea->filename[i] == '\\' || pArea->filename[i] == '*' || pArea->filename[i] == '?' || pArea->filename[i] == '\''
       || pArea->filename[i] == '\"'|| pArea->filename[i] == '|' || pArea->filename[i] == '<'  || pArea->filename[i] == '>' || pArea->filename[i] == ' ' )
         pArea->filename[i] = '_';
   }
   snprintf( filename, MAX_STRING_LENGTH, "../areas/%s", pArea->filename );
   json_dump_file( area, filename, JSON_INDENT(3) );

   return;
}
