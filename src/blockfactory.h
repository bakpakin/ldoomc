#ifndef BLOCKFACTORY_H_TWKIKJNB
#define BLOCKFACTORY_H_TWKIKJNB

#include "mesh.h"
#include "ldmath.h"

typedef struct {
    unsigned sides;
    float height;
    float * points;
} BlockMeshTemplate;

BlockMeshTemplate * bfactory_init_tpl(BlockMeshTemplate * tpl, float height, unsigned sides, float * base);

void bfactory_tomesh(Mesh * mesh, BlockMeshTemplate * tpl);

#endif /* end of include guard: BLOCKFACTORY_H_TWKIKJNB */
