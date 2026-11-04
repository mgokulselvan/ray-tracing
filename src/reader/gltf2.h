#ifndef GLTF2READER_H
#define GLTF2READER_H

#include "../external/tiny_gltf/tiny_gltf_v3.h"

#include "../core/hittable_list.h"
#include "../core/camera.h"
#include "../shape/tri.h"
#include <cstdio>
#include <stdexcept>

tg3_model* gltf2_open(const char* filepath) {
    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model *model = new tg3_model;

    tg3_parse_options_init(&opts);
    tg3_error_stack_init(&errors);

    tg3_error_code err = tg3_parse_file(model, &errors, filepath,
                                        10, &opts);
    if (err != TG3_OK) {
        for (uint32_t i = 0; i < errors.count; i++) {
            fprintf(stderr, "[%d] %s\n", (int)errors.entries[i].severity,
                    errors.entries[i].message ? errors.entries[i].message : "(null)");
        }
        tg3_model_free(model);
        tg3_error_stack_free(&errors);
        throw std::runtime_error("error reading gltf2 file");
    }

    return model;
}

void gltf2_close(tg3_model *model) {
    tg3_model_free(model);
}


#endif
