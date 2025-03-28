#pragma once

#include <WillowVox/WillowVoxDefines.h>
#include <WillowVox/rendering/Mesh.h>
#include <WillowVox/world/ChunkManager.h>
#include <WillowVox/world/Chunk.h>
#include <WillowVox/rendering/Shader.h>
#include <WillowVox/rendering/Camera.h>
#include <WillowVox/rendering/Texture.h>
#include <vector>

namespace WillowVox
{
    class WILLOWVOX_API World
    {
    public:
        ~World();

        void Start();
        void Update();
        void Render();

        Camera* m_mainCamera;
        ChunkManager* m_chunkManager;

    private:
        // vvv Test code vvv
        Shader* _solidShader;
        Shader* _fluidShader;
        Shader* _billboardShader;
        BaseMaterial* _solidMaterial;
        BaseMaterial* _fluidMaterial;
        BaseMaterial* _billboardMaterial;
        Texture* _tex;
    };
}