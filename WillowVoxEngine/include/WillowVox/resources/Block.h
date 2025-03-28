#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <cstdint>

namespace WillowVox
{
    struct WILLOWVOX_API Block
    {
        enum BLOCK_TYPE
        {
            SOLID,
            TRANSPARENT,
            LEAVES,
            BILLBOARD,
            LIQUID
        };

        uint16_t topMinX, topMinY, topMaxX, topMaxY;
        uint16_t bottomMinX, bottomMinY, bottomMaxX, bottomMaxY;
        uint16_t sideMinX, sideMinY, sideMaxX, sideMaxY;
        BLOCK_TYPE blockType;
        const char* blockName;

        Block(char minX, char minY, char maxX, char maxY, BLOCK_TYPE blockType, const char* blockName)
            : topMinX(minX), topMinY(minY), topMaxX(maxX), topMaxY(maxY),
            bottomMinX(minX), bottomMinY(minY), bottomMaxX(maxX), bottomMaxY(maxY),
            sideMinX(minX), sideMinY(minY), sideMaxX(maxX), sideMaxY(maxY),
            blockType(blockType), blockName(blockName) {}

        Block(char topMinX, char topMinY, char topMaxX, char topMaxY,
            char bottomMinX, char bottomMinY, char bottomMaxX, char bottomMaxY,
            char sideMinX, char sideMinY, char sideMaxX, char sideMaxY, BLOCK_TYPE blockType, const char* blockName)
            : topMinX(topMinX), topMinY(topMinY), topMaxX(topMaxX), topMaxY(topMaxY),
            bottomMinX(bottomMinX), bottomMinY(bottomMinY), bottomMaxX(bottomMaxX), bottomMaxY(bottomMaxY),
            sideMinX(sideMinX), sideMinY(sideMinY), sideMaxX(sideMaxX), sideMaxY(sideMaxY),
            blockType(blockType), blockName(blockName) {}

        Block(char texX, char texY, BLOCK_TYPE blockType, const char* blockName)
            : topMinX(texX), topMinY(texY), topMaxX(texX + 1), topMaxY(texY + 1),
            bottomMinX(texX), bottomMinY(texY), bottomMaxX(texX + 1), bottomMaxY(texY + 1),
            sideMinX(texX), sideMinY(texY), sideMaxX(texX + 1), sideMaxY(texY + 1),
            blockType(blockType), blockName(blockName) {}

        Block(char topTexX, char topTexY,
            char bottomTexX, char bottomTexY,
            char sideTexX, char sideTexY, BLOCK_TYPE blockType, const char* blockName)
            : topMinX(topTexX), topMinY(topTexY), topMaxX(topTexX + 1), topMaxY(topTexY + 1),
            bottomMinX(bottomTexX), bottomMinY(bottomTexY), bottomMaxX(bottomTexX + 1), bottomMaxY(bottomTexY + 1),
            sideMinX(sideTexX), sideMinY(sideTexY), sideMaxX(sideTexX + 1), sideMaxY(sideTexY + 1),
            blockType(blockType), blockName(blockName) {}
    };
}