#include "luainterop.h"
#include "platform.h"
#include "util.h"
#include "console.h"

lua_State * globalLuaState;

// Event signatures

// CODE EXECUTION

int luai_loadresource(const char * resource) {
    return luaL_loadfile(globalLuaState, platform_res2file_ez(resource));
}

int luai_doresource(const char * resource) {
    return luaL_dofile(globalLuaState, platform_res2file_ez(resource));
}

// EVENTS

void luai_event(const LuaEventSignature * les, ...) {
    va_list args;
    va_start(args, les);
    lua_getglobal(globalLuaState, "levent");
    int type = lua_type(globalLuaState, -1);
    if (type != LUA_TTABLE) {
        return;
    }
    lua_pushstring(globalLuaState, les->name);
    lua_gettable(globalLuaState, -2);
    if (lua_type(globalLuaState, -1) != LUA_TFUNCTION) {
        lua_settop(globalLuaState, 0);
        return;
    }
    for (int i = 0; i < les->num_args; i++) {
        int argtype = les->arg_types[i];
        switch (argtype) {
            case LUA_TSTRING:
                lua_pushstring(globalLuaState, va_arg(args, const char *));
                break;
            case LUA_TNUMBER:
                lua_pushnumber(globalLuaState, va_arg(args, double));
                break;
            case LUA_TBOOLEAN:
                lua_pushboolean(globalLuaState, va_arg(args, int));
                break;
            case LUA_TUSERDATA:
                lua_pushlightuserdata(globalLuaState, va_arg(args, void *));
                break;
            default:
                lua_pushnil(globalLuaState);
                break;
        }
    }
    va_end(args);
    if(lua_pcall(globalLuaState, les->num_args, 0, 0)) {
        console_log("Lua Event \"%s\" failed: %s", les->name, lua_tostring(globalLuaState, -1));
    }
    return;
}

// LUA OBJECT UTILS

void luai_pushreg(const luaL_Reg * regs) {
    lua_State * L = globalLuaState;
    for (const luaL_Reg * r = regs; r->name != NULL; r++) {
        lua_pushstring(L, r->name);
        lua_pushcfunction(L, r->func);
        lua_rawset(L, -3); // mt[name] = func
    }
}

void luai_newclass(const char * name, const luaL_Reg * methods, const luaL_Reg * metamethods) {
    lua_State * L = globalLuaState;
    luaL_newmetatable(L, name); // mt = {}, register metatable
    if (metamethods != NULL) {
        luai_pushreg(metamethods);
    }
    lua_pushstring(L, "__type");
    lua_pushstring(L, name);
    lua_rawset(L, -3); // mt['__type'] = name
    lua_newtable(L); // index = {}
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -4); // mt['__index'] = index
    if (methods != NULL)
        luai_pushreg(methods);
    lua_pop(L, 2);
}

void luai_addsubmodule(const char * name, const luaL_Reg * regs) {
    lua_State * L = globalLuaState;
    lua_newtable(L);
    lua_getglobal(L, "ldoom");
    lua_pushstring(L, name);
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    luai_pushreg(regs);
    lua_pop(L, 1);
}

void luai_addtomainmodule(const luaL_Reg * regs) {
    lua_State * L = globalLuaState;
    lua_getglobal(L, "ldoom");
    luai_pushreg(regs);
    lua_pop(L, 1);
}

// INITIALIZE

void luai_init() {
    lua_State * L = luaL_newstate();
    globalLuaState = L;
    luaL_openlibs(L);
    lua_newtable(L);
    lua_setglobal(L, "levent");
    lua_newtable(L);
    lua_setglobal(L, "ldoom");
    lua_settop(L, 0);
}

void luai_deinit() {
    lua_close(globalLuaState);
}
