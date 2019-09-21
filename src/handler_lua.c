/**
 * @file handler_lua.c
 * @author Will Sayin
 * @version 1.0
 *
 * @section DESCRIPTION
 * Lua handler. Defines custom Lua functions and C interface. Used in game
 * scripting.
 */
#include "mud.h"

/**
 * logs a string from the Lua environment
 */
static int l_logstring( lua_State *l )
{
   const char *log = luaL_checkstring( l, 1 );
   log_string( "((LUA)) %s", log );
   return 0;
}

/**
 * Loads the mob specified by vnum.
 * On success pushes a pointer to the mobile onto the stack.
 * On failure pushes nil onto the stack.
 */

static int l_mload( lua_State *l )
{
   D_MOBILE *m;
   int vnum = -1;

   vnum = luaL_checknumber( l, 1 );
   if( ( m = spawn_mobile( vnum ) ) == NULL )
   {
      log_string( "Lua Error (%s): Attempting to spawn non-existant mobile %i.", __func__, vnum );
      lua_pushnil( l );
      return 1;
   }

   lua_pushlightuserdata( l, m );
   return 1;
}

/**
 * Pops the pointer to a mobile and a vnum from the stack
 * On success, pushes a pointer to the room the mobile was transferred to
 * On failure, pushes nil onto the stack.
 */
static int l_mtrans( lua_State *l )
{
   D_MOBILE *m = lua_touserdata( l, 1 );
   int vnum = luaL_checknumber( l, 2 );
   D_ROOM *to = get_room_by_vnum( vnum );

   if( vnum < 1 || m == NULL || to == NULL )
   {
      lua_pushnil( l );
      return 1;
   }

   mob_to_room( m, to );
   lua_pushlightuserdata( l, to );

   return 1;
}

/**
 * Loads an object specified by its vnum.
 * On success pushes a pointer to the object onto the stack.
 * On failure pushes nil onto the stack.
 */
static int l_oload( lua_State *l )
{
   D_OBJECT *o;
   int vnum = -1;

   vnum = luaL_checknumber( l, 1 );
   if( ( o = spawn_object( vnum ) ) == NULL )
   {
      log_string( "Lua Error (%s): Attempting to spawn non-existant object %i.", __func__, vnum );
      lua_pushnil( l );
      return 1;
   }

   lua_pushlightuserdata( l, o );
   return 1;
}

/**
 * Puts an object into another object.
 * On success pushes a pointer to the receiving (container) object onto the stack.
 * On failure pushes nil onto the stack.
 */
static int l_objtoobj( lua_State *l )
{
   D_OBJECT *o, *con;
   o = lua_touserdata( l, 1 );
   con = lua_touserdata( l, 2 );

   if( o == NULL || con == NULL )
   {
      log_string( "Lua Error(%s): Invalid object provided.", __func__ );
      lua_pushnil( l );
      return 1;
   }

   if( o == con )
   {
      log_string( "Lua Error (%s): Attempting to place an object inside itself.", __func__ );
      lua_pushnil( l );
      return 1;
   }
   object_to_object( o, con );
   lua_pushlightuserdata( l, con );
   return 1;
}

/**
 * Places an object specified via pointer to the object into a room specified by vnum.
 * On success returns a pointer to the room the object was placed in.
 * On failure returns nil.
 */
static int l_objtoroom( lua_State *l )
{
   D_OBJECT *o = lua_touserdata( l, 1 );
   int vnum = luaL_checknumber( l, 2 );
   D_ROOM *to = get_room_by_vnum( vnum );

   if( o == NULL || to == NULL )
   {
      log_string( "Lua Error(%s): Invalid arguments provided.", __func__ );
      lua_pushnil( l );
      return 1;
   }

   object_to_room( o, to );
   lua_pushlightuserdata( l, to );
   return 1;
}

static int l_mequip( lua_State *l )
{
   D_MOBILE *m;
   D_OBJECT *o;
   
   m = lua_touserdata( l, 1 );
   o = lua_touserdata( l, 2 );

   if( o == NULL || m == NULL )
   {
      log_string( "Lua Error(%s): Invalid arguments.", __func__ );
      lua_pushnil( l );
      return 1;
   }
   if( equip_object( m, o ) == FALSE )
   {
      log_string( "Lua Error(%s): Mobile %s(%i) can not wear object %s(%i).", __func__, m->name, m->vnum, o->name, o->vnum );
      lua_pushnil( l );
      return 1;
   }
   lua_pushlightuserdata( l, m );
   return 1;
}

