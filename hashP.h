/* hashP.h -- private header file for hash tables.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Wed Nov 10 14:19:06 EET 2021
 * Copyright: (C) 2021 Luis Colorado.  All rights reserved.
 * License: BSD.
 */
#ifndef _HASHP_H
#define _HASHP_H

#include "hash.h"

struct ht_entryP {
	struct ht_entry ht_pub;
	struct ht_entryP *ht_next;
};

/* hash stores it's actual number of elements, as counting them
 * would be inefficient.  Then the array length is stored.  Then
 * an array of pointers (and also head pointers of the linked
 * list) appear as the list of entries with the same hash value.
 * Internally the entries are also organized as a linked list of
 * entries with the same key value. The key values are not
 * dup()ed so they should be considered readonly, and living all
 * the time the keys are used in the hash table. */
struct ht_hash {
	size_t (* ht_hash_f)(const char *key);
								/* hash function */
	int    (* ht_equals_f)(const char *key_a, const char *key_b);
								/* equals function */
	struct ht_entryP **ht_array; /* pointer to array of entries */
	size_t ht_size;				/* number of elements in table */
	size_t ht_capacity;			/* number of array entries,
								 * should be prime number */
	size_t ht_collisions;		/* number of collisions */
};

#endif /* _HASHP_H */
