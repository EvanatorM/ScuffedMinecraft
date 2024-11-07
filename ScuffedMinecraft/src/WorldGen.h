#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "NoiseSettings.h"
#include "SurfaceFeature.h"
#include "ChunkData.h"

namespace WorldGen
{
	void GenerateChunkData(const glm::ivec3& chunkPos, uint16_t* chunkData);
}