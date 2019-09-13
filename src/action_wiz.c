/**
 * This file handles all the admin commands
 */
#include "mud.h"

void cmd_restore( D_MOBILE *dMob, char *arg )
{
   D_MOBILE *target = NULL;

   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Restore who?" );
      return;
   }

   if( !strcasecmp( arg, "self" ) )
      target = dMob;
   else
      target = get_mobile_list( arg, dmobile_list );

   if( target == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find them." );
      return;
   }

   target->cur_hp = target->max_hp;
   for( enum bodyparts_tb i = BODY_HEAD; i < MAX_BODY; i++ )
   {
      target->body[i]->cur_hp = target->body[i]->max_hp;
      target->body[i]->wound_trauma = TRAUMA_NONE;
      target->body[i]->blunt_trauma = TRAUMA_NONE;
      target->body[i]->burn_trauma = TRAUMA_NONE;
   }
   text_to_mobile_j( dMob, "text", "Done." );
   if( dMob != target )
      text_to_mobile_j( target, "text", "Your health has been fully restored." );

   return;
}

void cmd_resetarea( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      reset_area( dMob->room->area );
      return;
   }

   ITERATOR Iter;
   D_AREA *pArea;

   AttachIterator( &Iter, darea_list );
   while( (pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      if( !strcasecmp( pArea->filename, arg ) )
      {
         reset_area( pArea );
         DetachIterator( &Iter );
         return;
      }
   }
   DetachIterator( &Iter );
   text_to_mobile_j( dMob, "error", "No area with filename '%s' found.", arg );
}

void cmd_copyover(D_MOBILE *dMob, char *arg)
{ 
   text_to_mobile_j( dMob, "error", "This command has been disabled." );
   return;
  FILE *fp;
  ITERATOR Iter;
  D_SOCKET *dsock;
  char buf[MAX_BUFFER];
  
  if ((fp = fopen(COPYOVER_FILE, "w")) == NULL)
  {
    text_to_mobile_j(dMob, "error", "Copyover file not writeable, aborted.\n\r");
    return;
  }

  strncpy(buf, "\n\r <*>            The world starts spinning             <*>\n\r", MAX_BUFFER);

  /* For each playing descriptor, save its state */
  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    compressEnd(dsock, dsock->compressing, FALSE);

    if (dsock->state != STATE_PLAYING)
    {
      text_to_socket(dsock, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r");
      close_socket(dsock, FALSE);
    }
    else
    {
      fprintf(fp, "%d %s %s\n",
        dsock->control, dsock->player->name, dsock->hostname);

      /* save the player */
      save_player(dsock->player);

      text_to_socket(dsock, buf);
    }
  }
  DetachIterator(&Iter);

  fprintf (fp, "-1\n");
  fclose (fp);

  /* close any pending sockets */
  recycle_sockets();
  
  /*
   * feel free to add any additional arguments between the 2nd and 3rd,
   * that is "SocketMud" and buf, but leave the last three in that order,
   * to ensure that the main() function can parse the input correctly.
   */
  snprintf(buf, MAX_BUFFER, "%d", control);
  execl( EXE_FILE, "cbm", buf, "copyover", (char *) 0 );

  /* Failed - sucessful exec will not return */
  text_to_mobile(dMob, "Copyover FAILED!\n\r");
}

void cmd_force( D_MOBILE *dMob, char *arg )
{
   D_MOBILE *who;

   char name[MAX_STRING_LENGTH];

   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Force _who_ to do _what_?" );
      return;
   }
   arg = one_arg( arg, name );
   
   if( ( who = get_mobile_list( name, dMob->room->mobiles ) ) == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find anyone who looks like that." );
      return;
   }

   handle_cmd_input( who, arg );
   return;
}

