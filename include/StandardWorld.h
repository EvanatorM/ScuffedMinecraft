#pragma once

#include <WillowVox/WillowVox.h>
#include <WillowVox/math/NoiseSettings.h>
#include <SMTerrainGen.h>

using namespace WillowVox;

namespace ScuffedMinecraft {
    class StandardWorld : public World
    {
    public:
        StandardWorld(Camera* player)
        {
            m_mainCamera = player;

            _surfaceNoise = new NoiseSettings2D[]{
                { 20.0f, 0.5f, 1, 0, 0, -5 },
                { 3.0f, 2.4f, 1, 0, 0, 0 },
            };

            _caveNoise = new CaveNoiseSettings[]{
                { 2.5f, 1, 0, 0, 0.5f }
            };

            _oreNoise = new OreNoiseSettings[]{
                { 4.5f, 1, 0, 0, 0.7f, 2, 14.0f, 34.0f, 23.0f }
            };

            _surfaceFeatures = new SurfaceFeature[]{
                // Tree
                {
                    { 1.0f, 38.0f, 1, 0, 0, 0, 25.23f, 2.53f },
                    {
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 2, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,

                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 5, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,

                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 5, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,

                        0, 6, 6, 6, 0,
                        6, 6, 6, 6, 6,
                        6, 6, 5, 6, 6,
                        6, 6, 6, 6, 6,
                        0, 6, 6, 6, 0,

                        0, 6, 6, 6, 0,
                        6, 6, 6, 6, 6,
                        6, 6, 5, 6, 6,
                        6, 6, 6, 6, 6,
                        0, 6, 6, 6, 0,

                        0, 0, 0, 0, 0,
                        0, 0, 6, 0, 0,
                        0, 6, 6, 6, 0,
                        0, 0, 6, 0, 0,
                        0, 0, 0, 0, 0,

                        0, 0, 0, 0, 0,
                        0, 0, 6, 0, 0,
                        0, 6, 6, 6, 0,
                        0, 0, 6, 0, 0,
                        0, 0, 0, 0, 0,

                    },
                    {
                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, true,  false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,

                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, true,  false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,

                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, true,  false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,

                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, true,  false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,

                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, true,  false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,

                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,

                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,
                    },
                    5,
                    7,
                    5,
                    -2,
                    0,
                    -2,
                    0.95f
                    },
                // Tall Grass
                {
                    { 1.0f, 35.0f, 1, 0, 0, 0, 23.03f, 18.58f },
                    {
                        1, 8, 9
                    },
                    {
                        false, false, false
                    },
                    1,
                    3,
                    1,
                    0,
                    0,
                    0,
                    0.9f
                },
                // Grass
                {
                    { 1.0f, 35.0f, 1, 0, 0, 0, 15.03f, 78.58f },
                    {
                        1, 7
                    },
                    {
                        false, false
                    },
                    1,
                    2,
                    1,
                    0,
                    0,
                    0,
                    0.8f
                },
                // Poppy
                {
                    { 1.0f, 20.0f, 1, 0, 0, 0, 76.2f, 203.54f },
                    {
                        1, 10
                    },
                    {
                        false, false
                    },
                    1,
                    2,
                    1,
                    0,
                    0,
                    0,
                    0.95f
                },
                // White Tulip
                {
                    { 1.0f, 20.0f, 1, 0, 0, 0, 16.58f, 84.02f },
                    {
                        1, 12
                    },
                    {
                        false, false
                    },
                    1,
                    2,
                    1,
                    0,
                    0,
                    0,
                    0.95f
                },
                // Pink Tulip
                {
                    { 1.0f, 20.0f, 1, 0, 0, 0, 82.38f, 17.59f },
                    {
                        1, 13
                    },
                    {
                        false, false
                    },
                    1,
                    2,
                    1,
                    0,
                    0,
                    0,
                    0.95f
                },
                // Orange Tulip
                {
                    { 1.0f, 20.0f, 1, 0, 0, 0, 75.28, 53.2f },
                    {
                        1, 11
                    },
                    {
                        false, false
                    },
                    1,
                    2,
                    1,
                    0,
                    0,
                    0,
                    0.95f
                },
            };

			_worldGen = new SMTerrainGen(0, _surfaceNoise, 1, _caveNoise, 1, _oreNoise, 1, _surfaceFeatures, 7,
                                        0, 5, 2);

            m_chunkManager = new ChunkManager(*_worldGen);
        }

        ~StandardWorld()
        {
            delete _surfaceNoise;
            delete _worldGen;
        }

    private:
        NoiseSettings2D* _surfaceNoise;;
        CaveNoiseSettings* _caveNoise;
        OreNoiseSettings* _oreNoise;
        SurfaceFeature* _surfaceFeatures;
        SMTerrainGen* _worldGen;
    };
}