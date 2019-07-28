#include "mud.h"

void cmd_guiredit( D_MOBILE *dMob, char *arg )
{
   if( IS_NPC( dMob ) )
      return;
   text_to_socket( dMob->socket, "%c{\"type\":\"build_room_data\", \"data\":{\"room\":%s}}%c", (char)2, json_dumps( room_to_json( dMob->room ), 0 ), (char)3 );
   return;
}
void cmd_makearea( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Use makearea <filename.json> to make a new area, where <filename.json> is an available filename." );
      return;
   }
   
   char filename[MAX_STRING_LENGTH];
   int len = strlen( arg );
   ITERATOR Iter;
   D_AREA *area;

   if( arg[0] == '.' )
      arg[0] = '_';
   if( arg[1] == '.' )
      arg[1] = '_';
   for( int i = 0; i < len; i++ )
   {
      if( arg[i] == '/' || arg[i] == '\\' || arg[i] == '*' || arg[i] == '?' || arg[i] == '\''
       || arg[i] == '\"'|| arg[i] == '|' || arg[i] == '<'  || arg[i] == '>' || arg[i] == ' ' )
      {
         arg[i] = '_';
      }
   }
   if( len > 5 && arg[len-1] == 'n' && arg[len-2] == 'o' && arg[len-3] == 's' && arg[len-4] == 'j' && arg[len-5] == '.' )
   {
      snprintf( filename, MAX_STRING_LENGTH, "%s", arg );
   }
   else
   {
      snprintf( filename, MAX_STRING_LENGTH, "%s.json", arg );
   }

   AttachIterator( &Iter, darea_list );
   while( ( area = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      if( !strcasecmp( area->filename, filename ) )
      {
         text_to_mobile_j( dMob, "error", "Can not make area. Area with filename %s already exists.", filename );
         DetachIterator( &Iter );
         return;
      }
   }
   DetachIterator( &Iter );

   area = new_area();
   area->filename = strdup( filename );
   AttachToList( area, darea_list );
   text_to_mobile_j( dMob, "text", "Area %s created and added to global area list.", area->filename );

   //Recreate arealist.json
   json_t *arealist = json_array();
   AttachIterator( &Iter, darea_list );
   while( ( area = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      json_array_append_new( arealist, areaheader_to_json( area ) );
   }
   DetachIterator( &Iter );
   json_dump_file( arealist, "../areas/arealist.json", JSON_INDENT(3) );
   json_decref( arealist );

   return;
}

void cmd_instazone( D_MOBILE *dMob, char *arg )
{
   ITERATOR Iter;
   D_RESET *pReset;

   AttachIterator( &Iter, dMob->room->area->resets );
   while( ( pReset = (D_RESET *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( pReset, dMob->room->area->resets );
      free_reset( pReset );
   }
   DetachIterator( &Iter );

   D_AREA *area = dMob->room->area;
   D_ROOM *room;
   D_MOBILE *pMob;
   D_OBJECT *pObj;
   for( int i = area->r_low; i <= area->r_hi; i++ )
   {
      room = get_room_by_vnum( i );
      if( room == NULL )
         continue;
      AttachIterator( &Iter, room->mobiles );
      while( ( pMob = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
      {
         if( IS_PC( pMob ) )
            continue;
         pReset = new_reset();
         pReset->type = RESET_MOB;
         pReset->location = pMob->room->vnum;
         pReset->data = mobile_to_reset( pMob );
         AppendToList( pReset, area->resets );
      }
      DetachIterator( &Iter );

      AttachIterator( &Iter, room->objects );
      while( ( pObj = (D_OBJECT *)NextInList( &Iter ) ) != NULL )
      {
         pReset = new_reset();
         pReset->type = RESET_OBJ;
         pReset->location = pObj->in_room->vnum;
         pReset->data = object_to_reset( pObj );
         AppendToList( pReset, area->resets );
      }
      DetachIterator( &Iter );
   }

   return;
}

void cmd_ocreate( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Use ocreate <vnum> to create an object, where <vnum> is a valid vnum within an area." );
      return;
   }
   if( dMob->hold_right != NULL && dMob->hold_left != NULL )
   {
      text_to_mobile_j( dMob, "error", "Your hands are full!" );
      return;
   }
   int vnum = atoi( arg );
   if( vnum == 0 )
   {
      text_to_mobile_j( dMob, "error", "Invalid vnum %s.", arg );
      return;
   }

   D_OBJECT *obj;

   if( ( obj = get_object_by_vnum( vnum ) ) != NULL )
   {
      text_to_mobile_j( dMob, "error", "An object already exists with vnum %i.", vnum );
      return;
   }

   ITERATOR Iter;
   AttachIterator( &Iter, darea_list );
   D_AREA *pArea;
   while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      if( vnum >= pArea->o_low && vnum <= pArea->o_hi )
         break;
   }
   DetachIterator( &Iter );

   if( pArea == NULL )
   {
      text_to_mobile_j( dMob, "error", "That vnum is not in the range of any known areas. Create an area with that vnum range or choose a different vnum." );
      return;
   }

   obj = new_object();
   obj->vnum = vnum;
   obj->name = strdup( "prototype object new" );
   obj->sdesc = strdup( "a newly created object" );
   obj->ldesc = strdup( "A newly created object sits here" );
   AppendToList( obj, object_protos );
   AppendToList( obj, dobject_list );

   object_to_mobile( obj, dMob );
   text_to_mobile_j( dMob, "text", "You sudo touch object.%i and create an object out of thin air.", vnum );

   return;
}

void cmd_mcreate( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Use mcreate <vnum> to create a mobile, where <vnum> is a valid vnum within an area." );
      return;
   }
   
   int vnum = atoi( arg );
   if( vnum == 0 )
   {
      text_to_mobile_j( dMob, "error", "Invalid vnum %s.", arg );
      return;
   }

   D_MOBILE *mob;

   if( ( mob = get_mobile_by_vnum( vnum ) ) != NULL )
   {
      text_to_mobile_j( dMob, "error", "A mobile already exists with vnum %i.", vnum );
      return;
   }

   ITERATOR Iter;
   AttachIterator( &Iter, darea_list );
   D_AREA *pArea;
   while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      if( vnum >= pArea->m_low && vnum <= pArea->m_hi )
         break;
   }
   DetachIterator( &Iter );

   if( pArea == NULL )
   {
      text_to_mobile_j( dMob, "error", "That vnum is not in the range of any known areas. Create an area with that vnum range or choose a different vnum." );
      return;
   }

   mob = new_mobile();
   mob->vnum = vnum;
   mob->name = strdup( "prototype mobile new" );
   mob->sdesc = strdup( "A newly created prototype NPC" );
   mob->ldesc = strdup( "A newly created prototype NPC is here" );
   AppendToList( mob, mobile_protos );
   AppendToList( mob, dmobile_list );

   mob_to_room( mob, dMob->room );
   text_to_mobile_j( dMob, "text", "You sudo touch mobile.%i and create an NPC out of thin air.", vnum );

   return;
}

void cmd_mset( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "What mobile do you want to edit?" );
      return;
   }

   D_MOBILE *pMob;
   char mname[MAX_STRING_LENGTH];
   char action[MAX_STRING_LENGTH];

   arg = one_arg( arg, mname );
   arg = one_arg( arg, action );

   if( mname[0] == '\0' || action[0] == '\0' || arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Usage: mset <mob> <action> <argument[s]>. See help mset for more info." );
      return;
   }

   if( is_number( mname ) )
   {
      pMob = get_mobile_by_vnum( atoi( mname ) );
   }
   else
   {
      pMob = get_mobile_list( mname, dMob->room->mobiles );
   }

   if( pMob == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find that mobile." );
      return;
   }

   if( !strcasecmp( action, "name" ) )
   {
      if( pMob->name ) free( pMob->name );
      pMob->name = strdup( arg );
   }
   else if( !strcasecmp( action, "short" ) || !strcasecmp( action, "sdesc" ) )
   {
      if( pMob->sdesc ) free( pMob->sdesc );
      pMob->sdesc = strdup( arg );
   }
   else if( !strcasecmp( action, "long" ) || !strcasecmp( action, "ldesc" ) )
   {
      if( pMob->ldesc ) free( pMob->ldesc );
      pMob->ldesc = strdup( arg );
   }
   else if( !strcasecmp( action, "position" ) )
   {
      enum position_t i;
      for( i = 0; i < MAX_POS; i++ )
      {
         if( is_prefix( arg, positions[i] ) )
            break;
      }
      if( i == MAX_POS )
      {
         text_to_mobile_j( dMob, "error", "Invalid mobile position %s.", arg );
         return;
      }
      else
      {
         pMob->position = i;
      }

   }
   else if( !strcasecmp( action, "cur_hp" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->cur_hp = atoi( arg );
   }
   else if( !strcasecmp( action, "max_hp" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->max_hp = atoi( arg );
   }
   else if( !strcasecmp( action, "brains" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->brains = atoi( arg );
   }
   else if( !strcasecmp( action, "brawn" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->brawn = atoi( arg );
   }
   else if( !strcasecmp( action, "senses" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->senses = atoi( arg );
   }
   else if( !strcasecmp( action, "stamina" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->stamina = atoi( arg );
   }
   else if( !strcasecmp( action, "coordination" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->coordination = atoi( arg );
   }
   else if( !strcasecmp( action, "cool" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->cool = atoi( arg );
   }
   else if( !strcasecmp( action, "luck" ) )
   {
      int i = atoi( arg );
      if( i < 1 || i > 10 )
      {
         text_to_mobile_j( dMob, "error", "Base statistics can only be between 1 - 10." );
         return;
      }
      pMob->luck = atoi( arg );
   }
   else if( !strcasecmp( action, "race" ) )
   {
      if( pMob->race ) free( pMob->race );
      pMob->race = strdup( arg );
   }
   else if( !strcasecmp( action, "eyecolor" ) )
   {
      if( pMob->eyecolor ) free( pMob->eyecolor );
      pMob->eyecolor = strdup( arg );
   }
   else if( !strcasecmp( action, "eyeshape" ) )
   {
      if( pMob->eyeshape ) free( pMob->eyeshape );
      pMob->eyeshape = strdup( arg );
   }
   else if( !strcasecmp( action, "haircolor" ) )
   {
      if( pMob->haircolor ) free( pMob->haircolor );
      pMob->haircolor = strdup( arg );
   }
   else if( !strcasecmp( action, "skincolor" ) )
   {
      if( pMob->skincolor ) free( pMob->skincolor );
      pMob->skincolor = strdup( arg );
   }
   else if( !strcasecmp( action, "build" ) )
   {
      if( pMob->build ) free( pMob->build );
      pMob->build = strdup( arg );
   }
   else if( !strcasecmp( action, "height" ) )
   {
      if( pMob->height ) free( pMob->height );
      pMob->height = strdup( arg );
   }
   else if( !strcasecmp( action, "age" ) )
   {
      int i = atoi( arg );
      if( i < 16 || i > 120 )
      {
         text_to_mobile_j( dMob, "error", "Age can only be between 16 - 120." );
         return;
      }
      pMob->age = i;
   }
   else if( !strcasecmp( action, "sex" ) || !strcasecmp( action, "gender" ) )
   {
      if( !strcasecmp( arg, "male" ) )
         pMob->gender = MALE;
      else if( !strcasecmp( arg, "female" ) )
         pMob->gender = FEMALE;
      else if( !strcasecmp( arg, "nonbinary" ) || !strcasecmp( arg, "neutral" ) )
         pMob->gender = NONBINARY;
      else
      {
         text_to_mobile_j( dMob, "error", "Invalid gender. Options are male, female, or nonbinary." );
         return;
      }
      text_to_mobile_j( dMob, "text", "Gender set to %s.", arg );
   }
   else if( !strcasecmp( action, "btc" ) || !strcasecmp( action, "bitcoin" ) || !strcasecmp( action, "money" ) )
   {
      float f = strtof( arg, NULL );
      if( f < 0 )
      {
         text_to_mobile_j( dMob, "error", "This must be a positive number." );
         return;
      }
      pMob->btc = f;
   }
   else if( !strcasecmp( action, "citizenship" ) )
   {
      if( pMob->citizenship ) free( pMob->citizenship );
      pMob->citizenship = strdup( arg );
   }
   else if( !strcasecmp( action, "association" ) )
   {
      if( pMob->association ) free( pMob->association );
      pMob->association = strdup( arg );
   }
   else
   {
      text_to_mobile_j( dMob, "error", "See help mset for usage." );
      return;
   }

   text_to_mobile_j( dMob, "text", "Ok." );
   return;
}

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

   if( !strcasecmp( action, "short" )  || !strcasecmp( action, "sdesc" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's short description to what?" );
         return;
      }
      if( obj->sdesc ) free( obj->sdesc );
      obj->sdesc = strdup( arg );
   }
   else if( !strcasecmp( action, "long" ) || !strcasecmp( action, "ldesc" ) )
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
   else if( !strcasecmp( action, "capacity" ) )
   {
      int cap = atoi( arg );
      if( cap < 0 || cap > 10000000 )
      {
         text_to_mobile_j( dMob, "error", "Capacity must be between 0 and 10,000,000 cubic centimeters (0-10,000 cubic liters, or 0-350 cubic feet)." );
         return;
      }
      obj->capacity_cm3 = cap;
      text_to_mobile_j( dMob, "text", "Item capacity set to %i cubic centimeters.", cap );
   }
   else if( !strcasecmp( action, "volume" ) )
   {
      int vol = atoi( arg );
      if( vol < 0 || vol > 10000000 )
      {
         text_to_mobile_j( dMob, "error", "Item volume (length x width x height in centimeters) must be between 0 and 10,000,000 cubic centimeters (0-10,000 cubic liters, or 0-350 cubic feet)." );
         return;
      }
      obj->volume_cm3 = vol;
      text_to_mobile_j( dMob, "text", "Item volume set to %i cubic centimeters.", vol );
   }
   else if( !strcasecmp( action, "weight" ) )
   {
      int weight = atoi( arg );
      if( weight < 0 || weight > 100000 )
      {
         text_to_mobile_j( dMob, "error", "Item weight must be between 0 and 100,000 kilograms (0-100 metric tons, 0-22,000 pounds)." );
         return;
      }
      obj->weight_g = weight * 1000;
      text_to_mobile_j( dMob, "text", "Item weight set to %i kilograms.", weight );
   }
   else if( !strcasecmp( action, "get" ) )
   {
      if( !strcasecmp( arg, "true" ) )
      {
         text_to_mobile_j( dMob, "text", "Item now able to be taken." );
         obj->can_get = TRUE;
      }
      else if( !strcasecmp( arg, "false" ) )
      {
         text_to_mobile_j( dMob, "text", "Item now not able to be taken." );
         obj->can_get = FALSE;
      }
      else
      {
         text_to_mobile_j( dMob, "error", "Syntax: oset <object> get [true/false] -- to allow or disallow players from getting the object." );
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

   if( !strcmp( pArea->filename, "arealist.json" ) )
   {
      log_string( "%s tried to save over the arealist.json file..... Naughty!!!", pMob->name );
      json_decref( area );
      return;
   }

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
   json_decref( area );

   //Recreate arealist.json
   json_t *arealist = json_array();
   AttachIterator( &Iter, darea_list );
   while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      json_array_append_new( arealist, areaheader_to_json( pArea ) );
   }
   DetachIterator( &Iter );
   json_dump_file( arealist, "../areas/arealist.json", JSON_INDENT(3) );
   json_decref( arealist );

   return;
}

void cmd_astat( D_MOBILE *dMob, char *arg )
{
   D_AREA *area = NULL;

   if( arg[0] == '\0' )
   {
      area = dMob->room->area;
   }
   else
   {
      ITERATOR Iter;
      D_AREA *pArea;

      AttachIterator( &Iter, darea_list );
      while( (pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
      {
         if( !strcasecmp( pArea->filename, arg ) )
         {
            area = pArea;
         }
      }
      DetachIterator( &Iter );
   }
   
   if( area == NULL )
   {
      text_to_mobile_j( dMob, "error", "Can't find area %s.", arg );
      return;
   }

   text_to_mobile_j( dMob, "text", "Name: %s\nFilename: %s\nAuthor: %s\nReset Interval: %i minutes\nRoom Range:    %i - %i\nObject Range:  %i - %i\nMobile Range:  %i - %i",
         area->name, area->filename, area->author, area->reset_interval, area->r_low, area->r_hi, area->o_low, area->o_hi, area->m_low, area->m_hi );
}

void cmd_aset( D_MOBILE *dMob, char *arg )
{
   D_AREA *area = NULL;

   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Usage: aset <filename> <action> <arg>. See help aset for more info." );
      return;
   }

   char filename[MAX_STRING_LENGTH];
   char action[MAX_STRING_LENGTH];
   
   arg = one_arg( arg, filename );
   arg = one_arg( arg, action );

   if( filename[0] == '\0' || action[0] == '\0' || arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Usage: aset <filename> <action> <arg>. See help aset for more info." );
      return;
   }

   ITERATOR Iter;
   AttachIterator( &Iter, darea_list );
   while( ( area = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      if( !strcasecmp( filename, area->filename ) )
         break;
   }
   DetachIterator( &Iter );

   if( area == NULL )
   {
      text_to_mobile_j( dMob, "error", "Can not find area %s.", filename );
      return;
   }

   if( !strcasecmp( action, "name" ) )
   {
      if( area->name ) free( area->name );
      area->name = strdup( arg );
   }
   else if( !strcasecmp( action, "author" ) )
   {
      if( area->author ) free( area->author );
      area->author = strdup( arg );
   }
   else if( !strcasecmp( action, "reset_interval" ) )
   {
      int i = atoi( arg );
      if( i < 5 || i > 60 )
      {
         text_to_mobile_j( dMob, "error", "Reset interval must be betwen 5 and 60 minutes." );
         return;
      }
      area->reset_interval = i;
   }
   else if( !strcasecmp( action, "o_low" ) )
   {
      int i = atoi( arg );
      if( i < 1 )
      {
         text_to_mobile_j( dMob, "error", "Area vnums must be greater than zero." );
         return;
      }
      D_AREA *pArea;
      AttachIterator( &Iter, darea_list );
      while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
      {
         if( i <= pArea->o_hi && i >= pArea->o_low )
            break;
      }
      DetachIterator( &Iter );
      if( pArea && pArea != area )
      {
         text_to_mobile_j( dMob, "error", "Requested low vnum overlaps with area %s ( %i - %i ).", pArea->filename, pArea->o_low, pArea->o_hi );
         return;
      }
      area->o_low = i;
   }
   else if( !strcasecmp( action, "m_low" ) )
   {
      int i = atoi( arg );
      if( i < 1 )
      {
         text_to_mobile_j( dMob, "error", "Area vnums must be greater than zero." );
         return;
      }
      D_AREA *pArea;
      AttachIterator( &Iter, darea_list );
      while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
      {
         if( i <= pArea->m_hi && i >= pArea->m_low )
            break;
      }
      DetachIterator( &Iter );
      if( pArea && pArea != area )
      {
         text_to_mobile_j( dMob, "error", "Requested low vnum overlaps with area %s ( %i - %i ).", pArea->filename, pArea->m_low, pArea->m_hi );
         return;
      }
      area->m_low = i;
   }
   else if( !strcasecmp( action, "r_low" ) )
   {
      int i = atoi( arg );
      if( i < 1 )
      {
         text_to_mobile_j( dMob, "error", "Area vnums must be greater than zero." );
         return;
      }
      D_AREA *pArea;
      AttachIterator( &Iter, darea_list );
      while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
      {
         if( i <= pArea->r_hi && i >= pArea->r_low )
            break;
      }
      DetachIterator( &Iter );
      if( pArea && pArea != area )
      {
         text_to_mobile_j( dMob, "error", "Requested low vnum overlaps with area %s ( %i - %i ).", pArea->filename, pArea->r_low, pArea->r_hi );
         return;
      }
      area->r_low = i;
   }
   else if( !strcasecmp( action, "r_hi" ) )
   {
      int i = atoi( arg );
      if( i < 1 )
      {
         text_to_mobile_j( dMob, "error", "Area vnums must be greater than zero." );
         return;
      }
      D_AREA *pArea;
      AttachIterator( &Iter, darea_list );
      while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
      {
         if( i <= pArea->r_hi && i >= pArea->r_low )
            break;
      }
      DetachIterator( &Iter );
      if( pArea && pArea != area )
      {
         text_to_mobile_j( dMob, "error", "Requested low vnum overlaps with area %s ( %i - %i ).", pArea->filename, pArea->r_low, pArea->r_hi );
         return;
      }
      area->r_hi = i;
   }
   else if( !strcasecmp( action, "m_hi" ) )
   {
      int i = atoi( arg );
      if( i < 1 )
      {
         text_to_mobile_j( dMob, "error", "Area vnums must be greater than zero." );
         return;
      }
      D_AREA *pArea;
      AttachIterator( &Iter, darea_list );
      while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
      {
         if( i <= pArea->m_hi && i >= pArea->m_low )
            break;
      }
      DetachIterator( &Iter );
      if( pArea && pArea != area )
      {
         text_to_mobile_j( dMob, "error", "Requested low vnum overlaps with area %s ( %i - %i ).", pArea->filename, pArea->m_low, pArea->m_hi );
         return;
      }
      area->m_hi = i;
   }
   else if( !strcasecmp( action, "o_hi" ) )
   {
      int i = atoi( arg );
      if( i < 1 )
      {
         text_to_mobile_j( dMob, "error", "Area vnums must be greater than zero." );
         return;
      }
      D_AREA *pArea;
      AttachIterator( &Iter, darea_list );
      while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
      {
         if( i <= pArea->o_hi && i >= pArea->o_low )
            break;
      }
      DetachIterator( &Iter );
      if( pArea && pArea != area )
      {
         text_to_mobile_j( dMob, "error", "Requested low vnum overlaps with area %s ( %i - %i ).", pArea->filename, pArea->o_low, pArea->o_hi );
         return;
      }
      area->o_hi = i;
   }
   else
   {
      text_to_mobile_j( dMob, "error", "See help aset for information on how to use this command." );
      return;
   }

   text_to_mobile_j( dMob, "text", "Ok." );
   return;
}

void cmd_mspawn( D_MOBILE *dMob, char *arg )
{
   D_MOBILE *mob;
   unsigned long vnum = strtoul( arg, NULL, 10 );

   if( ( mob = spawn_mobile( vnum ) ) == NULL )
   {
      text_to_mobile_j( dMob, "error",  "Can not find mobile with vnum '%u'", vnum );
      return;
   }

   AppendToList( mob, dMob->room->mobiles );
   mob->room = dMob->room;
   text_to_mobile_j( dMob, "text", "The entire world vibrates for a split second and %s appears before you.", mob->sdesc );
   return;
}

void cmd_mlist( D_MOBILE *dMob, char *arg )
{
   ITERATOR Iter;
   json_t *json = json_object();
   json_t *list = json_array();
   D_MOBILE *m;

   AttachIterator( &Iter, mobile_protos );
   while( ( m = NextInList( &Iter ) ) != NULL )
   {
      json_t *jm = json_object();
      json_object_set_new( jm, "vnum", json_integer( m->vnum ) );
      json_object_set_new( jm, "name", json_string( m->name ) );
      json_array_append_new( list, jm );
   }
   DetachIterator( &Iter );

   json_object_set_new( json, "type", json_string( "mlist" ) );
   json_object_set_new( json, "data", list );

   char *dump = json_dumps( json, 0 );
   send_json_m( dMob, "%s", dump );
   free( dump );
   json_decref( json );
   return;
}

void cmd_ostat( D_MOBILE *dMob, char *arg )
{
   D_OBJECT *obj;

   if( is_number( arg ) )
   {
      obj = get_object_by_vnum( atoi( arg ) );
   }
   else
   {
      if( dMob->hold_right && ( is_name( arg, dMob->hold_right->name ) || !strcmp( dMob->hold_right->guid, arg ) ) )
         obj = dMob->hold_right;
      else if( dMob->hold_left && ( is_name( arg, dMob->hold_left->name ) || !strcmp( dMob->hold_left->guid, arg ) ) )
         obj = dMob->hold_left;
      else
      {
         obj = get_object_list( arg, dMob->room->objects );
      }
   }

   if( obj == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find that here." );
      return;
   }

   json_t *json = json_object();
   json_object_set_new( json, "type", json_string( "ostat" ) );
   json_object_set_new( json, "data", object_to_json_cli( obj ) );
   char *dump = json_dumps( json, 0 );
   send_json_m( dMob, "%s", dump );
   json_decref( json );
   free( dump );
   return;
}

void cmd_mstat( D_MOBILE *dMob, char *arg )
{
   D_MOBILE *pMob;

   if( is_number( arg ) )
   {
      pMob = get_mobile_by_vnum( atoi( arg ) );
   }
   else
   {
      pMob = get_mobile_list( arg, dMob->room->mobiles );
   }

   if( pMob == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find that.\r\n" );
      return;
   }

   json_t *json = json_object();
   json_object_set_new( json, "type", json_string( "mstat" ) );
   json_object_set_new( json, "data", mobile_to_json( pMob, FALSE ) );
   char *dump = json_dumps( json, 0 );
   send_json_m( dMob, "%s", dump );
   json_decref( json );
   free( dump );

   return;
}
/*
 * olist
 * usage: olist [area name]
 *    lists objects assigned to a specific area.
 *    If area name is not supplied defaults to area player is in.
 */
void cmd_olist( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
      snprintf( arg, MAX_BUFFER, "%s", dMob->room->area->name );

   //For now we will just list all object prototypes in the game.
   //Support for the above will come later.

   ITERATOR Iter;
   D_OBJECT *pObj;
   json_t *olist = json_array();
   json_t *json = json_object();

   AttachIterator( &Iter, object_protos );
   while( ( pObj = NextInList( &Iter ) ) != NULL )
      json_array_append_new( olist, object_to_json( pObj ) );
   DetachIterator( &Iter );

   json_object_set_new( json, "type", json_string( "olist" ) );
   json_object_set_new( json, "data", olist );

   char *dump = json_dumps( json, 0 );
   send_json_m( dMob, "%s", dump );
   json_decref( json );
   free( dump );

   return;
}

void cmd_mpedit( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Edit which mob's scripts?" );
      return;
   }

   char type[MAX_STRING_LENGTH];
   char mname[MAX_STRING_LENGTH];
   D_MOBILE *pMob;

   arg = one_arg( arg, mname );
   arg = one_arg( arg, type );

   if( is_number( mname ) )
   {
      pMob = get_mobile_by_vnum( atoi( mname ) );
   }
   else
   {
      pMob = get_mobile_list( mname, dMob->room->mobiles );
      pMob = get_mobile_by_vnum( pMob->vnum );
   }

   if( pMob == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find that mobile." );
      return;
   }

   return;
}
