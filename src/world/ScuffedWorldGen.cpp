#include <world/ScuffedWorldGen.h>

#include <FastNoiseLite.h>

using namespace WillowVox;

namespace ScuffedMinecraft
{
    static FastNoiseLite noise;

    inline float GetSurfaceNoise(float x, float y, float z)
    {
        float n = std::min(noise.GetNoise(x * 1.1f, y * 1.1f, z * 1.1f) * 32.0f, 0.0f);
        n += noise.GetNoise(x * 1.6f, y * 1.6f, z * 1.6f) * 4.0f;
        n += noise.GetNoise(x * 2.1f, y * 2.1f, z * 2.1f) * 2.0f;
        return n;
    }

    inline float GetCaveNoise(float x, float y, float z)
    {
        float n = noise.GetNoise(x * 1.5f, y * 1.5f, z * 1.5f);
        n += noise.GetNoise(x * 2.5f, y * 2.5f, z * 2.5f);
        return n;
    }

    inline bool IsSolid(float x, float y, float z)
    {
        return GetSurfaceNoise(x, y, z) + y <= 0.0f;
    }

    void ScuffedWorldGen::InitWorldGen()
    {
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    }

    void ScuffedWorldGen::Generate(ChunkData* data, const glm::ivec3& chunkPos)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int worldZ = chunkPos.z + z;
            for (int x = 0; x < CHUNK_SIZE; x++)
            {
                int worldX = chunkPos.x + x;
                int depthBelowSurface = -1;
                for (int y = chunkPos.y + CHUNK_SIZE; y < chunkPos.y + CHUNK_SIZE + 4; y++)
                {
                    if (IsSolid(worldX, y, worldZ))
                        depthBelowSurface++;
                    else
                        break;
                }

                for (int y = CHUNK_SIZE - 1; y >= 0; y--)
                {
                    int worldY = chunkPos.y + y;

                    bool solid = IsSolid(worldX, worldY, worldZ);

                    bool cave = GetCaveNoise(worldX, worldY, worldZ) > 0.9f;

                    if (cave)
                        data->Set(x, y, z, 0);
                    else if (!solid)
                    {
                        // Air
                        data->Set(x, y, z, 0);
                        if (y < CHUNK_SIZE - 1 && depthBelowSurface == 0)
                            data->Set(x, y + 1, z, 0); // Prevent single floating blocks
                        depthBelowSurface = -1;
                    }
                    else
                    {
                        depthBelowSurface++;

                        if (depthBelowSurface == 0)
                            data->Set(x, y, z, 2); // Top block = Grass Block
                        else if (depthBelowSurface < 4)
                            data->Set(x, y, z, 1); // 4 below top = Dirt Block
                        else
                            data->Set(x, y, z, 3); // below 4 from top = Stone Block
                    }
                }
            }
        }
    }
}