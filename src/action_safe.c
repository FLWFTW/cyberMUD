/*
 * This file handles non-fighting player actions.
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"

void cmd_say(D_MOBILE *dMob, char *arg)
{
  if (arg[0] == '\0')
  {
    text_to_mobile_j(dMob, "error", "Say what?\n\r");
    return;
  }
  communicate(dMob, arg, COMM_LOCAL);
}

void cmd_chat( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      send_json_m( dMob, "{\"type\":\"text\", \"data\":{\"text\":\"Chat what?\"}}" );
      return;
   }
   communicate( dMob, arg, COMM_CHAT );
   return;
}

void cmd_enter( D_MOBILE *dMob, char *arg )
{
   ITERATOR Iter;
   D_EXIT *exit;

   if( !strcasecmp( arg, "north" ) || !strcasecmp( arg, "south" ) ) //we do this so north doesn't collide with northeast, etc.
   {
      AttachIterator( &Iter, dMob->room->exits );
      while( ( exit = (D_EXIT *)NextInList(&Iter) ) != NULL )
         if( !strcasecmp( arg, exit->name ) )
            break;
      DetachIterator( &Iter );
   }
   else
   {
      AttachIterator( &Iter, dMob->room->exits );
      while( ( exit = (D_EXIT *)NextInList(&Iter) ) != NULL )
         if( is_prefix( arg, exit->name ) )
            break;
      DetachIterator( &Iter );
   }

   if( exit == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't go that way.\r\n" );
      return;
   }
   if( exit->exit == EXIT_CLOSED )
   {
      text_to_mobile_j( dMob, "error", "It's closed." );
      return;
   }


   echo_around( dMob, "%s leaves through the %s\r\n", MOBNAME(dMob), exit->name );//Need to check for non n/s/e/w exits, wouldn't make sense to see "Blah leaves door"
   mob_to_room( dMob, exit->to_room );
   echo_around( dMob, "%s enters from the %s\r\n", MOBNAME(dMob), exit->farside_name );
   cmd_look( dMob, "" );
}

void cmd_open( D_MOBILE *dMob, char *arg )
{
   D_EXIT *exit;
   ITERATOR Iter;

   AttachIterator( &Iter, dMob->room->exits );
   while( ( exit = (D_EXIT *)NextInList(&Iter) ) != NULL )
      if( is_prefix( arg, exit->name ) )
         break;
   DetachIterator( &Iter );

   if( exit == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find that." );
      return;
   }

   if( exit->exit == EXIT_JAMMED )
   {
      text_to_mobile_j( dMob, "error", "The door is jammed shut." );
      return;
   }

   if( exit->exit == EXIT_FREE )
   {
      text_to_mobile_j( dMob, "error", "That's not a door." );
      return;
   }

   if( exit->lock == LOCK_LOCKED || exit->lock == LOCK_JAMMED )
   {
      text_to_mobile_j( dMob, "error", "It appears to be locked." );
      return;
   }

   if( exit->exit == EXIT_CLOSED )
   {
      text_to_mobile_j( dMob, "text", "You open the door to the %s.", exit->name );
      echo_around( dMob, "%s opens the door to the %s.", MOBNAME(dMob), exit->name );
      echo_room( exit->to_room, "The door to the %s opens.", exit->farside_exit->name );
      exit->exit = EXIT_OPEN;
      exit->farside_exit->exit = EXIT_OPEN;
      return;
   }

   text_to_mobile_j( dMob, "error", "It's already open." );
   return;
}

void cmd_close( D_MOBILE *dMob, char *arg )
{
   D_EXIT *exit;
   ITERATOR Iter;

   AttachIterator( &Iter, dMob->room->exits );
   while( ( exit = (D_EXIT *)NextInList(&Iter) ) != NULL )
      if( is_prefix( arg, exit->name ) )
         break;
   DetachIterator( &Iter );

   if( exit == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find that." );
      return;
   }

   if( exit->exit == EXIT_OPEN )
   {
      text_to_mobile_j( dMob, "text", "You close the door to the %s.", exit->name );
      echo_around( dMob, "%s closes the door to the %s.", MOBNAME(dMob), exit->name );
      echo_room( exit->to_room, "The door to the %s closes.", exit->farside_exit->name );
      exit->exit = EXIT_CLOSED;
      exit->farside_exit->exit = EXIT_CLOSED;
      return;
   }

   if( exit->exit == EXIT_FREE )
   {
      text_to_mobile_j( dMob, "error", "That's not a door." );
      return;
   }

   if( exit->exit == EXIT_BROKEN_OPEN )
   {
      text_to_mobile_j( dMob, "error", "The door is broken open and can't be closed." );
      return;
   }
   
   text_to_mobile_j( dMob, "error", "It's already closed." );
   return;
}


void cmd_west( D_MOBILE *dMob, char *arg )
{
   cmd_enter( dMob, "west" );
}

void cmd_east( D_MOBILE *dMob, char *arg )
{
   cmd_enter( dMob, "east" );
}

void cmd_south( D_MOBILE *dMob, char *arg )
{
   cmd_enter( dMob, "south" );
}

void cmd_north( D_MOBILE *dMob, char *arg )
{
   cmd_enter( dMob, "north" );
}

void cmd_southwest( D_MOBILE *dMob, char *arg )
{
   cmd_enter( dMob, "southwest" );
}

void cmd_southeast( D_MOBILE *dMob, char *arg )
{
   cmd_enter( dMob, "southeast" );
}

void cmd_northwest( D_MOBILE *dMob, char *arg )
{
   cmd_enter( dMob, "northwest" );
}

void cmd_northeast( D_MOBILE *dMob, char *arg )
{
   cmd_enter( dMob, "northeast" );
}

static void show_room_to_player( D_MOBILE *dMob )
{
   ITERATOR Iter;
   D_OBJECT *objects;
   D_MOBILE *mobiles;
   D_EXIT *exit;
   json_t *json = json_object();
   json_t *data = json_object();

   json_t *jexits = json_array();

   json_t *jobjects = json_array();

   json_t *jmobiles = json_array();


   json_object_set_new( json, "type", json_string( "room" ) );
   json_object_set_new( data, "name", json_string( dMob->room->name ) );
   json_object_set_new( data, "vnum", json_integer( dMob->room->vnum) );
   json_object_set_new( data, "description", json_string( dMob->room->description) );

   AttachIterator( &Iter, dMob->room->exits );
   while( ( exit = (D_EXIT *)NextInList( &Iter ) ) != NULL )
   {
      json_array_append_new( jexits, exit_to_json( exit ) );
   }
   DetachIterator( &Iter );
   json_object_set_new( data, "exits", jexits );
   AttachIterator( &Iter, dMob->room->mobiles );
   while( ( mobiles = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
   {
      if( mobiles == dMob )
         continue;
      json_array_append_new( jmobiles, mobile_to_json( mobiles, FALSE ) );
   }
   DetachIterator( &Iter );
   json_object_set_new( data, "mobiles", jmobiles );

   AttachIterator( &Iter, dMob->room->objects );
   while( ( objects = (D_OBJECT *)NextInList( &Iter ) ) != NULL )
   {
      json_array_append_new( jobjects, object_to_json_cli( objects ) );
   }
   DetachIterator( &Iter );
   json_object_set_new( data, "objects", jobjects );
   

   json_object_set_new( json, "data", data );
   char * dump = json_dumps( json, 0 );

   send_json_m( dMob, dump );
   free( dump );
   json_decref( json );

   return;
}

void cmd_examine( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Examine what?" );
      return;
   }

   D_OBJECT *obj = NULL;
   D_MOBILE *mob = NULL;

   if( ( ( obj = get_object_mob( dMob, arg ) ) == NULL ) && ( ( obj = get_object_list( arg, dMob->room->objects ) ) == NULL )
         && ( ( mob = get_mobile_list( arg, dMob->room->mobiles ) ) == NULL ) )
   {
      text_to_mobile_j( dMob, "error", "You don't see that here." );
      return;
   }

   if( obj )
   {
      cmd_look( dMob, obj->guid );
      return;
   }

   if( mob )
   {
      cmd_look( dMob, mob->guid );
      return;
   }
   bug( "Shouldn't have gotten down here in cmd_examine." );
   return;
}

void cmd_look( D_MOBILE *dMob, char *arg )
{
   D_MOBILE *m = NULL;
   D_OBJECT *obj = NULL;

   char buf[MAX_STRING_LENGTH];
   if( arg[0] == '\0' )
   {
      show_room_to_player( dMob );
      return;
   }

   char arg1[MAX_STRING_LENGTH];
   arg = one_arg( arg, arg1 );

   if( !strcasecmp( arg1, "in" ) || !strcasecmp( arg1, "inside" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Look in what?" );
         return;
      }

      if( ( obj = get_object_list( arg, dMob->room->objects ) ) == NULL 
       && ( obj = get_object_mob( dMob, arg ) ) == NULL )
      {
         text_to_mobile_j( dMob, "error", "You don't see that here." );
         return;
      }

      if( obj->capacity_cm3 < 1 )
      {
         text_to_mobile_j( dMob, "error", "%s %s is not a container.", AORAN( obj->sdesc ), obj->sdesc );
         return;
      }

      text_to_mobile_j( dMob, "text", "You look in %s %s.", AORAN( obj->sdesc ), obj->sdesc );
      show_mob_obj_list( dMob, obj->contents, 6 );

      return;
   }

   if( !strcasecmp( arg1, "self" ) )
      m = dMob;
   else
      m = get_mobile_list( arg1, dMob->room->mobiles );

   if( m )
   {
      snprintf( buf, MAX_STRING_LENGTH, "You look at %s.\r\n%s %s a %s %s. %s %s %s and %s. %s %s %s eyes.\r\n",
            MOBNAME(m), SUBJECTIVE(m), GENDERTERN( m, "is", "is", "are" ), GENDER(m), m->race, SUBJECTIVE(m), GENDERTERN( m, "is", "is", "are" ), m->height, m->build, SUBJECTIVE(m),
            GENDERTERN( m, "has", "has", "have" ), m->eyecolor );
      sentence_case(buf);
      text_to_mobile_j( dMob, "text", "%s", buf );
      text_to_mobile_j( dMob, "text", "%s %s wearing:", SUBJECTIVE(m), GENDERTERN(m, "is", "is", "are") );
      int count = 0;
      for( size_t i = 0; i < WEAR_NONE; i++ )
      {
         if( m->equipment[i]->worn[1] != NULL )
         {
            obj = m->equipment[i]->worn[1];
            if( obj->wear_pos == WEAR_SLUNG )
               text_to_mobile_j( dMob, "text", "    %s %s, %s", AORAN( obj->sdesc ), obj->sdesc, wear_pos[obj->wear_pos] );
            else
               text_to_mobile_j( dMob, "text", "    %s %s on %s %s", AORAN( obj->sdesc ), obj->sdesc, POSSESSIVE(m), wear_pos[obj->wear_pos] );
            count++;
         }
         else if( m->equipment[i]->worn[0] != NULL )
         {
            obj = m->equipment[i]->worn[0];
            if( obj->wear_pos == WEAR_SLUNG )
               text_to_mobile_j( dMob, "text", "    %s %s, %s", AORAN( obj->sdesc ), obj->sdesc, wear_pos[obj->wear_pos] );
            else
               text_to_mobile_j( dMob, "text", "    %s %s on %s %s", AORAN( obj->sdesc ), obj->sdesc, POSSESSIVE(m), wear_pos[obj->wear_pos] );
            count++;
         }
      }
      if( count == 0 )
      {
         text_to_mobile_j( dMob, "text", "Nothing." );
      }

      if( m->hold_left || m->hold_right )
      {
         text_to_mobile_j( dMob, "text", "%s is holding:", SUBJECTIVE(m) );
      }

      if( m->hold_left != NULL )
      {
         text_to_mobile_j( dMob, "text", "    %s %s in %s left hand.", 
               AORAN( m->hold_left->sdesc), m->hold_left->sdesc, POSSESSIVE(m) );
      }
      if( m->hold_right != NULL )
      {
         text_to_mobile_j( dMob, "text", "    %s %s in %s right hand.",
               AORAN( m->hold_right->sdesc), m->hold_right->sdesc, POSSESSIVE(m) );
      }
 
      return;
   }

   if( ( obj = get_object_list( arg1, dMob->room->objects ) ) != NULL 
         || ( obj = get_object_mob( dMob, arg1 ) ) != NULL )
   {
      text_to_mobile_j( dMob, "text", "You closely examine %s %s\r\nYou can see that it has a volume of %d and a weight of %d.", 
            AORAN(obj->sdesc), obj->sdesc, obj->volume_cm3, obj->weight_g );
      if( obj->capacity_cm3 > 0 )
      {
         text_to_mobile_j( dMob, "text", "It appears to be able to hold around %d cubic centimeters. It's about %d percent full.",
               (obj->capacity_cm3/100)*100, ((total_volume(obj)-obj->volume_cm3)*100)/obj->capacity_cm3);
      }
      text_to_mobile_j( dMob, "text", "It is %s and is worn on the %s", item_type[obj->type], wear_pos[obj->wear_pos] );
      return;
   }

   text_to_mobile_j( dMob, "error", "You don't see that here." );

   return;
}

void cmd_qui( D_MOBILE *dMob, char *arg )
{
   text_to_mobile_j( dMob, "error", "Spell out 'quit' to quit.\r\n" );
   return;
}

void cmd_quit(D_MOBILE *dMob, char *arg)
{
   char buf[MAX_BUFFER];

   /* log the attempt */
   snprintf(buf, MAX_BUFFER, "%s has left the game.", dMob->name);
   log_string(buf);

   save_player(dMob);

   dMob->socket->player = NULL;
   dMob->socket->state = STATE_MAIN_MENU;
   extern const char *mainMenu;
   text_to_buffer( dMob->socket, mainMenu );
   text_to_mobile_j( dMob, "ui_command", "hide_all_windows" );
   free_mobile(dMob);
}

