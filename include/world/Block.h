#pragma once

#include <wv/wvpch.h>

namespace ScuffedMinecraft
{
    struct Block
    {
        Block() = default;
        Block(const std::string& name, float texMinX, float texMaxX, float texMinY, float texMaxY)
            : name(name), topTexMinX(texMinX), topTexMaxX(texMaxX), topTexMinY(texMinY), topTexMaxY(texMaxY),
            bottomTexMinX(texMinX), bottomTexMaxX(texMaxX), bottomTexMinY(texMinY), bottomTexMaxY(texMaxY),
            sideTexMinX(texMinX), sideTexMaxX(texMaxX), sideTexMinY(texMinY), sideTexMaxY(texMaxY) {}

        Block(const std::string& name, float topTexMinX, float topTexMaxX, float topTexMinY, float topTexMaxY,
            float bottomTexMinX, float bottomTexMaxX, float bottomTexMinY, float bottomTexMaxY,
            float sideTexMinX, float sideTexMaxX, float sideTexMinY, float sideTexMaxY)
            : name(name), topTexMinX(topTexMinX), topTexMaxX(topTexMaxX), topTexMinY(topTexMinY), topTexMaxY(topTexMaxY),
            bottomTexMinX(bottomTexMinX), bottomTexMaxX(bottomTexMaxX), bottomTexMinY(bottomTexMinY), bottomTexMaxY(bottomTexMaxY),
            sideTexMinX(sideTexMinX), sideTexMaxX(sideTexMaxX), sideTexMinY(sideTexMinY), sideTexMaxY(sideTexMaxY) {}

        float topTexMinX, topTexMaxX, topTexMinY, topTexMaxY;
        float bottomTexMinX, bottomTexMaxX, bottomTexMinY, bottomTexMaxY;
        float sideTexMinX, sideTexMaxX, sideTexMinY, sideTexMaxY;
        std::string name;
    };
}