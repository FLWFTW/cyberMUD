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

   if( dMob->position != POS_STANDING )
   {
      text_to_mobile_j( dMob, "error", "Stand up, first!" );
      return;
   }

   if( IS_FIGHTING( dMob ) )
   {
      text_to_mobile_j( dMob, "error", "You're in the middle of a fight!" );
      return;
   }

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
      text_to_mobile_j( dMob, "text", "You look in %s%s.", NEEDTHE( obj->sdesc ), obj->sdesc );
      show_mob_obj_list( dMob, obj->contents, 6 );
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

      text_to_mobile_j( dMob, "text", "You look in %s%s.", NEEDTHE( obj->sdesc ), obj->sdesc );
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

   if( IS_NPC( dMob ) )
      return;

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

void cmd_stand( D_MOBILE *dMob, char *arg )
{
   if( dMob->position == POS_STANDING )
   {
      text_to_mobile_j( dMob, "error", "You're already standing." );
      return;
   }
   if( dMob->position == POS_RESTRAINED )
   {
      text_to_mobile_j( dMob, "error", "You struggle against your restraints but can not stand." );
      echo_around( dMob, "%s struggles against %s restraints", MOBNAME( dMob ), POSSESSIVE( dMob ) );
      return;
   }
   if( dMob->position == POS_UNCONSCIOUS )
   {
      text_to_mobile_j( dMob, "error", "You can't seem to wake up..." );
      return;
   }
   if( dMob->position == POS_SLEEPING )
   {
      text_to_mobile_j( dMob, "text", "You wake up and raise to your feet." );
      echo_around( dMob, "%s wakes up and raises to %s feet.", MOBNAME( dMob ), POSSESSIVE( dMob ) );
   }
   else
   {
      text_to_mobile_j( dMob, "text", "You raise to your feet." );
      echo_around( dMob, "%s raises to %s feet.", MOBNAME( dMob ), POSSESSIVE( dMob ) );
   }
   
   dMob->position = POS_STANDING;
}

void cmd_sit( D_MOBILE *dMob, char *arg ) ///@TODO: Add functionality to sit _on_ things
{
   if( dMob->position == POS_SITTING )
   {
      text_to_mobile_j( dMob, "error", "You're already sitting." );
      return;
   }
   if( dMob->position == POS_RESTRAINED )
   {
      text_to_mobile_j( dMob, "error", "You struggle against your restraints but can not move." );
      echo_around( dMob, "%s struggles against %s restraints", MOBNAME( dMob ), POSSESSIVE( dMob ) );
      return;
   }
   if( dMob->position == POS_UNCONSCIOUS )
   {
      text_to_mobile_j( dMob, "error", "You can't seem to wake up..." );
      return;
   }
   if( dMob->position == POS_SLEEPING )
   {
      text_to_mobile_j( dMob, "text", "You wake up and raise to a seated position." );
      echo_around( dMob, "%s wakes up and raises to a seated position.", MOBNAME( dMob ) );
   }
   else if( dMob->position == POS_RESTING || dMob->position == POS_PRONE )
   {
      text_to_mobile_j( dMob, "text", "You raise to a seated position." );
      echo_around( dMob, "%s raises to a seated position.", MOBNAME( dMob ) );
   }
   else
   {
      text_to_mobile_j( dMob, "text", "You sit on the ground." );
      echo_around( dMob, "%s sits on the ground.", MOBNAME( dMob ) );
   }
   
   dMob->position = POS_SITTING;
}

void cmd_kneel( D_MOBILE *dMob, char *arg )
{
   if( dMob->position == POS_KNEELING )
   {
      text_to_mobile_j( dMob, "error", "You're already kneeling." );
      return;
   }
   if( dMob->position == POS_RESTRAINED )
   {
      text_to_mobile_j( dMob, "error", "You struggle against your restraints but can not move." );
      echo_around( dMob, "%s struggles against %s restraints", MOBNAME( dMob ), POSSESSIVE( dMob ) );
      return;
   }
   if( dMob->position == POS_UNCONSCIOUS )
   {
      text_to_mobile_j( dMob, "error", "You can't seem to wake up..." );
      return;
   }
   if( dMob->position == POS_SLEEPING )
   {
      text_to_mobile_j( dMob, "text", "You wake up and raise to a kneeling position." );
      echo_around( dMob, "%s wakes up and raises to a kneeling position.", MOBNAME( dMob ) );
   }
   else if( dMob->position == POS_RESTING || dMob->position == POS_PRONE || dMob->position == POS_SITTING )
   {
      text_to_mobile_j( dMob, "text", "You raise to a kneeling position." );
      echo_around( dMob, "%s raises to a kneeling position.", MOBNAME( dMob ) );
   }
   else
   {
      text_to_mobile_j( dMob, "text", "You lower yourself into a kneeling position." );
      echo_around( dMob, "%s lowers %sself into a kneeling position.", MOBNAME( dMob ), OBJECTIVE( dMob ) );
   }
   
   dMob->position = POS_KNEELING;
}

