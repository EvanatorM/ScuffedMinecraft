#pragma once

#include <WillowVox/WillowVoxDefines.h>

namespace WillowVox
{
    class WILLOWVOX_API Texture
    {
    public:
        enum TexSlot
        {
            TEX00,
            TEX01,
            TEX02,
            TEX03,
            TEX04,
            TEX05,
            TEX06,
            TEX07,
            TEX08,
            TEX09,
            TEX10,
            TEX11,
            TEX12,
            TEX13,
            TEX14,
            TEX15,
            TEX16
        };

        virtual void BindTexture(TexSlot slot) = 0;

        int m_width, m_height;
    };
}