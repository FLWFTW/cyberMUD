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

