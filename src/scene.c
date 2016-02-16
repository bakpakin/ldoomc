#include "scene.h"
#include "opool.h"
#include "ldmath.h"
#include "vector.h"
#include "shader.h"
#include "platform.h"
#include "camera.h"
#include "log.h"
#include <assert.h>

#define CELLSIZE 2.0f
#define XCELLS 40
#define YCELLS 1
#define ZCELLS 40
#define CELLS (XCELLS * YCELLS * ZCELLS)
static inline float clamp(float x, float mn, float mx) { return x <= mn ? mn : (x >= mx ? mx : x); }
#define pos2cell(P, S) ((unsigned)clamp((P) / (float)CELLSIZE, 0.0f, S))
#define x2cell(X) pos2cell((X), XCELLS - 1)
#define y2cell(Y) pos2cell((Y), YCELLS - 1)
#define z2cell(Z) pos2cell((Z), ZCELLS - 1)
#define cell2index(X, Y, Z) (XCELLS * ZCELLS * (Y) + XCELLS * (Z) + (X))
#define pos2index(X, Y, Z) (cell2index(pos2cell(X, XCELLS), pos2cell(Y, YCELLS), pos2cell(Z, ZCELLS)))

// defines variables x, y, z, and itergrid_i for use in loop
#define itergrid(x1, y1, z1, x2, y2, z2) \
    for (unsigned _dz = XCELLS - x2 + x1 - 1, \
            _dy = XCELLS * (ZCELLS - z2 + z1 - 1), \
            itergrid_i = cell2index(x1, y1, z1), \
            _done = 1; \
            _done--;) \
        for (unsigned y = y1; y <= y2; y++, itergrid_i += _dy) \
        for (unsigned z = z1; z <= z2; z++, itergrid_i += _dz) \
        for (unsigned x = x1; x <= x2; x++, itergrid_i++) \

typedef struct {
    enum {
        IT_MOB,
        IT_BLOCK
    } type;
    union {
        Mob * mob;
        void * ptr;
    } data;
    unsigned flags;
} Item;

VECGEN(Item, item);
VECGEN(unsigned, uint);

Camera scene_camera;

static unsigned updates_per_frame = 1;

static Program diffuseshader;
static GLint diffuse_mvp_loc;
static GLint diffuse_diffuse_loc;
static GLuint gBuffer;

static Vector items;

static double timeBuffer = 0.0;

static Vector cells[XCELLS * YCELLS * ZCELLS];

static void testiter(unsigned x1, unsigned y1, unsigned z1, unsigned x2, unsigned y2, unsigned z2) {
    itergrid(x1, y1, z1, x2, y2, z2) {
        if (cell2index(x, y, z) != itergrid_i) {
            printf("Iterating from <%d, %d, %d> to <%d, %d, %d>.\n", x1, y1, z1, x2, y2, z2);
            printf("Failed cell: <%d, %d, %d>\n", x, y, z);
            printf("Expected Index: %d\tReceived Index: %d\n", cell2index(x, y, z), itergrid_i);
            exit(1);
        }
    }
}

void scene_init(void) {

    program_init_resource(&diffuseshader, "diffuseshader.glsl");
    diffuse_mvp_loc = glGetUniformLocation(diffuseshader.id, "u_mvp");
    diffuse_diffuse_loc = glGetUniformLocation(diffuseshader.id, "u_diffuse");

    PlatformWindow window;
    platform_get_window(&window);
    camera_init_perspective(&scene_camera, 1.5f, window.width / (float) window.height, 0.05f, 100.0f);

    vector_init_item(&items, 10);
    timeBuffer = 0.0;

    for (unsigned i = 0; i < CELLS; i++)
        vector_init_uint(cells + i, 10);

    // Generate buffers
    glGenFramebuffers(4, &gBuffer);

    testiter(0, 0, 0, XCELLS - 1, YCELLS - 1, ZCELLS - 1);
    testiter(1, 2, 1, XCELLS - 1, YCELLS - 1, ZCELLS - 1);
    testiter(0, 1, 2, XCELLS - 1, YCELLS - 1, ZCELLS - 1);
    testiter(2, 1, 0, 2, 1, 0);
    testiter(0, 0, 0, 0, 0, 0);
}

void scene_deinit() {

    // Deinitialize space for mobs
    vector_deinit_item(&items);

    // Deallocate buffers
    glDeleteFramebuffers(1, &gBuffer);

    for (unsigned i = 0; i < CELLS; i++)
        vector_deinit_uint(cells + i);

    program_deinit(&diffuseshader);
}

static void remove_from_cell(Vector * c, unsigned handle) {
}

