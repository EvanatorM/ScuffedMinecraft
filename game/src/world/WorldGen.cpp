#include <world/WorldGen.h>

namespace ScuffedMinecraft
{
	WorldGen::WorldGen(float min, float max, float scale, int octaves, float gain, float lacunarity)
	{
        auto simplex = FastNoise::New<FastNoise::Simplex>();
        simplex->SetOutputMin(min);
        simplex->SetOutputMax(max);
        simplex->SetScale(scale);
        _noise = FastNoise::New<FastNoise::FractalFBm>();
        _noise->SetSource(simplex);
	}

    void WorldGen::GenerateChunk(const glm::ivec3& id, ChunkData* data)
    {
        glm::ivec3 chunkOffset = { id.x * CHUNK_SIZE, id.y * CHUNK_SIZE, id.z * CHUNK_SIZE };

	    float heights[CHUNK_SIZE * CHUNK_SIZE];
	    _noise->GenUniformGrid2D(heights, chunkOffset.x, chunkOffset.z, CHUNK_SIZE, CHUNK_SIZE, 1.0f, 1.0f, 0);

        int index = 0;
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int worldZ = chunkOffset.z + z;
            for (int x = 0; x < CHUNK_SIZE; x++)
            {
                int worldX = chunkOffset.x + x;
                int height = std::round(heights[z * CHUNK_SIZE + x]);
                for (int y = 0; y < CHUNK_SIZE; y++)
                {
                    int worldY = chunkOffset.y + y;
                    if (worldY == height)
                        data->SetBlock(index, 3);
                    else if (worldY < height)
                        data->SetBlock(index, 2);

                    index++;
                }
            }
        }
    }
}