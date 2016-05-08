#include "lua_interop.h"
#include "lua_modules.h"
#include "console.h"

static int luai_console_log_impl(lua_State * L) {
    int n = lua_gettop(L);  /* number of arguments */
    int i;
    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
        const char *s;
        size_t l;
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        s = lua_tolstring(L, -1, &l);  /* get result */
        if (s == NULL) {
            console_clearflush();
            return luaL_error(L,
                    LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        }
        if (i>1) console_pushn("\t", 1);
        console_pushn(s, l);
        lua_pop(L, 1);  /* pop result */
    }
    return 0;
}

static int luai_console_log(lua_State * L) {
    console_clearflush();
    int result = luai_console_log_impl(L);
    console_flush(0);
    return result;
}

static int luai_console_logc(lua_State * L) {
    console_clearflush();
    int result = luai_console_log_impl(L);
    console_flush(1);
    return result;
}

void luai_load_console() {
    const luaL_Reg module[] = {
        {"log", luai_console_log},
        {"logc", luai_console_logc},
        {NULL, NULL}
    };
    luai_addsubmodule("console", module);
}
