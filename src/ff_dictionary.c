#include "private/ff_common.h"

#include "private/ff_dictionary.h"

struct dictionary_entry
{
	struct dictionary_entry *next;
	const void *key;
	const void *value;
};

struct ff_dictionary
{
	struct dictionary_entry **buckets;
	ff_dictionary_get_key_hash_func get_key_hash_func;
	ff_dictionary_is_equal_keys_func is_equal_keys_func;
	int order;
	int entries_cnt;
};

#define MASK(order) ((1ul << (order)) - 1)

static uint32_t get_bucket_num(struct ff_dictionary *dictionary, const void *key)
{
	uint32_t hash_value;
	uint32_t bucket_num;

	hash_value = dictionary->get_key_hash_func(key);
	bucket_num = hash_value & MASK(dictionary->order);
	return bucket_num;
}

struct ff_dictionary *ff_dictionary_create(int order, ff_dictionary_get_key_hash_func get_key_hash_func, ff_dictionary_is_equal_keys_func is_equal_keys_func)
{
	struct ff_dictionary *dictionary;
	uint32_t buckets_cnt;

	ff_assert(order >= 0);
	ff_assert(order <= 20);
	ff_assert(get_key_hash_func != NULL);
	ff_assert(is_equal_keys_func != NULL);
	buckets_cnt = 1ul << order;

	dictionary = (struct ff_dictionary *) ff_malloc(sizeof(*dictionary));
	dictionary->buckets = (struct dictionary_entry **) ff_calloc(buckets_cnt, sizeof(dictionary->buckets[0]));
	dictionary->get_key_hash_func = get_key_hash_func;
	dictionary->is_equal_keys_func = is_equal_keys_func;
	dictionary->order = order;
	dictionary->entries_cnt = 0;

	return dictionary;
}

void ff_dictionary_delete(struct ff_dictionary *dictionary)
{
	ff_assert(dictionary->entries_cnt == 0);

	ff_free(dictionary->buckets);
	ff_free(dictionary);
}

void ff_dictionary_remove_all_entries(struct ff_dictionary *dictionary)
{
	struct dictionary_entry **buckets;
	int buckets_cnt;
	int entries_cnt;
	int i;

	buckets_cnt = 1l << dictionary->order;
	entries_cnt = dictionary->entries_cnt;
	buckets = dictionary->buckets;
	for (i = 0; i < buckets_cnt; i++)
	{
		struct dictionary_entry *entry;

		entry = buckets[i];
		while (entry != NULL)
		{
			struct dictionary_entry *next_entry;

			next_entry = entry->next;
			ff_free(entry);
			entry = next_entry;
			entries_cnt--;
		}
		buckets[i] = NULL;
	}
	ff_assert(entries_cnt == 0);
	dictionary->entries_cnt = 0;
}

enum ff_result ff_dictionary_put_entry(struct ff_dictionary *dictionary, const void *key, const void *value)
{
	struct dictionary_entry *entry;
	ff_dictionary_is_equal_keys_func is_equal_keys_func;
	uint32_t bucket_num;
	enum ff_result result = FF_FAILURE;

	is_equal_keys_func = dictionary->is_equal_keys_func;
	bucket_num = get_bucket_num(dictionary, key);
	entry = dictionary->buckets[bucket_num];
	while (entry != NULL)
	{
		int is_equal;

		is_equal = is_equal_keys_func(key, entry->key);
		if (is_equal)
		{
			goto end;
		}
		entry = entry->next;
	}

	entry = (struct dictionary_entry *) ff_malloc(sizeof(*entry));
	entry->next = dictionary->buckets[bucket_num];
	entry->key = key;
	entry->value = value;
	dictionary->buckets[bucket_num] = entry;
	dictionary->entries_cnt++;
	result = FF_SUCCESS;

end:
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"the dictionary=%p already contains an entry with key=%p", dictionary, key);
	}
	return result;
}

enum ff_result ff_dictionary_get_entry(struct ff_dictionary *dictionary, const void *key, const void **value)
{
	struct dictionary_entry *entry;
	ff_dictionary_is_equal_keys_func is_equal_keys_func;
	uint32_t bucket_num;
	enum ff_result result = FF_FAILURE;

	is_equal_keys_func = dictionary->is_equal_keys_func;
	bucket_num = get_bucket_num(dictionary, key);
	entry = dictionary->buckets[bucket_num];
	while (entry != NULL)
	{
		int is_equal;

		is_equal = is_equal_keys_func(key, entry->key);
		if (is_equal)
		{
			*value = entry->value;
			result = FF_SUCCESS;
			break;
		}
		entry = entry->next;
	}
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"the entry with key=%p doesn't exist in the dictionary=%p", key, dictionary);
	}
	return result;
}

enum ff_result ff_dictionary_remove_entry(struct ff_dictionary *dictionary, const void *key, const void **value)
{
	struct dictionary_entry **entry_ptr;
	struct dictionary_entry *entry;
	ff_dictionary_is_equal_keys_func is_equal_keys_func;
	uint32_t bucket_num;
	enum ff_result result = FF_FAILURE;

	is_equal_keys_func = dictionary->is_equal_keys_func;
	bucket_num = get_bucket_num(dictionary, key);
	entry_ptr = &dictionary->buckets[bucket_num];
	entry = dictionary->buckets[bucket_num];
	while (entry != NULL)
	{
		int is_equal;

		is_equal = is_equal_keys_func(key, entry->key);
		if (is_equal)
		{
			*entry_ptr = entry->next;
			*value = entry->value;
			ff_free(entry);
			dictionary->entries_cnt--;
			result = FF_SUCCESS;
			break;
		}
		entry_ptr = &entry->next;
		entry = entry->next;
	}
	if (result != FF_SUCCESS)
	{
		ff_log_debug(L"the entry with key=%p doesn't exist in the dictionary=%p", key, dictionary);
	}
	return result;
}

void ff_dictionary_for_each_entry(struct ff_dictionary *dictionary, ff_dictionary_for_each_entry_func for_each_entry_func, void *ctx)
{
	struct dictionary_entry **buckets;
	int buckets_cnt;
	int i;

	buckets_cnt = 1l << dictionary->order;
	buckets = dictionary->buckets;
	for (i = 0; i < buckets_cnt; i++)
	{
		struct dictionary_entry *entry;

		entry = buckets[i];
		while (entry != NULL)
		{
			for_each_entry_func(entry->key, entry->value, ctx);
			entry = entry->next;
		}
	}
}
