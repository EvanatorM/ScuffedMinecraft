#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/math/NoiseSettings.h>
#include <vector>

namespace WillowVox
{
    struct WILLOWVOX_API SurfaceFeature
    {
        NoiseSettings2D noiseSettings;
        std::vector<unsigned int> blocks;
        std::vector<bool> replaceBlock;
        int sizeX, sizeY, sizeZ;
        int offsetX, offsetY, offsetZ;
        float chance;
    
        SurfaceFeature(NoiseSettings2D _noiseSettings, std::vector<unsigned int> _blocks, std::vector<bool> _replaceBlock, 
            int _sizeX, int _sizeY, int _sizeZ, 
            int _offsetX, int _offsetY, int _offsetZ,
            float _chance)
            : noiseSettings(_noiseSettings), blocks(_blocks), replaceBlock(_replaceBlock),
            sizeX(_sizeX), sizeY(_sizeY), sizeZ(_sizeZ),
            offsetX(_offsetX), offsetY(_offsetY), offsetZ(_offsetZ),
            chance(_chance) {}
    };
}
