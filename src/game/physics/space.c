#include "game/physics/space.h"
#include "game/physics/intersect.h"
#include "game/physics/collisions.h"
#include "utils/log.h"
#include <stdlib.h>

physics_space *global_space = NULL;

void physics_space_init() {
    global_space = malloc(sizeof(physics_space));
    vector_create(&global_space->objects, sizeof(object*));
}

void physics_space_tick() {
    iterator it;
    object *a, *b, **t;
    vec2i apos,bpos;
    unsigned int size;

    // Handle movement
    vector_iter_begin(&global_space->objects, &it);
    while((t = iter_next(&it)) != NULL) {
        a = *t;
        if(!a->is_static) {
            a->pos.x += a->vel.x;
            a->pos.y += a->vel.y;
            a->vel.y += a->gravity;
        }
    }
    
    // Check for collisions
    size = vector_size(&global_space->objects);
    for(int i = 0; i < size; i++) {
        a = *((object**)vector_get(&global_space->objects, i));
        for(int k = i+1; k < size; k++) {
            b = *((object**)vector_get(&global_space->objects, k));
            if((a->group != b->group || a->group == OBJECT_NO_GROUP || b->group == OBJECT_NO_GROUP) && 
                a->layers & b->layers) {

                apos = vec2f_to_i(a->pos);
                bpos = vec2f_to_i(b->pos);
                if(a->col_shape_hard != NULL && b->col_shape_hard != NULL) {
                    if(shape_intersect(a->col_shape_hard, apos, 
                                       b->col_shape_hard, bpos)) {
                        collision_handle(a, b);
                    }
                }
                if(a->col_shape_soft != NULL && b->col_shape_soft != NULL) {
                    if(shape_intersect(a->col_shape_soft, apos, b->col_shape_soft, bpos)) {
                        a->ev_collision(a,b,a->userdata,b->userdata);
                    }
                }
            }
        }
    }
}



void physics_space_add(object *obj) {
    vector_append(&global_space->objects, &obj);
}

void physics_space_remove(object *obj) {
    iterator it;
    object **o;
    vector_iter_begin(&global_space->objects, &it);
    while((o = iter_next(&it)) != NULL) {
        if(*o == obj) {
            vector_delete(&global_space->objects, &it);
        }
    }
}

void physics_space_close() {
    vector_free(&global_space->objects);
    free(global_space);
    global_space = NULL;
}
