#pragma once

#include "NoiseSettings.h"
#include <vector>

struct SurfaceFeature
{
	NoiseSettings noiseSettings;
	std::vector<unsigned int> blocks;
	std::vector<bool> replaceBlock;
	int sizeX, sizeY, sizeZ;
	int offsetX, offsetY, offsetZ;

	SurfaceFeature(NoiseSettings _noiseSettings, std::vector<unsigned int> _blocks, std::vector<bool> _replaceBlock, 
		int _sizeX, int _sizeY, int _sizeZ, 
		int _offsetX, int _offsetY, int _offsetZ);
};