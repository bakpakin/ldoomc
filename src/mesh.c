#include "mesh.h"
#include "vector.h"
#include "stddef.h"
#include "stdio.h"

#define ACTIVE_BIT 0x01
#define MEMINITED_BIT 0x02

/*
 * Unwrap the DrawType enum in the mesh.
 */
static inline GLenum get_usage(DrawType d) {
    switch (d) {
        case STATIC:
            return GL_STATIC_DRAW;
        case DYNAMIC:
            return GL_DYNAMIC_DRAW;
    }
}

/*
 * Get the size of a vertex of the given type.
 */
static size_t get_size(MeshType t) {
    switch (t) {
        case MESHTYPE_2D:
            return sizeof(Vertex2D);
        case MESHTYPE_SIMPLE_3D:
            return sizeof(SimpleVertex);
        case MESHTYPE_3D:
            return sizeof(Vertex);
    }
}

/*
 * Generate a Vertex Buffer Object, an Element Buffer Object, and a Vertex Array Object.
 */
static void generate_buffers(Mesh * m) {
    glGenVertexArrays(1, &m->VAO);
    glGenBuffers(1, &m->VBO);
    glGenBuffers(1, &m->EBO);

    glBindVertexArray(m->VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * m->icount, m->indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m->VBO);
    glBufferData(GL_ARRAY_BUFFER, get_size(m->mesh_type) * m->vcount, m->vertices.v, get_usage(m->draw_type));
}

/*
 *
 */
static void setup_mesh_vertex(Mesh * m) {
    // Enable the position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    // Enable normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, normal));

    // Enable texcoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, texcoords));
}

/*
 *
 */
static void setup_mesh_simplevertex(Mesh * m) {
    // Enable the position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);

    // Enable texcoords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid *) offsetof(SimpleVertex, texcoords));
}

/*
 *
 */
static void setup_mesh_vertex2d(Mesh * m) {
    // Enable the position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), 0);

    // Enable texcoords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (GLvoid *) offsetof(Vertex2D, texcoords));
}

Mesh * mesh_init(Mesh * m,
        MeshType mesh_type,
        DrawType draw_type,
        unsigned vertex_count,
        const Vertex * vertices,
        unsigned index_count,
        const GLushort * indices) {

    m->primitive_type = GL_TRIANGLES;
    m->flags= MEMINITED_BIT;
    m->draw_type = draw_type;
    m->mesh_type = mesh_type;

    size_t vsize = get_size(mesh_type) * vertex_count;
    size_t isize = sizeof(GLushort) * index_count;

    m->vcount = vertex_count;
    m->icount = index_count;
    void * ptr = malloc(vsize + isize);
    m->vertices.v = ptr + isize;
    m->indices = ptr;

    memcpy(m->vertices.v, vertices, vsize);
    memcpy(m->indices, indices, isize);

    mesh_load(m);

    return m;
}

Mesh * mesh_init_floats(Mesh * m,
        MeshType mesh_type,
        DrawType draw_type,
        unsigned vertdata_length,
        const GLfloat * vertdata,
        unsigned index_count,
        const GLushort * indices) {

    m->primitive_type = GL_TRIANGLES;
    m->flags= MEMINITED_BIT;
    m->draw_type = draw_type;
    m->mesh_type = mesh_type;

    size_t vsize = sizeof(GLfloat) * vertdata_length;
    size_t isize = sizeof(GLushort) * index_count;

    m->vcount = vsize / get_size(mesh_type);
    m->icount = index_count;
    void * ptr = malloc(vsize + isize);
    m->vertices.v = ptr + isize;
    m->indices = ptr;

    memcpy(m->vertices.v, vertdata, vsize);
    memcpy(m->indices, indices, isize);

    mesh_load(m);

    return m;
}

static Mesh * mesh_init_nocopy(Mesh * m,
        MeshType mesh_type,
        DrawType draw_type,
        unsigned vertdata_length,
        GLfloat * vertdata,
        unsigned index_count,
        GLushort * indices) {

    m->primitive_type = GL_TRIANGLES;
    m->flags = MEMINITED_BIT;
    m->draw_type = draw_type;
    m->mesh_type = mesh_type;

    size_t vsize = sizeof(GLfloat) * vertdata_length;
    size_t isize = sizeof(GLushort) * index_count;

    m->vcount = vsize / get_size(mesh_type);
    m->icount = index_count;
    m->vertices.floats = vertdata;
    m->indices = indices;

    mesh_load(m);

    return m;
}

