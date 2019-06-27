#include "mud.h"


void run_resets( LIST *rlist )
{
   if( rlist == NULL )
      return;

   D_RESET *reset;
   D_ROOM  *room;
   D_MOBILE *dMob;
   D_OBJECT *dObj;

   ITERATOR Iter;
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
