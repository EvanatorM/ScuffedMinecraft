#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <FastNoiseLite.h>
#include <WillowVox/math/NoiseSettings.h>

namespace WillowVox
{
    class WILLOWVOX_API Noise
    {
    public:
        static FastNoiseLite noise;

        static void InitNoise();

        static float GetValue2D(NoiseSettings2D& settings, int seed, float x, float y);

        static float GetValueLayered2D(NoiseSettings2D* settings, int layers, int seed, float x, float y);

        static float GetValue3D(NoiseSettings3D& settings, int seed, float x, float y, float z);
        static float GetValue3D(CaveNoiseSettings& settings, int seed, float x, float y, float z);
        static float GetValue3D(OreNoiseSettings& settings, int seed, float x, float y, float z);
    };
}