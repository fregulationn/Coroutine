#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "co_hash.h"

static int timeInMilliseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static int hashExpand(struct SchedTable *s, int size) {
    if (isRehashing(s) || s->ht[0].used > size)
        return 0;
    
    HashTable_t ht; /* the new hash table */
    ht.size = size;
    ht.sizemask = size - 1;
    ht.table = (HashEntry_t **)calloc(1, size * sizeof(HashEntry_t *));
    ht.used = 0;

    s->ht[1] = ht;
    s->rehashidx = 0;

    return 1;
}

static int hashExpandIfNeeded(struct SchedTable *s) {
    // 如果正在进行渐进式扩容，则返回OK
    if (isRehashing(s)) return 1;
    if (s->ht[0].used / s->ht[0].size > 2) {
        return hashExpand(s, s->ht[0].size * 2);
    }
    // 缩容
    if(s->ht[0].used > 0 && s->ht[0].size / s->ht[0].used > 2) {
        return hashExpand(s, s->ht[0].size / 2);
    }
    return 0;
}

static void hashReset(HashTable_t *ht) {
    ht->table = NULL;
    ht->size = 0;
    ht->sizemask = 0;
    ht->used = 0;
}

static int reHash(struct SchedTable *s, int n) {
    int empty_visits = n * 10; /* Max number of empty buckets to visit. */
    if (!isRehashing(s)) return 0;

    while(n-- && s->ht[0].used != 0) { 
        HashEntry_t *e, *nexte;
        /* Note that rehashidx can't overflow as we are sure there are more
         * elements because ht[0].used != 0 */
        assert(s->ht[0].size > s->rehashidx);
        while(s->ht[0].table[s->rehashidx] == NULL) {
            s->rehashidx++;
            if(--empty_visits == 0) return 1;
        }
        e = s->ht[0].table[s->rehashidx];
        /* Move all the keys in this bucket from the old to the new hash HT */
        while(e) {
            uint64_t h;

            nexte = e->next;
            /* Get the index in the new hash table */
            h = HASH(e->fd) & s->ht[1].sizemask;
            s->ht[0].table[s->rehashidx] = nexte;
            
            e->next = s->ht[1].table[h];
            s->ht[1].table[h] = e;

            s->ht[0].used--;
            s->ht[1].used++;
            e = nexte;
        }
        s->ht[0].table[s->rehashidx] = NULL;
        s->rehashidx++;
    }

    if (s->ht[0].used == 0) {
        free(s->ht[0].table);
        s->ht[0] = s->ht[1];
        hashReset(&s->ht[1]);
        s->rehashidx = -1;
        return 0;
    }

    /* more to rehash */
    return 1;
}

static int rehashMilliseconds(struct SchedTable *s, int ms) {
    uint64_t start = timeInMilliseconds();
    int rehashes = 0;

    while(reHash(s, 100) > 0) {
        rehashes += 100;
        if(timeInMilliseconds() - start > ms) break;
    }
    return rehashes;
}

void init_schedtab(struct SchedTable *s) {
    s->rehashidx = -1;
    s->ht[0].size = INIT_SIZE;
    s->ht[0].sizemask = INIT_SIZE - 1;
    s->ht[0].table = (HashEntry_t **)calloc(1, sizeof(HashEntry_t *) * INIT_SIZE);
    s->ht[0].used = 0;
}

void addToTable(struct SchedTable *s, int fd, coroutine_t *co) {
    if(hashExpandIfNeeded(s)) {
        rehashMilliseconds(s, 100);
    }

    int idx = 0;
    if(isRehashing(s)) idx = 1;

    int hashcode = HASH(fd) & (s->ht[idx].sizemask);
    HashEntry_t *e = (HashEntry_t *)malloc(sizeof(HashEntry_t));
    e->fd = fd;
    e->co = co;
    e->next = NULL;

    e->next = s->ht[idx].table[hashcode];
    s->ht[idx].table[hashcode] = e;
}

void delfromTable(struct SchedTable *s, int fd) {
    if(hashExpandIfNeeded(s)) {
        rehashMilliseconds(s, 100);
    }

    // 先去ht[0]
    int hashcode = HASH(fd) & (s->ht[0].sizemask);
    HashEntry_t *e = s->ht[0].table[hashcode];
    if(e->fd == fd) {
        s->ht[0].table[hashcode] = e->next;
        free(e);
        return;
    }

    HashEntry_t *prev = e;
    e = e->next;
    while(e) {
        if(e->fd == fd) {
            prev->next = e->next;
            free(e);
            return;
        }   
        prev = e;
        e = e->next;
    }

    // 再去ht[1]
    hashcode = HASH(fd) & (s->ht[1].sizemask);
    e = s->ht[1].table[hashcode];
    if(e->fd == fd) {
        s->ht[1].table[hashcode] = e->next;
        free(e);
        return;
    }

    prev = e;
    e = e->next;
    while(e) {
        if(e->fd == fd) {
            prev->next = e->next;
            free(e);
            return;
        }
        prev = e;
        e = e->next;
    }
}

coroutine_t *co_search(struct SchedTable *s, int fd) {
    if(hashExpandIfNeeded(s)) {
        rehashMilliseconds(s, 100);
    }

    // 先找ht[0]
    int hashcode = HASH(fd) & (s->ht[0].sizemask);
    HashEntry_t *e = s->ht[0].table[hashcode];
    while(e) {
        if(e->fd == fd) return e->co;
        e = e->next;
    }
    // 未找到则去ht[1]
    hashcode = HASH(fd) & (s->ht[1].sizemask);
    e = s->ht[1].table[hashcode];
    while(e) {
        if(e->fd == fd) return e->co;
        e = e->next;
    }
    return NULL;
}