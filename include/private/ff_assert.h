#ifndef FF_ASSERT_PUBLIC
#define FF_ASSERT_PUBLIC

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
