/***
    This file is part of hashlib.

    Copyright 2012 Matthias Ruester

    hashlib is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    hashlib is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with hashlib; if not, see <http://www.gnu.org/licenses>.
***/

#define _GNU_SOURCE

#include <err.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>

#include "hashlib.h"
#include "fnv.h"

#define errf(exit, format, ...)  err((exit), "%s: " format, __func__, ## __VA_ARGS__)
#define errfx(exit, format, ...) errx((exit), "%s: " format, __func__, ## __VA_ARGS__)

struct hashlib_entry {
    char *key;
    void *value;
};

extern unsigned int hashlib_index(char *key)
{
    unsigned int index;

    assert(key);

    index = fnv_64a_str(key, FNV1_64_INIT);

    return index;
}

static struct hashlib_entry *hashlib_entry_new(char *key, void *value)
{
    struct hashlib_entry *e;

    e = calloc(1, sizeof(*e));

    if (!e)
        errf(EXIT_FAILURE, "calloc");

    e->key   = key;
    e->value = value;

    return e;
}

static void hashlib_entry_delete(struct hashlib_entry *e)
{
    free(e);
}

/* this function is taken from glibc's misc/hsearch_r.c */
static int isprime(unsigned int number)
{
    /* no even number will be passed */
    unsigned int div = 3;

    while (div * div < number && number % div != 0)
        div += 2;

    return number % div != 0;
}

extern struct hashlib_hash *hashlib_hash_new(unsigned int size)
{
    struct hashlib_hash *hash;

    if (size > HASHLIB_MAX_TBLSIZE)
        errfx(EXIT_FAILURE, "table size too big");

    if (size <= 0)
        errfx(EXIT_FAILURE, "table size must be greater than zero");

    hash = calloc(1, sizeof(*hash));

    if (!hash)
        errf(EXIT_FAILURE, "calloc(hash)");

    /* the next five lines of code are taken from glibc's misc/hsearch_r.c */
    if (size < 3)
        size = 3;

    size |= 1;

    while (!isprime(size))
        size += 2;

    hash->tbl = calloc(size, sizeof(*(hash->tbl)));

    if (!hash->tbl)
        errf(EXIT_FAILURE, "calloc(hash->tbl)");

    hash->tblsize = size;

    return hash;
}

static void hashlib_tree_delete(void *a)
{
    hashlib_entry_delete(a);
}

static int hashlib_compare(const void *a, const void *b)
{
    struct hashlib_entry *e;
    struct hashlib_entry *f;

    e = (struct hashlib_entry *) a;
    f = (struct hashlib_entry *) b;

    return strcmp(e->key, f->key);
}

void hashlib_put(struct hashlib_hash *hash, char *key, void *value)
{
    unsigned int index;
    struct hashlib_entry *e;
    struct hashlib_entry *r;
    void *ret;

    assert(hash);
    assert(value);

    index = hashlib_index(key) % hash->tblsize;

    e = hashlib_entry_new(key, value);

    ret = tsearch(e, &(hash->tbl[index]), hashlib_compare);

    if (!ret)
        errf(EXIT_FAILURE, "tsearch");

    r = *(struct hashlib_entry **) ret;

    if (r != e)
        /* already in hash */
        hashlib_entry_delete(e);
    else
        /* e was inserted */
        hash->count++;
}

void *hashlib_get(struct hashlib_hash *hash, char *key)
{
    unsigned int index;
    struct hashlib_entry *e;
    struct hashlib_entry f;

    assert(hash);
    assert(key);

    index = hashlib_index(key) % hash->tblsize;

    if (!hash->tbl[index])
        return NULL;

    f.key = key;

    e = tfind(&f, &(hash->tbl[index]), hashlib_compare);
    e = *(struct hashlib_entry **) e;

    if (e)
        return e->value;

    return NULL;
}

extern void hashlib_set_free_function(struct hashlib_hash *hash,
                               void (*free_function)(void *element))
{
    assert(hash);

    hash->free_function = free_function;
}

extern void hashlib_hash_delete(struct hashlib_hash *hash)
{
    unsigned int i;

    assert(hash);

    for (i = 0; i < hash->tblsize; i++)
        if (hash->tbl[i])
            tdestroy(hash->tbl[i], hashlib_tree_delete);

    free(hash->tbl);
    free(hash);
}
