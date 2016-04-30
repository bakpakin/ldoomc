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

int luai_loadresource(const char * resource);

int luai_doresource(const char * resource);

void luai_init();

void luai_deinit();

void luai_event(const LuaEventSignature * les, ...);

void luai_pushreg(const luaL_Reg * regs);

void luai_newclass(const char * name, const luaL_Reg * methods, const luaL_Reg * metamethods);

void luai_addsubmodule(const char * name, const luaL_Reg * regs);

void luai_addtomainmodule(const luaL_Reg * regs);

// MACROS

#define LUAI_CHECKER(T) luai_##T##_check
#define LUAI_TYPESTRING(T) ("ldoom." ## T)
#define LUAI_MAKECHECKER(T) static T * LUAI_CHECKER(T) (lua_State * L) { \
    void * ud = luaL_checkudata(L, -1, "ldoom." ## T ); \
    luaL_

#endif /* end of include guard: LUAINTEROP_HEADER */
