#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct person {
    struct person *parent[2];
    char alleles[2];
} person;

person* create_family(int generation);

char random_allele();

void free_family(person *p);

void print_family(person *p, int generation);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: ./inheritance generations\n");
        return 1;
    }
    srand(time(NULL));
    int generations = atoi(argv[1]);

    person *p = create_family(generations);
    print_family(p, 0);
    free_family(p);
}

person* create_family(int generation)
{
    person *p = (person*)malloc(sizeof(person));
    if (generation == 1) {
        p->parent[0] = p->parent[1] = NULL;
        p->alleles[0] = random_allele();
        p->alleles[1] = random_allele();
    } else {
        p->parent[0] = create_family(generation - 1);
        p->parent[1] = create_family(generation - 1);
        p->alleles[0] = p->parent[0]->alleles[rand() % 2];
        p->alleles[1] = p->parent[1]->alleles[rand() % 2];
    }
    return p;
}

char random_allele()
{
    char alleles[] = {'A', 'B', 'O'};
    int n = rand() % 3;
    return alleles[n];
}

void free_family(person *p)
{
    if (p != NULL) {
        free_family(p->parent[0]);
        free_family(p->parent[1]);
        free(p);
    }
    return;
}

void print_family(person *p, int generation)
{
    if (p == NULL) return;

    for (int i = 0; i < generation; i++)
        printf("    ");

    if (generation == 0)
        printf("Child");
    else if (generation == 1)
        printf("Parent");
    else
        printf("Grandparent");

    printf(" (Generation %i): blood type %c%c\n", generation, p->alleles[0], p->alleles[1]);

    print_family(p->parent[0], generation + 1);
    print_family(p->parent[1], generation + 1);
}