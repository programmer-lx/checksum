#include "checksum/xxhash3_128.h"

/* Use x86 dispatch for runtime CPU detection on x86 platforms */
#if CKS_ARCH_X86
    #include "xxh_x86dispatch.h"
#else
    #include "xxhash.h"
#endif
#include <string.h>

CKS_API cks_xxHash3_128_Context cks_xxhash3_128_begin(void)
{
    cks_xxHash3_128_Context ctx;
    ctx.internal_state = XXH3_createState();
    if (ctx.internal_state != NULL)
    {
        XXH3_128bits_reset((XXH3_state_t*)ctx.internal_state);
    }
    return ctx;
}

CKS_API cks_xxHash3_128_Context cks_xxhash3_128_begin_with_seed(uint64_t seed)
{
    cks_xxHash3_128_Context ctx;
    ctx.internal_state = XXH3_createState();
    if (ctx.internal_state != NULL)
    {
        XXH3_128bits_reset_withSeed((XXH3_state_t*)ctx.internal_state, seed);
    }
    return ctx;
}

CKS_API void cks_xxhash3_128_update(cks_xxHash3_128_Context* ctx, const void* data, size_t len)
{
    if (ctx != NULL && ctx->internal_state != NULL)
    {
        XXH3_128bits_update((XXH3_state_t*)ctx->internal_state, data, len);
    }
}

CKS_API cks_xxHash3_128 cks_xxhash3_128_end(cks_xxHash3_128_Context* ctx)
{
    cks_xxHash3_128 result;
    if (ctx != NULL && ctx->internal_state != NULL)
    {
        XXH128_hash_t hash = XXH3_128bits_digest((XXH3_state_t*)ctx->internal_state);
        /* 使用canonical表示（大端序）与官方hex值一致 */
        XXH128_canonical_t canonical;
        XXH128_canonicalFromHash(&canonical, hash);
        memcpy(result.bytes, canonical.digest, 16);
        XXH3_freeState((XXH3_state_t*)ctx->internal_state);
        ctx->internal_state = NULL;
    }
    else
    {
        memset(result.bytes, 0, 16);
    }
    return result;
}
