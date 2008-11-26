#ifndef FF_PIPE_PUBLIC_H
#define FF_PIPE_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

struct ff_pipe;

/**
 * Creates two connected pipes with the given buffer_size.
 * The semantics is similar to the unix socketpair() system call.
 * Always returns correct pipes.
 */
FF_API void ff_pipe_create_pair(int buffer_size, struct ff_pipe **pipe1, struct ff_pipe **pipe2);

/**
 * Deletes the pipe.
 */
FF_API void ff_pipe_delete(struct ff_pipe *pipe);

/**
 * Reads len bytes from the pipe to the buf.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
FF_API enum ff_result ff_pipe_read(struct ff_pipe *pipe, void *buf, int len);

/**
 * Writes len bytes from the buf to the pipe.
 * Returns FF_SUCCESS on success, FF_FAILURE on error.
 */
FF_API enum ff_result ff_pipe_write(struct ff_pipe *pipe, const void *buf, int len);

/**
 * Disconnects the given pipe.
 * All subsequent calls to the ff_pipe_read() and ff_pipe_write() for the given pipe
 * and its paired pipe will immediately return FF_FAILURE.
 */
FF_API void ff_pipe_disconnect(struct ff_pipe *pipe);

#ifdef __cplusplus
}
#endif

#endif
