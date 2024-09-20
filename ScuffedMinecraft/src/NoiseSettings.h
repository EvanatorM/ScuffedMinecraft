#pragma once

struct NoiseSettings
{
public:
	float amplitude;
	float frequency;
	float offset;

	float chance;
	unsigned int block;
	int maxHeight;

	NoiseSettings(float _frequency, float _amplitude, float _offset);
	NoiseSettings(float _frequency, float _amplitude, float _offset, float _chance, unsigned int _block, int _maxHeight);
	~NoiseSettings();
};