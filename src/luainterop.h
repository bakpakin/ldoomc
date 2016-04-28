#ifndef LUAINTEROP_HEADER
#define LUAINTEROP_HEADER

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
extern LuaEventSignature les_load;
extern LuaEventSignature les_unload;

int luai_loadresource(const char * resource);

int luai_doresource(const char * resource);

int luai_init();

void luai_deinit();

void luai_event(LuaEventSignature * les, ...);

#endif /* end of include guard: LUAINTEROP_HEADER */
