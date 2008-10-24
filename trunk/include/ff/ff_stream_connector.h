#ifndef FF_STREAM_CONNECTOR_PUBLIC
#define FF_STREAM_CONNECTOR_PUBLIC

#include "ff/ff_common.h"
#include "ff/ff_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ff_stream_connector;

struct ff_stream_connector_vtable
{
	void (*delete)(struct ff_stream_connector *stream_connector);
	struct ff_stream *(*connect)(struct ff_stream_connector *stream_connector);
};

FF_API struct ff_stream_connector *ff_stream_connector_create(const struct ff_stream_connector_vtable *vtable, void *ctx);

FF_API void ff_stream_connector_delete(struct ff_stream_connector *stream_connector);

FF_API void *ff_stream_connector_get_ctx(struct ff_stream_connector *stream_connector);

FF_API struct ff_stream *ff_stream_connector_connect(struct ff_stream_connector *stream_connector);

#ifdef __cplusplus
}
#endif

#endif
