# Hashy

The Thread Safe Kind
Note: You can ignore reference to size in put and get functions.

Your hash map will need to be able to store any type in conjunction with the function pointers provided during construction. You have been given an empty struct which you will need to complete. While the key and value can be any type, the hash and comparison functions must work on the key values. Use the hash, comparison and destruct function pointers to map data, resolve collisions and deallocate memory.

It is recommended that you use a dynamic array to support the entries within your hash map.

Keep in mind, You will also need to ensure that the hash map is thread safe and performant. You will need to apply fine grained locking to the data structure to ensure that the hash map can support concurrent insertions and removals. Retrieving and placing an element should be constant time on average ( O(1)O(1) ) operation.

In this instance, it is advised you construct a hash map where each entry mapped by a hash is its own list (separate chaining). If more than one distinct key results in the same hash, the list should contain these keys at the same location.

Note: Open-addressing will hinder your performance progress.
