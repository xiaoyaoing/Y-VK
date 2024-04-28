#pragma once

#include "math.h"

#include <string>
#include <glm/vec3.hpp>
namespace ComplexIorList {

    bool lookup(const std::string& name, glm::vec3& eta, glm::vec3& k);

}