#ifndef MESH_HEADER
#define MESH_HEADER

#include "ldmath.h"
#include "glfw.h"

/*
 * Represents the kind of vertices in the mesh.
 */
typedef enum {
    MESHTYPE_2D, MESHTYPE_SIMPLE_3D, MESHTYPE_3D
} MeshType;

/*
 * Enum for GL_STATIC_DRAW and GL_DYNAMIC_DRAW
 */
typedef enum {
    STATIC, DYNAMIC
} DrawType;

/*
 * The vertex type for 2d meshes, like HUD and text.
 */
typedef struct {
    GLfloat position[2];
    GLfloat texcoords[2];
} Vertex2D;

/*
 * Vertex type for things like skyboxes that don't require normals and the like.
 */
typedef struct {
    GLfloat position[3];
    GLfloat texcoords[2];
} SimpleVertex;

/*
 * Vertex type for most complex models, especially things that require lighting.
 */
typedef struct {
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat texcoords[2];
} Vertex;

/*
 * A Type representing renderable geometry. Does not include the texture data.
 */
typedef struct {
	unsigned vcount;
    MeshType mesh_type;
    DrawType draw_type;
    GLenum primitive_type;
    unsigned char flags;
    union {
        Vertex2D * v2d;
        SimpleVertex * sv;
        Vertex * v;
        GLfloat * floats;
    } vertices;
	unsigned icount;
	GLushort * indices;
	GLuint VAO, VBO, EBO;
} Mesh;

/*
 * Initializes a mesh with the given data.
 */
Mesh * mesh_init(Mesh * m,
        MeshType mesh_type,
        DrawType draw_type,
        unsigned vertex_count,
        const Vertex * vertices,
        unsigned indices_count,
        const GLushort * indices);

/*
 * Initializes a mesh from an array of GLfloats. More convenient to use this to
 * Initialize programatically.
 */
Mesh * mesh_init_floats(Mesh * m,
        MeshType mesh_type,
        DrawType draw_type,
        unsigned vertex_count,
        const GLfloat * vertdata,
        unsigned indices_count,
        const GLushort * indices);

/*
 * Pushes the mesh data into the gl context. Called automatically after mesh_inits.
 */
void mesh_load(Mesh * m);

/*
 * Removes mesh data from the gl context.
 */
void mesh_unload(Mesh * m);

/*
 * Deinitializes a Mesh from the openGL context, and frees all vertex and other data.
 */
void mesh_deinit(Mesh * m);

/*
 * Clears the memory in the CPU. The mesh can still be rendered on gpu, but can't be updated.
 */
void mesh_clearcpumem(Mesh * m);

/*
 * Draws a mesh. Textures and shaders must be bound separately.
 */
void mesh_draw(Mesh * m);

// SPECIAL INITIALIZERS

/*
 * Initializes a quad mesh. Returns a mesh with 2d vertices, with
 * corners at (0.5, 0.5), (0.5, -0.5), (-0.5, 0.5), and (-0.5, -0.5).
 */
Mesh * mesh_init_quad(Mesh * m);

/*
 * Creates a cube without texture normals where each side maps to a whole texture.
 */
Mesh * mesh_init_quickcube(Mesh * m);

/*
 * Creates a cylindrical mesh with texture normals.
 */
Mesh * mesh_init_cylinder(Mesh * m, float height, float radius, int subdivisions);

#endif /* end of include guard: MESH_HEADER */
