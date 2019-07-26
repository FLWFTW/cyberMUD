#include "mud.h"


void reset_area( D_AREA *pArea )
{
   if( pArea == NULL )
      return;

   LIST *rlist = pArea->resets;
   D_RESET *reset;
   D_ROOM  *room;
   D_MOBILE *dMob;
   D_OBJECT *dObj;

   ITERATOR Iter;
   pArea->last_reset = time(NULL);
   if( pArea->reset_script[0] != '\0' &&
       luaL_dostring( globalLuaState, pArea->reset_script ) == 1 )
   {
      log_string( "Error running reset script for area %s.\n%s\n", pArea->name, pArea->reset_script );
   }
   AttachIterator( &Iter, rlist );
   while( ( reset = (D_RESET *)NextInList( &Iter ) ) != NULL )
   {
      if( ( room = get_room_by_vnum( reset->location ) ) == NULL )
      {
         bug( "Reset Error: Reset with null room (vnum: %i)", reset->location );
         continue;
      }
      if( reset->data == NULL )
      {
         bug( "Reset Error: Reset with null data!" );
         continue;
      }
      switch( reset->type )
      {
         case RESET_EXIT:
            {
               break;
            }
         case RESET_OBJ:
            {
               D_OBJECT *pObj = (D_OBJECT *)reset->what;
               if( pObj == NULL || pObj->in_room != room )
               {
                  if( ( dObj = json_to_object( reset->data ) ) == NULL )
                  {
                     bug( "Reset Error: Bad JSON data." );
                     break;
                  }
                  AppendToList( dObj, dobject_list );
                  obj_to_room( dObj, room );
               }
               break;
            }
         case RESET_MOB:
            {
               if( reset->what == NULL )
               {
                  if( ( dMob = json_to_mobile( reset->data ) ) == NULL )
                  {
                     bug( "Reset Error: Bad JSON data." );
                     break;
                  }
                  reset->what = dMob;
                  dMob->reset = reset;
                  AppendToList( dMob, dmobile_list );
                  mob_to_room( dMob, room );
                  break;
               }
            }
         case MAX_RESET:
            {
               break;
            }
         default:
            {
               bug( "Reset Error: Unknown reset type (%i)", reset->type );
               break;
            }
      }
   }
   DetachIterator( &Iter );


}