void cmd_shutdown(D_MOBILE *dMob, char *arg)
{
  shut_down = TRUE;
}

void cmd_commands(D_MOBILE *dMob, char *arg)
{
  BUFFER *buf = buffer_new(MAX_BUFFER);
  int i, col = 0;

  bprintf(buf, "    - - - - ----==== The full command list ====---- - - - -\n\n\r");
  for (i = 0; tabCmd[i].cmd_name[0] != '\0'; i++)
  {
    if (dMob->level < tabCmd[i].level) continue;

    bprintf(buf, " %-16.16s", tabCmd[i].cmd_name);
    if (!(++col % 4)) bprintf(buf, "\n\r");
  }
  if (col % 4) bprintf(buf, "\n\r");
  text_to_mobile(dMob, buf->data);
  buffer_free(buf);
}

void cmd_who(D_MOBILE *dMob, char *arg)
{
  D_MOBILE *xMob;
  D_SOCKET *dsock;
  ITERATOR Iter;
  BUFFER *buf = buffer_new(MAX_BUFFER);

  bprintf(buf, " - - - - ----==== Who's Online ====---- - - - -\n\r");

  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    if (dsock->state != STATE_PLAYING) continue;
    if ((xMob = dsock->player) == NULL) continue;

    bprintf(buf, " %-12s   %s\n\r", xMob->name, dsock->hostname);
  }
  DetachIterator(&Iter);

  bprintf(buf, " - - - - ----======================---- - - - -\n\r");
  text_to_mobile(dMob, buf->data);
  buffer_free(buf);
}

