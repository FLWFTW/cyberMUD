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

/* checks the help_list for the appropriate help file */
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

  if( pHelp == NULL )
  {
     text_to_mobile_j( dMob, "error", "No manual entry for %s.", helpfile );
  }
  else
  {
     text_to_mobile_j(dMob, "help", "[%i] %s\r\n\r\n%s", pHelp->level, pHelp->keyword, pHelp->text );
  }

  return found;
}

/*
 * Loads all the helpfiles found in ../help/help_data.json
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

