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

#ifndef HASHLIB_HASHLIB_H
#define HASHLIB_HASHLIB_H

#define HASHLIB_MAX_TBLSIZE ((unsigned) 1 << 31)

#define hashlib_count(hash) (hash)->count

#define HASHLIB_FP_FREE(fname) \
        void (*(fname))(void *)

#define HASHLIB_FP_SIZE(fname) \
        size_t (*(fname))(void *)

#define HASHLIB_FP_PACK(fname) \
        void (*(fname))(void *, size_t, int)

#define HASHLIB_FP_UNPACK(fname) \
        void *(*(fname))(void *, size_t)

#define HASHLIB_FCT_FREE(fname, arg) \
        void (fname)(void *(arg))

#define HASHLIB_FCT_SIZE(fname, arg) \
        size_t (fname)(void *(arg))

#define HASHLIB_FCT_PACK(fname, arg, bytes, fd) \
        void (fname)(void *(arg), size_t (bytes), int (fd))

#define HASHLIB_FCT_UNPACK(fname, arg, bytes) \
        void *(fname)(void *(arg), size_t (bytes))

struct hashlib_hash {
    void **tbl;
    size_t count;
    size_t tblsize;
    HASHLIB_FP_FREE(free_function);
    HASHLIB_FP_SIZE(size_function);
    HASHLIB_FP_PACK(pack_function);
};

void hashlib_set_free_function(struct hashlib_hash *hash,
                               HASHLIB_FP_FREE(free_function));
void hashlib_set_size_function(struct hashlib_hash *hash,
                               HASHLIB_FP_SIZE(size_function));
void hashlib_set_pack_function(struct hashlib_hash *hash,
                               HASHLIB_FP_PACK(pack_function));
void *hashlib_remove(struct hashlib_hash *hash, char *key);
struct hashlib_hash *hashlib_hash_new(unsigned int size);
int hashlib_put(struct hashlib_hash *hash, char *key, void *data);
void *hashlib_get(struct hashlib_hash *hash, char *key);
unsigned int hashlib_index(char *key);
void hashlib_hash_delete(struct hashlib_hash *hash);
void hashlib_store(struct hashlib_hash *hash, const char *filename);

#endif
