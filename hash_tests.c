#include "hashmap.h"
#include <stdlib.h>
#include <stdio.h>

/* All keys will be size_t
Values also size_t
*/

size_t hash(void* key){

    size_t casted_key = *((size_t*)key);

    size_t hash = casted_key * 44;

    return hash;
}

int compare(void* k1, void* k2){

    size_t key1 = *((size_t*)k1);
    size_t key2 = *((size_t*)k2);

    if(k1 == k2){
        return 1;
    } else {
        return 0;
    }


}

void key_dest(void* k){

    // free(k);

}

void value_dest(void* v){

    // free(v);


}




int main(){

    printf("Hello from hashtests\n");

    size_t (*hashptr)(void*);
    hashptr = &hash;

    int (*cmpptr)(void*, void*);
    cmpptr = &compare;

    void (*keydest)(void*);
    keydest = &key_dest;

    void (*valdest)(void*);
    valdest = &value_dest;

    
    struct hash_map* map = hash_map_new(hashptr, cmpptr, keydest, valdest);

    size_t key = 456732;
    size_t value = 12;

    hash_map_put_entry_move(map, &key, &value);

    size_t* res = hash_map_get_value_ref(map, &key);
    if(res == NULL){
        printf("res is null\n");
    }
    printf("Above printing result\n");
    printf("Retrieved %lu\n", *res);   

    hash_map_destroy(map);

    printf("Bottom of hashtests\n");


}