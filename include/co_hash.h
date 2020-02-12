#ifndef _CO_HASH_H
#define _CO_HASH_H

#include <stdint.h>

#define INIT_SIZE 64
#define HASH(x) (x - 3)
#define isRehashing(s) ((s)->rehashidx != -1)

typedef struct coroutine coroutine_t;
typedef struct HashEntry_s {
    int fd;
    coroutine_t *co;
    struct HashEntry_s *next;
}HashEntry_t;

typedef struct HashTable_s {
    HashEntry_t **table;
    uint64_t size;
    uint64_t sizemask;
    uint64_t used;
}HashTable_t;

struct SchedTable {
    HashTable_t ht[2];
    uint64_t rehashidx; /* rehashing not in progress if rehashidx == -1 */
};

void init_schedtab(struct SchedTable *);
void addToTable(struct SchedTable *, int, coroutine_t *);
void delfromTable(struct SchedTable *, int);
coroutine_t* co_search(struct SchedTable *, int);

#endif