// Implements a dictionary's functionality
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <strings.h>
#include <stdlib.h>
#include "dictionary.h"

// Represents a node in a hash table
typedef struct node
{
    char word[LENGTH + 1];
    struct node *next;
} node;

// Number of buckets in hash table
// Bigger = faster lookup, more memory. 65536 is a good balance.
#define N 65536
unsigned int word_count = 0;

// Hash table
node *table[N];

// Returns true if word is in dictionary, else false
bool check(const char *word)
{
    int i = hash(word);
    node *n = table[i];
    while (n) {
        if (strcasecmp(n->word, word) == 0)
            return true;
        n = n->next;
    }
    return false;
}

// Hashes word to a number
// djb2 hash, source: http://www.cse.yorku.ca/~oz/hash.html
unsigned int hash(const char *word)
{
    unsigned long h = 5381;
    int c;
    while ((c = tolower(*word++)))
        h = ((h << 5) + h) + c;  // h * 33 + c
    return h % N;
}

// Loads dictionary into memory, returning true if successful, else false
bool load(const char *dictionary)
{
    FILE *fp = fopen(dictionary, "r");
    if (fp == NULL) {
        return false;
    }

    node *n = (node*)malloc(sizeof(node));
    while (fscanf(fp, "%s", n->word) != EOF) {
        n->next = table[hash(n->word)];
        table[hash(n->word)] = n; // 前插node
        word_count++;
        n = malloc(sizeof(node));
    }
    free(n);
    fclose(fp);
    return true;
}

// Returns number of words in dictionary if loaded, else 0 if not yet loaded
unsigned int size(void)
{
    return word_count;
}

// Unloads dictionary from memory, returning true if successful, else false
bool unload(void)
{
    for (int i = 0; i < N; i++) {
        node *p = table[i];
        while (p) {
            node *q = p->next;
            free(p);
            p = q;
        }
    }
    return true;
}
