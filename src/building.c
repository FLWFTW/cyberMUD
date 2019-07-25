#include "mud.h"

void cmd_oset( D_MOBILE *dMob, char *arg )
{
   if( arg[0] == '\0' )
   {
      text_to_mobile_j( dMob, "error", "What object do you want to edit?" );
      return;
   }

   D_OBJECT *obj;
   char oname[MAX_STRING_LENGTH] = {0};
   char action[MAX_STRING_LENGTH] = {0};

   arg = one_arg( arg, oname );
   arg = one_arg( arg, action );

   if( is_number( oname ) )
   {
      obj = get_object_by_vnum( atoi( oname ) );
   }
   else
   {
      if( dMob->hold_right && is_name( oname, dMob->hold_right->name ) )
         obj = dMob->hold_right;
      else if( dMob->hold_left && is_name( oname, dMob->hold_left->name ) )
         obj = dMob->hold_left;
      else
         obj = get_object_list( oname, dMob->room->objects );
   }

   if( obj == NULL )
   {
      text_to_mobile_j( dMob, "error", "You can't find that object here." );
      return;
   }

   if( !strcasecmp( action, "short" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's short description to what?" );
         return;
      }
      if( obj->sdesc ) free( obj->sdesc );
      obj->sdesc = strdup( arg );
   }
   else if( !strcasecmp( action, "long" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's long description to what?" );
         return;
      }
      if( obj->ldesc ) free( obj->ldesc );
      obj->ldesc = strdup( arg );
   }
   else if( !strcasecmp( action, "name" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's name to what?" );
         return;
      }
      if( obj->name ) free( obj->name );
      obj->name = strdup( arg );
   }
   else if( !strcasecmp( action, "type" ) )
   {
      enum item_type_t i;
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the object's type to what?" );
         return;
      }
      for( i = ITEM_CLOTHING; i < MAX_ITEM; i++ )
      {
         if( is_prefix( arg, item_type[i] ) )
         {
            obj->type = i;
            break;
         }
      }
      if( i == MAX_ITEM )
      {
         text_to_mobile_j( dMob, "error", "Invalid object type." );
         return;
      }
   }
   else if( !strcasecmp( action, "wear" ) )
   {
      enum wear_pos_t i;
      if( arg[0] == '\0' )
      {
         text_to_mobile_j( dMob, "error", "Set the oject's wear position to what?" );
         return;
      }
      for( i = WEAR_HEAD; i < MAX_WEAR; i++ )
      {
         if( is_prefix( arg, wear_pos[i] ) )
         {
            obj->wear_pos = i;
            break;
         }
      }
      if( !is_prefix( arg, "none" ) && i == WEAR_NONE )
      {
         text_to_mobile_j( dMob, "error", "Invalid wear position." );
         return;
      }
   }
   else
   {
      text_to_mobile_j( dMob, "Error", "Invalid oset command." );
      return;
   }

   text_to_mobile_j( dMob, "text", "Ok." );
   return;

}

