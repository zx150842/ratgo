/*  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
  This source code is licensed under the BSD-style license found in the
  LICENSE file in the root directory of this source tree. An additional grant
  of patent rights can be found in the PATENTS file in the same directory.
 Copyright (c) 2011 The LevelDB Authors. All rights reserved.
  Use of this source code is governed by a BSD-style license that can be
  found in the LICENSE file. See the AUTHORS file for names of contributors.

  C bindings for leveldb.  May be useful as a stable ABI that can be
  used by programs that keep leveldb in a shared library, or for
  a JNI api.

  Does not support:
  . getters for the option types
  . custom comparators that implement key shortening
  . capturing post-write-snapshot
  . custom iter, db, env, cache implementations using just the C bindings

  Some conventions:

  (1) We expose just opaque struct pointers and functions to clients.
  This allows us to change internal representations without having to
  recompile clients.

  (2) For simplicity, there is no equivalent to the Slice type.  Instead,
  the caller has to pass the pointer and length as separate
  arguments.

  (3) Errors are represented by a null-terminated c string.  NULL
  means no error.  All operations that can raise an error are passed
  a "char** errptr" as the last argument.  One of the following must
  be true on entry:
     *errptr == NULL
     *errptr points to a malloc()ed null-terminated error message
  On success, a leveldb routine leaves *errptr unchanged.
  On failure, leveldb frees the old value of *errptr and
  set *errptr to a malloc()ed error message.

  (4) Bools have the type unsigned char (0 == false; rest == true)

  (5) All of the pointer arguments must be non-NULL.
*/

#ifndef STORAGE_ROCKSDB_INCLUDE_C_H_
#define STORAGE_ROCKSDB_INCLUDE_C_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* Exported types */

typedef struct leveldb_t               leveldb_t;
typedef struct leveldb_cache_t         leveldb_cache_t;
typedef struct leveldb_comparator_t    leveldb_comparator_t;
typedef struct leveldb_env_t           leveldb_env_t;
typedef struct leveldb_filelock_t      leveldb_filelock_t;
typedef struct leveldb_filterpolicy_t  leveldb_filterpolicy_t;
typedef struct leveldb_iterator_t      leveldb_iterator_t;
typedef struct leveldb_logger_t        leveldb_logger_t;
typedef struct leveldb_options_t       leveldb_options_t;
typedef struct leveldb_randomfile_t    leveldb_randomfile_t;
typedef struct leveldb_readoptions_t   leveldb_readoptions_t;
typedef struct leveldb_seqfile_t       leveldb_seqfile_t;
typedef struct leveldb_snapshot_t      leveldb_snapshot_t;
typedef struct leveldb_writablefile_t  leveldb_writablefile_t;
typedef struct leveldb_writebatch_t    leveldb_writebatch_t;
typedef struct leveldb_writeoptions_t  leveldb_writeoptions_t;
typedef struct leveldb_flushoptions_t  leveldb_flushoptions_t;


/* DB operations */

extern leveldb_t* leveldb_open(
    const leveldb_options_t* options,
    const char* name,
    char** errptr);

extern void leveldb_close(leveldb_t* db);

extern void leveldb_put(
    leveldb_t* db,
    const leveldb_writeoptions_t* options,
    const char* key, size_t keylen,
    const char* val, size_t vallen,
    char** errptr);

extern void leveldb_delete(
    leveldb_t* db,
    const leveldb_writeoptions_t* options,
    const char* key, size_t keylen,
    char** errptr);

extern void leveldb_merge(
    leveldb_t* db,
    const leveldb_writeoptions_t* options,
    const char* key, size_t keylen,
    const char* val, size_t vallen,
    char** errptr);

extern void leveldb_write(
    leveldb_t* db,
    const leveldb_writeoptions_t* options,
    leveldb_writebatch_t* batch,
    char** errptr);

/* Returns NULL if not found.  A malloc()ed array otherwise.
   Stores the length of the array in *vallen. */
extern char* leveldb_get(
    leveldb_t* db,
    const leveldb_readoptions_t* options,
    const char* key, size_t keylen,
    size_t* vallen,
    char** errptr);

extern void leveldb_multi_get(
    leveldb_t* db,
    const leveldb_readoptions_t* options,
    int key_num,
    const char* const* key_array,
    const size_t* key_array_length,
    char*** value_array,
    size_t** value_array_length,
    char*** errptr);

extern leveldb_iterator_t* leveldb_create_iterator(
    leveldb_t* db,
    const leveldb_readoptions_t* options);

extern const leveldb_snapshot_t* leveldb_create_snapshot(
    leveldb_t* db);

extern void leveldb_release_snapshot(
    leveldb_t* db,
    const leveldb_snapshot_t* snapshot);

