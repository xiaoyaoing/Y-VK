#prgma once
#include "Core/BoundingBox.h"

#include <span>
#include <vec3.hpp>
#include <vector>

using DataArray = std::span<uint8_t>;

struct MeshBuildVertexData {

    std::vector<glm::vec3> Positions;
    
};

struct MeshInputData {
    MeshBuildVertexData Vertices;
    std::vector<uint32_t> TriangleIndices;
    std::vector<uint32_t> TriangleCounts;
    std::vector<int32_t>  MaterialIndices;
    BBox VertexBounds;
    uint32_t NumTexCoords = 0;
    uint32_t NumBoneInfluences = 0;
    float PercentTriangles = 1.0f;
    float MaxDeviation = 0.0f;
};

struct MeshOutputData {
    MeshBuildVertexData Vertices;
    std::vector<uint32_t> TriangleIndices;
    FStaticMeshSectionArray Sections;
};

struct NaniteDAG {
    
};


struct Cluster {
    
};


struct NnaiteBVH {
    
};

struct GraphPartitioner {
    
};

struct MeshNaniteSettings {
    
};

class NaniteBuilder {
    void Build(
        MeshInputData& InputMeshData,
        MeshOutputData* OutFallbackMeshData,
        const MeshNaniteSettings& Settings
    );
};