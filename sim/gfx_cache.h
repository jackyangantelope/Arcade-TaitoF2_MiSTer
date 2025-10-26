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

struct GfxPalette
{
    uint32_t rgb[16];
    uint64_t hash;
    uint64_t count;
};

#define FNV_64_PRIME ((uint64_t)0x100000001b3ULL)

enum class GfxCacheFormat
{
    TC0200OBJ,
    TC0480SCP,
    TC0100SCN,
    TC0100SCN_FG
};

class GfxCache
{
public:
    std::map<uint64_t, GfxCacheEntry> m_cache;
    GfxPalette m_palettes[512];

    MemoryInterface *m_colormem;
    SDL_Renderer *m_renderer = nullptr;
    uint64_t m_used_idx = 0;


    static uint64_t CalcHash(const void *buf, size_t len, uint64_t hval)
    {
        const unsigned char *bp = (const unsigned char *)buf;
        const unsigned char *be = bp + len;

        while (bp < be)
        {
            hval *= FNV_64_PRIME;
            hval ^= (uint64_t)*bp++;
        }

        return hval;
    }

    const GfxPalette *GetPalette(uint16_t index)
    {
        index %= 512;

        GfxPalette *entry = &m_palettes[index];
        entry->count++;

        if (entry->hash != 0 && (entry->count & 0xff) != 0)
        {
            return entry;
        }

        uint16_t addr = index * 32;
        uint8_t rawpal[32];
        m_colormem->Read(addr, 32, rawpal);

        uint64_t hash = CalcHash(rawpal, sizeof(rawpal), 0);

        if (hash == entry->hash)
        {
            return entry;
        }

        entry->hash = hash;

        bool dar260 = G_F2_SIGNAL(cfg_260dar);
        bool bpp15 = G_F2_SIGNAL(cfg_bpp15);
        bool bppmix = G_F2_SIGNAL(cfg_bppmix);


        for (int i = 0; i < 16; i++)
        {
            uint8_t r, g, b;
            uint16_t p = (rawpal[i * 2 + 1]) | (rawpal[i * 2 + 0] << 8);

            if (dar260)
            {
                if (bpp15 && bppmix)
                {
                    r = ((p & 0xf000) >> 8) | ((p & 0x0008) >> 0);
                    g = ((p & 0x0f00) >> 4) | ((p & 0x0004) << 1);
                    b = ((p & 0x00f0) >> 0) | ((p & 0x0002) << 2);
                }
                else if (bpp15)
                {
                    r = ((p & 0x7c00) >> 7);
                    g = ((p & 0x03e0) >> 2);
                    b = ((p & 0x001f) << 3);
                }
                else
                {
                    r = ((p & 0xf000) >> 8);
                    g = ((p & 0x0f00) >> 4);
                    b = ((p & 0x00f0) << 0);
                }
            }
            else
            {
                b = ((p & 0xfc00) >> 7);
                g = ((p & 0x03e0) >> 2);
                r = ((p & 0x001f) << 3);
            }

            uint32_t c = (r << 24) | (g << 16) | (b << 8);
            if (i & 15)
                entry->rgb[i] = c | 0xff;
            else
                entry->rgb[i] = 0xff0000ff;
        }
        return entry;
    }