void cmd_help(D_MOBILE *dMob, char *arg)
{
  if (arg[0] == '\0')
  {
    HELP_DATA *pHelp;
    ITERATOR Iter;
    BUFFER *buf = buffer_new(MAX_BUFFER);
    int col = 0;

    bprintf(buf, "      - - - - - ----====//// HELP FILES  \\\\\\\\====---- - - - - -\n\n\r");

    AttachIterator(&Iter, help_list);
    while ((pHelp = (HELP_DATA *) NextInList(&Iter)) != NULL)
    {
      bprintf(buf, " %-19.18s", pHelp->keyword);
      if (!(++col % 4)) bprintf(buf, "\n\r");
    }
    DetachIterator(&Iter);

    if (col % 4) bprintf(buf, "\n\r");
    bprintf(buf, "\n\r Syntax: help <topic>\n\r");
    text_to_mobile(dMob, buf->data);
    buffer_free(buf);

    return;
  }

  if (!check_help(dMob, arg))
    text_to_mobile_j(dMob, "error", "Sorry, no such helpfile.\n\r");
}

void cmd_compress(D_MOBILE *dMob, char *arg)
{
  /* no socket, no compression */
  if (!dMob->socket)
    return;

  /* enable compression */
  if (!dMob->socket->out_compress)
  {
    text_to_mobile(dMob, "Trying compression.\n\r");
    text_to_buffer(dMob->socket, (char *) compress_will2);
    text_to_buffer(dMob->socket, (char *) compress_will);
  }
  else /* disable compression */
  {
    if (!compressEnd(dMob->socket, dMob->socket->compressing, FALSE))
    {
      text_to_mobile_j(dMob, "error", "Failed.\n\r");
      return;
    }
    text_to_mobile(dMob, "Compression disabled.\n\r");
  }
}

