#ifndef FF_DICTIONARY_PRIVATE_H
#define FF_DICTIONARY_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

struct ff_dictionary;

struct ff_dictionary *ff_dictionary_create(int order);

void ff_dictionary_delete(struct ff_dictionary *dictionary);

void ff_dictionary_put(struct ff_dictionary *dictionary, const void *key, const void *value);

enum ff_result ff_dictionary_get(struct ff_dictionary *dictionary, const void *key, const void **value);

enum ff_result ff_dictionary_remove_entry(struct ff_dictionary *dictionary, const void *key, const void **value);

#ifdef __cplusplus
}
#endif

#endif
