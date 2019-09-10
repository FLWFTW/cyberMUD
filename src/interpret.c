/*
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

void handle_edit_input( D_SOCKET *dsock, char *arg )
{
   if( !strcasecmp( arg, "/s" ) )
   {
      free( *dsock->edit_pointer );
      if( dsock->edit_buffer != NULL )
         *dsock->edit_pointer = strdup( dsock->edit_buffer );
      free( dsock->edit_buffer );
      dsock->edit_buffer = NULL;
      text_to_mobile_j( dsock->player, "text", "Saved." );
      dsock->state = STATE_PLAYING;
      return;
   }
   else if( !strcasecmp( arg, "/a" ) )
   {
      free( dsock->edit_buffer );
      dsock->edit_buffer = NULL;
      text_to_mobile_j( dsock->player, "text", "Editing aborted." );
      dsock->state = STATE_PLAYING;
      return;
   }
   else if( !strcasecmp( arg, "/l" ) )
   {
      text_to_mobile_j( dsock->player, "text", "%s", dsock->edit_buffer );
      return;
   }
   else if( !strcasecmp( arg, "/c" ) )
   {
      text_to_mobile_j( dsock->player, "text", "Buffer cleared." );
      free( dsock->edit_buffer );
      dsock->edit_buffer = NULL;
      return;
   }
   int curLen = 0, newLen = 0;
   if( dsock->edit_buffer == NULL )
   {
      dsock->edit_buffer = calloc( 1, sizeof( char ) );
   }
   curLen = strlen( dsock->edit_buffer );
   newLen = strlen( arg );
   dsock->edit_buffer = realloc( dsock->edit_buffer, ( curLen + newLen + 3 ) * sizeof( char ) );
   if( curLen > 0 )
      strncat( dsock->edit_buffer, "\r\n", curLen + newLen + 2 );
   strncat( dsock->edit_buffer, arg, curLen + newLen );
   return;
}

void handle_cmd_input(D_MOBILE *dMob, char *arg)
{
  char command[MAX_BUFFER];
  bool found_cmd = FALSE;
  size_t i;

   if( dMob->socket && dMob->socket->state == STATE_PLAYING )
      save_player( dMob );//save after every command, but only if they're currently playing
  arg = one_arg(arg, command);

  for( i = 0; i < strlen( arg ); i++ )
  {
     if( arg[i] < 32 ) //control characters
        arg[i] = ' ';
  }//Prevent people from sneaking in control characters

  /*
   * Check for room programs, mob programs, and object programs, then for scripted skills, then socials
   */

  for (i = 0; tabCmd[i].cmd_name[0] != '\0' && !found_cmd; i++)
  {
    if (tabCmd[i].level > dMob->level) continue;

    if (is_prefix(command, tabCmd[i].cmd_name))
    {
      found_cmd = TRUE;
      struct command_data *cmd = calloc( 1, sizeof( struct command_data ) );
      cmd->func = tabCmd[i].cmd_funct;
      cmd->dMob = dMob;
      strncpy( cmd->arg, arg, MAX_STRING_LENGTH );
      switch( tabCmd[i].type)
      {
         case CMD_WIZ:
            {
               AppendToList( cmd, dMob->wiz_cmd_list );
               break;
            }
         case CMD_ACT:
            {
               AppendToList( cmd, dMob->act_cmd_list );
               break;
            }
         case CMD_COM:
            {
               AppendToList( cmd, dMob->com_cmd_list );
               break;
            }
         case CMD_OOC:
            {
               AppendToList( cmd, dMob->ooc_cmd_list );
               break;
            }
         default:
            {
               bug( "Invalid command type for command %s (%s).", tabCmd[i].cmd_name, command );
               free( cmd );
               break;
            }
      }
    }
  }

  if (!found_cmd)
    text_to_mobile_j(dMob, "error", "Huh?\n\r");
}

/*
 * The command table, very simple, but easy to extend.
 */
