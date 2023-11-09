#include "RenderGraphId.h"
#include <glm/gtx/hash.hpp>

std::size_t RenderGraphHandle::getHash() const
{
    std::size_t result;
    glm::detail::hash_combine(result, index);
    glm::detail::hash_combine(result, version);
    return result;
}
