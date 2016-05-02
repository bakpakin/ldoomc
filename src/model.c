#include "model.h"
#include "platform.h"

Model * model_loadfile(Model * model, const char * file) {

    return model;
}

Model * model_loadresource(Model * model, const char * resource) {
    return model_loadfile(model, platform_res2file_ez(resource));
}

void model_deinit(Model * model) {

}

ModelInstance * model_instance(Model * mode, ModelInstance * instance) {

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
