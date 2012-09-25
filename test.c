#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "hashlib.h"

#define put(str) fputs((str), stdout)

#define TEST(str) put("Testing " str ": ")

struct translation {
    char *english;
    char *german;
    char *french;
    char *latin;
};

struct translation *translation_new(void)
{
    struct translation *t;

    t = calloc(1, sizeof(*t));

    if (!t)
        err(EXIT_FAILURE, "calloc");

    return t;
}

void translation_delete(void *a)
{
    struct translation *t;

    t = (struct translation *) a;

    free(t);
}

void failed(void)
{
    puts("failed");
}

void success(void)
{
    puts("success");
}

int main(void)
{
    struct translation example;
    struct translation *p;
    struct translation *r;
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

    TEST("hashlib_get");

    if (p != &example)
        puts("failed");
    else
        puts("success");

    TEST("hashlib_count");

    if (hashlib_count(hash) != 4)
        failed();
    else
        success();

    hashlib_remove(hash, "horse");
    hashlib_remove(hash, "Pferd");
    hashlib_remove(hash, "equus");
    hashlib_remove(hash, "cheval");

    TEST("hashlib_remove");

    p = translation_new();

    hashlib_put(hash, "test", p);

    r = hashlib_remove(hash, "test");

    if (r != p) {
        failed();
    } else {
        if (hashlib_get(hash, "test"))
            failed();
        else
            success();
    }

    TEST("hashlib_hash_delete");

    hashlib_set_free_function(hash, translation_delete);

    hashlib_put(hash, "test", p);

    hashlib_hash_delete(hash);

    success();

    return 0;
}
