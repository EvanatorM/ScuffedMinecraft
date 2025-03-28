#include <WillowVox/world/TerrainGen.h>

using namespace WillowVox;

namespace ScuffedMinecraft
{
	class SMTerrainGen : public TerrainGen
	{
	public:
		SMTerrainGen(int seed, NoiseSettings2D* surfaceNoiseSettings, int surfaceNoiseLayers, CaveNoiseSettings* caveNoiseSettings,
			int caveNoiseLayers, OreNoiseSettings* oreNoiseSettings, int oreNoiseLayers,
			SurfaceFeature* surfaceFeatures, int surfaceFeatureCount,
			int waterLevel, int dirtLayers, int sandLayers)

			: TerrainGen(seed, surfaceNoiseSettings, surfaceNoiseLayers, caveNoiseSettings, caveNoiseLayers,
				oreNoiseSettings, oreNoiseLayers, surfaceFeatures, surfaceFeatureCount),
			  m_waterLevel(waterLevel), m_dirtLayers(dirtLayers), m_sandLayers(sandLayers) {}

		inline uint16_t GetGroundBlock(int x, int y, int z, int surfaceBlock) override
		{
			if (y == surfaceBlock)
			{
				if (surfaceBlock < m_waterLevel + m_sandLayers)
					return 14;
				else
					return 1;
			}
			else if (y >= surfaceBlock - m_dirtLayers)
			{
				if (surfaceBlock < m_waterLevel + m_sandLayers)
					return 14;
				else
					return 2;
			}
			else
				return 3;
		}

		inline uint16_t GetSkyBlock(int x, int y, int z, int surfaceBlock) override
		{
			if (y <= m_waterLevel)
				return 4;
			else
				return 0;
		}

		inline bool IsValidSurfaceFeaturePlacement(int x, int y, int z, int surfaceBlock) override
		{
			if (IsCave(x, y, z, surfaceBlock))
				return false;

			if (y < m_waterLevel + m_sandLayers)
				return false;

			return true;
		}

		int m_waterLevel;
		int m_dirtLayers, m_sandLayers;
	};
}