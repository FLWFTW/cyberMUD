#ifndef __HANDLER_LUA_H__
#define __HANDLER_LUA_H__

int register_lua_functions( lua_State *l );
extern lua_State *globalLuaState;
lua_State *init_lua();
void close_lua( lua_State *l );

#endif

