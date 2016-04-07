#include "blockfactory.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Float array sections (Total 14N)
// 0..2N    - Base points (N sides, 2 floats per coord)
// 2N..4N   - Top uv coords (N verts on top, 2 floats perr uv coord.)
// 4N..6N   - Bottom uv coords
// 6N..10N  - Side UV coords (N sides, u1, v1, uwidth, vwidth)
#define calc_floats(N) ((N) * 10)
#define calc_size(N) (calc_floats(N) * sizeof(float))
#define btpl_base(X, N) (X)->points
#define btpl_base_size(N) (sizeof(float) + 2 * N)
#define btpl_top_uv(X, N) ((X)->points + sizeof(float) * 2 * N)
#define btpl_top_size(N) (sizeof(float) * 2 * N)
#define btpl_bot_uv(X, N) ((X)->points + sizeof(float) * 4 * N)
#define btpl_bot_size(N) (sizeof(float) * 2 * N)
#define btpl_sides_uv(X, N) ((X)->points + sizeof(float) * 6 * N)
#define btpl_sides_size(N) (sizeof(float) * 4 * N)

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
        btpl_sides_uv(tpl, sides)[4 * i] = uvx;
        btpl_sides_uv(tpl, sides)[4 * i + 1] = 0;
        btpl_sides_uv(tpl, sides)[4 * i + 2] = uvwidth;
        btpl_sides_uv(tpl, sides)[4 * i + 3] = yscale;
    }

    return tpl;
}

void btpl_deinit(BlockMeshTemplate * tpl) {
    free(tpl->points);
}

// For face:
// 0 : previous face top
// 1 : previous face bottom
// 2 : next face top
// 3 : next face bottom
// 4 : top face
// 5 : bottom face

#define BTPL_PFTOP 0
#define BTPL_PFBOT 1
#define BTPL_NFTOP 2
#define BTPL_NFBOT 3
#define BTPL_TOP 4
#define BTPL_BOT 5

static void btpl_extract_point(BlockMeshTemplate * tpl, unsigned side, int face, float * out) {

    unsigned sides = tpl->sides;
    int istop = (face % 2 == 0);
    float x = btpl_base(tpl, sides)[side * 2];
    float z = btpl_base(tpl, sides)[side * 2 + 1];

    // Calculate the normal of the vertex. Depends on the face.
    float nx, ny, nz;
    if (face == BTPL_TOP) {
        nx = nz = 0;
        ny = 1;
    } else if (face == BTPL_BOT) {
        nx = nz = 0;
        ny = -1;
    } else {
        ny = 0;
        float dx, dz, scale;
        unsigned otherside;
        if (face == BTPL_PFTOP || face == BTPL_PFBOT) {
            scale = 1;
            otherside = side == 0 ? sides - 1 : side - 1;
        } else {
            scale = -1;
            otherside = side == sides - 1 ? 0 : side + 1;
        }
        dx = x - btpl_base(tpl, sides)[otherside * 2];
        dz = z - btpl_base(tpl, sides)[otherside * 2 + 1];
        nx = dz * scale;
        nz = -dx * scale;
        float factor = 1 / sqrtf(nx * nx + nz * nz);
        nx *= factor;
        nz *= factor;
    }

    // grab uv
    float u, v;
    switch (face) {
        case BTPL_PFTOP:
            u = btpl_sides_uv(tpl, sides)[side * 4];
            v = btpl_sides_uv(tpl, sides)[side * 4 + 1];
            break;
        case BTPL_PFBOT:
            u = btpl_sides_uv(tpl, sides)[side * 4];
            v = btpl_sides_uv(tpl, sides)[side * 4 + 1] + btpl_sides_uv(tpl, sides)[side * 4 + 3];
            break;
        case BTPL_NFTOP:
            u = btpl_sides_uv(tpl, sides)[side * 4] + btpl_sides_uv(tpl, sides)[side * 4 + 2];
            v = btpl_sides_uv(tpl, sides)[side * 4 + 1];
            break;
        case BTPL_NFBOT:
            u = btpl_sides_uv(tpl, sides)[side * 4] + btpl_sides_uv(tpl, sides)[side * 4 + 2];
            v = btpl_sides_uv(tpl, sides)[side * 4 + 1] + btpl_sides_uv(tpl, sides)[side * 4 + 3];
            break;
        case BTPL_TOP:
            u = btpl_top_uv(tpl, sides)[side * 2];
            v = btpl_top_uv(tpl, sides)[side * 2 + 1];
            break;
        default: //case BTPL_BOT:
            u = btpl_bot_uv(tpl, sides)[side * 2];
            v = btpl_bot_uv(tpl, sides)[side * 2 + 1];
            break;
    }

    out[0] = x;
    out[1] = istop ? tpl->height : 0;
    out[2] = z;
    out[3] = nx;
    out[4] = ny;
    out[5] = nz;
    out[6] = u;
    out[7] = v;
}

#define BTPL_INDEX(S, F) (48 * S + 8 * F)

void btpl_tomesh(BlockMeshTemplate * tpl, Mesh * mesh, unsigned flags) {

    unsigned sides = tpl->sides;
    unsigned vcount = 8 * 6 * sides;
    unsigned ecount = 12 * sides;
    size_t vsize = sizeof(GLfloat) * vcount;
    size_t esize = sizeof(GLushort) * ecount;

    void * ptr = malloc(vsize + esize);
    GLfloat * vertices = ptr;
    GLushort * elements = ptr + vsize;
    GLfloat * v = vertices;
    GLushort * e = elements;

    // Vertices
    for (unsigned i = 0; i < sides; i++) {
        for (unsigned face = 0; face < 6; face++) {
            btpl_extract_point(tpl, i, face, v);
            v += 8;
        }
    }

    // Elements

    // Side polygons
    for (unsigned i = 0; i < sides; i++) {
        unsigned j = i == 0 ? sides - 1 : i - 1;
        e[0] = e[3] = BTPL_INDEX(j, BTPL_NFTOP);
        e[1] = BTPL_INDEX(j, BTPL_NFBOT);
        e[2] = e[4] = BTPL_INDEX(i, BTPL_PFBOT);
        e[5] = BTPL_INDEX(i, BTPL_PFTOP);
        e += 6;
    }

    // Top and Bottom polygons
    for (unsigned i = 2; i < sides; i++) {
        unsigned j = i - 1;
        // Bottom
        e[0] = BTPL_INDEX(0, BTPL_BOT);
        e[1] = BTPL_INDEX(i, BTPL_BOT);
        e[2] = BTPL_INDEX(j, BTPL_BOT);
        // Top
        e[3] = BTPL_INDEX(0, BTPL_TOP);
        e[4] = BTPL_INDEX(i, BTPL_TOP);
        e[5] = BTPL_INDEX(j, BTPL_TOP);
        e += 6;
    }

    mesh_init_mem(mesh, MESHTYPE_3D, GL_STATIC_DRAW, vcount, vertices, 1, ecount, elements, 0);

}
