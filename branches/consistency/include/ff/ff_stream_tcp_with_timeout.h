#ifndef FF_STREAM_TCP_WITH_TIMEOUT_PUBLIC
#define FF_STREAM_TCP_WITH_TIMEOUT_PUBLIC

#include "ff/ff_tcp.h"
#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

FF_API struct ff_stream *ff_stream_tcp_with_timeout_create(struct ff_tcp *tcp, int read_timeout, int write_timeout);

#ifdef __cplusplus
}
#endif

#endif
