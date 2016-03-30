#ifndef LUABOOTSTRAP_HEADER
#define LUABOOTSTRAP_HEADER

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern lua_State * globalLuaState;

int luaboot_loadresource(const char * resource);

int luaboot_doresource(const char * resource);

int luaboot_init();

void luaboot_deinit();

#endif /* end of include guard: LUABOOTSTRAP_HEADER */
