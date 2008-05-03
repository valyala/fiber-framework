#ifndef FF_DICTIONARY_PRIVATE
#define FF_DICTIONARY_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

struct ff_dictionary;

struct ff_dictionary *ff_dictionary_create();

void ff_dictionary_delete(struct ff_dictionary *dictionary);

void ff_dictionary_put(struct ff_dictionary *dictionary, void *key, void *value);

int ff_dictionary_get(struct ff_dictionary *dictionary, void *key, void **value);

int ff_dictionary_remove_entry(struct ff_dictionary *dictionary, void *key, void **value);

#ifdef __cplusplus
}
#endif

#endif
