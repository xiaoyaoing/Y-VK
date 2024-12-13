#pragma once
#include "Core/BoundingBox.h"
#include "Scene/Compoments/RenderPrimitive.h"

#include <functional>
#include <map>
#include <set>
#include <unordered_set>
#include <span>
#include <vec3.hpp>
#include <vector>
#include <memory>
#include <metis.h>

using DataArray = std::span<uint8_t>;

struct MeshBuildVertexData {
    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> UVs;
    std::vector<glm::vec3> colors;
};

struct MeshInputData {
    MeshBuildVertexData   Vertices;
    std::vector<uint32_t> TriangleIndices;
    std::vector<uint32_t> TriangleCounts;
    std::vector<int32_t>  MaterialIndices;
    BBox                  VertexBounds;
    uint32_t              NumTexCoords      = 0;
    uint32_t              NumBoneInfluences = 0;
    float                 PercentTriangles  = 1.0f;
    float                 MaxDeviation      = 0.0f;
    int                   mipLevel          = 0;
};

struct MeshOutputData {
    MeshBuildVertexData   Vertices;
    std::vector<uint32_t> TriangleIndices;
};

struct NaniteDAG {
};

struct AdjancyVertex {
    std::set<uint32_t> adjVertices;
};

struct Edge {
    uint32_t v0;
    uint32_t v1;
};

struct ClusterExternEdge {
    // Edge edge;
    uint32_t v0, v1;
    uint32_t adjCount = 0;
    uint32_t clusterIndex;
};

struct GraphAdjancy {
    //std::multimap<uint32_t, uint32_t> adjVertices;
    GraphAdjancy() = default;
    std::vector<AdjancyVertex> adjVertices;
    void                       addEdge(uint32_t v0, uint32_t v1) {
        adjVertices[v0].adjVertices.insert(v1);
        adjVertices[v1].adjVertices.insert(v0);
    }
    GraphAdjancy(uint32_t vertexCount) {
        adjVertices.resize(vertexCount);
    }
    void forAll(uint32_t vertexIndex, std::function<void(uint32_t, uint32_t)> callback) {
        for (auto adjIndex : adjVertices[vertexIndex].adjVertices) {
            callback(vertexIndex, adjIndex);
        }
    }
};
struct Cluster {
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_uvs;
    std::vector<uint32_t>  m_indexes;

    //Vertex A -> Vertex B
    //Vertex A in this cluster, Vertex B in another cluster
    // std::set<ClusterExternEdge> m_external_edges;
    std::vector<ClusterExternEdge> m_external_edges;
    BBox                           m_bounding_box;
    glm::vec3                      m_min_pos = glm::vec3(1e30f, 1e30f, 1e30f);
    glm::vec3                      m_max_pos = glm::vec3(-1e30f, -1e30f, -1e30f);

    std::vector<int>             face_parent_cluster_group;//size == m_positions.size / 3
    std::unordered_set<uint32_t> m_linked_cluster;
    uint64_t                     guid;
    uint32_t                     m_mip_level;
    Cluster() = default;
    float simplify(uint32_t targetNumTris);
    Cluster(std::vector<Cluster*>& clusters);
    Cluster(Cluster* source, uint32_t start, uint32_t end, std::vector<uint32_t>& indexes, GraphAdjancy& adjancy);
    glm::vec3 getPosition(uint32_t index) {
        return m_positions[index];
    }
    uint32_t getIndexes(uint32_t index) {
        return m_indexes[index];
    }
    GraphAdjancy buildAdjacency();
};

// struct ClusterGroup {
//
// };

struct NnaiteBVH {
};

struct GraphPartitioner {
    struct FGraphData {
        uint32_t Offset;
        uint32_t Num;

        std::vector<idx_t> Adjacency;
        std::vector<float> AdjacencyCost;
        std::vector<idx_t> AdjacencyOffset;
    };
    FGraphData* NewGraph(uint32_t NumAdjacency) const;
    GraphPartitioner(uint32_t elementsNum, uint32_t targetPart);
    void addAdjacency(FGraphData* graph, uint32_t index, float adjCount);
    struct Range {
        uint32_t start;
        uint32_t end;
    };
    uint32_t targetPart;
    // std::vector<Range> ranges;
    std::vector<idx_t> indexes;
    std::vector<idx_t> partitionIDs;
    uint32_t           numElements;
    void               partition(FGraphData& graph);
};

struct MeshNaniteSettings {
};

class NaniteBuilder {
public:
    static void Build(
        MeshInputData&            InputMeshData,
        MeshOutputData*           OutFallbackMeshData,
        const MeshNaniteSettings& Settings);
    static std::unique_ptr<MeshInputData> createNaniteExampleMeshInputData();
    static std::unique_ptr<Primitive>     createNaniteExamplePrimitive();
};

constexpr uint32_t NANITE_MAX_BVH_NODE_FANOUT_BITS = 2;// log2(NANITE_MAX_BVH_NODE_FANOUT)
constexpr float    MAX_FLT                         = std::numeric_limits<float>::max();
constexpr uint32   NANITE_MAX_BVH_NODE_FANOUT      = 4;

// 在文件开头添加以下结构体定义
struct BVHNode {
    BBox                bounds;
    std::vector<uint32> children;
    uint32_t            groupIndex;// 叶子节点对应的cluster索引
    uint32_t            lodLevel;    // LOD级别
    bool                isLeaf;      // 是否为叶子节点
};

struct TNode {
    SphereBox LODBounds[NANITE_MAX_BVH_NODE_FANOUT];
    BBox      Bounds[NANITE_MAX_BVH_NODE_FANOUT];
    float     MinLODError[NANITE_MAX_BVH_NODE_FANOUT];
    float     MaxLODError[NANITE_MAX_BVH_NODE_FANOUT];
    uint32    ChildrenStartIndex[NANITE_MAX_BVH_NODE_FANOUT];
    uint32    NumChildren[NANITE_MAX_BVH_NODE_FANOUT];
    uint32    ClusterGroupPartIndex[NANITE_MAX_BVH_NODE_FANOUT];
};

struct BVHTree {
    std::vector<BVHNode> nodes;
    uint32_t             rootIndex;
    uint32_t             lodLevel;
};

// 用于存储每个LOD级别的BVH树
struct NaniteBVH {
    std::vector<BVHTree> lodTrees;  // 每个LOD级别一个BVH树
};

