#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "hashmap.h"


void resize_map(struct hash_map* map);

struct bucket* search(struct hash_map* map, void* k);


// Returns a pointer to a struct hash_map
// Takes in 4 function pointer parameters
// Using the hash, comparison functions given. 
// Create a hash map to mape the keys and values
// No race condition here because it doesn't take in a struct hash_map*
// Compare function compares if two keys are the same
struct hash_map* hash_map_new(size_t (*hash)(void*), int (*cmp)(void*,void*),
    void (*key_destruct)(void*), void (*value_destruct)(void*)) {

	if(hash == NULL || cmp == NULL || key_destruct == NULL || value_destruct == NULL){
		return NULL;
	}

	// Create a hash_map struct that stores all of these function pointers
	// The hash map is returned, and other function will have access to these function pointers
	// As well as any memory I allocate into it
	struct hash_map* map_ptr = (struct hash_map*)malloc(sizeof(struct hash_map));

	map_ptr->hash = hash;
	map_ptr->compare = cmp;
	map_ptr->key_destruct = key_destruct;
	map_ptr->value_destruct = value_destruct;

	map_ptr->map_cap = STARTING_CAP;
	map_ptr->map_size = 0;

	// map_ptr->hmap = (struct bucket**)malloc(sizeof(struct bucket*) * STARTING_CAP);
	map_ptr->hmap = (struct bucket**)calloc(STARTING_CAP, sizeof(struct bucket*));

	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&map_ptr->map_lock, &attr);

	// pthread_mutex_init(&(map_ptr->map_lock), NULL);


	return map_ptr;
}


/*
Puts an entry in the hash map, associating a key with a value
if the key does not exist, a new entry should be created
If an entry exists, the old netry should be removed using hash_map_remove_entry
*/
void hash_map_put_entry_move(struct hash_map* map, void* k, void* v) {

	if(map == NULL || k == NULL || v == NULL){
		return;
	}

	pthread_mutex_lock(&map->map_lock);

	// MARK: Perform resize and repositioning here if the map_size > 0.75* map_cap
	// if(map->map_size >= 0.75*map->map_cap){
	// 	resize_map(map);
	// }

	void* result = search(map, k);

	// Hash the key using given hash function
	size_t hash_value = map->hash(k);

	// Modulo the hash value by the map capacity to get the index to store in hmap
	size_t index = hash_value % map->map_cap;

	if(result == NULL){
		// An entry with identical key was not found
		// We will be inserting this as a new entry into the hash map

		// Create a struct bucket to contain the k and v passed in
		struct bucket* buck_ptr = (struct bucket*)calloc(1, sizeof(struct bucket));
		buck_ptr->key = k;
		buck_ptr->value = v;
		buck_ptr->index = index;
		buck_ptr->next_buck = NULL;

		// Check whether the hash index is null
		if(map->hmap[index] == NULL){
			// There hasn't been an identical key hash stored here thus far

			// We can place the bucket at the given pointer
			buck_ptr->prev_buck = NULL;

			// dest_bucket = buck_ptr;
			// memcpy(&dest_bucket, &buck_ptr, sizeof(struct bucket*));
			map->hmap[index] = buck_ptr;

			map->map_size++;

		} else {

			// A key also has this hash value
			// Check next_buck iteratively until we reach end of the list

			struct bucket* cursor = map->hmap[index];

			while(cursor->next_buck != NULL){

				cursor = cursor->next_buck;

			}

			// At the bottom of this loop, dest_bucket->next_buck should be null
			// Which is where we will be storing the buck_ptr
			buck_ptr->prev_buck = cursor;
			cursor->next_buck = buck_ptr;


		}



	} else {
		
		// This key already exists in the hmap
		// In this case, destruct / free the old stored_val
		// And replace it with the new value

		/* One way to perform the value replacing operation here
		is by calling our hash_remove function
		then call our own put function again, however, this time it will no 
		longer exist in our hash map so it will 'put' successfully
		*/


		hash_map_remove_entry(map, k);

		hash_map_put_entry_move(map, k, v);

	}


	pthread_mutex_unlock(&map->map_lock);

	
}

