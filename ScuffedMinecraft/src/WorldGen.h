#pragma once

#include <vector>
#include "NoiseSettings.h"
#include "SurfaceFeature.h"
#include "ChunkData.h"
#include "glm/glm.hpp"

namespace WorldGen
{
	void GenerateChunkData(const glm::ivec3& chunkPos, uint16_t* chunkData);
}