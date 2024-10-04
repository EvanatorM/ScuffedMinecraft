#include "SurfaceFeature.h"

SurfaceFeature::SurfaceFeature(NoiseSettings _noiseSettings, std::vector<unsigned int> _blocks, std::vector<bool> _replaceBlock,
	int _sizeX, int _sizeY, int _sizeZ,
	int _offsetX, int _offsetY, int _offsetZ)
	: noiseSettings(_noiseSettings), blocks(_blocks), replaceBlock(_replaceBlock),
	sizeX(_sizeX), sizeY(_sizeY), sizeZ(_sizeZ),
	offsetX(_offsetX), offsetY(_offsetY), offsetZ(_offsetZ)
{

}