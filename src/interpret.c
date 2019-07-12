/*
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

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

  for (i = 0; tabCmd[i].cmd_name[0] != '\0' && !found_cmd; i++)
  {
    if (tabCmd[i].level > dMob->level) continue;

    if (is_prefix(command, tabCmd[i].cmd_name))
    {
      found_cmd = TRUE;
      (*tabCmd[i].cmd_funct)(dMob, arg);
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

  { "accept",        cmd_accept,     LEVEL_GUEST  },
  { "astat",         cmd_astat,      LEVEL_GOD    },
  { "areas",         cmd_areas,      LEVEL_GOD    },
  { "chat",          cmd_chat,       LEVEL_GUEST  },
  { "close",         cmd_close,      LEVEL_GUEST  },
  { "commands",      cmd_commands,   LEVEL_GUEST  },
  { "compress",      cmd_compress,   LEVEL_GUEST  },
  { "copyover",      cmd_copyover,   LEVEL_GOD    },
  { "draw",          cmd_draw,       LEVEL_GUEST  },
  { "drop",          cmd_drop,       LEVEL_GUEST  },
  { "e",             cmd_east,       LEVEL_GUEST  },
  { "east",          cmd_east,       LEVEL_GUEST  },
  { "enter",         cmd_enter,      LEVEL_GUEST  },
  { "equipment",     cmd_equipment,  LEVEL_GUEST  },
  { "examine",       cmd_examine,    LEVEL_GUEST  },
  { "fire",          cmd_fire,       LEVEL_GUEST  },
  { "force",         cmd_force,      LEVEL_ADMIN  },
  { "get",           cmd_get,        LEVEL_GUEST  },
  { "give",          cmd_give,       LEVEL_GUEST  },
  { "goto",          cmd_goto,       LEVEL_GOD    },
  { "help",          cmd_help,       LEVEL_GUEST  },
  { "holster",       cmd_holster,    LEVEL_GUEST  },
  { "inventory",     cmd_inventory,  LEVEL_GUEST  },
  { "kneel",         cmd_kneel,      LEVEL_GUEST  },
  { "l",             cmd_look,       LEVEL_GUEST  },
  { "lock",          cmd_lock,       LEVEL_GUEST  },
  { "look",          cmd_look,       LEVEL_GUEST  },
  { "linkdead",      cmd_linkdead,   LEVEL_ADMIN  },
  { "mlist",         cmd_mlist,      LEVEL_ADMIN  },
  { "mspawn",        cmd_mspawn,     LEVEL_GOD    },
  { "n",             cmd_north,      LEVEL_GUEST  },
  { "north",         cmd_north,      LEVEL_GUEST  },
  { "ne",            cmd_northeast,  LEVEL_GUEST  },
  { "northeast",     cmd_northeast,  LEVEL_GUEST  },
  { "nw",            cmd_northwest,  LEVEL_GUEST  },
  { "northwest",     cmd_northwest,  LEVEL_GUEST  },
  { "ospawn",        cmd_ospawn,     LEVEL_GOD    },
  { "ostat",         cmd_ostat,      LEVEL_GOD    },
  { "olist",         cmd_olist,      LEVEL_GOD    },
  { "open",          cmd_open,       LEVEL_GOD    },
  { "put",           cmd_put,        LEVEL_GUEST  },
  { "prompt",        cmd_prompt,     LEVEL_GUEST  },
  { "prone",         cmd_prone,      LEVEL_GUEST  },
  { "reboo",         cmd_reboo,      LEVEL_GOD    },
  { "reboot",        cmd_reboot,     LEVEL_GOD    },
  { "rest",          cmd_rest,       LEVEL_GUEST  },
  { "restore",       cmd_restore,    LEVEL_GOD    },
  { "remove",        cmd_remove,     LEVEL_GUEST  },
  { "resetarea",     cmd_resetarea,  LEVEL_GOD    },
  { "s",             cmd_south,      LEVEL_GUEST  },
  { "sheath",        cmd_sheath,     LEVEL_GUEST  },
  { "sleep",         cmd_sleep,      LEVEL_GUEST  },
  { "sling",         cmd_sling,      LEVEL_GUEST  },
  { "south",         cmd_south,      LEVEL_GUEST  },
  { "southeast",     cmd_southeast,  LEVEL_GUEST  },
  { "southwest",     cmd_southwest,  LEVEL_GUEST  },
  { "se",            cmd_southeast,  LEVEL_GUEST  },
  { "sw",            cmd_southwest,  LEVEL_GUEST  },
  { "say",           cmd_say,        LEVEL_GUEST  },
  { "save",          cmd_save,       LEVEL_GUEST  },
  { "sit",           cmd_sit,        LEVEL_GUEST  },
  { "slay",          cmd_slay,       LEVEL_GOD    },
  { "stand",         cmd_stand,      LEVEL_GUEST  },
  { "stow",          cmd_stow,       LEVEL_GUEST  },
  { "shutdow",       cmd_shutdow,    LEVEL_GOD    },
  { "shutdown",      cmd_shutdown,   LEVEL_GOD    },
  { "qui",           cmd_qui,        LEVEL_GUEST  },
  { "quit",          cmd_quit,       LEVEL_GUEST  },
  { "ungive",        cmd_ungive,     LEVEL_GUEST  },
  { "unlock",        cmd_unlock,     LEVEL_GUEST  },
  { "unsling",       cmd_unsling,    LEVEL_GUEST  },
  { "w",             cmd_west,       LEVEL_GUEST  },
  { "wake",          cmd_stand,      LEVEL_GUEST  },
  { "west",          cmd_west,       LEVEL_GUEST  },
  { "wear",          cmd_wear,       LEVEL_GUEST  },
  { "who",           cmd_who,        LEVEL_GUEST  },
  { "score",         cmd_score,      LEVEL_GUEST  },
  { "time",          cmd_time,       LEVEL_GUEST  },

  /* end of table */
  { "", 0 }
};
