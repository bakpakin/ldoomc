#ifndef LUABOOTSTRAP_HEADER
#define LUABOOTSTRAP_HEADER

#include <stdarg.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern lua_State * globalLuaState;

typedef struct {
    const char * name;
    int num_args;
    const int * arg_types;
} LuaEventSignature;

extern LuaEventSignature les_tick;
extern LuaEventSignature les_update;
extern LuaEventSignature les_draw;

int luaboot_loadresource(const char * resource);

int luaboot_doresource(const char * resource);

int luaboot_init();

void luaboot_deinit();

void luaboot_event(LuaEventSignature * les, ...);

#endif /* end of include guard: LUABOOTSTRAP_HEADER */
