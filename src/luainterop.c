#include "luainterop.h"
#include "platform.h"
#include "log.h"
#include "util.h"

// Libraries to wrap
#include "audio.h"

lua_State * globalLuaState;

static const int update_arg_types[1] = { LUA_TNUMBER };

LuaEventSignature les_tick;
LuaEventSignature les_update;
LuaEventSignature les_draw;
LuaEventSignature les_load;
LuaEventSignature les_unload;

// CODE EXECUTION

int luai_loadresource(const char * resource) {
    char file[200];
    platform_res2file(resource, file, 200);
    return luaL_loadfile(globalLuaState, file);
}

int luai_doresource(const char * resource) {
    char file[200];
    platform_res2file(resource, file, 200);
    return luaL_dofile(globalLuaState, file);
}

// EVENTS

void luai_event(LuaEventSignature * les, ...) {
    va_list args;
    va_start(args, les);
    lua_getglobal(globalLuaState, "ldoom");
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
            default:
                lua_pushnil(globalLuaState);
                break;
        }
    }
    va_end(args);
    if(lua_pcall(globalLuaState, les->num_args, 0, 0)) {
        ldlog("Lua Event \"%s\" failed: %s", les->name, lua_tostring(globalLuaState, -1));
    }
    return;
}

// LUA OBJECT UTILS

void luai_pushreg(lua_State * L, const luaL_Reg * regs) {
    for (const luaL_Reg * r = regs; r->name != NULL; r++) {
        lua_pushstring(L, r->name);
        lua_pushcfunction(L, r->func);
        lua_rawset(L, -3); // mt[name] = func
    }
}

void luai_newclass(lua_State * L, const char * name, const luaL_Reg * methods, const luaL_Reg * metamethods) {
    luaL_newmetatable(L, name); // mt = {}, register metatable
    lua_pushstring(L, "__type");
    lua_pushstring(L, name);
    lua_rawset(L, -3); // mt['__type'] = name
    lua_newtable(L); // index = {}
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -4); // mt['__index'] = index
    if (methods != NULL)
        luai_pushreg(L, methods);
    lua_pop(L, 1);
    if (metamethods != NULL)
        luai_pushreg(L, metamethods);
    lua_pop(L, 1);
}

void luai_addsubmodule(lua_State * L, const char * name, const luaL_Reg * regs) {
    lua_newtable(L);
    lua_getglobal(L, "ldoom");
    lua_pushstring(L, name);
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);
    lua_pop(L, 1);
    luai_pushreg(L, regs);
    lua_pop(L, 1);
}

// BASIC UTILS

// Printing goes to the logger instead of stdout.
int logprint(lua_State * L) {
    int nargs = lua_gettop(L);
    for (int i=1; i <= nargs; i++) {
        int t = lua_type(L, i);
        switch (t) {
            case LUA_TSTRING:
                ldlog_write(lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                ldlog_write(lua_toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:
                ldlog_write(lua_tostring(L, i));
                break;
            default:
                ldlog_write(lua_typename(L, t));
                break;
        }
    }
    ldlog_flush();
    return 0;
}

// AUDIO

int luai_audio_sound_make(lua_State * L) {
    const char * resource = lua_tostring(L, 1);
    Sound * s = lua_newuserdata(L, sizeof(Sound));
    luaL_getmetatable(L, "ldoom.Sound");
    lua_setmetatable(L , -2);
    audio_sound_init_resource(s, resource);
    return 1;
}

Sound * luai_audio_sound_check(lua_State * L) {
    void * ud = luaL_checkudata(L, 1, "ldoom.Sound");
    luaL_argcheck(L, ud != NULL, 1, "'Sound' expected.");
    return (Sound *) ud;
}

int luai_audio_sound_tostring(lua_State * L) {
    lua_pushstring(L, "ldoom.Sound");
    return 1;
}

int luai_audio_sound_delete(lua_State * L) {
    Sound * s = luai_audio_sound_check(L);
    audio_sound_deinit(s);
    return 0;
}

int luai_audio_sound_play(lua_State * L) {
    Sound * s = luai_audio_sound_check(L);
    audio_sound_play(s);
    return 0;
}

const luaL_Reg luai_audio_sound_methods [] = {
    {"play", luai_audio_sound_play},
    {"destory", luai_audio_sound_delete},
    {NULL, NULL}
};

const luaL_Reg luai_audio_sound_metamethods [] = {
    {"__gc", luai_audio_sound_delete},
    {"__tostring", luai_audio_sound_tostring},
    {NULL, NULL}
};

const luaL_Reg luai_audio_module [] = {
    {"loadSound", luai_audio_sound_make},
    {NULL, NULL}
};

void luai_audio_loadlib(lua_State * L) {
    luai_newclass(L, "ldoom.Sound", luai_audio_sound_methods, luai_audio_sound_metamethods);
    luai_addsubmodule(L, "audio", luai_audio_module);
}

// INITIALIZE

int luai_init() {

    // Set up Event Signatures
    les_update.name = "update";
    les_update.num_args = 1;
    les_update.arg_types = update_arg_types;
    les_tick.name = "tick";
    les_tick.num_args = 0;
    les_tick.arg_types = NULL;
    les_draw.name = "draw";
    les_draw.num_args = 0;
    les_draw.arg_types = NULL;
    les_load.name = "load";
    les_load.num_args = 0;
    les_load.arg_types = NULL;
    les_unload.name = "unload";
    les_unload.num_args = 0;
    les_unload.arg_types = NULL;

    lua_State * L = luaL_newstate();
    globalLuaState = L;
    luaL_openlibs(L);
    lua_newtable(L);
    lua_setglobal(L, "ldoom");
    lua_pushcfunction(L, logprint);
    lua_setglobal(L, "print");
    luai_doresource("scripts/bootstrap.lua");
    lua_settop(L, 0);

    // Initialize bindings
    luai_audio_loadlib(L);

    return 0;
}

void luai_deinit() {
    lua_close(globalLuaState);
}
