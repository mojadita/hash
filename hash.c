/* hash.c -- hash table implementation.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Wed Nov 10 14:38:17 EET 2021
 * Copyright: (C) 2021 Luis Colorado.  All rights reserved.
 * License: BSD.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "hashP.h"

ht_hash_t *
ht_new( const size_t initial_cap,
        size_t     (*hash_f)(
                        const char *key),
        int        (*equals_f)(
                        const char *key_a,
                        const char *key_b),
        size_t     (*sizeof_f)(
                        const char *key))
{
    ht_hash_t *ret_val     = malloc(sizeof *ret_val);
    if (!ret_val) return NULL;

    ret_val->ht_size       = 0;
    ret_val->ht_capacity   = initial_cap;
    ret_val->ht_array      = NULL;
    ret_val->ht_collisions = 0;
    ret_val->ht_hash_f     = hash_f;
    ret_val->ht_equals_f   = equals_f;
    ret_val->ht_size_f     = sizeof_f;
    /* this will just allocate the array for now. */
    ret_val->ht_array      = calloc(ret_val->ht_capacity,
                                    sizeof *ret_val->ht_array);
    return ret_val;
} /* ht_new */

/* clear hash_table */
void
ht_clear(
        ht_hash_t *table)
{
    for(size_t ix = 0; ix < table->ht_capacity; ix++) {
        /* free all entries at position ix in the array */
        struct ht_entryP *saved;
        while ((saved = table->ht_array[ix]) != NULL) {
            table->ht_array[ix] = saved->ht_next; /* unlink before free() */

			/* free() key */
            if (saved->ht_pub.ht_key)
                free((void *)saved->ht_pub.ht_key);
            free(saved);
            table->ht_size--; /* table size */
        }
        assert(table->ht_array[ix] == NULL);
    }
    assert(table->ht_size == 0); /* double check */
} /* ht_clear */

/* deallocator for the hash table. The hash table to deallocate
 * must have been allocated with new_hash() function. */
void
ht_delete(
        ht_hash_t *table)
{
    ht_clear(table);
    free(table->ht_array); /* free the array */
    free(table);
} /* ht_delete */

static size_t
getIndex(
        ht_hash_t *table,
        const char *key)
{
    return table->ht_hash_f(key) % table->ht_capacity;
} /* getIndex */

/* gets a reference to the pointer that points to the entry whose
 * key is 'key'. This function is used in the remove() below.  */
static struct ht_entryP **
getPtrRef(
        ht_hash_t *tab,
        struct ht_entryP **p,   /* the starting reference */
        const char *key)
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
} /* getPtrRef */

/* gets the pointer to the entry whose key is 'key'.  We use
 * the above getPtrRef() to implement this.  */
static struct ht_entryP *
getRef(
        ht_hash_t *tab,
        struct ht_entryP *p,
        const char *key)
{
    /* we use the above function */
    return *getPtrRef(tab, &p, key);
} /* getRef */

const struct ht_entry *
ht_get_entry(
        ht_hash_t *table,
        const char *key)
{
    size_t ix = getIndex(table, key);
    struct ht_entryP *p = getRef(table, table->ht_array[ix], key);
    /* return the reference to the 'h_pub' field */
    return p ? &p->ht_pub : NULL;
} /* getEntry */

/* getter for the hash table */
void *
ht_get(ht_hash_t *tab, const char *key)
{
    const ht_entry *aux = ht_get_entry(tab, key);
    return aux ? aux->ht_val : NULL;
} /* ht_get */

/* setter function for the hash table */
void *
ht_put(ht_hash_t *tab,
        const char *key,
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
    /* calculate key size */
    size_t key_sz = tab->ht_size_f(key);
    /* malloc */
    aux->ht_pub.ht_key = malloc(key_sz);
    assert(aux->ht_pub.ht_key != NULL);
    /* copy */
    memcpy((char *)aux->ht_pub.ht_key, key, key_sz);
    /* reference to the value */
    aux->ht_pub.ht_val = value;
    return old_val;
} /* ht_put */

/* remover for an entry. Depending on the semantics, the hidden
 * elements will be accessible after calling this method.
 * The function returns the element removed or NULL if there are no
 * more elements to be removed.  */
void *
ht_remove(
        ht_hash_t *tab,
        const char *key)
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
        free((void *)q->ht_pub.ht_key);
    }
    free(q);
    tab->ht_size--;
    return ret_val;
} /* ht_remove */

size_t
ht_get_collisions(
        ht_hash_t *tab)
{
    return tab->ht_collisions;
} /* ht_get_collisions */

size_t
ht_get_size(
        ht_hash_t *tab)
{
    return tab->ht_size;
} /* ht_get_size */

size_t
ht_get_capacity(
        ht_hash_t *tab)
{
    return tab->ht_capacity;
} /* ht_get_capacity */

size_t (*
ht_get_hash_f(
            ht_hash_t *tab)
)(const char *key)
{
    return tab->ht_hash_f;
} /* ht_get_hash_f */

int (*
ht_get_equals_f(
        ht_hash_t *tab)
)(const char *key_a, const char *key_b)
{
    return tab->ht_equals_f;
} /* ht_get_equals_f */

static size_t
escape(const char *s, char *buffer, size_t buff_sz)
{
    size_t ret_val = 0;
    while (*s && buff_sz > 2) {
        switch(*s) {
        case '\\': case '\"': case '\'':
            *buffer++ = '\\';
            *buffer++ = *s;
            ret_val += 2;
            break;
        case '\n':
            *buffer++ = '\\';
            *buffer++ = 'n';
            ret_val += 2;
            break;
        default:
            *buffer++ = *s;
            ret_val++;
            break;
        }
        s++;
    }
    *buffer++ = '\0';
    ret_val++;
    return ret_val;
} /* escape */

size_t
ht_print(
        ht_hash_t *tab,
        FILE *f)
{
    size_t ret_val = 0;
    char *sep = " ";
    ret_val += fprintf(f, "{");
    for(size_t ix = 0; ix < tab->ht_capacity; ix++) {
        for (struct ht_entryP *p = tab->ht_array[ix];
            p; p = p->ht_next)
        {
            char buffer[256];
            char *p_aux = buffer;
            size_t buf_sz = sizeof buffer;
            size_t n = escape(p->ht_pub.ht_key, p_aux, buf_sz);
            p_aux += n; buf_sz -= n;
            ret_val += fprintf(f, "%s\"%s\": %lu",
                    sep,
                    buffer,
                    (unsigned long)p->ht_pub.ht_val);
            sep = ",\n  ";
        }
    }
    ret_val += fprintf(f, "%s}\n", tab->ht_size ? "\n" : "");
    return ret_val;
} /* ht_print */