void cmd_save(D_MOBILE *dMob, char *arg)
{
   //You might be wondering why there is no specific save code in here. The MUD auto-saves
   //every player every time they input a command, so it would be redundant to save it a second
   //time within the save command.
  text_to_mobile(dMob, "Saved.\n\r");
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

void cmd_score( D_MOBILE *dMob, char *arg )
{
   char buf[MAX_BUFFER];
   arg[0] = '\0';//just so we don't get a warning about 'arg' not being used...

   snprintf( buf, MAX_BUFFER,
         "&W.-------------------[&B^W IdentiCorp Identification System &W]--------------------.&0\r\n"
         "&W|              WARNING: FALSE OR FRAUDULENT USAGE IS A CRIME.               |&0\r\n"
         "&W| .-[ IDENTIFICATION ]---------.  .-[ STATS ]----------------------.        |&0\r\n"
         "&W| | Name:       %14s |  | Brain:                     %3u |        |&0\r\n"
         "&W| | Sex:             %9s |  | Brawn:                     %3u |        |&0\r\n"
         "&W| | Race:     %16s |  | Senses:                    %3u |        |&0\r\n"
         "&W| | Height:         %10s |  | Stamina:                   %3u |        |&0\r\n"
         "&W| | Build:          %10s |  | Coordination:              %3u |        |&0\r\n"
         "&W| | Eye Color:           %5s |  | Cool:                      %3u |        |&0\r\n"
         "&W| | Serial No.:       %8s |  | Luck:                      %3u |        |&0\r\n"
         "&W| '----------------------------'  '--------------------------------'        |&0\r\n"
         "&W| .-[ BIOGRAPHICS ]------------.  .-[ NETSTAT ]--------------------.        |&0\r\n"
         "&W| | Citizenship:  %12s |  | Signal:                   %3u%% |        |&0\r\n"
         "&W| | Association:  %12s |  | Cur. Bandwidth:     %4ukbit/s |        |&0\r\n"
         "&W| | Bankroll:     %8.8g btc |  | Max. Bandwidth:     %4ukbit/s |        |&0\r\n"
         "&W| '----------------------------'  '--------------------------------'        |&0\r\n"
         "&W| .-[ VITALS ]-----------------.                                            |&0\r\n"
         "&W| | Health:           %3i/ %3i |     IDENTIC ORPIDENT.   .ICORPI.           |&0\r\n"
         "&W| | Encumberance:         %3i%% |       DEN   TIC  \"ORPI DENT  ICOR          |&0\r\n"
         "&W| | Sight:                %3i%% |       PID   ENT    ICO RPI    DEN          |&0\r\n"
         "&W| | Head:                 %3i%% |       TIC   ORP    IDE NTI                 |&0\r\n"
         "&W| | Body:                 %3i%% |       COR   PID    ENT ICO                 |&0\r\n"
         "&W| | L. Arm:               %3i%% |       RPI   DEN    TIC ORP                 |&0\r\n"
         "&W| | R. Arm:               %3i%% |       IDE   NTI    COR PID    ENT          |&0\r\n"
         "&W| | L. Leg:               %3i%% |       ICO   RPI  .DENT ICOR  PIDE          |&0\r\n"
         "&W| | R. Leg:               %3i%% |     NTICORP IDENTICO\"   \"RPIDEN\"           |&0\r\n"
         "&W| '----------------------------'         identicorp (c) 2088                |&0\r\n"
         "&W'---------------------------------------------------------------------------'&0\r\n",
         dMob->name, dMob->brains, dMob->gender == FEMALE ? "Female" : dMob->gender == MALE ? "Male" : "Nonbinary",
         dMob->brawn, dMob->race, dMob->senses, dMob->height, dMob->stamina,
         dMob->build, dMob->coordination, dMob->eyecolor, dMob->cool, "94F3DD21", dMob->luck,
         dMob->citizenship, dMob->signal, dMob->association, dMob->cur_bandwidth, dMob->btc, dMob->max_bandwidth,
         dMob->cur_hp, dMob->max_hp, dMob->encumberance, dMob->body[BODY_EYES]->health, dMob->body[BODY_HEAD]->health,
         dMob->body[BODY_TORSO]->health, dMob->body[BODY_LARM]->health, dMob->body[BODY_RARM]->health,
         dMob->body[BODY_LLEG]->health, dMob->body[BODY_RLEG]->health );

   text_to_buffer( dMob->socket, buf );
   return;
}

void cmd_prompt( D_MOBILE *dMob, char *arg )
{
   if( !arg || arg[0] == '\0' )
   {
      text_to_buffer( dMob->socket, "Prompt is set to: %s\r\n", dMob->prompt );
      return;
   }

   free( dMob->prompt );
   dMob->prompt = strdup( arg );
   return;
}

void cmd_goto( D_MOBILE *dMob, char *arg )
{
   ITERATOR Iter;
   D_ROOM *room;
   int vnum = atoi( arg );
   if( vnum < 1 )
   {
      text_to_mobile_j( dMob, "error", "Room #%i does not exist.\r\n", vnum );
      return;
   }


   AttachIterator( &Iter, droom_list );
   while( ( room = (D_ROOM *) NextInList( &Iter ) ) != NULL )
   {
      if( room->vnum == vnum )
      {
         dMob->room = room;
         cmd_look( dMob, "" );
         DetachIterator( &Iter );
         return;
      }
   }
   DetachIterator( &Iter );

   text_to_mobile_j( dMob, "error", "Room #%i does not exist.\r\n", vnum );
   return;
}

void cmd_areas( D_MOBILE *dMob, char *arg )
{
   ITERATOR Iter;
   D_AREA *area;

   text_to_mobile( dMob, "Areas\r\n" );
   text_to_mobile( dMob, "Name                     Author         Filename\r\n" );
   AttachIterator( &Iter, darea_list );
   while( (area = (D_AREA *) NextInList(&Iter)) != NULL)
   {
      text_to_mobile( dMob, "%-24.24s %-14.14s %s\r\n",
            area->name, area->author, area->filename );
   }
   DetachIterator( &Iter );

   return;
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
    text_to_mobile(dMob, "Copyover file not writeable, aborted.\n\r");
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

void cmd_lock( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Lock what?" );
      return;
   }

   D_EXIT *exit = NULL;
   if( ( exit = get_exit_by_name( dMob->room, arg ) ) != NULL )
   {
      /***Check for key***/
      if( exit->exit == EXIT_FREE )
      {
         text_to_mobile_j( dMob, "error", "There isn't a door in that direction." );
         return;
      }
      if( exit->exit == EXIT_OPEN )
      {
         text_to_mobile_j( dMob, "error", "You have to close it first." );
         return;
      }
      if( exit->lock == LOCK_FREE )
      {
         text_to_mobile_j( dMob, "error", "That doesn't have a lock." );
         return;
      }
      if( exit->lock == LOCK_LOCKED )
      {
         text_to_mobile_j( dMob, "error", "It's already locked." );
         return;
      }
      if( exit->lock == LOCK_JAMMED )
      {
         text_to_mobile_j( dMob, "error", "The lock is jammed." );
         return;
      }
      if( exit->lock == LOCK_UNLOCKED )
      {
         text_to_mobile_j( dMob, "text", "You lock the door to the %s.", exit->name );
         exit->lock = LOCK_LOCKED;
         exit->farside_exit->lock = LOCK_LOCKED;
         return;
      }
      bug( "Invalid lock state %d", exit->lock );
      return;
   }

   D_OBJECT *obj = NULL;
   if( ( obj = get_object_list( arg, dMob->room->objects ) ) != NULL )
   {
      /***Check for key***/
      return;
   }

   text_to_mobile_j( dMob, "error", "You can't find that here." );
   return;
}

void cmd_unlock( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Unlock what?" );
      return;
   }

   D_EXIT *exit = NULL;
   if( ( exit = get_exit_by_name( dMob->room, arg ) ) != NULL )
   {
      /***Check for key***/
      if( exit->exit == EXIT_FREE )
      {
         text_to_mobile_j( dMob, "error", "There isn't a door in that direction." );
         return;
      }
      if( exit->exit == EXIT_OPEN )
      {
         text_to_mobile_j( dMob, "error", "You have to close it first." );
         return;
      }
      if( exit->lock == LOCK_FREE )
      {
         text_to_mobile_j( dMob, "error", "That doesn't have a lock." );
         return;
      }
      if( exit->lock == LOCK_UNLOCKED )
      {
         text_to_mobile_j( dMob, "error", "It's already unlocked." );
         return;
      }
      if( exit->lock == LOCK_JAMMED )
      {
         text_to_mobile_j( dMob, "error", "The lock is jammed." );
         return;
      }
      if( exit->lock == LOCK_LOCKED )
      {
         text_to_mobile_j( dMob, "text", "You unlock the door to the %s.", exit->name );
         exit->lock = LOCK_UNLOCKED;
         exit->farside_exit->lock = LOCK_UNLOCKED;
         return;
      }
      bug( "Invalid lock state %d", exit->lock );
      return;
   }

   D_OBJECT *obj = NULL;
   if( ( ( obj = get_object_mob( dMob, arg ) ) != NULL )
      || ( obj = get_object_list( arg, dMob->room->objects ) ) != NULL )
   {
      return;
   }

   text_to_mobile_j( dMob, "error", "You can't find that here." );
   return;
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


