#include "mud.h"

D_ACCOUNT *load_account( const char *name, const char *password )
{
   json_t *json;
   D_ACCOUNT *account;
   if( !name || !password )
      return NULL;

   char filename[MAX_STRING_LENGTH];
   snprintf( filename, MAX_STRING_LENGTH, "../accounts/%c/%s.json", toupper(name[0]), name );

   if( ( json = json_load_file( filename, 0, NULL ) ) == NULL )
      return NULL;

   if( bcrypt_checkpw( password, json_string_value( json_object_get( json, "password" ) ) ) == 0 )
   {
      account = json_to_account( json );
      json_decref( json );
      return account;
   }
   json_decref( json );
   return NULL;
}

void save_account( D_ACCOUNT *account )
{
   json_t *json = json_object();
   json_t *characters = json_array();
   if( !account )
      return;

   char filename[MAX_STRING_LENGTH];
   snprintf( filename, MAX_STRING_LENGTH, "../accounts/%c/%s.json", toupper(account->name[0]), account->name );

   json_object_set_new( json, "name", json_string( account->name ) );
   json_object_set_new( json, "email", json_string( account->email ) );
   json_object_set_new( json, "password", json_string( account->password ) );
   if( account->characters != NULL )
   {
      ITERATOR Iter;
      AttachIterator(&Iter, account->characters );
      char *name;
      while ((name = (char *) NextInList(&Iter)) != NULL)
      {
         json_array_append_new( characters, json_string( name ) );
      }
      DetachIterator(&Iter);
   }
   json_object_set_new( json, "characters", characters );
   if( json_dump_file( json, filename, JSON_INDENT(3)|JSON_SORT_KEYS ) == -1 )
   {
      bug( "Error writing profile %s", account->name );
   }
   json_decref( json );
   return;
}