void cmd_prone( D_MOBILE *dMob, char *arg )
{
   if( dMob->position == POS_PRONE )
   {
      text_to_mobile_j( dMob, "error", "You're already prone." );
      return;
   }
   if( dMob->position == POS_RESTRAINED )
   {
      text_to_mobile_j( dMob, "error", "You struggle against your restraints but can not move." );
      echo_around( dMob, "%s struggles against %s restraints", MOBNAME( dMob ), POSSESSIVE( dMob ) );
      return;
   }
   if( dMob->position == POS_UNCONSCIOUS )
   {
      text_to_mobile_j( dMob, "error", "You can't seem to wake up..." );
      return;
   }
   if( dMob->position == POS_SLEEPING )
   {
      text_to_mobile_j( dMob, "text", "You wake up and roll into the prone position." );
      echo_around( dMob, "%s wakes up and rolls into the prone position.", MOBNAME( dMob ) );
   }
   else if( dMob->position == POS_RESTING )
   {
      text_to_mobile_j( dMob, "text", "You roll into the prone position." );
      echo_around( dMob, "%s rolls into the prone position.", MOBNAME( dMob ) );
   }
   else
   {
      text_to_mobile_j( dMob, "text", "You lower yourself into the prone position." );
      echo_around( dMob, "%s lowers %sself into the prone position.", MOBNAME( dMob ), OBJECTIVE( dMob ) );
   }
   
   dMob->position = POS_PRONE;
}

void cmd_rest( D_MOBILE *dMob, char *arg )
{
   if( dMob->position == POS_RESTING )
   {
      text_to_mobile_j( dMob, "error", "You're already resting." );
      return;
   }
   if( dMob->position == POS_RESTRAINED )
   {
      text_to_mobile_j( dMob, "error", "You struggle against your restraints but can not move." );
      echo_around( dMob, "%s struggles against %s restraints", MOBNAME( dMob ), POSSESSIVE( dMob ) );
      return;
   }
   if( dMob->position == POS_UNCONSCIOUS )
   {
      text_to_mobile_j( dMob, "error", "You can't seem to wake up..." );
      return;
   }

   if( dMob->position == POS_SLEEPING )
   {
      text_to_mobile_j( dMob, "text", "Your senses heighten as you emerge from your sleep." );
   }
   else
   {
      text_to_mobile_j( dMob, "text", "You sprawl out on the ground." );
      echo_around( dMob, "%s sprawls out on the ground.", MOBNAME( dMob ) );
   }
   
   dMob->position = POS_RESTING;
}

void cmd_sleep( D_MOBILE *dMob, char *arg )
{
   if( dMob->position == POS_SLEEPING )
   {
      text_to_mobile_j( dMob, "error", "You're already resting." );
      return;
   }
   if( dMob->position == POS_RESTRAINED )
   {
      text_to_mobile_j( dMob, "error", "You struggle against your restraints but can not move." );
      echo_around( dMob, "%s struggles against %s restraints", MOBNAME( dMob ), POSSESSIVE( dMob ) );
      return;
   }
   if( dMob->position == POS_UNCONSCIOUS )
   {
      text_to_mobile_j( dMob, "error", "You can't seem to wake up..." );
      return;
   }

   if( dMob->position == POS_RESTING || dMob->position == POS_PRONE )
   {
      text_to_mobile_j( dMob, "text", "You drift off into a deep sleep..." );
      echo_around( dMob, "%s drifts off to sleep...", MOBNAME( dMob ) );
   }
   else if( dMob->position == POS_SITTING )
   {
      text_to_mobile_j( dMob, "text", "You slump over into a deep sleep." );
      echo_around( dMob, "%s slumps over into a deep sleep.", MOBNAME( dMob ) );
   }
   else
   {
      text_to_mobile_j( dMob, "text", "You collapse into a deep sleep" );
      echo_around( dMob, "%s collapses into a deep sleep.", MOBNAME( dMob ) );
   }
   
   dMob->position = POS_SLEEPING;
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
         text_to_mobile_j( dMob, "error", "It's already open." );
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

void cmd_time( D_MOBILE *dMob, char *arg )
{
   char c[MAX_STRING_LENGTH], b[MAX_STRING_LENGTH];
   //System time:
   long  long int diff = (long long int)difftime( current_time, boot_time );
   long long int days = 0, hours = 0, minutes = 0, seconds = 0;

   days = diff / 86400;
   diff %= 86400;
   hours = diff / 3600;
   diff %= 3600;
   minutes = diff / 60;
   diff %= 60;
   seconds = diff;

   snprintf( c, MAX_STRING_LENGTH, "%s", ctime( &current_time ) );
   snprintf( b, MAX_STRING_LENGTH, "%s", ctime( &boot_time ) );

   text_to_mobile_j( dMob, "text", "System Time: %sBootup Time: %sMUD uptime: %lld days, %lld hours, %lld minutes, and %lld seconds.", c, b,  days, hours, minutes, seconds );
}

