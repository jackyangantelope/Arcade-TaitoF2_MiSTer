#if !defined(GFX_CACHE_H)
#define GFX_CACHE_H 1

#include <SDL.h>

#include "sim_core.h"
#include "sim_hierarchy.h"

#include "F2.h"
#include "F2___024root.h"

struct GfxCacheEntry
{
    SDL_Texture *texture;
    uint64_t last_used;

    GfxCacheEntry() : texture(nullptr), last_used(0) {}

    GfxCacheEntry(SDL_Texture *tex, uint64_t used) : texture(tex), last_used(used) {}

    GfxCacheEntry(GfxCacheEntry &&o)
    {
        texture = o.texture;
        last_used = o.last_used;
        o.texture = nullptr;
    }

    ~GfxCacheEntry()
    {
        if (texture)
            SDL_DestroyTexture(texture);
    }

    GfxCacheEntry(const GfxCacheEntry &) = delete;
};

#define FNV_64_PRIME ((uint64_t)0x100000001b3ULL)

enum class GfxCacheFormat
{
    TC0200OBJ,
    TC0200OBJ_6BPP,
    TC0480SCP,
    TC0100SCN
};

class GfxCache
{
public:
    std::map<uint64_t, GfxCacheEntry> m_cache;
    SDL_Renderer *m_renderer = nullptr;
    uint64_t m_used_idx = 0;
    GfxCacheFormat m_format = GfxCacheFormat::TC0200OBJ;
    const uint8_t *m_palette_low = nullptr;
    const uint8_t *m_palette_high = nullptr;
    const uint8_t *m_gfxmem = nullptr;


    static uint64_t CalcHash(void *buf, size_t len, uint64_t hval)
    {
        unsigned char *bp = (unsigned char *)buf;
        unsigned char *be = bp + len;

        while (bp < be)
        {
            hval *= FNV_64_PRIME;
            hval ^= (uint64_t)*bp++;
        }

        return hval;
    }

    void Init(SDL_Renderer *renderer, GfxCacheFormat format, const void *gfxmem, const void *palmem_low, const void *palmem_high)
    {
        m_cache.clear();
        m_used_idx = 0;
        m_renderer = renderer;
        m_format = format;
        m_gfxmem = (const uint8_t *)gfxmem;
        m_palette_low = (const uint8_t *)palmem_low;
        m_palette_high = (const uint8_t *)palmem_high;
    }

    void PruneCache()
    {
        if (m_cache.size() < 2048)
            return;

        size_t num_to_remove = 128;

        std::vector<std::pair<uint64_t, uint64_t>> hash_ages;
        for (const auto &it : m_cache)
        {
            hash_ages.push_back({it.second.last_used, it.first});
        }
        std::sort(hash_ages.begin(), hash_ages.end());
        for (size_t i = 0; i < num_to_remove; i++)
        {
            m_cache.erase(hash_ages[i].second);
        }
    }

