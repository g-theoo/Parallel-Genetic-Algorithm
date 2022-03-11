#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm_par.h"

pthread_barrier_t barrier;
pthread_mutex_t mutex;
individual *current_generation;
individual *next_generation;
individual *tmp;

void init(int number_of_threads, int object_count){
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&barrier, NULL, number_of_threads);
    current_generation = (individual *) calloc(object_count, sizeof(individual));
    next_generation = (individual *) calloc(object_count, sizeof(individual));
    tmp = NULL;
}

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count,
               int *number_of_threads, int argc, char *argv[])
{
    FILE *fp;

    if (argc < 4) {
        fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count number_of_threads\n");
        return 0;
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        return 0;
    }

    if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
        fclose(fp);
        return 0;
    }

    if (*object_count % 10) {
        fclose(fp);
        return 0;
    }

    sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

    for (int i = 0; i < *object_count; ++i) {
        if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
            free(objects);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);

    *generations_count = (int) strtol(argv[2], NULL, 10);
    *number_of_threads = (int) strtol(argv[3], NULL, 10);

    if (*generations_count == 0 || *number_of_threads == 0) {
        free(tmp_objects);

        return 0;
    }

    *objects = tmp_objects;

    return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
    for (int i = 0; i < object_count; ++i) {
        printf("%d %d\n", objects[i].weight, objects[i].profit);
    }
}

void print_generation(const individual *generation, int limit)
{
    for (int i = 0; i < limit; ++i) {
        for (int j = 0; j < generation[i].chromosome_length; ++j) {
            printf("%d ", generation[i].chromosomes[j]);
        }

        printf("\n%d - %d\n", i, generation[i].fitness);
    }
}

void print_best_fitness(const individual *generation)
{
    printf("%d\n", generation[0].fitness);
}

int min(int a, int b){
    if (a < b) return a;
    return b;
}

void compute_fitness_function(int ID, int number_of_threads, const sack_object *objects, individual *generation, int object_count, int sack_capacity)
{
    int weight;
    int profit;

    int start = ID * (double)object_count / number_of_threads;
    int end = min((ID + 1) * (double)object_count / number_of_threads, object_count);

    for (int i = start; i < end; ++i) {
        weight = 0;
        profit = 0;

        for (int j = 0; j < generation[i].chromosome_length; ++j) {
            if (generation[i].chromosomes[j]) {
                weight += objects[j].weight;
                profit += objects[j].profit;
            }
        }
        generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
    }
}

int cmpfunc(const void *a, const void *b)
{
    int i;
    individual *first = (individual *) a;
    individual *second = (individual *) b;

    int res = second->fitness - first->fitness; // decreasing by fitness
    if (res == 0) {
        int first_count = 0, second_count = 0;

        for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i) {
            first_count += first->chromosomes[i];
            second_count += second->chromosomes[i];
        }

        res = first_count - second_count; // increasing by number of objects in the sack
        if (res == 0) {
            return second->index - first->index;
        }
    }

    return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
    int i, mutation_size;
    int step = 1 + generation_index % (ind->chromosome_length - 2);

    if (ind->index % 2 == 0) {
        // for even-indexed individuals, mutate the first 40% chromosomes by a given step
        mutation_size = ind->chromosome_length * 4 / 10;

        for (i = 0; i < mutation_size; i += step) {
            ind->chromosomes[i] = 1 - ind->chromosomes[i];
        }
    } else {
        // for even-indexed individuals, mutate the last 80% chromosomes by a given step
        mutation_size = ind->chromosome_length * 8 / 10;

        for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
            ind->chromosomes[i] = 1 - ind->chromosomes[i];
        }
    }
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
    int step = 1 + generation_index % (ind->chromosome_length - 2);

    // mutate all chromosomes by a given step
    for (int i = 0; i < ind->chromosome_length; i += step) {
        ind->chromosomes[i] = 1 - ind->chromosomes[i];
    }
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
    individual *parent2 = parent1 + 1;
    individual *child2 = child1 + 1;
    int count = 1 + generation_index % parent1->chromosome_length;

    memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
    memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

    memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
    memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
    memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(int ID, int number_of_threads, individual *generation)
{
    int i;
    int start, end;

    start = ID * (double)generation->chromosome_length / number_of_threads;
    end = min((ID + 1) * (double)generation->chromosome_length / number_of_threads, generation->chromosome_length);

    for (i = start; i < end; ++i) {
        free(generation[i].chromosomes);
        generation[i].chromosomes = NULL;
        generation[i].fitness = 0;
    }
}

void merge(int low, int mid, int high)
{
    int i, j;
    int k = low;
    int left_size = mid - low;
    int right_size = high - mid;

    individual *left = malloc(left_size * sizeof(individual));
    individual *right = malloc(right_size * sizeof(individual));

    for (i = 0; i < left_size; i++) {
        left[i] = current_generation[i + low];
    }

    for (i = 0; i < right_size; i++) {
        right[i] = current_generation[i + mid];
    }

    i = 0;
    j = 0;

    while (i < left_size && j < right_size) {
        if (cmpfunc(&left[i], &right[j]) > 0) {
            current_generation[k++] = right[j++];
        }
        else {
            current_generation[k++] = left[i++];
        }
    }

    while (i < left_size) {
        current_generation[k++] = left[i++];
    }

    while (j < right_size) {
        current_generation[k++] = right[j++];
    }

    free(left);
    free(right);
}

void free_resources() {
    free(current_generation);
    free(next_generation);
}

