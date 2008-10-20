#ifndef FF_STREAM_TCP_PUBLIC
#define FF_STREAM_TCP_PUBLIC

#include "ff/ff_tcp.h"
#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

FF_API struct ff_stream *ff_stream_tcp_create(struct ff_tcp *tcp);

#ifdef __cplusplus
}
#endif

#endif
