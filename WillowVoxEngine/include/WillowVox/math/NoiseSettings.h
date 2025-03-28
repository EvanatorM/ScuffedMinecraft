#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <cstdint>

namespace WillowVox
{
    struct WILLOWVOX_API NoiseSettings2D
    {
        NoiseSettings2D(float amplitude, float frequency, int octaves, 
                        float persistence, float lacunarity, float heightOffset, float xOffset = 0, float yOffset = 0)
            : m_amplitude(amplitude), m_frequency(frequency), m_octaves(octaves), 
              m_persistence(persistence), m_lacunarity(lacunarity), m_heightOffset(heightOffset),
              m_xOffset(xOffset), m_yOffset(yOffset) {}

        float m_amplitude;
        float m_frequency;
        int m_octaves;
        float m_persistence;
        float m_lacunarity;
        float m_heightOffset;
        float m_xOffset, m_yOffset;
    };
    
    struct WILLOWVOX_API NoiseSettings3D
    {
        NoiseSettings3D(float amplitude, float frequency, int octaves, 
                        float persistence, float lacunarity, float xOffset = 0, float yOffset = 0, float zOffset = 0)
            : m_amplitude(amplitude), m_frequency(frequency), m_octaves(octaves), 
              m_persistence(persistence), m_lacunarity(lacunarity),
              m_xOffset(xOffset), m_yOffset(yOffset), m_zOffset(zOffset) {}

        float m_amplitude;
        float m_frequency;
        int m_octaves;
        float m_persistence;
        float m_lacunarity;
        float m_xOffset, m_yOffset, m_zOffset;
    };

    struct WILLOWVOX_API CaveNoiseSettings
    {
        CaveNoiseSettings(float frequency, int octaves,
            float persistence, float lacunarity, float noiseThreshold, float xOffset = 0, float yOffset = 0, float zOffset = 0)
            : m_amplitude(1.0f), m_frequency(frequency), m_octaves(octaves),
            m_persistence(persistence), m_lacunarity(lacunarity), m_noiseThreshold(noiseThreshold),
            m_xOffset(xOffset), m_yOffset(yOffset), m_zOffset(zOffset) {}

        float m_amplitude;
        float m_frequency;
        int m_octaves;
        float m_persistence;
        float m_lacunarity;
        float m_noiseThreshold;
        float m_xOffset, m_yOffset, m_zOffset;
    };

    struct WILLOWVOX_API OreNoiseSettings
    {
        OreNoiseSettings(float frequency, int octaves,
            float persistence, float lacunarity, float noiseThreshold, uint16_t replaceBlock, float xOffset = 0, float yOffset = 0, float zOffset = 0)
            : m_amplitude(1.0f), m_frequency(frequency), m_octaves(octaves),
            m_persistence(persistence), m_lacunarity(lacunarity), m_noiseThreshold(noiseThreshold), m_replaceBlock(replaceBlock),
            m_xOffset(xOffset), m_yOffset(yOffset), m_zOffset(zOffset) {}

        float m_amplitude;
        float m_frequency;
        int m_octaves;
        float m_persistence;
        float m_lacunarity;
        float m_noiseThreshold;
        uint16_t m_replaceBlock;
        float m_xOffset, m_yOffset, m_zOffset;
    };
}