static void mob_add_to_cells(Mob * mob) {
    float r = mob->type->radius;
    float h = mob->type->height;
    unsigned x1 = x2cell(mob->position[0] - r);
    unsigned y1 = y2cell(mob->position[1]);
    unsigned z1 = z2cell(mob->position[2] - r);
    unsigned x2 = x2cell(mob->position[0] + r);
    unsigned y2 = y2cell(mob->position[1] + h);
    unsigned z2 = z2cell(mob->position[2] + r);
    unsigned sceneIndex = mob->sceneIndex;
    itergrid(x1, y1, z1, x2, y2, z2) {
        vector_push_uint(cells + itergrid_i, sceneIndex);
    }
}

static void mob_remove_from_cells(Mob * mob) {
    float r = mob->type->radius;
    float h = mob->type->height;
    unsigned x1 = x2cell(mob->position[0] - r);
    unsigned y1 = y2cell(mob->position[1]);
    unsigned z1 = z2cell(mob->position[2] - r);
    unsigned x2 = x2cell(mob->position[0] + r);
    unsigned y2 = y2cell(mob->position[1] + h);
    unsigned z2 = z2cell(mob->position[2] + r);
    unsigned handle = mob->sceneIndex;
    itergrid(x1, y1, z1, x2, y2, z2) {
        Vector * c = cells + itergrid_i;
        for (unsigned i = 0; i < c->count; i++) {
            if (handle == vector_get_uint(c, i)) {
                vector_bag_remove_uint(c, i);
                break;
            }
        }
    }
}

static void mob_update(Mob * mob) {

	unsigned handle = mob->sceneIndex;

    float r = mob->type->radius;
    float h = mob->type->height;

    float * p = mob->position;
    float * pp = mob->prev_position;

    unsigned x1 = x2cell(pp[0] - r);
    unsigned y1 = y2cell(pp[1]);
    unsigned z1 = z2cell(pp[2] - r);
    unsigned x2 = x2cell(pp[0] + r);
    unsigned y2 = y2cell(pp[1] + h);
    unsigned z2 = z2cell(pp[2] + r);

    unsigned nx1 = x2cell(p[0] - r);
    unsigned ny1 = y2cell(p[1]);
    unsigned nz1 = z2cell(p[2] - r);
    unsigned nx2 = x2cell(p[0] + r);
    unsigned ny2 = y2cell(p[1] + h);
    unsigned nz2 = z2cell(p[2] + r);

    itergrid(x1, y1, z1, x2, y2, z2) {
		if (x < nx1 || x > nx2 || y < ny1 || y > ny2 || z < nz1 || z > nz2) {
			Vector * c = cells + itergrid_i;
			for (unsigned i = 0; i < c->count; i++) {
				if (handle == vector_get_uint(c, i)) {
					vector_bag_remove_uint(c, i);
					break;
				}
			}
		}
	}

	itergrid(nx1, ny1, nz1, nx2, ny2, nz2) {
		if (x < x1 || x > x2 || y < y1 || y > y2 || z < z1 || z > z2) {
			vector_push_uint(cells + itergrid_i, handle);
		}
	}

}

void scene_add_mob(Mob * mob) {
    Item newitem;
    newitem.type = IT_MOB;
    newitem.data.mob = mob;
    mob->sceneIndex = items.count;
    vector_push_item(&items, newitem);
    mob_add_to_cells(mob);
}

void scene_remove_mob(Mob * mob) {
	unsigned index = mob->sceneIndex;
	Item otheritem = vector_get_item(&items, items.count - 1);
	vector_bag_remove_item(&items, index);
	mob_remove_from_cells(mob);
	if (otheritem.data.ptr != mob) {
		if (otheritem.type == IT_MOB) {
			otheritem.data.mob->sceneIndex = index;
			mob_add_to_cells(otheritem.data.mob);
		} else {
			// Do something else
		}
	}
}

void scene_resize(int width, int height) {
    camera_set_perspective(&scene_camera, scene_camera.data.perspective.fovY, width / (float) height, 0.05f, 100.0f);
}

