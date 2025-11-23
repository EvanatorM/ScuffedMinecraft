#pragma once

#include <wv/VoxelWorlds.h>

namespace ScuffedMinecraft
{
    class ScuffedWorldGen : public WillowVox::WorldGen
    {
    public:
        static void InitWorldGen();
        void Generate(WillowVox::ChunkData* data, const glm::ivec3& chunkPos) override;
    };
}