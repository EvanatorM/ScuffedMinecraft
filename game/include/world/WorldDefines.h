#pragma once

constexpr int CHUNK_SIZE = 32;
constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

using BlockId = uint8_t;

constexpr BlockId INVALID_BLOCK_POS = 0;
constexpr BlockId AIR = 1;