void* run_genetic_algorithm(void* arg)
{
    my_args args = *(my_args *)arg;
    sack_object *objects = args.objects;
    int object_count = args.object_count;
    int sack_capacity = args.sack_capacity;
    int generations_count = args.generations_count;
    int number_of_threads = args.number_of_threads;
    int ID = args.id;
    static int swap = 0;

    int start, end, mid;
    int count, cursor;

    // set initial generation (composed of object_count individuals with a single item in the sack)
    {
        start = ID * (double) object_count / number_of_threads;
        end = min((ID + 1) * (double) object_count / number_of_threads, object_count);

        for (int i = start; i < end; ++i) {
            current_generation[i].fitness = 0;
            current_generation[i].chromosomes = (int *) calloc(object_count, sizeof(int));
            current_generation[i].chromosomes[i] = 1;
            current_generation[i].index = i;
            current_generation[i].chromosome_length = object_count;

            next_generation[i].fitness = 0;
            next_generation[i].chromosomes = (int *) calloc(object_count, sizeof(int));
            next_generation[i].index = i;
            next_generation[i].chromosome_length = object_count;
        }
    }
    pthread_barrier_wait(&barrier);

    //iterate for each generation
    for (int k = 0; k < generations_count; ++k) {
        swap = 1;
        cursor = 0;

        // compute fitness and sort by it
        {
            // fitness
            compute_fitness_function(ID, number_of_threads, objects, current_generation, object_count, sack_capacity);
            pthread_barrier_wait(&barrier);

            // sorting
            start = ID * (double)object_count / number_of_threads;
            end = min((ID + 1) * (double)object_count / number_of_threads, object_count);
            qsort(current_generation + start, end - start, sizeof(individual), cmpfunc);
            pthread_barrier_wait(&barrier);

            start = 0;

            if (ID == 1){
                mid = ID * (double)object_count / number_of_threads;
                merge(start, mid, end);
            }
            pthread_barrier_wait(&barrier);

            if (ID == 2){
                mid = ID * (double)object_count / number_of_threads;
                merge(start, mid, end);
            }
            pthread_barrier_wait(&barrier);
            if (ID == 3){
                mid = ID * (double)object_count / number_of_threads;
                merge(start, mid, end);
            }
            pthread_barrier_wait(&barrier);
        }

        // keep first 30% children (elite children selection)
        {
            count = object_count * 3 / 10;
            start = ID * (double) count / number_of_threads;
            end = min((ID + 1) * (double) count / number_of_threads, count);

            for (int i = start; i < end; ++i) {
                copy_individual(current_generation + i, next_generation + i);
            }

            pthread_barrier_wait(&barrier);
        }

        // mutate first 20% children with the first version of bit string mutation
        {
            cursor = count;
            count = object_count * 2 / 10;

            start = ID * (double) count / number_of_threads;
            end = min((ID + 1) * (double) count / number_of_threads, count);

            for (int i = start; i < end; ++i) {
                copy_individual(current_generation + i, next_generation + cursor + i);
                mutate_bit_string_1(next_generation + cursor + i, k);
            }
            pthread_barrier_wait(&barrier);
        }


        // mutate next 20% children with the second version of bit string mutation
        {
            cursor += count;
            count = object_count * 2 / 10;

            start = ID * (double) count / number_of_threads;
            end = min((ID + 1) * (double) count / number_of_threads, count);

            for (int i = start; i < end; ++i) {
                copy_individual(current_generation + i + count, next_generation + cursor + i);
                mutate_bit_string_2(next_generation + cursor + i, k);
            }
            pthread_barrier_wait(&barrier);
        }

        // crossover first 30% parents with one-point crossover
        {
            cursor += count;
            count = object_count * 3 / 10;

            if (count % 2 == 1) {
                copy_individual(current_generation + object_count - 1, next_generation + cursor + count - 1);
                count--;
            }

            pthread_barrier_wait(&barrier);

            for (int i = 0; i < count; i += 2) {
                crossover(current_generation + i, next_generation + cursor + i, k);
            }
            pthread_barrier_wait(&barrier);
        }

        // switch to new generation
        {
            pthread_mutex_lock(&mutex);
            if (swap == 1) {
                swap = 0;
                tmp = current_generation;
                current_generation = next_generation;
                next_generation = tmp;
            }
            pthread_mutex_unlock(&mutex);

            pthread_barrier_wait(&barrier);
        }

        // update generation
        {
            start = ID * (double) object_count / number_of_threads;
            end = min((ID + 1) * (double) object_count / number_of_threads, object_count);

            for (int i = start; i < end; ++i) {
                current_generation[i].index = i;
            }
            pthread_barrier_wait(&barrier);
        }

        if (ID == 0) {
            if (k % 5 == 0) {
                print_best_fitness(current_generation);
            }
        }
    }

    //compute fitness for the last generation
    {
        compute_fitness_function(ID, number_of_threads, objects, current_generation, object_count, sack_capacity);
        pthread_barrier_wait(&barrier);
    }

    // sort the last generation by fitness
    {
        start = ID * (double) object_count / number_of_threads;
        end = min((ID + 1) * (double) object_count / number_of_threads, object_count);

        qsort(current_generation + start, end - start, sizeof(individual), cmpfunc);
        pthread_barrier_wait(&barrier);

        start = 0;

        if (ID == 1) {
            mid = ID * (double) object_count / number_of_threads;
            merge(start, mid, end);
        }
        pthread_barrier_wait(&barrier);

        if (ID == 2) {
            mid = ID * (double) object_count / number_of_threads;
            merge(start, mid, end);
        }
        pthread_barrier_wait(&barrier);
        if (ID == 3) {
            mid = ID * (double) object_count / number_of_threads;
            merge(start, mid, end);
        }
        pthread_barrier_wait(&barrier);
    }

    if (ID == 0) {
        print_best_fitness(current_generation);
    }

    // free resources for old generation
    free_generation(ID, number_of_threads, current_generation);
    free_generation(ID, number_of_threads, next_generation);

    pthread_exit(NULL);
}

