/*
 * This file contains the dynamic help system.
 * If you wish to update a help file, simply edit
 * the entry in ../help/ and the mud will load the
 * new version next time someone tries to access
 * that help file.
 */
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h> 

/* include main header file */
#include "mud.h"

LIST     *  help_list = NULL;   /* the linked list of help files     */
char     *  greeting;           /* the welcome greeting              */
char     *  motd;               /* the MOTD help file                */
char     *  death_message;

HELP_DATA *new_help()
{
   HELP_DATA *help;
   help = calloc( 1, sizeof( HELP_DATA ) );
   if( help == NULL )
   {
      bug( "Unable to allocate memory for HELP_DATA" );
      exit( 1 );
   }
   return help;
}

/*
 * Check_help()
 *
 * This function first sees if there is a valid
 * help file in the help_list, should there be
 * no helpfile in the help_list, it will check
 * the ../help/ directory for a suitable helpfile
 * entry. Even if it finds the helpfile in the
 * help_list, it will still check the ../help/
 * directory, and should the file be newer than
 * the currently loaded helpfile, it will reload
 * the helpfile.
 */
bool check_help(D_MOBILE *dMob, char *helpfile)
{
  HELP_DATA *pHelp;
  ITERATOR Iter;
  bool found = FALSE;

  AttachIterator(&Iter, help_list);
  while ((pHelp = (HELP_DATA *) NextInList(&Iter)) != NULL)
  {
    if (is_name(helpfile, pHelp->keyword) && dMob->level >= pHelp->level )
    {
      found = TRUE;
      break;
    }
  }
  DetachIterator(&Iter);

  text_to_mobile_j(dMob, "help", "[%i] %s\r\n\r\n%s", pHelp->level, pHelp->keyword, pHelp->text );

  return found;
}

/*
 * Loads all the helpfiles found in ../help/
 */

void load_helps()
{
   json_t *helps = json_load_file( "../help/help_data.json", 0, NULL );
   help_list = AllocList();
   HELP_DATA *pHelp;
   size_t index;
   json_t *help;

   if( helps == NULL )
   {
      bug( "Syntax error in help_data.json" );
      return;
   }

   json_array_foreach( helps, index, help )
   {
      pHelp = json_to_help( help );
      AppendToList( pHelp, help_list );
      if (!strcasecmp("GREETING", pHelp->keyword))
         greeting = pHelp->text;
      else if (!strcasecmp("MOTD", pHelp->keyword))
         motd = pHelp->text;
      else if( !strcasecmp( "death_message", pHelp->keyword ) )
         death_message = pHelp->text;
   }

   json_decref( helps );

}
/*
void load_helps2()
{
  HELP_DATA *new_help;
  char buf[MAX_BUFFER];
  char *s;
  DIR *directory;
  struct dirent *entry;

  log_string("Load_helps: getting all help files.");

  help_list = AllocList();

  directory = opendir("../help/");
  for (entry = readdir(directory); entry; entry = readdir(directory))
  {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;

    snprintf(buf, MAX_BUFFER, "../help/%s", entry->d_name);
    s = read_help_entry(buf);

    if (s == NULL)
    {
      bug("load_helps: Helpfile %s does not exist.", buf);
      continue;
    }

    if ((new_help = malloc(sizeof(*new_help))) == NULL)
    {
      bug("Load_helps: Cannot allocate memory.");
      abort();
    }

    new_help->keyword    =  strdup(entry->d_name);
    new_help->text       =  strdup(s);
    new_help->load_time  =  time(NULL);
    AttachToList(new_help, help_list);

    if (!strcasecmp("GREETING", new_help->keyword))
      greeting = new_help->text;
    else if (!strcasecmp("MOTD", new_help->keyword))
      motd = new_help->text;
    else if( !strcasecmp( "death_message", new_help->keyword ) )
       death_message = new_help->text;
  }
  closedir(directory);
}
*/

