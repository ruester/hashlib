#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <ctype.h>
#include <string.h>

#include "hashlib.h"

#define put(str) fputs((str), stdout)

#define TEST(str) put("Testing " str ": ")

struct translation {
    char *english;
    char *german;
    char *french;
    char *latin;
};


int read_value(char *line)
{
    size_t len;

    while (!isdigit(*line))
        line++;

    len = strlen(line);

    /* cut off ' kb' */
    line[len - 3] = '\0';

    return atoi(line);
}

unsigned long read_VmRSS(void)
{
    FILE* file;
    unsigned long result;
    char line[BUFSIZ];

    result = 0;
    file   = fopen("/proc/self/status", "r");

    if (!file)
        err(EXIT_FAILURE, "fopen");

    while (fgets(line, BUFSIZ, file)) {
        if (!strncmp(line, "VmRSS:", 6)) {
            result = read_value(line);
            break;
        }
    }

    if (fclose(file) == EOF)
        err(EXIT_FAILURE, "fclose");

    return result * 1024L;
}

unsigned long get_VmRSS(void)
{
    unsigned long old, value;

    old = value = read_VmRSS();

    do {
        old   = value;
        value = read_VmRSS();
    } while (old != value);

    return value;
}

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

void test1(void)
{
    struct translation example;
    struct translation *a, *b, *c, *d, *p;
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

    a = hashlib_get(hash, "horse");
    b = hashlib_get(hash, "Pferd");
    c = hashlib_get(hash, "equus");
    d = hashlib_get(hash, "cheval");

    p = &example;

    TEST("hashlib_get");

    if (a != p || b != p || c != p || d != p)
        puts("failed");
    else
        puts("success");

    hashlib_hash_delete(hash);
}

void test2(void)
{
    struct translation a, b;
    struct hashlib_hash *hash;

    hash = hashlib_hash_new(1000);

    hashlib_put(hash, "one", &a);
    hashlib_put(hash, "two", &b);

    TEST("hashlib_count");

    if (hashlib_count(hash) != 2)
        failed();
    else
        success();

    hashlib_hash_delete(hash);
}

void test3(void)
{
    struct hashlib_hash *hash;
    struct translation *p, *r;

    hash = hashlib_hash_new(1000);
    p    = translation_new();

    hashlib_put(hash, "test", p);

    r = hashlib_remove(hash, "test");

    TEST("hashlib_remove");

    if (r != p) {
        failed();
    } else {
        if (hashlib_get(hash, "test"))
            failed();
        else
            success();
    }

    translation_delete(p);
    hashlib_hash_delete(hash);
}

void test4(void)
{
    struct hashlib_hash *hash;
    unsigned long before, after;
    struct translation *p;

    hash = hashlib_hash_new(1000);
    p    = translation_new();

    hashlib_set_free_function(hash, translation_delete);

    hashlib_put(hash, "test", p);

    before = get_VmRSS();

    hashlib_remove(hash, "test");

    after = get_VmRSS();

    TEST("hashlib_remove with free_function");

    /* this does not work properly yet */
    if (after > before || hashlib_get(hash, "test"))
        failed();
    else
        success();

    hashlib_hash_delete(hash);
}

void test5(void)
{
    unsigned long before, after;
    struct hashlib_hash *hash;
    struct translation *p, *q;

    before = get_VmRSS();
    hash   = hashlib_hash_new(1000);
    p      = translation_new();
    q      = translation_new();

    hashlib_set_free_function(hash, translation_delete);

    hashlib_put(hash, "foo", p);
    hashlib_put(hash, "bar", q);

    hashlib_hash_delete(hash);

    after = get_VmRSS();

    TEST("hashlib_hash_delete");

    if (after != before)
        failed();
    else
        success();
}

#define TESTS 5

int main(void)
{
    int i;
    void (*arr[TESTS])(void) = {
        test1, test2, test3, test4, test5
    };

    for (i = 0; i < TESTS; i++)
        arr[i]();

    return 0;
}
