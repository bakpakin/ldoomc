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

int luai_load(const char * file);

int luai_do(const char * file);

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

#define LUAI_MAKECHECKER(T) static T * LUAI_CHECKER(T) (lua_State * L) { \
    void * ud = luaL_checkudata(L, 1, "ldoom." #T ); \
    luaL_argcheck(L, ud != NULL, 1, "'ldoom." #T "' expected."); \
    return (T *) ud; \
}

#define LUAI_F(T, UID) luai_##T##_function_##UID

#define LUAI_FWRAPPER(T, UID, BODY) static int LUAI_F(T, UID) (lua_State * L) { \
    T * t = LUAI_CHECKER(T)(L); \
    if (t) { \
        BODY \
    } \
    return 0; \
}

#define LUAI_GETTER1N(T, UID, EXP) LUAI_FWRAPPER(T, UID, lua_pushnumber(L, EXP); return 1;)
#define LUAI_GETTER1S(T, UID, EXP) LUAI_FWRAPPER(T, UID, lua_pushstring(L, EXP); return 1;)
#define LUAI_GETTER1B(T, UID, EXP) LUAI_FWRAPPER(T, UID, lua_pushboolean(L, EXP); return 1;)
#define LUAI_GETTER2N(T, UID, EXP1, EXP2) LUAI_FWRAPPER(T, UID, lua_pushnumber(L, EXP1); lua_pushnumber(L, EXP2); return 2;)
#define LUAI_GETTER2S(T, UID, EXP1, EXP2) LUAI_FWRAPPER(T, UID, lua_pushstring(L, EXP1); lua_pushstring(L, EXP2); return 2;)

#define LUAI_SETTER1N(T, UID, STMNT) LUAI_FWRAPPER(T, UID, \
        double v1 = luaL_checknumber(L, 2); \
        if (v1 != 0 || lua_isnumber(L, 2)) { \
            STMNT; \
        })

#define LUAI_SETTER2N(T, UID, STMNT1, STMNT2) LUAI_FWRAPPER(T, UID, \
        double v1 = luaL_checknumber(L, 2); \
        double v2 = luaL_checknumber(L, 3); \
        if ((v1 != 0 || lua_isnumber(L, 2)) && (v2 != 0 || lua_isnumber(L, 3))) { \
            STMNT1; \
            STMNT2; \
        })

#define LUAI_SETTER1S(T, UID, STMNT) LUAI_FWRAPPER(T, UID, \
        const char * v1 = luaL_checkstring(L, 2); \
        if (v1 != 0 || lua_isstring(L, 2)) { \
            STMNT; \
        })

#define LUAI_SETTER1B(T, UID, STMNT) LUAI_FWRAPPER(T, UID, \
        int v1 = lua_toboolean(L, 2); \
        STMNT; )

#endif /* end of include guard: LUAINTEROP_HEADER */
