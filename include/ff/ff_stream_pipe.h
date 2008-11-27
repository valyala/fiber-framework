#ifndef FF_STREAM_PIPE_PUBLIC_H
#define FF_STREAM_PIPE_PUBLIC_H

#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates pair of connected pipe streams.
 * Always returns correct result.
 */
FF_API void ff_stream_pipe_create_pair(int buffer_size, struct ff_stream **stream1, struct ff_stream **stream2);

#ifdef __cplusplus
}
#endif

#endif
