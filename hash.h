/* Definitions for a hash table.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Wed Nov 10 10:35:34 EET 2021
 * Copyright: (C) 2021 Luis Colorado.  All rights reserved.
 * License: BSD.
 */
#ifndef _HASH_H /* double inclussion guard, don't remove. */
#define _HASH_H /* ... */
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h> /* size_t is defined here */
#include <stdio.h> /* for FILE */

/* this type is opaque to avoid the user to access the main hash
 * table fields.  Look in hashP.h for the definitions of this
 * structure. */
typedef struct ht_hash ht_hash_t;

/* this is an entry of the table, it handles a pointer to the
 * key, a pointer to the value, and a pointer to the next entry, in
 * case you allow more than an entry (in the PUSH_POP) semantics.
 * This will allow you to access (through the next field) the hidden
 * values under the same key, in the case of a PUSH_POP semantics.
 */
typedef struct ht_entry {
    const char *ht_key;     /* the key of the entry */
    void       *ht_val;     /* the value of the entry's element */
} ht_entry;

/* hash table constructor.  Creates a hash table with given initial
 * capacity, hash function and equal function. */
ht_hash_t *
ht_new(
        const size_t initial_cap,
        size_t (*hash_f)(const char *key),
        int (*equals_f)(const char *key_a, const char *key_b),
        size_t (*sizeof_f) (const char *key));

/* clear hash_table */
void ht_clear(
        ht_hash_t *table);

/* deallocator for the hash table. The hash table to deallocate
 * must have been allocated with new_hash() function.  After freeing
 * a hash table the pointer reference is no longer valid and using it
 * in any of the functions defined here results in undefined behaviour.
 * */
void
ht_delete(
        ht_hash_t *table); /* hash table to be freed. */

/* getter for an entry of the hash table.  It returns an structure
 * with the pair of key and value stored in the hash table.  The
 * returned structure should not be modified (it is marked as const
 * for that reason) as it is the internal structure to hold the pairs
 * of data. */
const ht_entry *
ht_get_entry(
        ht_hash_t *table, /* hash table */
        const char *key); /* key to be searched for */

/* getter for the hash table.  It returns the data reference corresponding
 * to the requested key.  */
void *
ht_get(
        ht_hash_t *tab, /* hash table */
        const char *key); /* key to be searched for */

/* setter function for the hash table. It sets an entry for given key on
 * given data value.  It returns the previous value stored for key or NULL,
 * in case there was none. */
void *
ht_put(ht_hash_t *tab, /* hash table */
        const char *key, /* key of data to be stored. */
        void *value); /* value of data to be stored */

/* remover for an entry.  Deletes the entry stored in given key and returns it.
 * It returns NULL in case no entry existed with such a key. */
void *
ht_remove(
        ht_hash_t *tab, /* hash table */
        const char *key); /* key corresponding to data to be removed */

/* returns the number of collisions actually in the table. A collision is when
 * two keys give the same index in the table and so lengthen the search for any
 * of the two keys that have the same entry.  Growing the table and selecting a
 * good hash function are key to have an efficient searching method. */
size_t
ht_get_collisions(
        ht_hash_t *tab); /* hash table */

/* gets the number of pairs stored in the hash table. */
size_t
ht_get_size(
        ht_hash_t *tab); /* hash table */

/* gets the number of cell entries in the array actually */
size_t
ht_get_capacity(
        ht_hash_t *tab); /* hash table */

/* gets the hash function pointer stored to the caller */
size_t (*
ht_get_hash_f(
        ht_hash_t *tab) /* hash table */
)(const char *key);

/* gets the equals function pointer stored to the caller. */
int (*
ht_get_equals_f(
        ht_hash_t *tab) /* hash table */
)(const char *key_a, const char *key_b);

/* prints the contents of the hash table to file descriptor f. */
size_t
ht_print(
        ht_hash_t *tab, /* hash table */
        FILE *f); /* file descriptor to output hash table. */

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _HASH_H */