void cmd_goto( D_MOBILE *dMob, char *arg )
{
   D_ROOM *room;
   int vnum = atoi( arg );
   if( vnum < 1 )
   {
      text_to_mobile_j( dMob, "error", "Room #%i does not exist.\r\n", vnum );
      return;
   }

   if( ( room = get_room_by_vnum( vnum ) ) == NULL )
   {
      ITERATOR Iter;
      D_AREA *area;
      AttachIterator( &Iter, darea_list );
      while( ( area = (D_AREA *)NextInList( &Iter ) ) != NULL )
      {
         if( vnum >= area->r_low && vnum <= area->r_hi )
            break;
      }
      DetachIterator( &Iter );
      if( area == NULL )
      {
         text_to_mobile_j( dMob, "error", "That vnum is not in the range of any known areas. Create an area with that vnum range or choose a different vnum." );
         return;
      }
      text_to_mobile_j( dMob, "text", "You type sudo touch room.%i and the room digitizes into existance.\r\n", vnum );
      room = new_room();
      room->vnum = vnum;
      room->description = strdup( "A newly created room fresh from /dev/null" );
      room->name        = strdup( "A newly created room" );
      room->area = area;
      AppendToList( room, droom_list );
   }

   mob_to_room( dMob, room );
   cmd_look( dMob, "" );

   return;
}

void cmd_linkdead(D_MOBILE *dMob, char *arg)
{
  D_MOBILE *xMob;
  ITERATOR Iter;
  char buf[MAX_BUFFER];
  bool found = FALSE;

  AttachIterator(&Iter, dmobile_list);
  while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
  {
    if (!xMob->socket)
    {
      snprintf(buf, MAX_BUFFER, "%s is linkdead.\n\r", xMob->name);
      text_to_mobile(dMob, buf);
      found = TRUE;
    }
  }
  DetachIterator(&Iter);

  if (!found)
    text_to_mobile(dMob, "Noone is currently linkdead.\n\r");
}
void cmd_shutdown(D_MOBILE *dMob, char *arg)
{
   system( "touch shutdown.txt" );
   shut_down = TRUE;
}

void cmd_reboot( D_MOBILE *dMob, char *arg )
{
   shut_down = TRUE;
}

void cmd_reboo( D_MOBILE *dMob, char *arg )
{
   text_to_mobile_j( dMob, "error", "Spell out 'reboot' to reboot the MUD." );
   return;
}

void cmd_shutdow( D_MOBILE *dMob, char *arg )
{
   text_to_mobile_j( dMob, "error", "Spell out 'shutdown' to shut the MUD down." );
   return;
}

void cmd_hedit( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "edit which help entry?" );
      return;
   }

   char action[MAX_STRING_LENGTH] = { 0 };
   char helpname[MAX_STRING_LENGTH] = { 0 };

   arg = one_arg( arg, action );
   arg = one_arg( arg, helpname );

   if( !strcasecmp( action, "save" ) )
   {
      ITERATOR Iter;
      HELP_DATA *pHelp;

      AttachIterator( &Iter, help_list );
      json_t *allhelps = json_array();
      while( ( pHelp = NextInList( &Iter ) ) != NULL )
      {
         json_array_append_new( allhelps, help_to_json( pHelp ) );
      }
      DetachIterator( &Iter );
      json_dump_file( allhelps, "../help/help_data.json", JSON_INDENT(3) );
      json_decref( allhelps );
   }
   else if( !strcasecmp( action, "reload" ) )
   {
      FreeList( help_list );
      load_helps();
   }

      text_to_mobile_j( dMob, "text", "Ok." );

   return;
}

void cmd_skillset( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "skillset <skill> <action> <argument> or skillset save to save the skill list to disk." );
      return;
   }

   if( !strcasecmp( arg, "save" ) )
   {
      save_skill_list();
   }
   else
   {
   }

   text_to_mobile_j( dMob, "text", "Ok." );

   return;
}

void cmd_peace( D_MOBILE *dMob, char *arg )
{
   ITERATOR Iter;
   D_MOBILE *pMob;

   AttachIterator( &Iter, dMob->room->mobiles );
   while( ( pMob = NextInList( &Iter ) ) != NULL )
   {
      if( pMob->fighting )
         pMob->fighting->fighting = NULL;
      pMob->fighting = NULL;
   }
   DetachIterator( &Iter );
   text_to_mobile_j( dMob, "text", "Ok." );
   return;
}
