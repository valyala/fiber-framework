#ifndef FF_LOOPBACK_PRIVATE_H
#define FF_LOOPBACK_PRIVATE_H


#ifdef __cplusplus
extern "C" {
#endif

struct ff_loopback;

struct ff_loopback *ff_loopback_create(int buffer_size);

void ff_loopback_delete(struct ff_loopback *loopback);

enum ff_result ff_loopback_read(struct ff_loopback *loopback, void *buf, int len);

enum ff_result ff_loopback_write(struct ff_loopback *loopback, const void *buf, int len);

void ff_loopback_disconnect(struct ff_loopback *loopback);

#ifdef __cplusplus
}
#endif

#endif
