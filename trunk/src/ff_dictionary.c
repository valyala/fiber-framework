#include "private/ff_common.h"

#include "private/ff_dictionary.h"

struct dictionary_entry
{
	struct dictionary_entry *next;
	void *key;
	void *value;
};

struct ff_dictionary
{
	struct dictionary_entry *head;
};

struct ff_dictionary *ff_dictionary_create()
{
	struct ff_dictionary *dictionary;

	dictionary = (struct ff_dictionary *) ff_malloc(sizeof(*dictionary));
	dictionary->head = NULL;

	return dictionary;
}

void ff_dictionary_delete(struct ff_dictionary *dictionary)
{
	ff_assert(dictionary->head == NULL);

	ff_free(dictionary);
}

void ff_dictionary_put(struct ff_dictionary *dictionary, void *key, void *value)
{
	struct dictionary_entry *entry;

	entry = (struct dictionary_entry *) ff_malloc(sizeof(*entry));
	entry->next = dictionary->head;
	entry->key = key;
	entry->value = value;
	dictionary->head = entry;
}

int ff_dictionary_get(struct ff_dictionary *dictionary, void *key, void **value)
{
	int is_found = 0;
	struct dictionary_entry *entry;

	entry = dictionary->head;
	while (entry != NULL)
	{
		if (entry->key == key)
		{
			*value = entry->value;
			is_found = 1;
			break;
		}
		entry = entry->next;
	}
	return is_found;
}

int ff_dictionary_remove_entry(struct ff_dictionary *dictionary, void *key, void **value)
{
	int is_removed = 0;
	struct dictionary_entry **entry_ptr;
	struct dictionary_entry *entry;

	entry_ptr = &dictionary->head;
	entry = dictionary->head;
	while (entry != NULL)
	{
		if (entry->key == key)
		{
			*entry_ptr = entry->next;
			*value = entry->value;
			ff_free(entry);
			is_removed = 1;
			break;
		}
		entry_ptr = &entry->next;
		entry = entry->next;
	}
	return is_removed;
}