extern void leveldb_flush(leveldb_t *db, 
  leveldb_flushoptions_t* options, char** errptr);

/* Returns NULL if property name is unknown.
   Else returns a pointer to a malloc()-ed null-terminated value. */
extern char* leveldb_property_value(
    leveldb_t* db,
    const char* propname);

extern void leveldb_approximate_sizes(
    leveldb_t* db,
    int num_ranges,
    const char* const* range_start_key, const size_t* range_start_key_len,
    const char* const* range_limit_key, const size_t* range_limit_key_len,
    uint64_t* sizes);

extern void leveldb_compact_range(
    leveldb_t* db,
    const char* start_key, size_t start_key_len,
    const char* limit_key, size_t limit_key_len);

/* Management operations */

extern void leveldb_destroy_db(
    const leveldb_options_t* options,
    const char* name,
    char** errptr);

extern void leveldb_repair_db(
    const leveldb_options_t* options,
    const char* name,
    char** errptr);

/* Backup */
extern void leveldb_disable_file_deletions(
  leveldb_t *db);

extern void leveldb_enable_file_deletions(
  leveldb_t *db);

extern void leveldb_get_live_files(
  leveldb_t* db, 
  char*** file_array,
  size_t** file_array_length,
  int *file_num,
  uint64_t* manifest_size,
  unsigned char flush_memtable,
  char** errptr);


/* Iterator */

extern void leveldb_iter_destroy(leveldb_iterator_t*);
extern unsigned char leveldb_iter_valid(const leveldb_iterator_t*);
extern void leveldb_iter_seek_to_first(leveldb_iterator_t*);
extern void leveldb_iter_seek_to_last(leveldb_iterator_t*);
extern void leveldb_iter_seek(leveldb_iterator_t*, const char* k, size_t klen);
extern void leveldb_iter_next(leveldb_iterator_t*);
extern void leveldb_iter_prev(leveldb_iterator_t*);
extern const char* leveldb_iter_key(const leveldb_iterator_t*, size_t* klen);
extern const char* leveldb_iter_value(const leveldb_iterator_t*, size_t* vlen);
extern void leveldb_iter_get_error(const leveldb_iterator_t*, char** errptr);

/* Write batch */

extern leveldb_writebatch_t* leveldb_writebatch_create();
extern void leveldb_writebatch_destroy(leveldb_writebatch_t*);
extern void leveldb_writebatch_clear(leveldb_writebatch_t*);
extern void leveldb_writebatch_put(
    leveldb_writebatch_t*,
    const char* key, size_t klen,
    const char* val, size_t vlen);
extern void leveldb_writebatch_delete(
    leveldb_writebatch_t*,
    const char* key, size_t klen);
extern void leveldb_writebatch_iterate(
    leveldb_writebatch_t*,
    void* state,
    void (*put)(void*, const char* k, size_t klen, const char* v, size_t vlen),
    void (*deleted)(void*, const char* k, size_t klen));

/* Options */

extern leveldb_options_t* leveldb_options_create();
extern void leveldb_options_destroy(leveldb_options_t*);
extern void leveldb_options_set_comparator(
    leveldb_options_t*,
    leveldb_comparator_t*);
extern void leveldb_options_set_compression_per_level(
  leveldb_options_t* opt,
  int* level_values,
  size_t num_levels);
extern void leveldb_options_set_filter_policy(
    leveldb_options_t*,
    leveldb_filterpolicy_t*);
extern void leveldb_options_set_create_if_missing(
    leveldb_options_t*, unsigned char);
extern void leveldb_options_set_error_if_exists(
    leveldb_options_t*, unsigned char);
extern void leveldb_options_set_paranoid_checks(
    leveldb_options_t*, unsigned char);
extern void leveldb_options_set_env(leveldb_options_t*, leveldb_env_t*);
// buffer & cache
extern void leveldb_options_set_write_buffer_size(leveldb_options_t*, size_t);
extern void leveldb_options_set_max_open_files(leveldb_options_t*, int);
extern void leveldb_options_set_cache(leveldb_options_t*, leveldb_cache_t*);
extern void leveldb_options_set_compressed_cache(leveldb_options_t* opt, leveldb_cache_t* c);
extern void leveldb_options_set_max_write_buffer_number(leveldb_options_t*, size_t);
extern void leveldb_options_set_min_write_buffer_number_to_merge(leveldb_options_t* opt, size_t);
// block
extern void leveldb_options_set_block_size(leveldb_options_t*, size_t);
extern void leveldb_options_set_block_restart_interval(leveldb_options_t*, int);
// sync
extern void leveldb_options_set_use_fsync(
    leveldb_options_t*, unsigned char);
