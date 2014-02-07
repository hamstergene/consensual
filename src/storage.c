#include <consensual/storage.h>

#include <string.h> // memset
#include <assert.h>

// http://www.burtleburtle.net/bob/hash/doobs.html
// Public Domain
static
uint32_t jenkins_one_at_a_time_hash(const uint8_t* key, size_t len)
{
    uint32_t hash, i;
    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

// I have no idea what quality it has, just needed something working so here it is
uint32_t
cns_storage_defaultBytesHash32(cns_Runtime* cns, cns_Bytes* bytes)
{
    if (!cns || !bytes)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return 0;
    }

    const uint8_t* p = cns_bytes_ptr(cns, bytes);
    assert(p != 0);
    if (!p)
        return 0;

    cns_Index length = cns_bytes_length(cns, bytes);
    return jenkins_one_at_a_time_hash(p, length);
}

typedef struct _cns_Storage_BucketItem
{
    cns_Bytes* key;
    cns_Bytes* value;
    struct _cns_Storage_BucketItem* next;
} _cns_Storage_BucketItem;

struct cns_Storage
{
    cns_Storage_BytesHash32Fn byteshashfn;
    int log2numbuckets;
    int count;
    _cns_Storage_BucketItem** buckets;
};

static _cns_Storage_BucketItem* _cns_storage_itemForKey(cns_Runtime* cns, cns_Storage* storage, cns_Bytes* key, _cns_Storage_BucketItem*** bucket, _cns_Storage_BucketItem** previousitem)
{
    uint32_t keyhash = storage->byteshashfn(cns, key);
    keyhash &= (1 << storage->log2numbuckets) - 1;
    if (bucket)
        *bucket = &storage->buckets[keyhash];

    _cns_Storage_BucketItem* first = storage->buckets[keyhash];
    _cns_Storage_BucketItem* item = first;
    if (previousitem)
        *previousitem = 0;
    while (item && !cns_bytes_equal(cns, item->key, key))
    {
        if (previousitem)
            *previousitem = item;
        item = item->next;
    }
    return item;
}

static void _cns_item_free(cns_Runtime* cns, _cns_Storage_BucketItem* item)
{
    cns_bytes_free(cns, item->key);
    cns_bytes_free(cns, item->value);
    cns_runtime_free(cns, item);
}

static cns_Bool _cns_storage_changeCapacityBase(cns_Runtime* cns, cns_Storage* storage, int newCapacityBase)
{
    _cns_Storage_BucketItem** old_buckets = storage->buckets;
    int old_count = storage->count;
    int old_base = storage->log2numbuckets;

    storage->log2numbuckets = newCapacityBase;
    cns_Index bucketmemsize = (1 << newCapacityBase) * sizeof(_cns_Storage_BucketItem*);
    storage->buckets = (_cns_Storage_BucketItem**) cns_runtime_alloc(cns, bucketmemsize);
    if (storage->buckets)
    {
        storage->count = 0;
        memset(storage->buckets, 0, bucketmemsize);

        // copy items to the new location
        for (int i = 0; i < (1 << old_base); ++i)
        {
            _cns_Storage_BucketItem* item = old_buckets[i];
            while (item)
            {
                _cns_Storage_BucketItem* next = item->next;
                cns_storage_set(cns, storage, item->key, item->value);
                item = next;
            }
        }

        assert(storage->count == old_count);

        // free old items
        for (int i = 0; i < (1 << old_base); ++i)
        {
            _cns_Storage_BucketItem* item = old_buckets[i];
            while (item)
            {
                _cns_Storage_BucketItem* next = item->next;
                _cns_item_free(cns, item);
                item = next;
            }
        }
        cns_runtime_free(cns, old_buckets);
    }
    else
    {
        // failing to allocate more memory is not fatal here, we can proceed with the old buckets
        storage->buckets = old_buckets;
        storage->log2numbuckets = old_base;
        return CNS_NO;
    }
    return CNS_YES;
}

cns_Storage*
cns_storage_newMemoryStorage(cns_Runtime* cns, cns_Storage_BytesHash32Fn byteshashfn)
{
    if (!cns)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return 0;
    }

    cns_Storage* rv = (cns_Storage*) cns_runtime_alloc(cns, sizeof(cns_Storage));
    if (rv)
    {
        rv->byteshashfn = (byteshashfn ? byteshashfn : cns_storage_defaultBytesHash32);
        rv->log2numbuckets = 4; // start with 16 buckets
        cns_Index bucketmemsize = (1 << rv->log2numbuckets) * sizeof(_cns_Storage_BucketItem*);
        rv->buckets = (_cns_Storage_BucketItem**) cns_runtime_alloc(cns, bucketmemsize);
        if (!rv->buckets)
        {
            cns_Error err = cns_lasterr(cns);
            cns_runtime_free(cns, rv);
            cns_setlasterr(cns, err);
            return 0;
        }
        rv->count = 0;
        memset(rv->buckets, 0, bucketmemsize);
        cns_setlasterr(cns, CNS_OK);
    }
    return rv;
}

