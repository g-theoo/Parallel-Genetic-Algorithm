//
// Created by Grigoras Theodor on 02.11.2021.
//
#include "sack_object.h"
#include "individual.h"

#ifndef TEMA1_HELPER_H
#define TEMA1_HELPER_H

typedef struct args {
    int id;
    sack_object *objects;
    int object_count;
    int sack_capacity;
    int generations_count;
    int number_of_threads;
//    individual *current_generation;
//    individual *next_generation;
//    pthread_barrier_t barrier;
//    pthread_mutex_t *mutex;
} my_args;

#endif //TEMA1_HELPER_H
