#ifndef FF_CONTAINER_PRIVATE_H
#define FF_CONTAINER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

struct ff_container;

struct ff_container_entry;

struct ff_container *ff_container_create();

void ff_container_delete(struct ff_container *container);

struct ff_container_entry *ff_container_add_entry(struct ff_container *container, const void *data);

void ff_container_remove_entry(struct ff_container_entry *entry);

typedef void (*ff_container_for_each_func)(const void *data, void *ctx);

void ff_container_for_each(struct ff_container *container, ff_container_for_each_func for_each_func, void *ctx);

int ff_container_is_empty(struct ff_container *container);

#ifdef __cplusplus
}
#endif

#endif
