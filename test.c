#include <stdio.h>

#include "hashlib.h"

#define put(str) fputs((str), stdout)

struct translation {
    char *english;
    char *german;
    char *french;
    char *latin;
};

int main(void)
{
    struct translation example;
    struct translation *p;
    struct hashlib_hash *hash;

    example.english = "horse";
    example.german  = "Pferd";
    example.latin   = "equus";
    example.french  = "cheval";

    hash = hashlib_hash_new(1000);

    hashlib_put(hash, "horse",  &example);
    hashlib_put(hash, "Pferd",  &example);
    hashlib_put(hash, "equus",  &example);
    hashlib_put(hash, "cheval", &example);

    p = hashlib_get(hash, "horse");

    put("Testing hashlib_get: ");

    if (p != &example)
        puts("failed");
    else
        puts("success");

    put("Testing hashlib_count: ");

    if (hashlib_count(hash) != 4)
        puts("failed");
    else
        puts("success");

    hashlib_hash_delete(hash);

    return 0;
}
