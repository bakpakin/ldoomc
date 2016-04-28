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

void luai_event(LuaEventSignature * les, ...) {
    va_list args;
    va_start(args, les);
    lua_getglobal(globalLuaState, "ldoom");
    lua_pushstring(globalLuaState, les->name);
    lua_gettable(globalLuaState, -2);
    if (lua_type(globalLuaState, lua_gettop(globalLuaState)) != LUA_TFUNCTION) {
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
        ldlog("Lua Event \"%s\" failed.", les->name);
    }
    return;
}

// Lua object utils
void luai_make_userdata_type(const char * name) {

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
    audio_sound_init_resource(s, resource);
    return 1;
}

int luai_audio_sound_delete(lua_State * L) {
    Sound * s = (Sound *) lua_touserdata(L, 1);
    audio_sound_deinit(s);
    return 0;
}

int luai_audio_sound_play(lua_State * L) {
    Sound * s = (Sound *) lua_touserdata(L, 1);
    audio_sound_play(s);
    return 0;
}

// Add basic library
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
    lua_pushcfunction(L, logprint);
    lua_setglobal(L, "print");
    luai_doresource("scripts/bootstrap.lua");
    lua_settop(L, 0);

    // Initialize bindings
    lua_pushcfunction(L, luai_audio_sound_make);
    lua_setglobal(L, "makesound");
    lua_pushcfunction(L, luai_audio_sound_delete);
    lua_setglobal(L, "deletesound");
    lua_pushcfunction(L, luai_audio_sound_play);
    lua_setglobal(L, "playsound");

    return 0;
}

void luai_deinit() {
    lua_close(globalLuaState);
}
