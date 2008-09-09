#ifndef FF_ASSERT_PRIVATE
#define FF_ASSERT_PRIVATE

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @public
 * The same as assert()
 */
#define ff_assert assert

#ifdef __cplusplus
}
#endif

#endif