    void Init(SDL_Renderer *renderer, MemoryInterface& colormem)
    {
        m_cache.clear();
        m_used_idx = 0;
        m_renderer = renderer;
        m_colormem = &colormem;
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

    SDL_Texture *GetTexture(MemoryRegion region, GfxCacheFormat format, uint16_t code, uint8_t palette_idx)
    {
        return GetTexture(g_sim_core.Memory(region), format, code, palette_idx);
    }

    SDL_Texture *GetTexture(MemoryInterface& gfxmem, GfxCacheFormat format, uint16_t code, uint8_t palette_idx)
    {
        const GfxPalette *palette = GetPalette(palette_idx);

        int size;
        int bytesize;
        bool dynamic;
        
        const bool bpp6 = G_F2_SIGNAL(tc0200obj,ctrl_6bpp);

        switch(format)
        {
            case GfxCacheFormat::TC0200OBJ:
            {
                size = 16;
                bytesize = bpp6 ? 16 * 16 : 16 * 8;
                dynamic = false;
                break;
            }

            case GfxCacheFormat::TC0100SCN:
            {
                size = 8;
                bytesize = 8 * 4;
                dynamic = false;
                break;
            }

            case GfxCacheFormat::TC0100SCN_FG:
            {
                size = 8;
                bytesize = 8 * 2;
                dynamic = true;
                break;
            }

            case GfxCacheFormat::TC0480SCP:
            {
                size = 16;
                bytesize = 16 * 8;
                dynamic = false;
                break;
            }
        }
       
        uint32_t addr = (code * bytesize);
        uint8_t src_data[16 * 16];

        uint64_t hash;

        if (dynamic)
        {
            gfxmem.Read(addr, bytesize, src_data);
            hash = CalcHash(src_data, bytesize, palette->hash);
        }
        else
        {
            hash = CalcHash(&code, sizeof(code), palette->hash);
        }

        auto it = m_cache.find(hash);
        if (it != m_cache.end())
        {
            it->second.last_used = m_used_idx;
            m_used_idx++;
            return it->second.texture;
        }

        if (!dynamic)
        {
            gfxmem.Read(addr, bytesize, src_data);
        }

        uint32_t pixels[16 * 16];
        const uint32_t *pal32 = palette->rgb;
        
        uint32_t *dest = pixels;
        const uint8_t *src = src_data;

        if (format == GfxCacheFormat::TC0200OBJ)
        {
            size = 16;
            if (bpp6)
            {
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
                for (int i = 0; i < 128; i++)
                {
                    dest[1] = pal32[((*src & 0xf0) >> 4)];
                    dest[0] = pal32[((*src & 0x0f) >> 0)];
                    dest += 2;
                    src++;
                }
            }
        }
        else if (format == GfxCacheFormat::TC0480SCP)
        {
            for (int i = 0; i < 16; i++)
            {
                dest[15] = pal32[((src[4] & 0xf0) >> 4)];
                dest[14] = pal32[((src[4] & 0x0f) >> 0)];
                dest[13] = pal32[((src[5] & 0xf0) >> 4)];
                dest[12] = pal32[((src[5] & 0x0f) >> 0)];
                dest[11] = pal32[((src[6] & 0xf0) >> 4)];
                dest[10] = pal32[((src[6] & 0x0f) >> 0)];
                dest[ 9] = pal32[((src[7] & 0xf0) >> 4)];
                dest[ 8] = pal32[((src[7] & 0x0f) >> 0)];
                dest[ 7] = pal32[((src[0] & 0xf0) >> 4)];
                dest[ 6] = pal32[((src[0] & 0x0f) >> 0)];
                dest[ 5] = pal32[((src[1] & 0xf0) >> 4)];
                dest[ 4] = pal32[((src[1] & 0x0f) >> 0)];
                dest[ 3] = pal32[((src[2] & 0xf0) >> 4)];
                dest[ 2] = pal32[((src[2] & 0x0f) >> 0)];
                dest[ 1] = pal32[((src[3] & 0xf0) >> 4)];
                dest[ 0] = pal32[((src[3] & 0x0f) >> 0)];
                dest += 16;
                src += 8;
            }
        }
        else if (format == GfxCacheFormat::TC0100SCN)
        {
            for (int i = 0; i < 8; i++)
            {
                dest[2] = pal32[((src[0] & 0xf0) >> 4)];
                dest[3] = pal32[((src[0] & 0x0f) >> 0)];
                dest[0] = pal32[((src[1] & 0xf0) >> 4)];
                dest[1] = pal32[((src[1] & 0x0f) >> 0)];
                dest[6] = pal32[((src[2] & 0xf0) >> 4)];
                dest[7] = pal32[((src[2] & 0x0f) >> 0)];
                dest[4] = pal32[((src[3] & 0xf0) >> 4)];
                dest[5] = pal32[((src[3] & 0x0f) >> 0)];
                dest += 8;
                src += 4;
            }
        }
        else if (format == GfxCacheFormat::TC0100SCN_FG)
        {
            for (int i = 0; i < 8; i++)
            {
                dest[7] = pal32[((src[1] >> 0) & 0x1) | ((src[0] << 1) & 0x2)];
                dest[6] = pal32[((src[1] >> 1) & 0x1) | ((src[0] << 0) & 0x2)];
                dest[5] = pal32[((src[1] >> 2) & 0x1) | ((src[0] >> 1) & 0x2)];
                dest[4] = pal32[((src[1] >> 3) & 0x1) | ((src[0] >> 2) & 0x2)];
                dest[3] = pal32[((src[1] >> 4) & 0x1) | ((src[0] >> 3) & 0x2)];
                dest[2] = pal32[((src[1] >> 5) & 0x1) | ((src[0] >> 4) & 0x2)];
                dest[1] = pal32[((src[1] >> 6) & 0x1) | ((src[0] >> 5) & 0x2)];
                dest[0] = pal32[((src[1] >> 7) & 0x1) | ((src[0] >> 6) & 0x2)];
                dest += 8;
                src += 2;
            }
        }

        SDL_Texture *tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STATIC, size, size);

        SDL_UpdateTexture(tex, nullptr, pixels, size * 4);

        auto r = m_cache.emplace(hash, GfxCacheEntry(tex, m_used_idx));
        return r.first->second.texture;
    }
};

#endif

