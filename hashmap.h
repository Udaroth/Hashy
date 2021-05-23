#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>

#define STARTING_CAP 32


struct bucket {

    void* key;
    void* value;
    // Index of bucket on hashmap
    size_t index;
    struct bucket* next_buck;
    struct bucket* prev_buck;

};

struct hash_map {

    // Function pointers
    size_t (*hash)(void*);
    int (*compare)(void*, void*);
    void (*key_destruct)(void*);
    void (*value_destruct)(void*);

    // Memory Allocation for the hash map

    size_t map_cap;
    size_t map_size;

    pthread_mutex_t map_lock;
    
    struct bucket** hmap;



}; //Modify this!

struct hash_map* hash_map_new(size_t (*hash)(void*), int (*cmp)(void*,void*),
    void (*key_destruct)(void*), void (*value_destruct)(void*));

void hash_map_put_entry_move(struct hash_map* map, void* k, void* v);

void hash_map_remove_entry(struct hash_map* map, void* k);

void* hash_map_get_value_ref(struct hash_map* map, void* k);

void hash_map_destroy(struct hash_map* map);

#endif