extern void leveldb_options_set_disable_data_sync(
    leveldb_options_t*, unsigned char);
// log
extern void leveldb_options_set_info_log(leveldb_options_t*, leveldb_logger_t*);
extern void leveldb_options_set_db_log_dir(leveldb_options_t*, const char*);
extern void leveldb_options_set_WAL_ttl_seconds(leveldb_options_t* opt, uint64_t ttl);
extern void leveldb_options_set_WAL_size_limit_MB(leveldb_options_t*, uint64_t);

// compaction
extern void leveldb_options_set_compression_options(
    leveldb_options_t* opt, int w_bits, int level, int strategy);
extern void leveldb_options_disable_auto_compaction(leveldb_options_t* opt, unsigned char v);
extern void leveldb_options_set_target_file_size_base(
    leveldb_options_t*, uint64_t);
extern void leveldb_options_set_target_file_size_multiplier(
    leveldb_options_t*, int);
extern void leveldb_options_set_max_bytes_for_level_base(
    leveldb_options_t*, uint64_t);
extern void leveldb_options_set_max_bytes_for_level_multiplier(
    leveldb_options_t*, int);
extern void leveldb_options_set_expanded_compaction_factor(
    leveldb_options_t*, int);
extern void leveldb_options_set_max_grandparent_overlap_factor(
    leveldb_options_t*, int);
extern void leveldb_options_set_num_levels(leveldb_options_t* opt, int);
extern void leveldb_options_set_level0_file_num_compaction_trigger(
    leveldb_options_t*, int);
extern void leveldb_options_set_level0_slowdown_writes_trigger(
    leveldb_options_t*, int);
extern void leveldb_options_set_level0_stop_writes_trigger(
    leveldb_options_t*, int);
extern void leveldb_options_set_max_mem_compaction_level(
    leveldb_options_t* opt, int);

enum {
  leveldb_no_compression = 0,
  leveldb_snappy_compression = 1
};
extern void leveldb_options_set_compression(leveldb_options_t*, int);

/* Comparator */

extern leveldb_comparator_t* leveldb_comparator_create(
    void* state,
    void (*destructor)(void*),
    int (*compare)(
        void*,
        const char* a, size_t alen,
        const char* b, size_t blen),
    const char* (*name)(void*));
extern void leveldb_comparator_destroy(leveldb_comparator_t*);

/* Filter policy */

extern leveldb_filterpolicy_t* leveldb_filterpolicy_create(
    void* state,
    void (*destructor)(void*),
    char* (*create_filter)(
        void*,
        const char* const* key_array, const size_t* key_length_array,
        int num_keys,
        size_t* filter_length),
    unsigned char (*key_may_match)(
        void*,
        const char* key, size_t length,
        const char* filter, size_t filter_length),
    const char* (*name)(void*));
extern void leveldb_filterpolicy_destroy(leveldb_filterpolicy_t*);

extern leveldb_filterpolicy_t* leveldb_filterpolicy_create_bloom(
    int bits_per_key);

/* Read options */

extern leveldb_readoptions_t* leveldb_readoptions_create();
extern void leveldb_readoptions_destroy(leveldb_readoptions_t*);
extern void leveldb_readoptions_set_verify_checksums(
    leveldb_readoptions_t*,
    unsigned char);
extern void leveldb_readoptions_set_fill_cache(
    leveldb_readoptions_t*, unsigned char);
extern void leveldb_readoptions_set_snapshot(
    leveldb_readoptions_t*,
    const leveldb_snapshot_t*);

extern void leveldb_readoptions_set_read_prefix(
    leveldb_readoptions_t*,
    const char* prefix,
    size_t prefix_len);

/* Write options */

extern leveldb_writeoptions_t* leveldb_writeoptions_create();
extern void leveldb_writeoptions_destroy(leveldb_writeoptions_t*);
extern void leveldb_writeoptions_set_sync(
    leveldb_writeoptions_t*, unsigned char);
extern void leveldb_writeoptions_set_disable_wal(
    leveldb_writeoptions_t*, unsigned char);

/* Cache */

extern leveldb_cache_t* leveldb_cache_create_lru(size_t capacity);
extern void leveldb_cache_destroy(leveldb_cache_t* cache);

/* Env */

extern leveldb_env_t* leveldb_create_default_env();
extern void leveldb_env_destroy(leveldb_env_t*);

/* Utility */
/* Calls free(ptr).
   REQUIRES: ptr was malloc()-ed and returned by one of the routines
   in this file.  Note that in certain cases (typically on Windows), you
   may need to call this routine instead of free(ptr) to dispose of
   malloc()-ed memory returned by this library. */

extern void leveldb_free(void* ptr);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif  /* STORAGE_ROCKSDB_INCLUDE_C_H_ */