void mesh_load(Mesh * m) {
    if (m->flags & ACTIVE_BIT)
        return;
    generate_buffers(m);
    switch(m->mesh_type) {
        case MESHTYPE_3D:
            setup_mesh_vertex(m);
            break;
        case MESHTYPE_SIMPLE_3D:
            setup_mesh_simplevertex(m);
            break;
        case MESHTYPE_2D:
            setup_mesh_vertex2d(m);
            break;
    }
    glBindVertexArray(0);
    m->flags |= ACTIVE_BIT;
}

void mesh_unload(Mesh * m) {
    if (!(m->flags & ACTIVE_BIT))
        return;
    glDeleteVertexArrays(1, &m->VAO);
    glDeleteBuffers(1, &m->VBO);
    glDeleteBuffers(1, &m->EBO);
    m->flags &= ~ACTIVE_BIT;
}

void mesh_clearcpumem(Mesh * m) {
    if (!(m->flags & MEMINITED_BIT))
        return;
    free(m->indices);
    m->flags &= ~MEMINITED_BIT;
}

void mesh_deinit(Mesh * m) {
    mesh_unload(m);
    mesh_clearcpumem(m);
}

void mesh_draw(Mesh * m) {
    glBindVertexArray(m->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m->VBO);
    glDrawElements(m->primitive_type, m->icount, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}

static const GLfloat quad_verts[] = {
    0.5, 0.5, 1, 1,
    0.5, -0.5, 1, 0,
    -0.5, 0.5, 0, 1,
    -0.5, -0.5, 0, 0
};

static const GLushort quad_indices[] = {
    0, 1, 2, 3
};

Mesh * mesh_init_quad(Mesh * m) {
    mesh_init_floats(m, MESHTYPE_2D, STATIC, 16, quad_verts, 4, quad_indices);
    m->primitive_type = GL_TRIANGLE_STRIP;
    return m;
}

static const GLfloat cube_verts[] = {

    // X pos
    0.5,  0.5,  0.5, 1, 0, 0, 1, 1,
    0.5,  0.5, -0.5, 1, 0, 0, 1, 0,
    0.5, -0.5,  0.5, 1, 0, 0, 0, 1,
    0.5, -0.5, -0.5, 1, 0, 0, 0, 0,

    // X neg
    -0.5,  0.5,  0.5, -1, 0, 0, 1, 1,
    -0.5,  0.5, -0.5, -1, 0, 0, 1, 0,
    -0.5, -0.5,  0.5, -1, 0, 0, 0, 1,
    -0.5, -0.5, -0.5, -1, 0, 0, 0, 0,

    // Z pos
     0.5,  0.5, 0.5, 0, 0, 1, 1, 1,
     0.5, -0.5, 0.5, 0, 0, 1, 1, 0,
    -0.5,  0.5, 0.5, 0, 0, 1, 0, 1,
    -0.5, -0.5, 0.5, 0, 0, 1, 0, 0,

    // Z neg
     0.5,  0.5, -0.5, 0, 0, -1, 1, 1,
     0.5, -0.5, -0.5, 0, 0, -1, 1, 0,
    -0.5,  0.5, -0.5, 0, 0, -1, 0, 1,
    -0.5, -0.5, -0.5, 0, 0, -1, 0, 0,

    // Y pos
     0.5, 0.5,  0.5, 0, 1, 0, 1, 1,
    -0.5, 0.5,  0.5, 0, 1, 0, 1, 0,
     0.5, 0.5, -0.5, 0, 1, 0, 0, 1,
    -0.5, 0.5, -0.5, 0, 1, 0, 0, 0,

    // Y neg
     0.5, -0.5,  0.5, 0, -1, 0, 1, 1,
    -0.5, -0.5,  0.5, 0, -1, 0, 1, 0,
     0.5, -0.5, -0.5, 0, -1, 0, 0, 1,
    -0.5, -0.5, -0.5, 0, -1, 0, 0, 0
};

static const GLushort cube_indices[] = {
    0, 1, 2, 2, 1, 3,
    5, 4, 6, 5, 6, 7,
    8, 9, 10, 10, 9, 11,
    13, 12, 14, 13, 14, 15,
    16, 17, 18, 18, 17, 19,
    21, 20, 22, 21, 22, 23,
};

Mesh * mesh_init_quickcube(Mesh * m) {
    mesh_init_floats(m, MESHTYPE_3D, STATIC, 6 * 4 * 8, cube_verts, 6 * 6, cube_indices);
    return m;
}

static inline void addv(GLfloat * fs,
        float x, float y, float z,
        float nx, float ny, float nz,
        float u, float v) {
    fs[0] = x;
    fs[1] = y;
    fs[2] = z;
    fs[3] = nx;
    fs[4] = ny;
    fs[5] = nz;
    fs[6] = u;
    fs[7] = v;
}

Mesh * mesh_init_cylinder(Mesh * m, float height, float radius, int subdivisions) {
    unsigned numverts = 4 * subdivisions + 2;
    unsigned numindices = 12 * subdivisions;
    size_t vsize = sizeof(Vertex) * numverts;
    size_t isize = sizeof(GLushort) * numindices;

    void * ptr = malloc(vsize + isize);
    GLushort * is = m->indices = ptr;
    GLfloat * fs = m->vertices.floats = ptr + isize;

    float afactor = 2.0 * LD_PI / subdivisions;
    float ufactor = 6.0 / subdivisions;

    // Add bottom and top verts
    addv(fs, 0, 0, 0, 0, -1, 0, 0.5, 0.5);
    addv(fs + 8, 0, height, 0, 0, 1, 0, 0.5, 0.5);

    for (int i = 0; i < subdivisions; i++) {

        // Add verts

        float ang = i * afactor;
        float u = i * ufactor;

        float x = radius * cosf(i * afactor);
        float y = radius * sinf(i * afactor);
        float nx = x / radius;
        float ny = y / radius;
        float U = 0.5 + nx / 2;
        float V = 0.5 + ny / 2;

        unsigned off = i * 32 + 16; // 32 floats in four verts, plus first two verts

        addv(fs + off +  0, x,      0, y, nx,  0, ny, 1, u); // Wall bottom
        addv(fs + off +  8, x, height, y, nx,  0, ny, 0, u); // Wall top
        addv(fs + off + 16, x,      0, y,  0, -1,  0, U, V); // Base bottom
        addv(fs + off + 24, x, height, y,  0,  1,  0, U, V); // Base top

        // Add indices

        int voff = i * 4 - 2;
        int ioff = i * 12;

        if (0 == i) { // First wall face connects the first and last two vertices on the wall
            is[ioff+0] = numverts - 4;
            is[ioff+1] = voff + 4;
            is[ioff+2] = numverts - 3;
            is[ioff+3] = numverts - 3;
            is[ioff+4] = voff + 4;
            is[ioff+5] = voff + 5;
            is[ioff+6] = numverts - 2;
            is[ioff+7] = 0;
            is[ioff+8] = voff + 6;
            is[ioff+9] = numverts - 1;
            is[ioff+10] = voff + 7;
            is[ioff+11] = 1;
        } else {
            is[ioff+0] = voff;
            is[ioff+1] = voff + 4;
            is[ioff+2] = voff + 1;
            is[ioff+3] = voff + 1;
            is[ioff+4] = voff + 4;
            is[ioff+5] = voff + 5;
            is[ioff+6] = voff + 2;
            is[ioff+7] = 0;
            is[ioff+8] = voff + 6;
            is[ioff+9] = voff + 3;
            is[ioff+10] = voff + 7;
            is[ioff+11] = 1;
        }
    }

    mesh_init_nocopy(m, MESHTYPE_3D, STATIC, numverts * 8, fs, numindices, is);

    mesh_load(m);

    return m;
}

#undef MEMINITED_BIT
#undef ACTIVE_BIT
