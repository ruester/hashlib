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

struct hashlib_hash {
    void **tbl;
    unsigned int count;
    unsigned int tblsize;
    void (*free_function)(void *element);
};

void hashlib_set_free_function(struct hashlib_hash *hash,
                               void (*free_function)(void *element));
struct hashlib_hash *hashlib_hash_new(unsigned int size);
void hashlib_put(struct hashlib_hash *hash, char *key, void *data);
void *hashlib_get(struct hashlib_hash *hash, char *key);
unsigned int hashlib_index(char *key);
void hashlib_hash_delete(struct hashlib_hash *hash);

#endif