void
cns_storage_free(cns_Runtime* cns, cns_Storage* storage)
{
    if (!cns || !storage)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return;
    }

    for (int i = 0; i < (1 << storage->log2numbuckets); ++i)
    {
        _cns_Storage_BucketItem* item = storage->buckets[i];
        while (item)
        {
            _cns_Storage_BucketItem* next = item->next;
            _cns_item_free(cns, item);
            item = next;
        }
    }
    cns_runtime_free(cns, storage->buckets);
    cns_runtime_free(cns, storage);
    cns_setlasterr(cns, CNS_OK);
}

void
cns_storage_set(cns_Runtime* cns, cns_Storage* storage, cns_Bytes* key, cns_Bytes* value)
{
    if (!cns || !storage || !key || !value)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return;
    }

    _cns_Storage_BucketItem** bucket = 0;
    _cns_Storage_BucketItem* item = _cns_storage_itemForKey(cns, storage, key, &bucket, 0);
    _cns_Storage_BucketItem* first = *bucket;

    if (item)
    {
        cns_Bytes* discardedValue = item->value;
        item->value = cns_bytes_copy(cns, value);
        if (!item->value)
        {
            // out of memory?
            item->value = discardedValue;
            return;
        }
        cns_bytes_free(cns, discardedValue);
    }
    else
    {
        // FIXME: when to rehash?
        if (storage->count > 2 * (1 << storage->log2numbuckets))
        {
            _cns_storage_changeCapacityBase(cns, storage, storage->log2numbuckets + 1);
            _cns_storage_itemForKey(cns, storage, key, &bucket, 0);
            first = *bucket;
        }

        item = cns_runtime_alloc(cns, sizeof(_cns_Storage_BucketItem));
        if (!item)
            return;
        item->key = cns_bytes_copy(cns, key);
        if (!item->key)
        {
            cns_runtime_free(cns, item);
            return;
        }
        item->value = cns_bytes_copy(cns, value);
        if (!item->value)
        {
            cns_bytes_free(cns, item->key);
            cns_runtime_free(cns, item);
            return;
        }
        item->next = first;
        *bucket = item;
        ++storage->count;
    }
    cns_setlasterr(cns, CNS_OK);
}

cns_Bytes*
cns_storage_get(cns_Runtime* cns, cns_Storage* storage, cns_Bytes* key)
{
    if (!cns || !storage || !key)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return 0;
    }

    _cns_Storage_BucketItem* item = _cns_storage_itemForKey(cns, storage, key, 0, 0);
    cns_setlasterr(cns, CNS_OK);
    return item ? cns_bytes_copy(cns, item->value) : 0;
}

cns_Bool
cns_storage_delete(cns_Runtime* cns, cns_Storage* storage, cns_Bytes* key)
{
    if (!cns || !storage || !key)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return CNS_NO;
    }

    _cns_Storage_BucketItem** bucket = 0;
    _cns_Storage_BucketItem* previousitem = 0;
    _cns_Storage_BucketItem* item = _cns_storage_itemForKey(cns, storage, key, &bucket, &previousitem);

    if (!item)
    {
        cns_setlasterr(cns, CNS_OK);
        return CNS_NO;
    }

    if (previousitem)
        previousitem->next = item->next;
    else
        *bucket = item->next;
    cns_bytes_free(cns, item->key);
    cns_bytes_free(cns, item->value);
    cns_runtime_free(cns, item);
    --storage->count;

    // FIXME: when to rehash?
    if (storage->count < 4 * (1 << storage->log2numbuckets))
    {
        _cns_storage_changeCapacityBase(cns, storage, storage->log2numbuckets - 2);
    }

    cns_setlasterr(cns, CNS_OK);
    return CNS_YES;
}

cns_Index
cns_storage_count(cns_Runtime* cns, cns_Storage* storage)
{
    if (!cns || !storage)
    {
        cns_setlasterr(cns, CNS_ERR_BADARG);
        return 0;
    }
    return storage->count;
}

