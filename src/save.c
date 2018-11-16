#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* main header file */
#include "mud.h"

D_MOBILE *load_player( const char *name )
{
   json_t *json;
   char filename[MAX_STRING_LENGTH];
   D_MOBILE *mobile;
   
   snprintf( filename, MAX_STRING_LENGTH, "../players/%c/%s.json", toupper(name[0]), name );
   
   if( ( json = json_load_file( filename, 0, NULL ) ) == NULL )
   {
      bug( "ERROR: (JSON) Can not load player file %s(%s)", filename );
      return NULL;
   }
   mobile = json_to_mobile( json );
   json_decref( json );
   return mobile;
}

void save_player( D_MOBILE *dMob )
{
   char  filename[MAX_BUFFER];
   json_t *json = player_to_json( dMob, TRUE );
   
   snprintf( filename, MAX_BUFFER, "../players/%c/%s.json", toupper(dMob->name[0]), dMob->name );
  
   if( json_dump_file( json, filename, JSON_INDENT(3)|JSON_SORT_KEYS ) == -1 )
   {
      bug( "Error writing profile %s", dMob->name );
   }
   json_decref( json );
   return;
}