const struct typCmd tabCmd [] =
{

 /* command          function        Req. Level   */
 /* --------------------------------------------- */

  { "accept",        cmd_accept,     LEVEL_GUEST,     CMD_WIZ  },
  { "astat",         cmd_astat,      LEVEL_GOD,       CMD_WIZ  },
  { "aset",          cmd_aset,       LEVEL_GOD,       CMD_WIZ  },
  { "areas",         cmd_areas,      LEVEL_GOD,       CMD_OOC  },
  { "chat",          cmd_chat,       LEVEL_GUEST,     CMD_COM  },
  { "close",         cmd_close,      LEVEL_GUEST,     CMD_ACT  },
  { "commands",      cmd_commands,   LEVEL_GUEST,     CMD_OOC  },
  { "compress",      cmd_compress,   LEVEL_GUEST,     CMD_WIZ  },
  { "copyover",      cmd_copyover,   LEVEL_GOD,       CMD_WIZ  },
  { "down",          cmd_down,       LEVEL_GUEST,     CMD_ACT  },
  { "draw",          cmd_draw,       LEVEL_GUEST,     CMD_ACT  },
  { "drop",          cmd_drop,       LEVEL_GUEST,     CMD_ACT  },
  { "e",             cmd_east,       LEVEL_GUEST,     CMD_ACT  },
  { "east",          cmd_east,       LEVEL_GUEST,     CMD_ACT  },
  { "enter",         cmd_enter,      LEVEL_GUEST,     CMD_ACT  },
  { "equipment",     cmd_equipment,  LEVEL_GUEST,     CMD_OOC  },
  { "eset",          cmd_eset,       LEVEL_GOD,       CMD_WIZ  },
  { "examine",       cmd_examine,    LEVEL_GUEST,     CMD_ACT  },
  { "fill",          cmd_fill,       LEVEL_GUEST,     CMD_ACT  },
  { "fire",          cmd_fire,       LEVEL_GUEST,     CMD_ACT  },
  { "force",         cmd_force,      LEVEL_ADMIN,     CMD_WIZ  },
  { "get",           cmd_get,        LEVEL_GUEST,     CMD_ACT  },
  { "give",          cmd_give,       LEVEL_GUEST,     CMD_ACT  },
  { "goto",          cmd_goto,       LEVEL_GOD,       CMD_WIZ  },
  { "gui-redit",     cmd_guiredit,   LEVEL_GOD,       CMD_WIZ  },
  { "help",          cmd_help,       LEVEL_GUEST,     CMD_OOC  },
  { "hedit",         cmd_hedit,      LEVEL_GOD,       CMD_WIZ  },
  { "holster",       cmd_holster,    LEVEL_GUEST,     CMD_ACT  },
  { "inventory",     cmd_inventory,  LEVEL_GUEST,     CMD_OOC  },
  { "instazone",     cmd_instazone,  LEVEL_GOD,       CMD_WIZ  },
  { "kneel",         cmd_kneel,      LEVEL_GUEST,     CMD_ACT  },
  { "l",             cmd_look,       LEVEL_GUEST,     CMD_ACT  },
  { "load",          cmd_load,       LEVEL_GUEST,     CMD_ACT  },
  { "lock",          cmd_lock,       LEVEL_GUEST,     CMD_ACT  },
  { "look",          cmd_look,       LEVEL_GUEST,     CMD_ACT  },
  { "linkdead",      cmd_linkdead,   LEVEL_ADMIN,     CMD_WIZ  },
  { "makearea",      cmd_makearea,   LEVEL_ADMIN,     CMD_WIZ  },
  { "mlist",         cmd_mlist,      LEVEL_ADMIN,     CMD_WIZ  },
  { "mcreate",       cmd_mcreate,    LEVEL_GOD,       CMD_WIZ  },
  { "mspawn",        cmd_mspawn,     LEVEL_GOD,       CMD_WIZ  },
  { "mset",          cmd_mset,       LEVEL_GOD,       CMD_WIZ  },
  { "mstat",         cmd_mstat,      LEVEL_GOD,       CMD_WIZ  },
  { "n",             cmd_north,      LEVEL_GUEST,     CMD_ACT  },
  { "north",         cmd_north,      LEVEL_GUEST,     CMD_ACT  },
  { "ne",            cmd_northeast,  LEVEL_GUEST,     CMD_ACT  },
  { "northeast",     cmd_northeast,  LEVEL_GUEST,     CMD_ACT  },
  { "nw",            cmd_northwest,  LEVEL_GUEST,     CMD_ACT  },
  { "northwest",     cmd_northwest,  LEVEL_GUEST,     CMD_ACT  },
  { "ocreate",       cmd_ocreate,    LEVEL_GOD,       CMD_WIZ  },
  { "ospawn",        cmd_ospawn,     LEVEL_GOD,       CMD_WIZ  },
  { "oset",          cmd_oset,       LEVEL_GOD,       CMD_WIZ  },
  { "ostat",         cmd_ostat,      LEVEL_GOD,       CMD_WIZ  },
  { "olist",         cmd_olist,      LEVEL_GOD,       CMD_WIZ  },
  { "open",          cmd_open,       LEVEL_GOD,       CMD_ACT  },
  { "put",           cmd_put,        LEVEL_GUEST,     CMD_ACT  },
  { "prompt",        cmd_prompt,     LEVEL_GUEST,     CMD_OOC  },
  { "prone",         cmd_prone,      LEVEL_GUEST,     CMD_ACT  },
  { "reboo",         cmd_reboo,      LEVEL_GOD,       CMD_WIZ  },
  { "reboot",        cmd_reboot,     LEVEL_GOD,       CMD_WIZ  },
  { "redit",         cmd_redit,      LEVEL_GOD,       CMD_WIZ  },
  { "reload",        cmd_load,       LEVEL_GUEST,     CMD_ACT  },
  { "rest",          cmd_rest,       LEVEL_GUEST,     CMD_ACT  },
  { "restore",       cmd_restore,    LEVEL_GOD,       CMD_WIZ  },
  { "remove",        cmd_remove,     LEVEL_GUEST,     CMD_ACT  },
  { "resetarea",     cmd_resetarea,  LEVEL_GOD,       CMD_WIZ  },
  { "s",             cmd_south,      LEVEL_GUEST,     CMD_ACT  },
  { "save",          cmd_save,       LEVEL_GUEST,     CMD_ACT  },
  { "savearea",      cmd_savearea,   LEVEL_GOD,       CMD_WIZ  },
  { "sheath",        cmd_sheath,     LEVEL_GUEST,     CMD_ACT  },
  { "skillset",      cmd_skillset,   LEVEL_GOD,       CMD_WIZ  },
  { "sleep",         cmd_sleep,      LEVEL_GUEST,     CMD_ACT  },
  { "sling",         cmd_sling,      LEVEL_GUEST,     CMD_ACT  },
  { "south",         cmd_south,      LEVEL_GUEST,     CMD_ACT  },
  { "southeast",     cmd_southeast,  LEVEL_GUEST,     CMD_ACT  },
  { "southwest",     cmd_southwest,  LEVEL_GUEST,     CMD_ACT  },
  { "se",            cmd_southeast,  LEVEL_GUEST,     CMD_ACT  },
  { "sw",            cmd_southwest,  LEVEL_GUEST,     CMD_ACT  },
  { "say",           cmd_say,        LEVEL_GUEST,     CMD_COM  },
  { "save",          cmd_save,       LEVEL_GUEST,     CMD_OOC  },
  { "sit",           cmd_sit,        LEVEL_GUEST,     CMD_ACT  },
  { "slay",          cmd_slay,       LEVEL_GOD,       CMD_WIZ  },
  { "stand",         cmd_stand,      LEVEL_GUEST,     CMD_ACT  },
  { "stow",          cmd_stow,       LEVEL_GUEST,     CMD_ACT  },
  { "shutdow",       cmd_shutdow,    LEVEL_GOD,       CMD_WIZ  },
  { "shutdown",      cmd_shutdown,   LEVEL_GOD,       CMD_WIZ  },
  { "qui",           cmd_qui,        LEVEL_GUEST,     CMD_OOC  },
  { "quit",          cmd_quit,       LEVEL_GUEST,     CMD_OOC  },
  { "up",            cmd_up,         LEVEL_GUEST,     CMD_ACT  },
  { "ungive",        cmd_ungive,     LEVEL_GUEST,     CMD_ACT  },
  { "unlock",        cmd_unlock,     LEVEL_GUEST,     CMD_ACT  },
  { "unsling",       cmd_unsling,    LEVEL_GUEST,     CMD_ACT  },
  { "w",             cmd_west,       LEVEL_GUEST,     CMD_ACT  },
  { "wake",          cmd_stand,      LEVEL_GUEST,     CMD_ACT  },
  { "west",          cmd_west,       LEVEL_GUEST,     CMD_ACT  },
  { "wear",          cmd_wear,       LEVEL_GUEST,     CMD_ACT  },
  { "who",           cmd_who,        LEVEL_GUEST,     CMD_OOC  },
  { "score",         cmd_score,      LEVEL_GUEST,     CMD_OOC  },
  { "time",          cmd_time,       LEVEL_GUEST,     CMD_OOC  },
    
  /* end of table */
  { "", 0, 0, MAX_CMD }
};
