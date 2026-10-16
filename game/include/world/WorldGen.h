#pragma once

#include <world/ChunkData.h>
#include <FastNoise/FastNoise.h>

namespace ScuffedMinecraft
{
	class WorldGen
	{
	public:
		WorldGen(float min, float max, float scale, int octaves, float gain, float lacunarity);
		~WorldGen() = default;

		void GenerateChunk(const glm::ivec3& id, ChunkData* data);

	private:
		FastNoise::SmartNode<FastNoise::FractalFBm> _noise;
	};
}