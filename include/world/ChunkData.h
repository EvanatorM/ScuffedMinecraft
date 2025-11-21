#pragma once

#include <wv/core.h>
#include <cassert>

namespace ScuffedMinecraft
{
    constexpr int CHUNK_SIZE = 32;
    using BlockId = uint32_t;

    struct ChunkData
    {
        ~ChunkData()
        {

        }

        static constexpr bool InBounds(int x, int y, int z) noexcept
        {
            return 0 <= x && x < CHUNK_SIZE &&
                0 <= y && y < CHUNK_SIZE &&
                0 <= z && z < CHUNK_SIZE;
        }

        static constexpr int Index(int x, int y, int z) noexcept
        {
            return y + CHUNK_SIZE * (x + CHUNK_SIZE * z);
        }

        inline BlockId Get(int x, int y, int z) const noexcept
        {
            assert(InBounds(x, y, z));
            return voxels[Index(x, y, z)];
        }

        inline void Set(int x, int y, int z, BlockId value) noexcept
        {
            assert(InBounds(x, y, z));
            voxels[Index(x, y, z)] = value;
        }

        inline void Clear(bool value = false) noexcept
        {
            for (auto& v : voxels)
                v = value;
        }

        BlockId voxels[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    };
}