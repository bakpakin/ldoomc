#include "lua_interop.h"
#include "lua_modules.h"
#include "fntdraw.h"
#include <string.h>

static TextOptions fntdraw_lua_default_options;

static const char * fntdraw_get_align(TextAlign ta) {
    switch(ta) {
        case ALIGN_LEFT: return "left";
        case ALIGN_RIGHT: return "right";
        case ALIGN_CENTER: return "center";
        case ALIGN_TOP: return "top";
        case ALIGN_BOTTOM: return "bottom";
        default: return "center";
    }
}

static TextAlign fntdraw_set_align(const char * align) {
    switch(*align) {
        case 'l': if (strcmp(align, "left") == 0) return ALIGN_LEFT;
        case 'r': if (strcmp(align, "right") == 0) return ALIGN_RIGHT;
        case 'c': if (strcmp(align, "center") == 0) return ALIGN_CENTER;
        case 't': if (strcmp(align, "top") == 0) return ALIGN_TOP;
        case 'b': if (strcmp(align, "bottom") == 0) return ALIGN_BOTTOM;
        default:
        return ALIGN_CENTER;
    }
}

LUAI_MAKECHECKER(FontDef);

static int luai_fntdraw_loadfont(lua_State * L) {
    const char * resource = lua_tostring(L, 1);
    FontDef * fd = lua_newuserdata(L, sizeof(FontDef));
    luaL_getmetatable(L, "ldoom.FontDef");
    lua_setmetatable(L, -2);
    fnt_init(fd, resource);
    return 1;
}

static int luai_fntdraw_deinit(lua_State * L) {
    FontDef * fd = luai_FontDef_check(L);
    if (fd)
        fnt_deinit(fd);
    return 0;
}

static int luai_fnt_maketext(lua_State * L) {
    FontDef * fd = luai_FontDef_check(L);
    if (fd) {
        TextOptions * to = &fntdraw_lua_default_options;
        to->pt = 54;
        to->font = fd;
        // TODO: Use custom text otions if provided.
        Text * text = lua_newuserdata(L, sizeof(Text));
        luaL_getmetatable(L, "ldoom.Text");
        lua_setmetatable(L, -2);
        size_t l;
        const char * string = lua_tolstring(L, 2, &l);
        text_initn(text, to, string, l);
        return 1;
    }
    return 0;
}

LUAI_MAKECHECKER(Text)
static Text * luai_text_check(lua_State * L) {
    void * ud = luaL_checkudata(L, 1, "ldoom.Text");
    luaL_argcheck(L, ud != NULL, 1, "'ldoom.Text' expected.");
    return (Text *) ud;
}

static int luai_text_deinit(lua_State * L) {
    Text * t = luai_text_check(L);
    if (t)
        text_deinit(t);
    return 0;
}

static int luai_text_draw(lua_State * L) {
    Text * t = luai_text_check(L);
    if (t) {
        text_draw_screen(t);
    }
    return 0;
}

LUAI_SETTER1N(Text, setX, t->position[0] = v1)
LUAI_SETTER1N(Text, setY, t->position[1] = v1)
LUAI_SETTER2N(Text, setPosition, t->position[0] = v1, t->position[1] = v2)
LUAI_SETTER1S(Text, setText, text_set(t, v1))
LUAI_SETTER1S(Text, setHAlign, t->halign = fntdraw_set_align(v1); text_mark2update(t);)
LUAI_SETTER1S(Text, setVAlign, t->valign = fntdraw_set_align(v1); text_mark2update(t);)
LUAI_SETTER1N(Text, setSmoothing, t->smoothing = ldm_clamp(v1, 0, 1));
LUAI_SETTER1N(Text, setThreshold, t->threshold = ldm_clamp(v1, 0, 1));
LUAI_SETTER1N(Text, setPoint, t->pt = v1; text_mark2update(t););
LUAI_SETTER1N(Text, setWidth, t->max_width = v1; text_mark2update(t););
LUAI_SETTER1B(Text, setUseDistanceField,
        if (v1)
            t->flags &= ~FNTDRAW_TEXT_NODF_BIT;
        else
            t->flags |= FNTDRAW_TEXT_NODF_BIT;)
