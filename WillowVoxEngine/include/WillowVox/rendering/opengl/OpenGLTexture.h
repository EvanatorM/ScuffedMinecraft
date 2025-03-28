#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/Texture.h>

namespace WillowVox
{
    class WILLOWVOX_API OpenGLTexture : public Texture
    {
    public:
        OpenGLTexture(const char* path);
        ~OpenGLTexture();

        void BindTexture(TexSlot slot) override;

    private:
        unsigned int _textureId;
    };
}