static int l_force( lua_State *l )
{
   D_MOBILE *m = lua_touserdata( l, 1 );
   const char *cmd = luaL_checkstring( l, 2 );

   if( m->level > LEVEL_PLAYER )
   {
      log_string( "Lua Error(%s): Attempting to force a MUD admin!" );
      lua_pushboolean( l, FALSE );
      return 1;
   }

   handle_cmd_input( m, (char*)cmd );
   lua_pushboolean( l, TRUE );

   return 1;
}

static int l_get_mob_room( lua_State *l )
{
   D_ROOM *room = current_mob->room;
   const char *name = luaL_checkstring( l, 1 );

   D_MOBILE *dMob = get_mobile_list( name, room->mobiles );
   lua_pushlightuserdata( l, dMob );
   return 1;
}

static int l_get_mob_global( lua_State *l )
{
   const char *name = luaL_checkstring( l, 1 );
   D_MOBILE *dMob = get_mobile_list( name, dmobile_list );
   lua_pushlightuserdata( l, dMob );
   return 1;
}

static int l_damage( lua_State *l )
{
   D_MOBILE *dMob = lua_touserdata( l, 1 );
   int dam = luaL_checknumber( l, 2 );
   int loc = get_bodypart_code( luaL_checkstring( l, 3 ) );
   int type = get_damagetype_code( luaL_checkstring( l, 4 ) );

   lua_pushnumber( l, damage( dMob, dam, loc, type ) );
   return 1;
}   

static lua_CFunction stackDump (lua_State *L) //useful for debugging purposes
{
   int i;
   int top = lua_gettop(L);
   char buf[MAX_STRING_LENGTH];
   char *place = buf;
   place += snprintf( place, MAX_STRING_LENGTH-(place-buf), "\nLua Stack\n---------\n\tType   Data\n\t-----------\n" );
   for (i = 1; i <= top; i++)
   {  /* repeat for each level */
      int t = lua_type(L, i);
      place += snprintf( place, MAX_STRING_LENGTH-(place-buf), "%i", i );
      switch (t)
      {
 
         case LUA_TSTRING:  /* strings */
            place += snprintf( place, MAX_STRING_LENGTH-(place-buf),"\tString: `%s'\n", lua_tostring(L, i));
            break;
 
         case LUA_TBOOLEAN:  /* booleans */
            place += snprintf( place, MAX_STRING_LENGTH-(place-buf),"\tBoolean: %s", lua_toboolean(L, i) ? "\ttrue\n" : "\tfalse\n");
            break;
 
         case LUA_TNUMBER:  /* numbers */
            place += snprintf( place, MAX_STRING_LENGTH-(place-buf),"\tNumber: %g\n", lua_tonumber(L, i));
            break;
 
         default:  /* other values */
            place += snprintf( place, MAX_STRING_LENGTH-(place-buf),"\tOther: %s\n", lua_typename(L, t));
            break;
 
     } 
   }
   bug("%s",buf);  /* end the listing */

   return 0;
}

/*********************************************************************************
 ***                                                                           ***
 ***                 [[ KEEP ALL THIS STUFF AT THE BOTTOM ]]                   ***
 ***                                                                           ***
 ********************************************************************************/
static const struct luaL_Reg mudLib[] = 
{
   {"lpmtrans", l_mtrans },
   {"lplogstring", l_logstring},
   {"lpmload", l_mload},
   {"lpoload", l_oload},
   {"lpobj2obj", l_objtoobj},
   {"lpobj2room", l_objtoroom},
   {"lpmequip", l_mequip},
   {"lpforce", l_force},
   {"get_mob_room", l_get_mob_room},
   {"get_mob_global", l_get_mob_global},
   {"lpdamage", l_damage},
   {NULL, NULL}
};

int register_lua_functions( lua_State *l )
{
   const struct luaL_Reg *reg = mudLib;

   while( reg->name != NULL )
   {
      lua_pushcfunction( l, reg->func );
      lua_setglobal( l, reg->name );
      reg++;
   }
   return 0;
}

lua_State *init_lua()
{
   lua_State *state;
   if( ( state = luaL_newstate() ) == NULL )
   {
      return NULL;
   }
   luaL_openlibs( state );
   register_lua_functions( state );
   if( luaL_dofile( state, "./lua/modules/JSON.lua" ) == 1 )
   {
      fprintf( stderr, "Error loading required Lua library JSON.lua" );
      exit(1);
   }
   lua_atpanic( state, (lua_CFunction)stackDump );

   return state;
}

void close_lua( lua_State *l )
{
   if( l == NULL )
      return;
   lua_close( l );
   l = NULL;
   return;
}

