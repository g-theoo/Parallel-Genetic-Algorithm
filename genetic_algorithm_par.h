//
// Created by Grigoras Theodor on 31.10.2021.
//

#ifndef APD_GENETIC_ALGORITHM_PAR_H
#define APD_GENETIC_ALGORITHM_PAR_H

#include "sack_object.h"
#include "individual.h"
#include "helper.h"
#include <pthread.h>

void init(int number_of_threads, int object_count);
// reads input from a given file
int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count,
               int *number_of_threads , int argc, char *argv[]);

// displays all the objects that can be placed in the sack
void print_objects(const sack_object *objects, int object_count);

// displays all or a part of the individuals in a generation
void print_generation(const individual *generation, int limit);

// displays the individual with the best fitness in a generation
void print_best_fitness(const individual *generation);

// get the minimun of two numbers;
int min(int a, int b);

// computes the fitness function for each individual in a generation
void compute_fitness_function(int ID, int number_of_threads, const sack_object *objects, individual *generation, int object_count, int sack_capacity);

// compares two individuals by fitness and then number of objects in the sack (to be used with qsort)
int cmpfunc(const void *a, const void *b);

// performs a variant of bit string mutation
void mutate_bit_string_1(const individual *ind, int generation_index);

// performs a different variant of bit string mutation
void mutate_bit_string_2(const individual *ind, int generation_index);

// performs one-point crossover
void crossover(individual *parent1, individual *child1, int generation_index);

// copies one individual
void copy_individual(const individual *from, const individual *to);

// deallocates a generation
void free_generation(int ID, int number_of_threads, individual *generation);

// function to merge two arrays
void merge(int low, int mid, int high);

// free memory
void free_resources();

// runs the genetic algorithm
void* run_genetic_algorithm(void* arg);

#endif //APD_GENETIC_ALGORITHM_PAR_H
