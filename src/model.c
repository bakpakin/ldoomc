#include "model.h"
#include "iqm.h"
#include "util.h"
#include "platform.h"
#include "ldmath.h"
#include <string.h>

int model_loadfile(Model * model, const char * file) {

    long flen;
    char * data = util_slurp(file, &flen);

    struct iqmheader * header = (struct iqmheader *) data;

    // Verify magic bytes
    if (strcmp(header->magic, IQM_MAGIC) != 0) {
        return 1;
    }
    if (header->version != 2) {
        return 1;
    }

    // Allocate all memory required in one go
    size_t ssize = 0;
    const char * text = (char *) (data + header->ofs_text);
    for (uint32_t i = 0; i < header->num_text; i++) {
        size_t bufi = strlen(text) + 1;
        ssize += bufi;
        text += bufi;
    }
    uint32_t numv = header->num_vertexes;
    uint32_t numt = header->num_triangles;
    uint32_t numb = header->num_joints;
    uint32_t numa = header->num_anims;
    uint32_t nump = header->num_poses;
    uint32_t numm = header->num_meshes;
    size_t vsize = sizeof(ModelVertex) * numv;
    size_t tsize = sizeof(ModelTriangle) * numt;
    size_t bsize = sizeof(ModelBone) * numb;
    size_t asize = sizeof(ModelAnimation) * numa;
    size_t psize = sizeof(ModelBonePose) * nump;
    size_t msize = sizeof(ModelMesh) * numm;
    void * memory = malloc(vsize + tsize + bsize + asize + psize + msize + ssize);
    ModelVertex * verts = (ModelVertex *) memory;
    ModelTriangle * triangles = (ModelTriangle *) (memory + vsize);
    ModelBone * bones = (ModelBone *) ((void *) triangles + tsize);
    ModelAnimation * animations = (ModelAnimation *) ((void *) bones + bsize);
    ModelBonePose * poses = (ModelBonePose *) ((void *) animations + asize);
    ModelMesh * meshes = (ModelMesh *) ((void *) poses + psize);
    uint8_t * textdata = (uint8_t *) ((void *) meshes + msize);

#define BAILOUT do { free(memory); free(data); return 1;} while(0)

    // Construct Vertex Arrays
    struct iqmvertexarray * va_first = (struct iqmvertexarray *) (data + header->ofs_vertexarrays);
    for (uint32_t i = 0; i < numv; i++) {

       struct iqmvertexarray * va = va_first + i;
       float * fp;
       uint8_t * up;
       unsigned j = 0;

       switch (va->format) {
           case IQM_FLOAT:
               fp = (float *) (data + va->offset);
               break;
            case IQM_UBYTE:
               up = (uint8_t*) (data + va->offset);
               break;
            default:
               BAILOUT;
       }

       switch (va->type) {
           case IQM_POSITION:
               if (va->format != IQM_FLOAT)
                   BAILOUT;
               for (; j < numv; fp += 3, j++) {
                   memcpy(verts[j].position, fp, sizeof(float) * 3);
               }
               break;
           case IQM_TEXCOORD:
               if (va->format != IQM_FLOAT)
                   BAILOUT;
               for (; j < numv; fp += 2, j++) {
                   memcpy(verts[i].texcoord, fp, sizeof(float) * 2);
               }
               break;
           case IQM_NORMAL:
               if (va->format != IQM_FLOAT)
                   BAILOUT;
               for (; j < numv; fp += 3, j++) {
                   memcpy(verts[i].normal, fp, sizeof(float) * 3);
               }
               break;
           case IQM_BLENDINDEXES:
               if (va->format != IQM_UBYTE)
                   BAILOUT;
               for (; j < numv; up += 4, j++) {
                   memcpy(verts[i].boneIndicies, up, 4 * sizeof(uint8_t));
               }
               break;
           case IQM_BLENDWEIGHTS:
               if (va->format != IQM_UBYTE)
                   BAILOUT;
               for (; j < numv; up += 4, j++) {
                   memcpy(verts[i].boneWeights, up, 4 * sizeof(uint8_t));
               }
               break;
           case IQM_TANGENT:
           case IQM_COLOR:
           case IQM_CUSTOM:
           default:
               break;
       }
    }

    // Read Triangles
    memcpy(triangles, data + header->ofs_triangles, sizeof(uint32_t) * 3 * numt);

    // Read Meshes
    struct iqmmesh * mesh_first = (struct iqmmesh *) (data + header->ofs_meshes);
    for (uint32_t i = 0; i < numm; i++) {
        meshes[i].firstVertex = mesh_first[i].first_vertex;
        meshes[i].vertexCount = mesh_first[i].num_vertexes;
        meshes[i].firstTriangle = mesh_first[i].first_triangle;
        meshes[i].triangleCount = mesh_first[i].num_triangles;
        meshes[i].materialid = mesh_first[i].material;
        meshes[i].name = mesh_first[i].name;
    }

    // Read Bones
    struct iqmjoint * bone_first = (struct iqmjoint *) (data + header->ofs_joints);
    for (uint32_t i = 0; i < numb; i++) {
        vec3_assign(bones[i].restingScale, bone_first[i].scale);
        vec4_assign(bones[i].restingRotation, bone_first[i].rotate);
        vec3_assign(bones[i].restingPosition, bone_first[i].translate);
        bones[i].name = bone_first[i].name;
        bones[i].parent = bone_first[i].parent;
    }

#undef BAILOUT

    // Set flags and model variables
    model->flags = MODEL_OWNS_VERTICIES_BIT;
    model->vertices = verts; model->vertexCount = numv;
    model->meshes = meshes;  model->meshCount = numm;
    model->triangles = triangles; model->triangleCount = numt;
    model->frames = poses; model->frameCount = nump;
    model->animations = animations; model->animationCount = numa;
    model->bones = bones; model->boneCount = numb;
    model->textSize = ssize; model->textData = textdata;

    free(data);
    return 0;
}

int model_loadresource(Model * model, const char * resource) {
    return model_loadfile(model, platform_res2file_ez(resource));
}

void model_deinit(Model * model) {
    if (model->flags & MODEL_OWNS_VERTICIES_BIT) {
        free(model->vertices);
    }
    if (model->flags & MODEL_OWNS_TRIANGLES_BIT) {
        free(model->triangles);
    }
    if (model->flags & MODEL_OWNS_BONES_BIT) {
        free(model->bones);
    }
    if (model->flags & MODEL_OWNS_ANIMATIONS_BIT) {
        free(model->animations);
    }
    if (model->flags & MODEL_OWNS_MATERIALS_BIT) {
        free(model->materials);
    }
    if (model->flags & MODEL_OWNS_TEXT_BIT) {
        free(model->textData);
    }
    if (model->flags & MODEL_OWNS_MESHES_BIT) {
        free(model->meshes);
    }
}

void model_drawdebug(Model * model) {

}

ModelInstance * model_instance(Model * model, ModelInstance * instance) {

    return instance;
}

void modeli_deinit(ModelInstance * instance) {

}

void modeli_set_animation(ModelInstance * instance) {

}

void modeli_update(ModelInstance * instance, float dt) {

}

void modeli_drawdebug(ModelInstance * instance) {

}
