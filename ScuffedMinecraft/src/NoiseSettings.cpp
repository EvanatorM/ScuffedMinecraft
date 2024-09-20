#include "NoiseSettings.h"

NoiseSettings::NoiseSettings(float _frequency, float _amplitude, float _offset)
	: frequency(_frequency), amplitude(_amplitude), offset(_offset), chance(0), block(0)
{

}

NoiseSettings::NoiseSettings(float _frequency, float _amplitude, float _offset, float _chance, unsigned int _block, int _maxHeight)
	: frequency(_frequency), amplitude(_amplitude), offset(_offset), chance(_chance), block(_block), maxHeight(_maxHeight)
{

}

NoiseSettings::~NoiseSettings()
{

}