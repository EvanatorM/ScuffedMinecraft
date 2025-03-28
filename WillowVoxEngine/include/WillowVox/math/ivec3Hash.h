#include <WillowVox/WillowVoxDefines.h>
#include <glm/glm.hpp>
#include <unordered_map>

namespace WillowVox
{
    struct WILLOWVOX_API ivec3Hash
    {
        std::size_t operator()(const glm::ivec3& vec) const
        {
            std::hash<int> hasher;
            std::size_t seed = 0;
            seed ^= hasher(vec.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}