LUAI_SETTER1B(Text, setUseMarkup,
        if (v1)
            t->flags |= FNTDRAW_TEXT_MARKUP_BIT;
        else
            t->flags &= ~FNTDRAW_TEXT_MARKUP_BIT;
        text_mark2update(t);)

LUAI_GETTER1N(Text, getX, t->position[0])
LUAI_GETTER1N(Text, getY, t->position[1])
LUAI_GETTER2N(Text, getPosition, t->position[0], t->position[1])
LUAI_GETTER1S(Text, getText, t->text);
LUAI_GETTER1S(Text, getHAlign, fntdraw_get_align(t->halign))
LUAI_GETTER1S(Text, getVAlign, fntdraw_get_align(t->valign))
LUAI_GETTER1N(Text, getSmoothing, t->smoothing);
LUAI_GETTER1N(Text, getThreshold, t->threshold);
LUAI_GETTER1N(Text, getPoint, t->pt);
LUAI_GETTER1N(Text, getWidth, t->max_width);
LUAI_GETTER1B(Text, getUseMarkup, (t->flags & FNTDRAW_TEXT_NEEDS_BUFFER_UPDATE))
LUAI_GETTER1B(Text, getUseDistanceField, (!(t->flags & FNTDRAW_TEXT_NODF_BIT)))

void luai_load_fntdraw() {
    fnt_default_options(NULL, &fntdraw_lua_default_options);
    const luaL_Reg textmethods[] = {
        {"draw", luai_text_draw},
        // Setters
        {"setX", LUAI_F(Text, setX)},
        {"setY", LUAI_F(Text, setY)},
        {"setPosition", LUAI_F(Text, setPosition)},
        {"setText", LUAI_F(Text, setText)},
        {"setHAlign", LUAI_F(Text, setHAlign)},
        {"setVAlign", LUAI_F(Text, setVAlign)},
        {"setSmoothing", LUAI_F(Text, setSmoothing)},
        {"setThreshold", LUAI_F(Text, setThreshold)},
        {"setPoint", LUAI_F(Text, setPoint)},
        {"setWidth", LUAI_F(Text, setWidth)},
        {"setUseMarkup", LUAI_F(Text, setUseMarkup)},
        {"setUseDistanceField", LUAI_F(Text, setUseDistanceField)},
        // Getters
        {"getX", LUAI_F(Text, getX)},
        {"getY", LUAI_F(Text, getY)},
        {"getPosition", LUAI_F(Text, getPosition)},
        {"getText", LUAI_F(Text, getText)},
        {"getHAlign", LUAI_F(Text, getHAlign)},
        {"getVAlign", LUAI_F(Text, getVAlign)},
        {"getSmoothing", LUAI_F(Text, getSmoothing)},
        {"getThreshold", LUAI_F(Text, getThreshold)},
        {"getPoint", LUAI_F(Text, getPoint)},
        {"getWidth", LUAI_F(Text, getWidth)},
        {"getUseMarkup", LUAI_F(Text, getUseMarkup)},
        {"getUseDistanceField", LUAI_F(Text, getUseDistanceField)},
        {NULL, NULL}
    };
    const luaL_Reg textmetamethods[] = {
        {"__gc", luai_text_deinit},
        {NULL, NULL}
    };
    const luaL_Reg fontmethods[] = {
        {"newText", luai_fnt_maketext},
        {NULL, NULL}
    };
    const luaL_Reg fontmetamethods[] = {
        {"__gc", luai_fntdraw_deinit},
        {NULL, NULL}
    };
    const luaL_Reg module[] = {
        {"loadFont", luai_fntdraw_loadfont},
        {NULL, NULL}
    };
    luai_newclass("ldoom.Text", textmethods, textmetamethods);
    luai_newclass("ldoom.FontDef", fontmethods, fontmetamethods);
    luai_addsubmodule("text", module);
}
