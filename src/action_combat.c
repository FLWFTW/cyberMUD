/*
 * This file handles player combat actions.
 */

#include "mud.h"

void cmd_slay( D_MOBILE *dMob, char *arg )
{
   D_MOBILE *victim;
   char name[MAX_STRING_LENGTH];
   
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "Slay who?" );
      return;
   }

   arg = one_arg ( arg, name );

   if( ( victim = get_mobile_list( name, dMob->room->mobiles ) ) == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find anyone who looks like that." );
      return;
   }

   if( victim == dMob )
   {
      text_to_mobile_j( dMob, "error", "You should probably see a psychologist." );
      return;
   }

   text_to_mobile_j( dMob, "combat", "You slay %s in cold blood!", victim->name );
   free_mobile( victim );
}
