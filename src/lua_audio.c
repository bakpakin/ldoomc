#include "audio.h"
#include "lua_modules.h"
#include "lua_interop.h"

static int luai_audio_sound_make(lua_State * L) {
    const char * resource = lua_tostring(L, 1);
    Sound * s = lua_newuserdata(L, sizeof(Sound));
    luaL_getmetatable(L, "ldoom.Sound");
    lua_setmetatable(L , -2);
    audio_sound_init_resource(s, resource);
    return 1;
}

static Sound * luai_audio_sound_check(lua_State * L) {
    void * ud = luaL_checkudata(L, 1, "ldoom.Sound");
    luaL_argcheck(L, ud != NULL, 1, "'ldoom.Sound' expected.");
    return (Sound *) ud;
}

static int luai_audio_sound_tostring(lua_State * L) {
    lua_pushstring(L, "ldoom.Sound");
    return 1;
}

static int luai_audio_sound_delete(lua_State * L) {
    Sound * s = luai_audio_sound_check(L);
    audio_sound_deinit(s);
    return 0;
}

static int luai_audio_sound_play(lua_State * L) {
    Sound * s = luai_audio_sound_check(L);
    audio_sound_play(s);
    return 0;
}

void luai_load_audio() {
    const luaL_Reg methods [] = {
        {"play", luai_audio_sound_play},
        {"destory", luai_audio_sound_delete},
        {NULL, NULL}
    };
    const luaL_Reg metamethods [] = {
        {"__gc", luai_audio_sound_delete},
        {"__tostring", luai_audio_sound_tostring},
        {NULL, NULL}
    };
    const luaL_Reg module [] = {
        {"loadOgg", luai_audio_sound_make},
        {NULL, NULL}
    };
    luai_newclass("ldoom.Sound", methods, metamethods);
    luai_addsubmodule("audio", module);
}
