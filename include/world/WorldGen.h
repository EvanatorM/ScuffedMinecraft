#pragma once

#include <world/ChunkData.h>

namespace ScuffedMinecraft
{
    namespace WorldGen
    {
        void InitWorldGen();
        void Generate(ChunkData* data, const glm::ivec3& chunkPos);
    }
}