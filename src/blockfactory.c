#include "blockfactory.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Float array sections (Total 14N)
// 0..2N    - Base points (N sides, 2 floats per coord)
// 2N..4N   - Top uv coords (N verts on top, 2 floats perr uv coord.)
// 4N..6N   - Bottom uv coords
// 6N..14N  - Side UV coords (N sides, 4 verts per side, 2 floats per coord.)
#define calc_floats(N) ((N) * 14)
#define calc_size(N) (calc_floats(N) * sizeof(float))
#define btpl_base(X, N) (X)->points
#define btpl_base_size(N) (sizeof(float) + 2 * N)
#define btpl_top_uv(X, N) ((X)->points + sizeof(float) * 2 * N)
#define btpl_top_size(N) (sizeof(float) * 2 * N)
#define btpl_bot_uv(X, N) ((X)->points + sizeof(float) * 4 * N)
#define btpl_bot_size(N) (sizeof(float) * 2 * N)
#define btpl_sides_uv(X, N) ((X)->points + sizeof(float) * 6 * N)
#define btpl_sides_size(N) (sizeof(float) * 8 * N)

BlockMeshTemplate * btpl_init(BlockMeshTemplate * tpl, float height, unsigned sides, float * base) {
    tpl->height = height;
    tpl->sides = sides;
    tpl->points = malloc(calc_size(sides));
    memcpy(btpl_base(tpl, sides), base, btpl_base_size(sides));

    // Set good defaults for uvs on top
    for (unsigned i = 0; i < 2 * sides; i += 2) {
        unsigned j = i + 1;
        float x = base[i];
        float y = base[j];
        btpl_top_uv(tpl, sides)[i] = x;
        btpl_top_uv(tpl, sides)[j] = y;
    }

    // Copy to bottom
    memcpy(btpl_bot_uv(tpl, sides), btpl_top_uv(tpl, sides), btpl_top_size(sides));

    // Set up sides
    float x1, y1, x2, y2;
    x2 = btpl_base(tpl, sides)[2 * sides - 2];
    y2 = btpl_base(tpl, sides)[2 * sides - 1];
    float perimeter = 0;
    for (unsigned i = 0; i < 2 * sides; i += 2) {
        x1 = btpl_base(tpl, sides)[i];
        y1 = btpl_base(tpl, sides)[i + 1];
        float dx = x2 - x1;
        float dy = y2 - y1;
        perimeter += sqrt(dx * dx + dy * dy);
        x2 = x1;
        y2 = y1;
    }
    float yscale = 1;
    float scaledPerimeter = perimeter / height;
    if (scaledPerimeter < 1) {
        yscale /= scaledPerimeter;
        scaledPerimeter = 1;
    } else if (ceilf(scaledPerimeter) != scaledPerimeter) {
        yscale *= ceilf(scaledPerimeter) / scaledPerimeter;
        scaledPerimeter = ceilf(scaledPerimeter);
    }

    // Set sides uv. Each set of 4 goes top left, bottom left, bottom right, top right, assuming
    // the base is wound counter clockwise.
    unsigned j = sides - 1;
    float uvx = 0;
    for (unsigned i = 0; i < sides; j = i++) {
        x1 = btpl_base(tpl, sides)[2 * j];
        y1 = btpl_base(tpl, sides)[2 * j + 1];
        x2 = btpl_base(tpl, sides)[2 * i];
        y2 = btpl_base(tpl, sides)[2 * i + 1];
        float dx = x2 - x1;
        float dy = y2 - y1;
        float uvwidth = sqrt(dx * dx + dy * dy) / scaledPerimeter;
        btpl_sides_uv(tpl, sides)[8 * i] = uvx;
        btpl_sides_uv(tpl, sides)[8 * i + 1] = yscale;
        btpl_sides_uv(tpl, sides)[8 * i + 2] = uvx;
        btpl_sides_uv(tpl, sides)[8 * i + 3] = 0;
        btpl_sides_uv(tpl, sides)[8 * i + 4] = uvx + uvwidth;
        btpl_sides_uv(tpl, sides)[8 * i + 5] = 0;
        btpl_sides_uv(tpl, sides)[8 * i + 6] = uvx + uvwidth;
        btpl_sides_uv(tpl, sides)[8 * i + 7] = yscale;
    }

    return tpl;
}

void btpl_deinit(BlockMeshTemplate * tpl) {
    free(tpl->points);
}

void btpl_tomesh(BlockMeshTemplate * tpl, Mesh * mesh, unsigned flags) {

    /* GLfloat * vertices = malloc(sizeof(GLfloat) * 8 * tpl->sides * 4 * 3); */
    /* GLushort * elements = malloc(sizeof(GLushort) * tpl->sides * 4 * 3); */

    /* unsigned j = tpl->sides - 1; */
    /* for (unsigned i = 0; i < tpl->sides; j = i++) { */

    /* } */

}
