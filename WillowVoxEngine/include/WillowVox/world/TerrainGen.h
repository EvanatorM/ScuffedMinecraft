#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/world/WorldGen.h>
#include <WillowVox/math/NoiseSettings.h>
#include <WillowVox/world/SurfaceFeature.h>
#include <cstdint>

namespace WillowVox
{
    class WILLOWVOX_API TerrainGen : public WorldGen
    {
    public:
        TerrainGen(int seed, NoiseSettings2D* surfaceNoiseSettings, int surfaceNoiseLayers, CaveNoiseSettings* caveNoiseSettings, 
            int caveNoiseLayers, OreNoiseSettings* oreNoiseSettings, int oreNoiseLayers,
            SurfaceFeature* surfaceFeatures, int surfaceFeatureCount)
            
            : WorldGen(seed), 
            m_surfaceNoiseSettings(surfaceNoiseSettings), m_surfaceNoiseLayers(surfaceNoiseLayers),
            m_caveNoiseSettings(caveNoiseSettings), m_caveNoiseLayers(caveNoiseLayers),
            m_oreNoiseSettings(oreNoiseSettings), m_oreNoiseLayers(oreNoiseLayers),
            m_surfaceFeatures(surfaceFeatures), m_surfaceFeatureCount(surfaceFeatureCount) {}

        void GenerateChunkData(ChunkData& chunkData) override;
        // === Generation Steps ===
        /* These exist so that developers can change these behaviors
           without having to remake the whole GenerateChunkData function */
        virtual inline void GenerateChunkBlocks(ChunkData& chunkData);
        virtual inline void GenerateSurfaceFeatures(ChunkData& chunkData);
        // ========================

        uint16_t GetBlock(int x, int y, int z) override;
        // === Block Picking Functions ===
        /* These exist so that developers can change these behaviors
           without having to remake the whole GetBlock function */
        
        // GetSkyBlock called when block is above the surface (good to override for adding water)
        virtual inline uint16_t GetSkyBlock(int x, int y, int z, int surfaceBlock);
        // GetGroundBlock called when block is solid (not cave or ore)
        virtual inline uint16_t GetGroundBlock(int x, int y, int z, int surfaceBlock);
        // GetCaveBlock called when block is below the sky but is a cave
        virtual inline uint16_t GetCaveBlock(int x, int y, int z, int surfaceBlock);
        // GetOreBlock checks to see if the block is an ore
        virtual inline uint16_t GetOreBlock(int x, int y, int z, int surfaceBlock, uint16_t block);

        // Checks if block is cave using cave noise settings
        virtual inline bool IsCave(int x, int y, int z, int surfaceBlock);
        // Checks to see if the block is an ore and returns the type if so. Returns 0 if not.
        virtual inline uint16_t IsOre(int x, int y, int z, int surfaceBlock);
        // Gets the block that the surface is on
        virtual inline int GetSurfaceBlock(int x, int z);
        // ===============================

        // === Surface Feature Placement ===
        /* These exist so that developers can change these behaviors
           without having to remake the whole GenerateSurfaceFeatures function */

        // Override to change the conditions of surface feature placement
        // Base function checks if it is a cave or not
        virtual inline bool IsValidSurfaceFeaturePlacement(int x, int y, int z, int surfaceBlock);
        // =================================

        NoiseSettings2D* m_surfaceNoiseSettings;
        CaveNoiseSettings* m_caveNoiseSettings;
        OreNoiseSettings* m_oreNoiseSettings;
        int m_surfaceNoiseLayers, m_caveNoiseLayers, m_oreNoiseLayers;
        SurfaceFeature* m_surfaceFeatures;
        int m_surfaceFeatureCount;
    };
}