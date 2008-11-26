#ifndef FF_STREAM_PIPE_PUBLIC_H
#define FF_STREAM_PIPE_PUBLIC_H

#include "ff/ff_pipe.h"
#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates pipe stream using the given pipe.
 * This function acquires the pipe, so the caller mustn't delete the pipe!
 * Always returns correct result.
 */
FF_API struct ff_stream *ff_stream_pipe_create(struct ff_pipe *pipe);

#ifdef __cplusplus
}
#endif

#endif