// Call the destruct functions we are given
void hash_map_remove_entry(struct hash_map* map, void* k) {

	if(map == NULL || k == NULL){
		return;
	}

	pthread_mutex_lock(&map->map_lock);

	struct bucket* result = search(map, k);

	if(result == NULL){

		// No entry with given key
		pthread_mutex_unlock(&map->map_lock);
		return;

	} else {


		// Remove key and value from the map
		map->key_destruct(result->key);
		map->value_destruct(result->value);

		// Organise bucket linked_list before freeing result bucket

		// Three scenarios to handle

		if(result->next_buck == NULL){
			// If result bucket is at the end of the list
			struct bucket* prev = result->prev_buck;

			
			if(prev != NULL){
				// Remove current bucket from previous bucket's next_buck ptr
				prev->next_buck = NULL;
			} else {
				// Handle if its also the only item on the list
				// Remove pointer from hmap list
				map->hmap[result->index] = NULL;
			}

			free(result);

		} else if (result->prev_buck == NULL){
			// If result bucket is at the head of the list
			// No need to handle being only item in list scenario because it's been handled above

			// have the hashmap point to the next bucket instead
			map->hmap[result->index] = result->next_buck;

			// Set the prev_buck to NULL to indicate it being the first bucket
			result->next_buck->prev_buck = NULL;

			free(result);

		} else {
			// Otherwise, it should mean that the result bucket is somewhere in the middle
			// With both a next_buck and a prev_buck

			struct bucket* prev = result->prev_buck;
			struct bucket* next = result->next_buck;

			prev->next_buck = next;
			next->prev_buck = prev;

			free(result);

		}




	}

	pthread_mutex_unlock(&map->map_lock);
	
}

void* hash_map_get_value_ref(struct hash_map* map, void* k) {



	if(map == NULL || k == NULL){
		return NULL;
	}

	pthread_mutex_lock(&map->map_lock);

	struct bucket* result = search(map, k);

	if(result == NULL){

		// No entry with given key
		return NULL;

	} else {

		return result->value;

	}

	pthread_mutex_unlock(&map->map_lock);

}


void hash_map_destroy(struct hash_map* map) {

	if(map == NULL){
		return;
	}

	struct bucket** old_map = map->hmap;

	// For each slot in the hashmap
	for(size_t i = 0; i < map->map_cap; i++){

		struct bucket* cursor = old_map[i];

		if(cursor == NULL){
			// The current hash slot is empty
			// Move onto next hash slot
			continue;

		} else {

			// An element exists at this slot
			// Use put function to remap into a new slot
			while(cursor != NULL){

				map->key_destruct(cursor->key);
				map->value_destruct(cursor->value);

				// Let cursor point at the next bucket in the linked_list
				// If no more elements with the same hash exist,
				// It will break from while loop
				struct bucket* used_bucket = cursor;
				cursor = cursor->next_buck;

				// Free the bucket
				free(used_bucket);
			}
			

		}

	
	}
		
	free(old_map);

	free(map);

	
}


// HELPER FUNCTIONS:
void resize_map(struct hash_map* map){

	// Malloc new space for the map to have double the map_cap
	struct bucket** new_space = (struct bucket**)calloc(map->map_cap*2, sizeof(struct bucket*));

	// Variables to help iterate through all existing buckets
	struct bucket** old_map = map->hmap;
	map->hmap = new_space;

	// size_t old_size = map->map_size;
	map->map_size = 0;

	map->map_cap *= 2;

	// Rehash all existing elements in the old map, and remap it into the new_space
	// For each slot in the hashmap
	for(size_t i = 0; i < map->map_cap/2; i++){

		struct bucket* cursor = old_map[i];

		if(cursor == NULL){
			// The current hash slot is empty
			// Move onto next hash slot
			continue;
		} else {

			// An element exists at this slot
			// Use put function to remap into a new slot
			while(cursor != NULL){

				hash_map_put_entry_move(map, cursor->key, cursor->value);

				// Let cursor point at the next bucket in the linked_list
				// If no more elements with the same hash exist,
				// It will break from while loop
				struct bucket* used_bucket = cursor;
				cursor = cursor->next_buck;

				// Free up the bucket because it has been recreated and remapped
				free(used_bucket);
			}
			

		}

	
	}
		
	// Free up the previous hash map
	free(old_map);



}

struct bucket* search(struct hash_map* map, void* k){
	
		// Hash the key using given hash function
	size_t hash_value = map->hash(k);
	
	// Modulo the hash value by the map capacity to get the index to store in hmap
	size_t index = hash_value % map->map_cap;

	// Store the pointer to the first bucket with given hash index
	struct bucket* cursor = map->hmap[index];


	while(cursor != NULL){

		if(map->compare(k, cursor->key)){

			// We found an entry with the same key

			return cursor;

		}

		cursor = cursor->next_buck;

	}


	return NULL;
}

