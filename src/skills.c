#include "mud.h"

void save_skill_list()
{
   json_t *skill_table = json_array(), *skill = NULL;
   size_t i;

   for( i = 0; *tabCmd[i].cmd_funct != NULL; i++ )
   {
      skill = json_object();
      json_object_set_new( skill, "name", json_string( tabCmd[i].cmd_name ) );
      json_object_set_new( skill, "function", json_string( tabCmd[i].cmd_name ) );
      json_object_set_new( skill, "level", json_integer( tabCmd[i].level ) );
      json_object_set_new( skill, "type", json_string( command_types[tabCmd[i].type] ) );
      json_array_append_new( skill_table, skill );
   }

   json_dump_file( skill_table, "../scripts/skill_list.json", JSON_INDENT(3) );

   return;
}

