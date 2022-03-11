#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "genetic_algorithm_par.h"

int main(int argc, char *argv[]) {

    // array with all the objects that can be placed in the sack
    sack_object *objects = NULL;

    // number of objects
    int object_count = 0;

    // maximum weight that can be carried in the sack
    int sack_capacity = 0;

    // number of generations
    int generations_count = 0;

    // number of threads
    int number_of_threads = 0;

    if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &number_of_threads, argc, argv)) {
        return 0;
    }

    init(number_of_threads, object_count);

    pthread_t threads[number_of_threads];

    int r;
    my_args args[number_of_threads];

    for (int i = 0; i < number_of_threads; i++) {
        args[i].id = i;
        args[i].object_count = object_count;
        args[i].generations_count = generations_count;
        args[i].objects = objects;
        args[i].sack_capacity = sack_capacity;
        args[i].number_of_threads = number_of_threads;
        r = pthread_create(&threads[i], NULL, run_genetic_algorithm, &args[i]);
        if (r) {
            printf("Eroare la crearea thread-ului %d\n", i);
            exit(-1);
        }
    }

    for (int i = 0; i < number_of_threads; i++) {
        r = pthread_join(threads[i], NULL);

        if (r) {
            printf("Eroare la asteptarea thread-ului %d\n", i);
            exit(-1);
        }
    }

    free_resources();
    free(objects);

    return 0;
}