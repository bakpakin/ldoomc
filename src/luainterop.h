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

#define LUAI_MAKECHECKER(T) static T * LUAI_CHECKER(T) (lua_State * L) { \
    void * ud = luaL_checkudata(L, 1, "ldoom." #T ); \
    luaL_argcheck(L, ud != NULL, 1, "'ldoom." #T "' expected."); \
    return (T *) ud; \
}

#define LUAI_F(T, UID) luai_##T##_function_##UID

#define LUAI_GETTER1N(T, UID, EXP) static int LUAI_F(T, UID) (lua_State * L) { \
    T * t = LUAI_CHECKER(T)(L); \
    if (t) { \
        lua_pushnumber(L, EXP); \
        return 1; \
    } \
    return 0; \
}

#define LUAI_GETTER2N(T, UID, EXP1, EXP2) static int LUAI_F(T, UID) (lua_State * L) { \
    T * t = LUAI_CHECKER(T)(L); \
    if (t) { \
        lua_pushnumber(L, EXP1); \
        lua_pushnumber(L, EXP2); \
        return 2; \
    } \
    return 0; \
}

#define LUAI_SETTER1N(T, UID, STMNT) static int LUAI_F(T, UID) (lua_State * L) { \
    T * t = LUAI_CHECKER(T)(L); \
    if (t) { \
        double v1 = luaL_checknumber(L, -1); \
        if (v1 != 0 || lua_isnumber(L, -1)) { \
            STMNT; \
        } \
    } \
    return 0; \
}

#define LUAI_SETTER2N(T, UID, STMNT1, STMNT2) static int LUAI_F(T, UID) (lua_State * L) { \
    T * t = LUAI_CHECKER(T)(L); \
    if (t) { \
        double v1 = luaL_checknumber(L, -2); \
        double v2 = luaL_checknumber(L, -1); \
        if ((v1 != 0 || lua_isnumber(L, -2)) && (v2 != 0 || lua_isnumber(L, -1))) { \
            STMNT1; \
            STMNT2; \
        } \
    } \
    return 0; \
}

#endif /* end of include guard: LUAINTEROP_HEADER */
