#include "ComplexIor.hpp"

// http://homepages.rpi.edu/~schubert/Educational-resources/Materials-Refractive-index-and-extinction-coefficient.pdf
//https://benedikt-bitterli.me/tungsten.html
#include "ComplexIorData.hpp"

namespace ComplexIorList {
    bool lookup(const std::string& name, glm::vec3& eta, glm::vec3& k) {
        for (int i = 0; i < ComplexIorCount; ++i) {
            if (complexIorList[i].name == name) {
                eta = complexIorList[i].eta;
                k   = complexIorList[i].k;
                return true;
            }
        }
        return false;
    }
}