    SDL_Texture *GetTexture(uint16_t code, uint8_t palette)
    {
        uint16_t pal_ofs = (palette & 0xfc) * 16;

        uint16_t rawpal[64];

        for (int i = 0; i < 64; i++)
        {
            rawpal[i] = (m_palette_high[pal_ofs + i] << 8) |
                        (m_palette_low[pal_ofs + i] << 0);
        }

        uint64_t hash = CalcHash(rawpal, sizeof(rawpal), code);
        auto it = m_cache.find(hash);
        if (it != m_cache.end())
        {
            it->second.last_used = m_used_idx;
            m_used_idx++;
            return it->second.texture;
        }

        bool dar260 = G_F2_SIGNAL(cfg_260dar);
        bool bpp15 = G_F2_SIGNAL(cfg_bpp15);
        bool bppmix = G_F2_SIGNAL(cfg_bppmix);

        bool bpp6 = true;

        uint32_t pal32[64];
        for (int i = 0; i < 64; i++)
        {
            uint8_t r, g, b;
            uint16_t p = rawpal[i];

            if (dar260)
            {
                if (bpp15 && bppmix)
                {
                    r = ((rawpal[i] & 0xf000) >> 8) | ((rawpal[i] & 0x0008) >> 0);
                    g = ((rawpal[i] & 0x0f00) >> 4) | ((rawpal[i] & 0x0004) << 1);
                    b = ((rawpal[i] & 0x00f0) >> 0) | ((rawpal[i] & 0x0002) << 2);
                }
                else if (bpp15)
                {
                    r = ((p & 0x7c00) >> 7);
                    g = ((p & 0x03e0) >> 2);
                    b = ((p & 0x001f) << 3);
                }
                else
                {
                    r = ((rawpal[i] & 0xf000) >> 8);
                    g = ((rawpal[i] & 0x0f00) >> 4);
                    b = ((rawpal[i] & 0x00f0) << 0);
                }
            }
            else
            {
                b = ((p & 0xfc00) >> 7);
                g = ((p & 0x03e0) >> 2);
                r = ((p & 0x001f) << 3);
            }

            uint32_t c = (r << 24) | (g << 16) | (b << 8);
            pal32[i] = c;
        }


        const uint8_t *src;
        int size;

        uint32_t pixels[16 * 16];
        uint32_t *dest = pixels;

        int pal_align = (palette & 0x3) << 4;

        if (m_format == GfxCacheFormat::TC0200OBJ)
        {
            size = 16;
            if (bpp6)
            {
                src = m_gfxmem + (code * (size * size));
                for (int i = 0; i < 64; i++)
                {
                    dest[1] = pal32[((src[1] & 0xf0) >> 4)];
                    dest[0] = pal32[((src[1] & 0x0f) >> 0)];
                    dest[3] = pal32[((src[0] & 0xf0) >> 4)];
                    dest[2] = pal32[((src[0] & 0x0f) >> 0)];
                    dest += 4;
                    src  += 4;
                }
            }
            else
            {
                src = m_gfxmem + (code * (size * size) / 2);
                for (int i = 0; i < 128; i++)
                {
                    dest[1] = pal32[pal_align + ((*src & 0xf0) >> 4)];
                    dest[0] = pal32[pal_align + ((*src & 0x0f) >> 0)];
                    dest += 2;
                    src++;
                }
            }
        }
        else if (m_format == GfxCacheFormat::TC0480SCP)
        {
            size = 16;
            src = m_gfxmem + (code * (size * size) / 2);
            for (int i = 0; i < 16; i++)
            {
                dest[15] = pal32[pal_align + ((src[4] & 0xf0) >> 4)];
                dest[14] = pal32[pal_align + ((src[4] & 0x0f) >> 0)];
                dest[13] = pal32[pal_align + ((src[5] & 0xf0) >> 4)];
                dest[12] = pal32[pal_align + ((src[5] & 0x0f) >> 0)];
                dest[11] = pal32[pal_align + ((src[6] & 0xf0) >> 4)];
                dest[10] = pal32[pal_align + ((src[6] & 0x0f) >> 0)];
                dest[ 9] = pal32[pal_align + ((src[7] & 0xf0) >> 4)];
                dest[ 8] = pal32[pal_align + ((src[7] & 0x0f) >> 0)];
                dest[ 7] = pal32[pal_align + ((src[0] & 0xf0) >> 4)];
                dest[ 6] = pal32[pal_align + ((src[0] & 0x0f) >> 0)];
                dest[ 5] = pal32[pal_align + ((src[1] & 0xf0) >> 4)];
                dest[ 4] = pal32[pal_align + ((src[1] & 0x0f) >> 0)];
                dest[ 3] = pal32[pal_align + ((src[2] & 0xf0) >> 4)];
                dest[ 2] = pal32[pal_align + ((src[2] & 0x0f) >> 0)];
                dest[ 1] = pal32[pal_align + ((src[3] & 0xf0) >> 4)];
                dest[ 0] = pal32[pal_align + ((src[3] & 0x0f) >> 0)];
                dest += 16;
                src += 8;
            }
        }
        else if (m_format == GfxCacheFormat::TC0100SCN)
        {
            size = 8;
            src = m_gfxmem + (code * (size * size) / 2);
            for (int i = 0; i < 8; i++)
            {
                dest[2] = pal32[pal_align + ((src[0] & 0xf0) >> 4)];
                dest[3] = pal32[pal_align + ((src[0] & 0x0f) >> 0)];
                dest[0] = pal32[pal_align + ((src[1] & 0xf0) >> 4)];
                dest[1] = pal32[pal_align + ((src[1] & 0x0f) >> 0)];
                dest[6] = pal32[pal_align + ((src[2] & 0xf0) >> 4)];
                dest[7] = pal32[pal_align + ((src[2] & 0x0f) >> 0)];
                dest[4] = pal32[pal_align + ((src[3] & 0xf0) >> 4)];
                dest[5] = pal32[pal_align + ((src[3] & 0x0f) >> 0)];
                dest += 8;
                src += 4;
            }
        }

        SDL_Texture *tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBX8888,
                                             SDL_TEXTUREACCESS_STATIC, size, size);

        SDL_UpdateTexture(tex, nullptr, pixels, size * 4);

        auto r = m_cache.emplace(hash, GfxCacheEntry(tex, m_used_idx));
        return r.first->second.texture;
    }
};

#endif

