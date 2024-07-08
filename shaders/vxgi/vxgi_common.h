#ifndef COMMONS_HOST_DEVICE
#define COMMONS_HOST_DEVICE

#ifdef __cplusplus
#include <glm.hpp>
using vec2  = glm::vec2;
using ivec3 = glm::ivec3;
using vec3  = glm::vec3;
using vec4  = glm::vec4;
using mat4  = glm::mat4;
using uvec4 = glm::uvec4;
using ivec2 = glm::ivec2;
using uint  = unsigned int;
using uvec2 = glm::uvec2;
#else
#define NAMESPACE_BEGIN(name)
#define NAMESPACE_END()
#define ALIGN16
#endif

#endif