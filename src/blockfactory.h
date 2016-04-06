#ifndef BLOCKFACTORY_H_TWKIKJNB
#define BLOCKFACTORY_H_TWKIKJNB

#include "mesh.h"
#include "ldmath.h"

#define BLOCKFACTORY_NOTOP 1
#define BLOCKFACTORY_NOBOT 2
#define BLOCKFACTORY_NOSIDES 4

typedef struct {
    unsigned sides;
    float height;
    float * points;
} BlockMeshTemplate;

BlockMeshTemplate * btpl_init(BlockMeshTemplate * tpl, float height, unsigned sides, float * base);

void btpl_deinit(BlockMeshTemplate * tpl);

void bfactory_tomesh(BlockMeshTemplate * tpl, Mesh * mesh, unsigned flags);

#endif /* end of include guard: BLOCKFACTORY_H_TWKIKJNB */
