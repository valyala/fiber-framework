#ifndef FF_DICTIONARY_PUBLIC_H
#define FF_DICTIONARY_PUBLIC_H

#include "ff/ff_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_dictionary;

/**
 * This callback must calculate hash value for the given key.
 * This hash value is used only inside ff_dictionary, so it is safe to calculate
 * different hash values for low-endian and big-endian machines.
 */
typedef uint32_t (*ff_dictionary_get_key_hash_func)(const void *key);

/**
 * This callback must return non-zero if the key1 is equal to the key2.
 * Otherwise it must return zero.
 */
typedef int (*ff_dictionary_is_equal_keys_func)(const void *key1, const void *key2);

/**
 * this callback is called when removing corresponding entry from the dictionary using
 * the ff_dictionary_remove_all_entries() function.
 */
typedef void (*ff_dictionary_remove_entry_func)(const void *key, const void *value, void *remove_entry_ctx);

/**
 * Creates a dictionary with the fixed hash table size equal to the (1 << order).
 * In order to achieve better speed use the order, which is calculated from the given formula:
 * (1 << order) >= expected_dictionary_size.
 * The order value cannot exceed 20.
 * This function always returns correct result.
 */
FF_API struct ff_dictionary *ff_dictionary_create(int order, ff_dictionary_get_key_hash_func get_key_hash_func, ff_dictionary_is_equal_keys_func is_equal_keys_func);

/**
 * Deletes the given dictionary.
 * The dictionary must be empty before calling this function.
 * The dictionary can be empties by either calling ff_dictionary_remove_entry() for all the added entries
 * either by calling the ff_dictionary_remove_all_entries().
 */
FF_API void ff_dictionary_delete(struct ff_dictionary *dictionary);

/**
 * Deletes all added entries from the dictionary.
 * The remove_entry_func is called for each entry before removing it from the dictionary.
 * This callback mustn't access or modify the dictionary (directly or indirectly).
 * The callback must be responsible for deleting the key and value passed to it.
 * If the remove_entry_func can block, then the dictionary must be protected by the ff_mutex().
 * the remove_entry_ctx is passed to the remove_entry_func.
 */
FF_API void ff_dictionary_remove_all_entries(struct ff_dictionary *dictionary, ff_dictionary_remove_entry_func remove_entry_func, void *remove_entry_ctx);

/**
 * Adds the entry with the given key and the given value to the dictionary.
 * Returns FF_SUCCESS if the entry has been put to the dictionary.
 * Returns FF_FAILURE if the dictionary already contains an entry with the given key.
 * In this case the entry won't be put to the dictionary.
 */
FF_API enum ff_result ff_dictionary_add_entry(struct ff_dictionary *dictionary, const void *key, const void *value);

/**
 * Obtains the entry value with the given key from the given dictionary.
 * Returns FF_SUCCESS if the entry has been obtained.
 * Returns FF_FAILURE is there is no entry with the given key in the dictionary.
 */
FF_API enum ff_result ff_dictionary_get_entry(struct ff_dictionary *dictionary, const void *key, const void **value);

/**
 * Removes the entry with the given key from the dictionary.
 * The key and value, which where passed to the ff_dictionary_add_entry(), are returned to the entry_key and entry_value.
 * The caller is responsible for deleting the entry_key and entry_value.
 * Returns FF_SUCCESS if the entry has been deleted.
 * Returns FF_FAILURE if there is no entry with the given key in the dictionary.
 */
FF_API enum ff_result ff_dictionary_remove_entry(struct ff_dictionary *dictionary, const void *key, const void **entry_key, const void **entry_value);

#ifdef __cplusplus
}
#endif

#endif
