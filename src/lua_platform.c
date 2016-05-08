#include "lua_interop.h"
#include "lua_modules.h"
#include "platform.h"

static int luai_platform_quit(lua_State * L) {
    platform_exit();
    return 0;
}

static int luai_platform_getFPS(lua_State * L) {
    lua_pushnumber(L, platform_fps());
    return 1;
}

static int luai_platform_getDelta(lua_State * L) {
    lua_pushnumber(L, platform_delta());
    return 1;
}

void luai_load_platform() {
    const luaL_Reg module[] = {
        {"quit", luai_platform_quit},
        {"getDelta", luai_platform_getDelta},
        {"getFPS", luai_platform_getFPS},
        {NULL, NULL}
    };
    luai_addtomainmodule(module);
}
