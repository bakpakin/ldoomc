#include "lua_interop.h"
#include "lua_modules.h"
#include "quickdraw.h"
#include <string.h>

static const struct {
    unsigned val;
    const char * name;
} mappings[] = {
    {QD_NONE, "none"},
    {QD_LINES, "lines"},
    {QD_LINESTRIP, "linestrip"},
    {QD_LINESTRIP, "lineloop"},
    {QD_TRIANGLES, "triangles"},
    {QD_TRIANGLESTRIP, "strip"},
    {QD_TRIANGLEFAN, "fan"},
    {QD_POINTS, "points"},
    {QD_STROKE, "stroke"},
    {QD_FILL, "fill"}
};

static unsigned getDrawType(const char * name) {
    for (unsigned i = 0; i < (sizeof(mappings) / sizeof(mappings[0])); i++) {
        if (strcmp(name, mappings[i].name) == 0) {
            return mappings[i].val;
        }
    }
    return QD_NONE;
}

static int luai_qd_rect(lua_State *L) {
    unsigned drawType = getDrawType(lua_tostring(L, 1));
    float x = lua_tonumber(L, 2);
    float y = lua_tonumber(L, 3);
    float w = lua_tonumber(L, 4);
    float h = lua_tonumber(L, 5);
    qd_rect(x, y, w, h, drawType);
    return 0;
}

void luai_load_quickdraw() {
    luaL_Reg module[] = {
        {"rect", luai_qd_rect},
        {NULL, NULL}
    };
    luai_addsubmodule("quickdraw", module);
}
