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
#include <unistd.h>
#include <fcntl.h>

#include "hashlib.h"

/* identifier 0x4A5411B0 */
#define HASHLIB_FILE_HEADER (0xB011544A)

#define errf(exit, format, ...)  err((exit), "%s: " format, __func__, ## __VA_ARGS__)
#define errfx(exit, format, ...) errx((exit), "%s: " format, __func__, ## __VA_ARGS__)

#define QUERY (-1)
#define RESET (-2)

struct hashlib_entry {
    char *key;
    void *value;
    HASHLIB_FP_FREE(free_function);
    HASHLIB_FP_SIZE(size_function);
    HASHLIB_FP_PACK(pack_function);
};

static inline void *hashlib_calloc(size_t nmemb, size_t size)
{
    void *p;

    p = calloc(nmemb, size);

    if (!p)
        errf(EXIT_FAILURE, "calloc");

    return p;
}

static inline int hashlib_open(const char *filename, int flags)
{
    int fd;

    fd = open(filename, flags);

    if (fd == -1)
        errf(EXIT_FAILURE, "open");

    return fd;
}

static inline void hashlib_close(int fd)
{
    int ret;

    ret = close(fd);

    if (ret == -1)
        errf(EXIT_FAILURE, "close");
}

static inline void hashlib_write(int fd, void *data, size_t bytes)
{
    ssize_t ret;

    ret = write(fd, data, bytes);

    if (ret == -1)
        errf(EXIT_FAILURE, "write");
}

static int hashlib_current_fd(int fd)
{
    static int cfd = QUERY;

    if (fd == RESET) {
        cfd = QUERY;
        return RESET;
    }

    if (fd == QUERY)
        assert(cfd != QUERY);
    else
        cfd = fd;

    return cfd;
}

static HASHLIB_FCT_SIZE(hashlib_default_size_function, e)
{
    return sizeof(e);
}

static HASHLIB_FCT_PACK(hashlib_default_pack_function, e, bytes, fd)
{
    int ret;

    ret = write(fd, e, bytes);

    if (ret == -1)
        errf(EXIT_FAILURE, "write");
}

static HASHLIB_FCT_UNPACK(hashlib_default_unpack_function, data, bytes)
{
    void *e;

    e = malloc(bytes);

    if (!e)
        errf(EXIT_FAILURE, "malloc");

    memcpy(e, data, bytes);

    return e;
}

extern unsigned int hashlib_index(char *key)
{
    unsigned int index;

    assert(key);

    index = 0;

    while (*key) {
        index = 5 * index + *key;
        key++;
    }

    return index;
}

static struct hashlib_entry *hashlib_entry_new(char *key, void *value,
                                               HASHLIB_FP_FREE(free_function),
                                               HASHLIB_FP_SIZE(size_function),
                                               HASHLIB_FP_PACK(pack_function))
{
    struct hashlib_entry *e;

    e = calloc(1, sizeof(*e));

    if (!e)
        errf(EXIT_FAILURE, "calloc");

    e->key             = strdup(key);
    e->value           = value;
    e->free_function   = free_function;
    e->size_function   = size_function;
    e->pack_function   = pack_function;

    return e;
}

static void hashlib_entry_delete(struct hashlib_entry *e)
{
    if (e->free_function)
        e->free_function(e->value);

    free(e->key);
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

    hash->tblsize         = size;
    hash->size_function   = hashlib_default_size_function;
    hash->pack_function   = hashlib_default_pack_function;

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

extern int hashlib_put(struct hashlib_hash *hash, char *key, void *value)
{
    unsigned int index;
    struct hashlib_entry *e;
    struct hashlib_entry *r;
    void *ret;

    assert(hash);
    assert(key);
    assert(value);

    index = hashlib_index(key) % hash->tblsize;

    e = hashlib_entry_new(key, value, hash->free_function,
                          hash->size_function, hash->pack_function);

    ret = tsearch(e, &(hash->tbl[index]), hashlib_compare);

    if (!ret)
        errf(EXIT_FAILURE, "tsearch");

    r = *(struct hashlib_entry **) ret;

    if (r != e) {
        /* already in hash */
        hashlib_entry_delete(e);
        return 0;
    }

    /* e was inserted */
    hash->count++;

    return 1;
}

extern void *hashlib_get(struct hashlib_hash *hash, char *key)
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
                                      HASHLIB_FP_FREE(free_function))
{
    assert(hash);
    hash->free_function = free_function;
}

extern void hashlib_set_size_function(struct hashlib_hash *hash,
                                      HASHLIB_FP_SIZE(size_function))
{
    assert(hash);
    hash->size_function = size_function;
}

extern void hashlib_set_pack_function(struct hashlib_hash *hash,
                               HASHLIB_FP_PACK(pack_function))
{
    assert(hash);
    hash->pack_function = pack_function;
}

extern void *hashlib_remove(struct hashlib_hash *hash, char *key)
{
    struct hashlib_entry *e;
    struct hashlib_entry f;
    void *ret;
    unsigned int index;

    assert(hash);
    assert(key);

    f.key = key;
    index = hashlib_index(key) % hash->tblsize;

    e = tfind(&f, &(hash->tbl[index]), hashlib_compare);

    if (!e)
        return NULL;

    e   = *(struct hashlib_entry **) e;
    ret = e->value;

    tdelete(&f, &(hash->tbl[index]), hashlib_compare);

    hashlib_entry_delete(e);

    return ret;
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

static void hashlib_store_action(const void *nodep,
                                 const VISIT which,
                                 const int depth)
{
    struct hashlib_entry *e;
    int fd;
    size_t bytes;
    size_t keylen;

    if (which == preorder || which == endorder)
        return;

    e = *(struct hashlib_entry **) nodep;

    bytes = e->size_function(e);
    fd    = hashlib_current_fd(QUERY);

    /* write size */
    hashlib_write(fd, &bytes, sizeof(bytes));

    /* write data */
    e->pack_function(e->value, bytes, fd);

    keylen = strlen(e->key);

    /* write length of key */
    hashlib_write(fd, &keylen, sizeof(keylen));

    /* write key */
    hashlib_write(fd, e->key, sizeof(*(e->key)) * keylen);

    return;

    /* avoid gcc warning */
    (void) depth;
}

static void hashlib_write_header(struct hashlib_hash *hash, int fd)
{
    size_t h;

    assert(hash);
    assert(fd >= 0);

    h = HASHLIB_FILE_HEADER;

    hashlib_write(fd, &h, sizeof(h));
    hashlib_write(fd, &(hash->tblsize), sizeof(hash->tblsize));
    hashlib_write(fd, &(hash->count), sizeof(hash->count));
}

extern void hashlib_store(struct hashlib_hash *hash, const char *filename)
{
    int fd;
    unsigned int i;

    assert(hash);
    assert(filename);

    fd = hashlib_open(filename, O_WRONLY | O_TRUNC | O_CREAT);

    hashlib_current_fd(fd);

    hashlib_write_header(hash, fd);

    for (i = 0; i < hash->tblsize; i++) {
        if (!hash->tbl[i])
            continue;

        twalk(hash->tbl[i], hashlib_store_action);
    }

    hashlib_current_fd(RESET);

    hashlib_close(fd);
}
