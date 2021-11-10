/* hash.c -- hash table implementation.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Wed Nov 10 14:38:17 EET 2021
 * Copyright: (C) 2021 Luis Colorado.  All rights reserved.
 * License: BSD.
 */
#include <assert.h>
#include <stdlib.h>

#include "hashP.h"

/* this function grows a hash table and reindexes all the entries
 * that were in the old array. */
static void
grow(
		hash_t *table,
		size_t new_size)
{
	/* 1.- save the old array and capacity. */
	struct ht_entryP **old_array = table->ht_array;
	size_t old_cap = table->ht_capacity;

	/* 2.- create the new array and populate it with NULLs.
	 * we use calloc() as it gives a NULL initialized array of
	 * pointers */
	table->ht_size = 0;
	table->ht_array = calloc(sizeof *table->ht_array, new_size);
	table->ht_capacity = new_size;
	table->ht_collisions = 0;
	if (old_array) {
		/* 3.- navigate the old array for all the original entries */
		for (int ix = 0; ix < old_cap; ix++) {
			struct ht_entryP * p = old_array[ix];
			while (p) {
				/* 4.- and put them in the new table */
				ht_put(table, p->ht_pub.ht_key, p->ht_pub.ht_val);
				/* 5.- unlink old entry */
				struct ht_entryP *q = p;
				p = p->ht_next;
				/* 6.- and free */
				free(q);
            }
        }
		/* 7.- and free old_array */
		free(old_array);
    }
} /* grow */

hash_t *
new_hash(
		const size_t initial_cap,
		size_t (*hash_f)(const void *key),
		int (*equals_f)(const void *key_a, const void *key_b),
		const void *(*dup_f)(const void *),
		void (*free_f)(const void *))
{
	hash_t *ret_val = malloc(sizeof *ret_val);
	if (!ret_val) {
		return NULL;
	}
	ret_val->ht_size = 0;
	ret_val->ht_capacity = 0;
	ret_val->ht_array = NULL;
	ret_val->ht_collisions = 0;
	ret_val->ht_hash_f = hash_f;
	ret_val->ht_equals_f = equals_f;
	ret_val->ht_dup_f = dup_f;
	ret_val->ht_free_f = free_f;
	/* this will just allocate the array for now. */
	grow(ret_val, initial_cap);
	return ret_val;
} /* new_hash */

/* deallocator for the hash table. The hash table to deallocate
 * must have been allocated with new_hash() function. */
void
free_hash(
		hash_t *table)
{
    for(size_t ix = 0; ix < table->ht_capacity; ix++) {
		/* free all entries at position ix in the array */
        struct ht_entryP *p = table->ht_array[ix];
		while (p) {
			struct ht_entryP *q = p; /* save pointer */
			p = p->ht_next; /* unlink before free() */
			if (q->ht_pub.ht_key)
				table->ht_free_f(q->ht_pub.ht_key);
			free(q);
			table->ht_size--; /* table size */
		}
	}
	assert(table->ht_size == 0); /* double check */
	free(table->ht_array); /* free the array */
	free(table);
} /* free_hash */

static size_t
getIndex(
        hash_t *table,
        const void *key)
{
	return table->ht_hash_f(key) % table->ht_capacity;
} /* getIndex */

/* gets a reference to the pointer that points to the entry whose
 * key is 'key'. This function is used in the remove() below.  */
static struct ht_entryP **
getPtrRef(
		hash_t *tab,
		struct ht_entryP **p,	/* the starting reference */
		const void *key)
{
	assert(p != NULL); /* double check */
	while(*p) { /* while the pointer referenced is not null */
		if (tab->ht_equals_f(key, (*p)->ht_pub.ht_key)) { /* found */
			return p;
		}
		/* update the reference to the address of the 'h_next'
		 * pointer. */
		p = &(*p)->ht_next;
	}
    assert(p != NULL && *p == NULL);
	return p;
}

/* gets the pointer to the entry whose key is 'key'.  We use
 * the above getPtrRef() to implement this.  */
static struct ht_entryP *
getRef(
		hash_t *tab,
		struct ht_entryP *p,
		const void *key)
{
	/* we use the above function */
	return *getPtrRef(tab, &p, key);
}

const struct ht_entry *
ht_get_entry(
		hash_t *table,
		const void *key)
{
	size_t ix = getIndex(table, key);
	struct ht_entryP *p = getRef(table, table->ht_array[ix], key);
    /* return the reference to the 'h_pub' field */
	return p ? &p->ht_pub : NULL;
} /* getEntry */

/* getter for the hash table */
void *
ht_get(hash_t *tab, const void *key)	
{
	const ht_entry *aux = ht_get_entry(tab, key);
	return aux ? aux->ht_val : NULL;
}

/* setter function for the hash table */
void *
ht_put(hash_t *tab,
		const void *key,
		void *value)
{
	size_t ix = getIndex(tab, key);
	struct ht_entryP *aux
			= getRef(tab, tab->ht_array[ix], key);
	void *old_val = NULL;
	if (aux) { /* exists */
		old_val = aux->ht_pub.ht_val;
	} else {
		aux = malloc(sizeof *aux);
		assert(aux != NULL);
		if (tab->ht_array[ix]) {
            /* not empty, there's already an
             * entry.  */
			tab->ht_collisions++;
		}
		/* insert in the list */
		aux->ht_next = tab->ht_array[ix];
		tab->ht_array[ix] = aux;
		tab->ht_size++;
	}
	aux->ht_pub.ht_key = tab->ht_dup_f(key);
	aux->ht_pub.ht_val = value;
	return old_val;
}

/* remover for an entry. Depending on the semantics, the hidden
 * elements will be accessible after calling this method.
 * The function returns the element removed or NULL if there are no
 * more elements to be removed.  */
void *
ht_remove(
        hash_t *tab,
        void *key)
{
	size_t ix = getIndex(tab, key);
	void *ret_val = NULL;
	struct ht_entryP **ref
			= getPtrRef(tab, &tab->ht_array[ix], key);
	struct ht_entryP *q = *ref;
	if (q) {
		ret_val = q->ht_pub.ht_val;
	}
	*ref = q->ht_next; /* unlink */
	if (tab->ht_collisions) {
		tab->ht_collisions--;
	}
	if (q->ht_pub.ht_key) {
		tab->ht_free_f(q->ht_pub.ht_key);
	}
	free(q);
	tab->ht_size--;
	return ret_val;
}

size_t
ht_get_collisions(
        hash_t *tab)
{
	return tab->ht_collisions;
}

size_t
ht_get_size(
        hash_t *tab)
{
	return tab->ht_size;
}

size_t
ht_get_capacity(
        hash_t *tab)
{
	return tab->ht_capacity;
}

size_t (*
ht_get_hash_f(
            hash_t *tab)
)(const void *key)
{
	return tab->ht_hash_f;
}

int (*
ht_get_equals_f(
        hash_t *tab)
)(const void *key_a, const void *key_b)
{
	return tab->ht_equals_f;
}
