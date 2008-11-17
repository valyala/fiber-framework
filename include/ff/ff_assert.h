#ifndef FF_ASSERT_PUBLIC_H
#define FF_ASSERT_PUBLIC_H

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
