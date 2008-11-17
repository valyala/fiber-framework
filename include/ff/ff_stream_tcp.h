#ifndef FF_STREAM_TCP_PUBLIC_H
#define FF_STREAM_TCP_PUBLIC_H

#include "ff/ff_tcp.h"
#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates tcp stream using the given tcp.
 * This function acquires the tcp, so the caller mustn't delete the tcp!
 * Always returns correct result.
 */
FF_API struct ff_stream *ff_stream_tcp_create(struct ff_tcp *tcp);

#ifdef __cplusplus
}
#endif

#endif
