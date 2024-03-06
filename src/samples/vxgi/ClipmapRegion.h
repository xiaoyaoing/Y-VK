#pragma once
#include "VxgiCommon.h"
#include "Core/BoundingBox.h"

#include <glm.hpp>

struct ClipmapRegion {
    ClipmapRegion() {}
    ClipmapRegion(const glm::ivec3& minPos, const glm::ivec3& extent, float voxelSize)
        : minCoord(minPos), extent(extent), voxelSize(voxelSize) {}

    /**
     * Returns the minimum position of the clip region in world coordinates.
     */
    glm::vec3 getMinPosWorld() const { return glm::vec3(minCoord) * voxelSize; }

    /**
    * Returns the maximum position of the clip region in world coordinates.
    */
    glm::vec3 getMaxPosWorld() const { return glm::vec3(getMaxCoord()) * voxelSize; }

    /**
    * Returns the minimum position of the clip region in image coordinates using toroidal addressing.
    * Note: The % operator is expected to be defined by the C++11 standard.
    */
    glm::ivec3 getMinPosImage(const glm::ivec3& imageSize) const { return ((minCoord % imageSize) + imageSize) % imageSize; }

    /**
    * Returns the maximum position of the clip region in image coordinates using toroidal addressing.
    * Note: The % operator is expected to be defined by the C++11 standard.
    */
    glm::ivec3 getMaxPosImage(const glm::ivec3& imageSize) const { return ((getMaxCoord() % imageSize) + imageSize) % imageSize; }

    /**
     * Returns the maximum position in local voxel coordinates.
     */
    glm::ivec3 getMaxCoord() const { return minCoord + extent; }

    glm::ivec3 getMinCoord() const { return minCoord; }

    BBox getBoundingBox() const {
        return BBox(getMinPosWorld(), getMaxPosWorld());
    }

    /**
     * Returns the extent in world coordinates.
     */
    glm::vec3 getExtentWorld() const { return glm::vec3(extent) * voxelSize; }

    glm::vec3 getCenterPosWorld() const { return getMinPosWorld() + getExtentWorld() * 0.5f; }

    ClipmapRegion toPrevLevelRegion() const {
        return ClipmapRegion(minCoord * 2, extent * 2, voxelSize / 2.0f);
    }

    ClipmapRegion toNextLevelRegion() const {
        // extent + 1 is used to make sure that the upper bound is computed
        return ClipmapRegion(minCoord / 2, (extent + 1) / 2, voxelSize * 2.0f);
    }

    VoxelizationParamater getVoxelizationParam() const {
        VoxelizationParamater param;
        // param.clipmapMinCoord    = minCoord;
        // param.clipmapMaxCoord    = getMaxCoord();
        param.voxelResolution = VOXEL_RESOLUTION;
        param.clipmapMinWorldPos = getMinPosWorld();
        param.clipmapMaxWorldPos = getMaxPosWorld();
        param.voxelSize          = voxelSize;
        param.maxExtentWorld     = getExtentWorld().x;
        return param;
    }
    
    glm::ivec3 minCoord;       // The minimum position in local voxel coordinates
    glm::ivec3 extent{0};      // The extent of the region in local voxel coordinates
    float      voxelSize{0.0f};// Voxel size in world coordinates

    // Note: the voxel size in local voxel coordinates is always 1
};