void scene_render() {

    // For now, just render everything as diffuse. No spatial partitioning yet.
    glUseProgram(diffuseshader.id);
    for (unsigned i = 0; i < items.count; i++) {
        Item * item = vector_ptr_item(&items, i);
        if (item->type != IT_MOB) continue;
        Mob * mob = item->data.mob;
        glActiveTexture(GL_TEXTURE0);
        Model * model = mob->type->model;
        glBindTexture(GL_TEXTURE_2D, model->diffuse.id);
        glUniform1i(diffuse_diffuse_loc, 0);
        mat4 mvp;
        mat4 rot;
        mat4_rot_y(rot, atan2(mob->facing[0], mob->facing[2]));
        mat4_translation_vec3(mvp, mob->position);
        mat4_mul(mvp, rot, mvp);
        mat4_mul(mvp, mvp, camera_matrix(&scene_camera));
        glUniformMatrix4fv(diffuse_mvp_loc, 1, GL_FALSE, mvp);
        mesh_draw(model->mesh);
    }

}

#define COLLISION_ITERATIONS 3
#define PHYSICS_TIMESTEP (1.0/60.0)

void scene_update(double dt) {

    static const vec3 gravity = {0, -0.02f, 0};

    for (unsigned update_count = 0; update_count < updates_per_frame; update_count++) {

        // Iterate mobs and integrate for new position.
        for (unsigned i = 0; i < items.count; i++) {
            Item * item = vector_ptr_item(&items, i);
            if (item->type != IT_MOB) continue;
            Mob * m = item->data.mob;
            vec3 delta;
            vec3_assign(m->prev_position, m->position);
            vec3_sub(delta, m->position, m->prev_position);
            vec3_scale(delta, delta, m->continuation); // Simulate drag
            vec3_add(delta, delta, m->impulse);
            vec3_add(delta, delta, gravity);
            vec3_add(m->position, delta, m->position);

            // Check for ground collision
            if (m->position[1] < 0) m->position[1] = 0;
        }

        // Iterate over each cell, and check for collisions between mobs in each cell.
        // If they collide, resolve the collisions by adjusting positions. Verlet integration
        // takes care of motion.
        for (int iteration = 0; iteration < COLLISION_ITERATIONS; iteration++)
            for (Vector * v = cells; v < cells + CELLS; v++) {
                for (unsigned i = 0; i < v->count; i++) {
                    Item * itema = vector_ptr_item(&items, vector_get_uint(v, i));
                    if (itema->type != IT_MOB) continue;
                    Mob * a = itema->data.mob;
                    float ax = a->position[0];
                    float ay = a->position[1];
                    float az = a->position[2];
                    float ar = a->type->radius;
                    float ah = a->type->height;
                    float a_inv_mass = a->type->inv_mass;
                    float squishy_a = a->type->squishyness;
                    for (unsigned j = i + 1; j < v->count; j++) {
                        Item * itemb = vector_ptr_item(&items, vector_get_uint(v, j));
                        if (itemb->type != IT_MOB) continue;
                        Mob * b = itemb->data.mob;

                        //check height differences
                        float by = b->position[1];
                        float bh = b->type->height;

                        if (by + bh < ay || ay + ah < by) continue;
                        float dy;
                        if (fabs(ay - by - bh) > fabs(ay + ah - by)) {
                            dy = ay + ah - by;
                        } else {
                            dy = ay - by + bh;
                        }

                        float br = b->type->radius;
                        float bx = b->position[0];
                        float bz = b->position[2];
                        float r = ar + br;
                        float r2 = r * r;
                        float dx = bx - ax;
                        float dz = bz - az;
                        float d2 = dx * dx + dz * dz;
                        if (d2 == 0) {
                            dx = 0.00000001f;
                            d2 = dx * dx;
                        }
                        if (r2 > d2) {

                            float b_inv_mass = b->type->inv_mass;
                            float squishy_b = b->type->squishyness;
                            float d = sqrtf(d2);
                            float squishyfactor = (1 - squishy_a) * (1 - squishy_b);
                            float afactor = squishyfactor * a_inv_mass / (a_inv_mass + b_inv_mass);
                            float bfactor = squishyfactor * b_inv_mass / (a_inv_mass + b_inv_mass);

                            if (fabs(r - d) > fabs(dy)) { // Vertical (Y) correction
                                a->position[1] -= afactor * dy;
                                b->position[1] += bfactor * dy;
                            } else { // horizontal (XZ) correction
                                float factor = (r - d) / d;
                                a->position[0] -= factor * afactor * dx;
                                a->position[2] -= factor * afactor * dz;
                                b->position[0] += factor * bfactor * dx;
                                b->position[2] += factor * bfactor * dz;
                            }

                        }
                    }
                }
            }

        // Update in cells
        for (unsigned i = 0; i < items.count; i++) {
            Item * item = vector_ptr_item(&items, i);
            if (item->type != IT_MOB) continue;
            Mob * m = item->data.mob;
            // Check for ground collision again
            if (m->position[1] < 0) m->position[1] = 0;
            mob_update(m);
        }

    }

}
