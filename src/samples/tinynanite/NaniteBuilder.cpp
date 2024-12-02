#include "NaniteBuilder.h"

#include "Core/RenderContext.h"
#include "Scene/SceneLoader/ObjLoader.hpp"
#include "Scene/SceneLoader/SceneLoaderInterface.h"

#include <unordered_map>
#include <unordered_set>
#include <metis.h>

static constexpr uint32_t ClusterSize = 128;

struct BuildCluster {
};

struct AdjVertex {
    std::unordered_set<uint32_t> adjVertices;
};

static __forceinline uint32_t MurmurFinalize32(uint32_t Hash) {
    Hash ^= Hash >> 16;
    Hash *= 0x85ebca6b;
    Hash ^= Hash >> 13;
    Hash *= 0xc2b2ae35;
    Hash ^= Hash >> 16;
    return Hash;
}

static __forceinline uint32_t Murmur32(std::initializer_list<uint32_t> InitList) {
    uint32_t Hash = 0;
    for (auto Element : InitList) {
        Element *= 0xcc9e2d51;
        Element = (Element << 15) | (Element >> (32 - 15));
        Element *= 0x1b873593;

        Hash ^= Element;
        Hash = (Hash << 13) | (Hash >> (32 - 13));
        Hash = Hash * 5 + 0xe6546b64;
    }

    return MurmurFinalize32(Hash);
}

uint32_t HashPosition(const glm::vec3& Position) {
    union {
        float    f;
        uint32_t i;
    } x;
    union {
        float    f;
        uint32_t i;
    } y;
    union {
        float    f;
        uint32_t i;
    } z;

    x.f = Position.x;
    y.f = Position.y;
    z.f = Position.z;

    return Murmur32({Position.x == 0.0f ? 0u : x.i,
                     Position.y == 0.0f ? 0u : y.i,
                     Position.z == 0.0f ? 0u : z.i});
}

inline uint32_t cycle3(uint32_t Value) {
    uint32_t ValueMod3  = Value % 3;
    uint32_t Value1Mod3 = (1 << ValueMod3) & 3;
    return Value - ValueMod3 + Value1Mod3;
}

// struct Edge {
//     uint32_t v0, v1;
//
//     Edge(uint32_t a, uint32_t b) {
//         // 总是将较小的索引放在v0
//         v0 = std::min(a, b);
//         v1 = std::max(a, b);
//     }
//
//     bool operator==(const Edge& other) const {
//         return (v0 == other.v0 && v1 == other.v1);
//     }
// };
//
// struct EdgeHash {
//     size_t operator()(const Edge& edge) const {
//         // 使用更好的哈希函数
//         return (static_cast<size_t>(edge.v0) << 32) | edge.v1;
//     }
// };

struct PointHash {
    std::unordered_map<uint32_t, std::vector<uint32_t>> hashTable;
    std::unordered_map<size_t, std::vector<uint32_t>>   hashTable1;
    template<typename GerPosition, typename Function>
    void ForAllMatching(uint32_t index, bool bAdd, GerPosition&& getPosition, Function&& callback) {

        vec3   position0 = getPosition(index);
        vec3   position1 = getPosition(cycle3(index));
        uint32 hash0     = HashPosition(position0);
        uint32 hash1     = HashPosition(position1);
        uint32 hash      = Murmur32({hash0, hash1});
        // hash             = hash0;

        // uint32_t index0 = getPosition(index);
        // uint32_t index1 = getPosition(cycle3(index));
        // {
        //     auto temp = std::min(index0, index1);
        //     auto temp1 = std::max(index0, index1);
        //     index0 = temp;
        //     index1 = temp1;
        // }
        //  size_t hash = static_cast<size_t>(index0) << 32 | index1;

        if (hashTable1.contains(hash)) {
            for (auto& anotherEdge : hashTable1[hash]) {
                callback(index, anotherEdge);
            }
        } else {
        }

        hash = Murmur32({hash1, hash0});
        //   hash = static_cast<size_t>(index1) << 32 | index0;

        if (bAdd) {
            if (!hashTable1.contains(hash)) {
                hashTable1[hash] = std::vector<uint32_t>();
            }
            hashTable1[hash].push_back(index);
        }
    }
};

uint32_t getTriangleIndexByVertexIndex(uint32_t vertexIndex) {
    return vertexIndex / 3;
}

template<typename GerPosition>
GraphAdjancy buildAdjancy(std::span<uint32_t> indexes, GerPosition&& getPosition) {
    GraphAdjancy graphData(indexes.size());
    PointHash    hash;
    for (uint32_t i = 0; i < indexes.size(); i += 3) {
        for (int k = 0; k < 3; k++) {
            hash.ForAllMatching(i + k, true, [&](uint32_t index) { return getPosition(index); }, [&](uint32_t edgeIndex, uint32_t otherEdgeIndex) { graphData.addEdge(edgeIndex, otherEdgeIndex); });
        }
    }
    std::vector<uint32_t> adjOffsets;
    uint32_t              offset = 0;
    for (uint32_t i = 0; i < graphData.adjVertices.size(); i++) {
        adjOffsets.push_back(offset);
        offset += graphData.adjVertices[i].adjVertices.size();
    }
    return graphData;
}

GraphAdjancy buildClusterGroupAdjancy(std::span<Cluster> clusters) {
    std::vector<ClusterExternEdge> externEdges;
    uint32_t                       externalEdgeCount = 0;
    PointHash                      edgehash;
    GraphAdjancy                   graphData(clusters.size());
    for (uint32_t i = 0; i < clusters.size(); i++) {
        externalEdgeCount += clusters[i].m_external_edges.size();
    }
    externEdges.resize(externalEdgeCount);
    uint32_t externalEdgeOffset = 0;
    for (uint32_t i = 0; i < clusters.size(); i++) {
        for (auto edge : clusters[i].m_external_edges) {
            uint32_t index0                   = clusters[i].getIndexes(edge.v0);
            uint32_t index1                   = clusters[i].getIndexes(edge.v1);
            auto     position0                = clusters[i].getPosition(index0);
            auto     position1                = clusters[i].getPosition(index1);
            auto     hash_0                   = HashPosition(position0);
            auto     hash_1                   = HashPosition(position1);
            auto     hash_value               = Murmur32({hash_0, hash_1});
            externEdges[externalEdgeOffset++] = edge;
            edgehash.hashTable[hash_value].push_back(externalEdgeOffset);
        }
    }

    for (uint32 i = 0; i < clusters.size(); i++) {
        for (auto edge : clusters[i].m_external_edges) {
            auto     hash_0     = HashPosition(clusters[i].m_positions[edge.v0]);
            auto     hash_1     = HashPosition(clusters[i].m_positions[edge.v1]);
            auto     hash_value = Murmur32({hash_1, hash_0});
            uint32_t index0     = clusters[i].getIndexes(edge.v0);
            uint32_t index1     = clusters[i].getIndexes(edge.v1);
            auto     position0  = clusters[i].getPosition(index0);
            auto     position1  = clusters[i].getPosition(index1);
            for (uint32_t group : edgehash.hashTable[hash_value]) {
                auto externalEdge   = externEdges[group];
                auto otherCluster   = clusters[externalEdge.clusterIndex];
                auto otherIndex0    = otherCluster.getIndexes(externalEdge.v0);
                auto otherIndex1    = otherCluster.getIndexes(externalEdge.v1);
                auto otherPosition0 = otherCluster.getPosition(otherIndex0);
                auto otherPosition1 = otherCluster.getPosition(otherIndex1);
                if (externalEdge.clusterIndex != i && position0 == otherPosition0 && position1 == otherPosition1) {
                    graphData.addEdge(i, externalEdge.clusterIndex);
                    clusters[i].m_linked_cluster.insert(externalEdge.clusterIndex);
                }
            }
        }
    }

    return graphData;
}

struct Triangle {
    vec3 position[3];
};

void PrintTriangle(const Triangle& tri) {
    //LOGI("Triangle: ");
    // for (int i = 0; i < 3; i++) {
    // LOGI("Position: {} {} {}", tri.position[i].x, tri.position[i].y, tri.position[i].z);
    //}
}

bool isTriangleAdjancy(const Triangle& tri0, const Triangle& tri1) {
    uint32_t count = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (tri0.position[i] == tri1.position[j]) {
                count++;
            }
        }
    }
    // if (count >= 2) {
    //     LOGI("count {}", count);
    // }
    return count >= 1;
}

template<typename GerPosition>
void checkTriangleAdjancy(const GraphPartitioner::FGraphData& graph, const std::vector<uint32_t>& indexes, GerPosition&& getPosition) {
    for (int i = 0; i < graph.AdjacencyOffset.size() - 1; i++) {
        Triangle tri;
        tri.position[0] = getPosition(i * 3);
        tri.position[1] = getPosition(i * 3 + 1);
        tri.position[2] = getPosition(i * 3 + 2);
        for (int j = graph.AdjacencyOffset[i]; j < graph.AdjacencyOffset[i + 1]; j++) {
            auto     adjIndex = graph.Adjacency[j];
            Triangle adjTri;
            adjTri.position[0] = getPosition(adjIndex * 3);
            adjTri.position[1] = getPosition(adjIndex * 3 + 1);
            adjTri.position[2] = getPosition(adjIndex * 3 + 2);
            bool isAdj         = isTriangleAdjancy(tri, adjTri);
            if (!isAdj) {
                LOGE("Triangle %d and %d is not adjancy", i, adjIndex);
            }
        }
    }
}

void clusterTriangles2(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
    // 安全检查
    if (numTriangles == 0) {
        LOGE("No triangles to cluster");
        return;
    }

    // 计算目标cluster数量，确保至少有1个
    uint32_t targetClusterCount = std::max(1u, (numTriangles + ClusterSize - 1) / ClusterSize);

    // 构建图分区器
    GraphPartitioner partitioner(numTriangles, targetClusterCount);
    auto             graph = partitioner.NewGraph(numTriangles * 6);// 每个三角形最多6个邻接（每边2个）

    if (!graph) {
        LOGE("Failed to create graph");
        return;
    }

    // 初始化图的偏移数组
    graph->AdjacencyOffset[0] = 0;

    // 构建邻接关系
    std::vector<std::vector<uint32_t>> triangleAdjacency(numTriangles);

    // 首先构建完整的邻接关系
    for (uint32_t i = 0; i < numTriangles; i++) {
        uint32_t i0 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 0];
        uint32_t i1 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 1];
        uint32_t i2 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 2];

        // 检查每条边
        for (uint32_t j = 0; j < numTriangles; j++) {
            if (i == j) continue;

            uint32_t j0 = InputMeshData.TriangleIndices[baseTriangle + j * 3 + 0];
            uint32_t j1 = InputMeshData.TriangleIndices[baseTriangle + j * 3 + 1];
            uint32_t j2 = InputMeshData.TriangleIndices[baseTriangle + j * 3 + 2];

            // 检查是否共享边
            bool isAdjacent = false;
            isAdjacent |= (i0 == j0 && i1 == j1) || (i0 == j1 && i1 == j0);
            isAdjacent |= (i1 == j1 && i2 == j2) || (i1 == j2 && i2 == j1);
            isAdjacent |= (i2 == j2 && i0 == j0) || (i2 == j0 && i0 == j2);

            if (isAdjacent) {
                triangleAdjacency[i].push_back(j);
            }
        }
    }

    // 然后将邻接关系添加到图中
    for (uint32_t i = 0; i < numTriangles; i++) {
        const auto& adjacentTris = triangleAdjacency[i];

        // 设置偏移
        graph->AdjacencyOffset[i + 1] = graph->AdjacencyOffset[i] + adjacentTris.size();

        // 添加邻接关系
        for (uint32_t adjTri : adjacentTris) {
            // 计算边权重
            float weight = 1.0f;

            graph->Adjacency.push_back(adjTri);
            graph->AdjacencyCost.push_back(weight);
        }
    }

    // 验证图的有效性
    bool isValid = true;
    for (uint32_t i = 0; i < numTriangles; i++) {
        if (graph->AdjacencyOffset[i] > graph->AdjacencyOffset[i + 1]) {
            LOGE("Invalid adjacency offset at %d", i);
            isValid = false;
            break;
        }
    }

    if (!isValid) {
        delete graph;
        return;
    }

    // 添加调试输出
    LOGI("Graph stats:");
    LOGI("  Triangles: %d", numTriangles);
    LOGI("  Target clusters: %d", targetClusterCount);
    LOGI("  Total edges: %d", graph->Adjacency.size());
    LOGI("  Average edges per triangle: %.2f",
         static_cast<float>(graph->Adjacency.size()) / numTriangles);

    // 执行分区
    partitioner.partition(*graph);

    // ... 后续的cluster创建代码 ...

    delete graph;
}

void clusterTriangles1(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles, std::vector<idx_t>& part_result) {
    GraphAdjancy     graphData          = buildAdjancy(std::span<uint32_t>(InputMeshData.TriangleIndices.data() + baseTriangle, numTriangles * 3), [&](uint32_t index) {
        return InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[index]];
        // return InputMeshData.TriangleIndices[index];
    });
    uint32_t         targetClusterCount = numTriangles / ClusterSize;
    GraphPartitioner partitioner(numTriangles, targetClusterCount);
    auto             graph = partitioner.NewGraph(numTriangles * 3);
    for (uint32_t i = 0; i < numTriangles; i++) {
        graph->AdjacencyOffset[i] = graph->Adjacency.size();
        for (int k = 0; k < 3; k++) {
            graphData.forAll(3 * i + k, [&](uint32_t edgeIndex0, uint32_t edgeIndex1) {
                partitioner.addAdjacency(graph, edgeIndex1 / 3, 4 * 65);
            });
        }
    }
    graph->AdjacencyOffset[numTriangles] = graph->Adjacency.size();

    checkTriangleAdjancy(*graph, InputMeshData.TriangleIndices, [&](uint32_t index) { return InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[index]]; });

    partitioner.partition(*graph);

    std::vector<std::unordered_map<uint32_t, uint32_t>> oldToNewIndexMaps(targetClusterCount);
    clusters.resize(clusters.size() + targetClusterCount);
    for (int i = 0; i < numTriangles; i++) {
        int   partId           = partitioner.partitionIDs[i];
        auto  clusterId        = clusters.size() - targetClusterCount + partId;
        auto& oldToNewIndexMap = oldToNewIndexMaps[partId];
        for (int k = 0; k < 3; k++) {
            uint32_t index = InputMeshData.TriangleIndices[baseTriangle + i * 3 + k];
            if (oldToNewIndexMap.find(index) == oldToNewIndexMap.end()) {
                oldToNewIndexMap[index] = clusters[clusterId].m_positions.size();
                clusters[clusterId].m_positions.push_back(InputMeshData.Vertices.Positions[index]);
                clusters[clusterId].m_normals.push_back(InputMeshData.Vertices.Normals[index]);
                clusters[clusterId].m_uvs.push_back(InputMeshData.Vertices.UVs[index]);
            } else {
                clusters[clusterId].m_indexes.push_back(oldToNewIndexMap[index]);
            }
        }
    }

    part_result = partitioner.partitionIDs;
    delete graph;
}

void clusterTriangles3(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
    if (numTriangles == 0) return;

    uint32_t         targetClusterCount = std::max(1u, (numTriangles + ClusterSize - 1) / ClusterSize);
    GraphPartitioner partitioner(numTriangles, targetClusterCount);
    auto             graph = partitioner.NewGraph(numTriangles * 6);

    // 1. 修改Edge结构确保方向无关性
    struct Edge {
        uint32_t v0, v1;

        Edge(uint32_t a, uint32_t b) {
            // 总是将较小的索引放在v0
            v0 = std::min(a, b);
            v1 = std::max(a, b);
        }

        bool operator==(const Edge& other) const {
            return (v0 == other.v0 && v1 == other.v1);
        }
    };

    struct EdgeHash {
        size_t operator()(const Edge& edge) const {
            // 使用更好的哈希函数
            return (static_cast<size_t>(edge.v0) << 32) | edge.v1;
        }
    };

    // 2. 创建边到三角形的映射
    std::unordered_map<Edge, std::vector<uint32_t>, EdgeHash> edgeToTriangles;
    edgeToTriangles.reserve(numTriangles * 3);

    // 3. 收集所有边和相邻三角形
    for (uint32_t triIdx = 0; triIdx < numTriangles; triIdx++) {
        uint32_t idx = baseTriangle + triIdx * 3;
        uint32_t i0  = InputMeshData.TriangleIndices[idx + 0];
        uint32_t i1  = InputMeshData.TriangleIndices[idx + 1];
        uint32_t i2  = InputMeshData.TriangleIndices[idx + 2];

        // 添加三条边
        edgeToTriangles[Edge(i0, i1)].push_back(triIdx);
        edgeToTriangles[Edge(i1, i2)].push_back(triIdx);
        edgeToTriangles[Edge(i2, i0)].push_back(triIdx);
    }

    // 4. 构建邻接关系
    std::vector<std::vector<std::pair<uint32_t, float>>> triangleAdjacency(numTriangles);

    for (const auto& pair : edgeToTriangles) {
        const auto& triangles = pair.second;
        // 如果一条边被多个三角形共享，它们都应该互相连接
        for (size_t i = 0; i < triangles.size(); ++i) {
            for (size_t j = i + 1; j < triangles.size(); ++j) {
                triangleAdjacency[triangles[i]].push_back(std::make_pair(triangles[j], 1.0f));
                triangleAdjacency[triangles[j]].push_back(std::make_pair(triangles[i], 1.0f));
            }
        }
    }

    // 5. 填充图结构
    uint32_t currentOffset    = 0;
    graph->AdjacencyOffset[0] = 0;

    for (uint32_t i = 0; i < numTriangles; i++) {
        const auto& adjacentTris = triangleAdjacency[i];

        for (const auto& [adjTri, weight] : adjacentTris) {
            graph->Adjacency.push_back(adjTri);
            graph->AdjacencyCost.push_back(weight);
            currentOffset++;
        }

        graph->AdjacencyOffset[i + 1] = currentOffset;
    }

    // 6. 执行分区
    partitioner.partition(*graph);

    // 7. 创建clusters
    const uint32_t oldClusterCount = clusters.size();
    clusters.resize(oldClusterCount + targetClusterCount);
    std::vector<std::unordered_map<uint32_t, uint32_t>> oldToNewIndexMaps(targetClusterCount);

    // 收集顶点
    for (uint32_t i = 0; i < numTriangles; i++) {
        int   partId   = partitioner.partitionIDs[i];
        auto& cluster  = clusters[oldClusterCount + partId];
        auto& indexMap = oldToNewIndexMaps[partId];

        for (int k = 0; k < 3; k++) {
            uint32_t globalIndex = InputMeshData.TriangleIndices[baseTriangle + i * 3 + k];
            if (indexMap.find(globalIndex) == indexMap.end()) {
                indexMap[globalIndex] = cluster.m_positions.size();
                cluster.m_positions.push_back(InputMeshData.Vertices.Positions[globalIndex]);
                cluster.m_normals.push_back(InputMeshData.Vertices.Normals[globalIndex]);
                cluster.m_uvs.push_back(InputMeshData.Vertices.UVs[globalIndex]);
                cluster.m_bounding_box.unite(cluster.m_positions.back());
                cluster.m_min_pos = glm::min(cluster.m_min_pos, cluster.m_positions.back());
                cluster.m_max_pos = glm::max(cluster.m_max_pos, cluster.m_positions.back());
            } else {
                cluster.m_indexes.push_back(indexMap[globalIndex]);
            }
        }
    }

    // 添加索引
    for (uint32_t i = 0; i < numTriangles; i++) {
        int   partId   = partitioner.partitionIDs[i];
        auto& cluster  = clusters[oldClusterCount + partId];
        auto& indexMap = oldToNewIndexMaps[partId];

        for (int k = 0; k < 3; k++) {
            uint32_t globalIndex = InputMeshData.TriangleIndices[baseTriangle + i * 3 + k];
            cluster.m_indexes.push_back(indexMap[globalIndex]);
        }
    }

    // 在创建clusters之后，添加以下代码来构建cluster间的连接关系

    // 1. 创建边到cluster的映射
    struct ClusterEdge {
        uint32_t v0, v1;
        ClusterEdge(uint32_t a, uint32_t b) : v0(std::min(a, b)),
                                              v1(std::max(a, b)) {}

        bool operator==(const ClusterEdge& other) const {
            return v0 == other.v0 && v1 == other.v1;
        }
    };

    struct ClusterEdgeHash {
        size_t operator()(const ClusterEdge& edge) const {
            return (static_cast<size_t>(edge.v0) << 32) | edge.v1;
        }
    };

    // 2. 收集每个cluster的边界边
    std::unordered_map<ClusterEdge, std::vector<uint32_t>, ClusterEdgeHash> clusterEdges;

    for (uint32_t i = 0; i < numTriangles; i++) {
        int      partId    = partitioner.partitionIDs[i];
        uint32_t clusterId = oldClusterCount + partId;
        auto&    cluster   = clusters[clusterId];
        auto&    indexMap  = oldToNewIndexMaps[partId];

        // 获取三角形的全局顶点索引
        uint32_t i0 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 0];
        uint32_t i1 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 1];
        uint32_t i2 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 2];

        // 添加三条边
        clusterEdges[ClusterEdge(i0, i1)].push_back(clusterId);
        clusterEdges[ClusterEdge(i1, i2)].push_back(clusterId);
        clusterEdges[ClusterEdge(i2, i0)].push_back(clusterId);
    }

    // 3. 为每个cluster创建外部边信息
    for (const auto& [edge, clusterIds] : clusterEdges) {
        // 如果一条边被多个cluster共享
        if (clusterIds.size() > 1) {
            for (size_t i = 0; i < clusterIds.size(); ++i) {
                uint32_t clusterId = clusterIds[i];
                auto&    cluster   = clusters[clusterId];
                auto&    indexMap  = oldToNewIndexMaps[clusterId - oldClusterCount];

                // 获取在当前cluster中的局部顶点索引
                uint32_t localV0 = indexMap[edge.v0];
                uint32_t localV1 = indexMap[edge.v1];

                // 添加与其他cluster的连接关系
                for (size_t j = 0; j < clusterIds.size(); ++j) {
                    if (i != j) {
                        cluster.m_external_edges.push_back({
                            localV0,     // 局部顶点索引0
                            localV1,     // 局部顶点索引1
                            clusterIds[j]// 相邻cluster的ID
                        });
                    }
                }
            }
        }
    }

    // 4. 更新cluster的其他信息
    for (uint32_t i = oldClusterCount; i < clusters.size(); ++i) {
        auto& cluster = clusters[i];

        // 更新包围盒和min/max位置
        for (const auto& pos : cluster.m_positions) {
            cluster.m_min_pos = glm::min(cluster.m_min_pos, pos);
            cluster.m_max_pos = glm::max(cluster.m_max_pos, pos);
            cluster.m_bounding_box.unite(pos);
        }

        // 设置cluster的guid
        cluster.guid = i;// 或者使用其他生成guid的方式

        // 更新linked clusters (基于external edges)
        for (const auto& edge : cluster.m_external_edges) {
            cluster.m_linked_cluster.insert(edge.clusterIndex);
        }
    }

    delete graph;
}

// 辅助函数：计算边权重
inline float calculateEdgeWeight(const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& normal1, const glm::vec3& normal2) {
    float distWeight   = glm::length2(pos1 - pos2);// 使用length2避免开方
    float normalWeight = 1.0f - glm::dot(normal1, normal2);
    return distWeight * (1.0f + normalWeight * 2.0f);
}

void clusterTriangles(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
    // GraphAdjancy graphData;
    // for (uint32_t i = 0; i < numTriangles; i++) {
    //     glm::vec3 position0 = InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[baseTriangle + i * 3 + 0]];
    //     glm::vec3 position1 = InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[baseTriangle + i * 3 + 1]];
    //     glm::vec3 position2 = InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[baseTriangle + i * 3 + 2]];
    //
    //     uint32_t hash0 = HashPosition(position0);
    //     uint32_t hash1 = HashPosition(position1);
    //     uint32_t hash2 = HashPosition(position2);
    //
    //     graphData.addEdge(hash0, hash1);
    //     graphData.addEdge(hash1, hash2);
    //     graphData.addEdge(hash2, hash0);
    // }
    //
    // std::vector<idx_t> adjOffsets;
    // std::vector<idx_t> adjVertices;
    // for (uint32_t i = 0; i < graphData.adjVertices.size(); i++) {
    //     adjOffsets.push_back(adjVertices.size());
    //     for (uint32_t adjVertex : graphData.adjVertices[i].adjVertices) {
    //         adjVertices.push_back(adjVertex);
    //     }
    // }
    //
    // idx_t options[METIS_NOPTIONS];
    // METIS_SetDefaultOptions(options);
    // options[METIS_OPTION_UFACTOR] = 200;
    //
    // int                numVertices = graphData.adjVertices.size();
    // idx_t              nWeights    = 1;
    // idx_t              nParts      = (numVertices + 127) / 128;
    // std::vector<idx_t> partResult;
    // partResult.resize(numVertices);
    // METIS_PartGraphKway(&numVertices, &nWeights, adjOffsets.data(), adjVertices.data(), nullptr, nullptr, nullptr, &nParts, nullptr, nullptr, nullptr, nullptr, partResult.data());
    //
    // clusters.resize(clusters.size() + nParts);
    // for (uint32_t i = 0; i < numVertices; i++) {
    //     glm::vec3 pos_a = InputMeshData.Vertices.Positions[i];
    //     glm::vec3 pos_b = InputMeshData.Vertices.Positions[i];
    //     glm::vec3 pos_c = InputMeshData.Vertices.Positions[i];
    //
    //     uint32_t hash_a = HashPosition(pos_a);
    //     uint32_t hash_b = HashPosition(pos_b);
    //     uint32_t hash_c = HashPosition(pos_c);
    //
    //     uint32_t vtx_idx_a = graphData.vertexIndexMap[hash_a];
    //     uint32_t vtx_idx_b = graphData.vertexIndexMap[hash_b];
    //     uint32_t vtx_idx_c = graphData.vertexIndexMap[hash_c];
    //
    //     idx_t part_a = partResult[vtx_idx_a] + clusters.size() - nParts;
    //     idx_t part_b = partResult[vtx_idx_b] + clusters.size() - nParts;
    //     idx_t part_c = partResult[vtx_idx_c] + clusters.size() - nParts;
    //
    //     uint32_t part_num = 1;
    //     idx_t    parts[3];
    //     parts[0] = part_a;
    //
    //     if (part_a != part_b) {
    //         parts[1] = part_b;
    //         part_num++;
    //     }
    //     if (part_a != part_c && part_b != part_c) {
    //         parts[2] = part_c;
    //         part_num++;
    //     }
    //
    //     if (part_num == 2) {
    //         clusters[parts[0]].m_linked_cluster.insert(parts[1]);
    //         clusters[parts[1]].m_linked_cluster.insert(parts[0]);
    //     }
    //     if (part_num == 3) {
    //         clusters[parts[0]].m_linked_cluster.insert(parts[1]);
    //         clusters[parts[0]].m_linked_cluster.insert(parts[2]);
    //         clusters[parts[1]].m_linked_cluster.insert(parts[0]);
    //         clusters[parts[1]].m_linked_cluster.insert(parts[2]);
    //         clusters[parts[2]].m_linked_cluster.insert(parts[0]);
    //         clusters[parts[2]].m_linked_cluster.insert(parts[1]);
    //     }
    //     idx_t min_part = clusters[part_a].m_positions.size() < clusters[part_b].m_positions.size() ? part_a : part_b;
    //     min_part       = clusters[min_part].m_positions.size() < clusters[part_c].m_positions.size() ? min_part : part_c;
    //     clusters[min_part].m_positions.push_back(pos_a);
    //     clusters[min_part].m_positions.push_back(pos_b);
    //     clusters[min_part].m_positions.push_back(pos_c);
    // }
}

void PartionClusters(std::vector<Cluster>& clusters, std::vector<ClusterGroup> outGroups) {
}

static uint32_t MaxClusterGroupSize = 32;
static uint32_t MinClusterGroupSize = 8;

Cluster::Cluster(std::vector<Cluster*>& clusters) {
    const uint32_t NumTrisGuess = clusters.size() * 128;
    m_positions.reserve(NumTrisGuess * 3);
    m_normals.reserve(NumTrisGuess * 3);
    m_uvs.reserve(NumTrisGuess * 3);
    m_indexes.reserve(NumTrisGuess * 3);
    m_external_edges.clear();

    for (Cluster* cluster : clusters) {
        for (uint32_t i = 0; i < cluster->m_positions.size(); i++) {
            m_positions.push_back(cluster->m_positions[i]);
            m_normals.push_back(cluster->m_normals[i]);
            m_uvs.push_back(cluster->m_uvs[i]);
            m_indexes.push_back(m_positions.size() - 1);
        }
        // for (auto edge : cluster->m_external_edges) {
        //     m_external_edges.insert({m_positions.size() - 1, edge.v1});
        // }
        m_bounding_box = m_bounding_box + cluster->m_bounding_box;
        m_min_pos      = glm::min(m_min_pos, cluster->m_min_pos);
        m_max_pos      = glm::max(m_max_pos, cluster->m_max_pos);
    }

    GraphAdjancy adjancy;// = buildAdjancy(m_indexes, [&](uint32_t index) { return m_positions[index]; });

    uint32_t minIndex = 0;
    uint32_t maxIndex = clusters[0]->m_external_edges.size();
    for (auto edgeIndex = 0; edgeIndex < m_external_edges.size(); edgeIndex++) {
        if (edgeIndex >= maxIndex) {
            minIndex = maxIndex;
            maxIndex += clusters[edgeIndex]->m_external_edges.size();
        }
        auto adjCount = m_external_edges[edgeIndex].adjCount;
        adjancy.forAll(edgeIndex, [&](uint32_t edgeIndex, uint32_t adjIndex) {
            if (adjIndex < minIndex || adjIndex >= maxIndex) {
                adjCount++;
            }
        });
        adjCount                             = std::max(adjCount, 0u);
        m_external_edges[edgeIndex].adjCount = adjCount;
    }
}
Cluster::Cluster(Cluster* source, uint32_t start, uint32_t end, std::vector<uint32_t>& indexes, GraphAdjancy& adjancy) {
    m_positions.resize((end - start) * 3);
    m_normals.resize((end - start) * 3);
    m_uvs.resize((end - start) * 3);
    m_indexes.resize((end - start) * 3);
    m_external_edges.clear();
    m_bounding_box = BBox();
    m_min_pos      = glm::vec3(1e30f, 1e30f, 1e30f);
    m_max_pos      = glm::vec3(-1e30f, -1e30f, -1e30f);
    for (uint32_t i = start; i < end; i++) {
        uint32_t index         = indexes[i];
        m_positions[i - start] = source->m_positions[index];
        m_normals[i - start]   = source->m_normals[index];
        m_uvs[i - start]       = source->m_uvs[index];
        m_indexes[i - start]   = source->m_indexes[index];
        m_bounding_box         = m_bounding_box + m_positions[i - start];
        m_min_pos              = glm::min(m_min_pos, m_positions[i - start]);
        m_max_pos              = glm::max(m_max_pos, m_positions[i - start]);
    }
    for (uint32_t i = 0; i < m_indexes.size(); i++) {
        uint32_t index = m_indexes[i];
        for (uint32_t adjIndex : adjancy.adjVertices[index].adjVertices) {
            if (adjIndex < start || adjIndex >= end) {
                // m_external_edges.insert({index, adjIndex});
            }
        }
    }
}
GraphAdjancy Cluster::buildAdjacency() {
    return GraphAdjancy();
    // return buildAdjancy(m_indexes, [&](uint32_t index) { return m_positions[index]; });
}

// static void DAGReduce( TArray< FClusterGroup >& Groups, TArray< FCluster >& Clusters, TAtomic< uint32 >& NumClusters, TArrayView< uint32 > Children, uint32 NumParents, int32 GroupIndex, uint32 MeshIndex )

static void DAGReduce(std::vector<ClusterGroup>& Groups, std::vector<Cluster>& Clusters, std::atomic<uint32_t>& NumClusters, std::span<uint32_t> Children, uint32_t NumParents, uint32_t GroupIndex, uint32_t MeshIndex) {
    std::vector<Cluster*> mergeList;
    mergeList.resize(Children.size());
    for (uint32_t i = 0; i < Children.size(); i++) {
        mergeList[i] = &Clusters[Children[i]];
    }
    std::sort(mergeList.begin(), mergeList.end(), [](Cluster* a, Cluster* b) { return a->guid < b->guid; });
    Cluster          mergedCluster(mergeList);
    auto             adjancy       = mergedCluster.buildAdjacency();
    uint32           ParentStart   = 0;
    uint32           ParentEnd     = 0;
    int              triangleCount = mergedCluster.m_indexes.size() / 3;
    GraphPartitioner partitioner(triangleCount, triangleCount / ClusterSize);

    // partitioner.partition(adjancy);

    uint32_t targetClusterSize = ClusterSize - 2;
    uint32_t targetNumTris     = NumParents * targetClusterSize;
    float    error             = mergedCluster.simplify(targetNumTris);
    if (partitioner.ranges.size() <= NumParents) {
        NumParents  = partitioner.ranges.size();
        ParentEnd   = (NumClusters += NumParents);
        ParentStart = ParentEnd - NumParents;

        uint32_t Parent = ParentStart;
        for (auto& Range : partitioner.ranges) {
            //Clusters[Parent] = Cluster(&mergedCluster, Range.start, Range.end, partitioner.indexes, adjancy);
            Parent++;
        }
    }
    // std::vector<uint32_t> adjOffsets;
    // uint32_t
    // for(int i = 0;i<mergedCluster.m_indexes.size();i++) {
    //     adjOffsets.push_back(adjancy.adjVertices[mergedCluster.m_indexes[i]].size());
    // }
}

void BuildDAG(std::vector<ClusterGroup>& groups, std::vector<Cluster>& clusters, uint32_t ClusterStart, uint32_t clusterRangenNum, uint32_t MeshIndex, BBox MeshBounds) {
    bool                  bFirstLevel = true;
    std::atomic<uint32_t> numClusters = 0;
    uint32_t              levelOffset = ClusterStart;
    while (true) {
        std::span<Cluster> levelClusters(&clusters[levelOffset], bFirstLevel ? clusterRangenNum : clusters.size() - levelOffset);
        bFirstLevel = false;

        if (levelClusters.size() < 2) {
            break;
        }
        if (levelClusters.size() <= MaxClusterGroupSize) {
            std::vector<uint32_t> children;
            uint32_t              numGroupElements = 0;
            for (Cluster& cluster : levelClusters) {
                numGroupElements += cluster.m_indexes.size();
                children.push_back(levelOffset++);
            }
            uint32_t maxParents = numGroupElements / (ClusterSize * 2);
            DAGReduce(groups, clusters, numClusters, children, maxParents, groups.size() - clusterRangenNum, MeshIndex);
        } else {
            GraphAdjancy     adjancy = buildClusterGroupAdjancy(levelClusters);
            GraphPartitioner partitioner(levelClusters.size(), MinClusterGroupSize);
            // partitioner.partition(adjancy);
            for (auto& Range : partitioner.ranges) {
                // std::span<uint32_t> children(&partitioner.indexes[Range.start], Range.end - Range.start);
                uint32_t numGroupElements = 0;
                for (uint32_t i = Range.start; i < Range.end; i++) {
                    numGroupElements += clusters[partitioner.indexes[i]].m_indexes.size();
                }
                uint32_t maxParents        = numGroupElements / (ClusterSize * 2);
                uint32_t clusterGroupIndex = groups.size() - partitioner.ranges.size();
                // DAGReduce(groups, clusters, numClusters, children, maxParents, clusterGroupIndex, MeshIndex);
            }
        }
    }
}

uint jenkinsHash(uint a)
{
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23cu) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09u) ^ (a >> 16);
    return a;
}

vec3 pseudocolor(uint value)
{
    uint h = jenkinsHash(value);
    return glm::vec3(glm::uvec3(h, h >> 8, h >> 16) & 0xffu) / 255.f;
}

GraphPartitioner::FGraphData* GraphPartitioner::NewGraph(uint32_t maxEdges) const {
    auto graph = new FGraphData();

    // 预分配空间，避免动态扩容
    graph->AdjacencyOffset.resize(numElements + 1, 0);
    graph->Adjacency.reserve(maxEdges);
    graph->AdjacencyCost.reserve(maxEdges);

    return graph;
}

void GraphPartitioner::addAdjacency(FGraphData* graph, uint32_t toVertex, float weight) {
    if (!graph || toVertex >= numElements) {
        LOGE("Invalid parameters in addAdjacency");
        return;
    }

    // 添加边
    graph->Adjacency.push_back(toVertex);
    graph->AdjacencyCost.push_back(weight);
}
void NaniteBuilder::Build(MeshInputData& InputMeshData, MeshOutputData* OutFallbackMeshData, const MeshNaniteSettings& Settings) {
    LOGI("NaniteBuilder::Build Vertex Count: %d", InputMeshData.Vertices.Positions.size());

    std::vector<Cluster>  clusters;
    std::vector<uint32_t> clusterPerMesh;
    {
        uint32_t baseTriangle = 0;
        for (uint32_t numTriangles : InputMeshData.TriangleCounts) {
            uint32_t clusterOffset = clusters.size();
            // 使用新的优化版本
            std::vector<idx_t> part_result;
            clusterTriangles1(clusters, InputMeshData, baseTriangle, numTriangles,part_result);
            clusterPerMesh.push_back(clusters.size() - clusterOffset);
            baseTriangle += numTriangles;
        }
    }

    // ... 其余代码保持不变 ...
}

template<typename T1, typename T2>
void ConvertData(std::vector<T1>& data, std::vector<T2>& outData) {
    outData.resize(data.size() * sizeof(T1) / sizeof(T2));
    std::memcpy(outData.data(), data.data(), data.size() * sizeof(T1));
}

std::unique_ptr<MeshInputData> NaniteBuilder::createNaniteExampleMeshInputData() {
    auto primData      = PrimitiveLoader::loadPrimitive(FileUtils::getResourcePath("tiny_nanite/bunny.obj"));
    auto meshInputData = std::make_unique<MeshInputData>();
    ConvertData(primData->buffers[POSITION_ATTRIBUTE_NAME], meshInputData->Vertices.Positions);
    ConvertData(primData->buffers[NORMAL_ATTRIBUTE_NAME], meshInputData->Vertices.Normals);
    ConvertData(primData->buffers[TEXCOORD_ATTRIBUTE_NAME], meshInputData->Vertices.UVs);
    ConvertData(primData->indexs, meshInputData->TriangleIndices);
    meshInputData->TriangleCounts.push_back(meshInputData->TriangleIndices.size() / 3);
    meshInputData->MaterialIndices = {0};
    return meshInputData;
}

glm::vec3 getClusterColor(idx_t clusterId) {
    const float goldenRatio = 0.618033988749895f;
    const float saturation  = 0.6f;
    const float value       = 0.95f;

    float hue = fmod(clusterId * goldenRatio, 1.0f);

    // HSV to RGB
    float h = hue * 6.0f;
    float c = value * saturation;
    float x = c * (1.0f - fabs(fmod(h, 2.0f) - 1.0f));
    float m = value - c;

    glm::vec3 rgb;
    if (h < 1.0f)
        rgb = glm::vec3(c, x, 0.0f);
    else if (h < 2.0f)
        rgb = glm::vec3(x, c, 0.0f);
    else if (h < 3.0f)
        rgb = glm::vec3(0.0f, c, x);
    else if (h < 4.0f)
        rgb = glm::vec3(0.0f, x, c);
    else if (h < 5.0f)
        rgb = glm::vec3(x, 0.0f, c);
    else
        rgb = glm::vec3(c, 0.0f, x);

    return rgb + glm::vec3(m);
}

glm::vec4 getClusterColorPalette(uint32_t clusterId) {
    static const glm::vec4 palette[] = {
        {0.957f, 0.263f, 0.212f, 1.0f},  // Red
        {0.133f, 0.545f, 0.133f, 1.0f},  // Green
        {0.231f, 0.455f, 0.969f, 1.0f},  // Blue
        {0.945f, 0.769f, 0.059f, 1.0f},  // Yellow
        {0.608f, 0.349f, 0.714f, 1.0f},  // Purple
        {0.004f, 0.588f, 0.533f, 1.0f},  // Teal
        {0.957f, 0.643f, 0.376f, 1.0f},  // Orange
        {0.741f, 0.718f, 0.420f, 1.0f},  // Olive
        {0.404f, 0.227f, 0.718f, 1.0f},  // Indigo
        {0.914f, 0.118f, 0.388f, 1.0f},  // Pink
        {0.475f, 0.333f, 0.282f, 1.0f},  // Brown
        {0.612f, 0.153f, 0.690f, 1.0f},  // Deep Purple
    };
        
    const size_t paletteSize = sizeof(palette) / sizeof(palette[0]);
    return palette[clusterId % paletteSize];
}

glm::vec3 getClusterColorCombined(uint32_t clusterId) {
    // 使用预定义调色板的基础颜色
    glm::vec4 baseColor = getClusterColorPalette(clusterId / 12);
        
    // 使用黄金比例法微调色相
    float hueShift = fmod(clusterId * 0.618033988749895f, 0.2f) - 0.1f;
        
    // RGB to HSV
    float maxVal = std::max(std::max(baseColor.r, baseColor.g), baseColor.b);
    float minVal = std::min(std::min(baseColor.r, baseColor.g), baseColor.b);
    float delta = maxVal - minVal;
        
    float hue = 0.0f;
    if (delta > 0.0f) {
        if (maxVal == baseColor.r) {
            hue = fmod((baseColor.g - baseColor.b) / delta + 6.0f, 6.0f) / 6.0f;
        } else if (maxVal == baseColor.g) {
            hue = ((baseColor.b - baseColor.r) / delta + 2.0f) / 6.0f;
        } else {
            hue = ((baseColor.r - baseColor.g) / delta + 4.0f) / 6.0f;
        }
    }
        
    // 调整色相
    hue = fmod(hue + hueShift + 1.0f, 1.0f);
        
    // HSV to RGB
    float h = hue * 6.0f;
    float c = maxVal * delta;
    float x = c * (1.0f - fabs(fmod(h, 2.0f) - 1.0f));
    float m = maxVal - c;

    glm::vec3 rgb;
    if(h < 1.0f)      rgb = glm::vec3(c, x, 0.0f);
    else if(h < 2.0f) rgb = glm::vec3(x, c, 0.0f);
    else if(h < 3.0f) rgb = glm::vec3(0.0f, c, x);
    else if(h < 4.0f) rgb = glm::vec3(0.0f, x, c);
    else if(h < 5.0f) rgb = glm::vec3(x, 0.0f, c);
    else              rgb = glm::vec3(c, 0.0f, x);

    return rgb + glm::vec3(m);
}

std::unique_ptr<Primitive> NaniteBuilder::createNaniteExamplePrimitive() {
    auto primData      = PrimitiveLoader::loadPrimitive(FileUtils::getResourcePath("tiny_nanite/bunny.obj"));
    auto meshInputData = std::make_unique<MeshInputData>();
    ConvertData(primData->buffers[POSITION_ATTRIBUTE_NAME], meshInputData->Vertices.Positions);
    ConvertData(primData->buffers[NORMAL_ATTRIBUTE_NAME], meshInputData->Vertices.Normals);
    ConvertData(primData->buffers[TEXCOORD_ATTRIBUTE_NAME], meshInputData->Vertices.UVs);
    ConvertData(primData->indexs, meshInputData->TriangleIndices);
    meshInputData->TriangleCounts.push_back(meshInputData->TriangleIndices.size() / 3);
    meshInputData->MaterialIndices = {0};
    std::vector<Cluster> clusters;
    std::vector<idx_t>   partId;
    clusterTriangles1(clusters, *meshInputData, 0, meshInputData->TriangleIndices.size() / 3, partId);

    std::unordered_map<uint32_t,glm::vec3> idToColors;
    for(int i =0;i<meshInputData->TriangleIndices.size();i++) {
        auto index = meshInputData->TriangleIndices[i];
        auto part = partId[i/3];
        if(idToColors.find(index) == idToColors.end()) {
            idToColors[index] = getClusterColor(part);
        }
    }

    std::vector<glm::vec3> colors(idToColors.size());
    for(auto& [id,color] : idToColors) {
        colors[id] = color;
    }
    // for (int i = 0; i < meshInputData->TriangleIndices.size() / 3; i++) {
    //     for (int k = 0; k < 3; k++) {
    //         idToColors.push_back(pseudocolor(partId[i]));
    //     }
    // }
    auto primitve                       = std::make_unique<Primitive>(0, 0, meshInputData->Vertices.Positions.size(), meshInputData->TriangleIndices.size(), 0);
    primData->vertexAttributes["color"] = {VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)};
    primData->buffers["color"]          = std::vector<uint8_t>((uint8_t*)colors.data(), (uint8_t*)colors.data() + idToColors.size() * sizeof(glm::vec3));

    return SceneLoaderInterface::loadPrimitiveFromPritiveData(g_context->getDevice(),primData.get());
}

float Cluster::simplify(uint32_t targetNumTris) {
    return 0.f;
}
GraphPartitioner::GraphPartitioner(uint32_t elementsNum, uint32_t targetPart) {
    this->targetPart = targetPart;
    numElements      = elementsNum;
    indexes.resize(numElements);
    for (uint32_t i = 0; i < numElements; i++) {
        indexes[i] = i;
    }
    partitionIDs.resize(numElements);
}
void GraphPartitioner::partition(FGraphData& graph) {
    idx_t NumConstraints = 1;
    int   NumParts       = targetPart;
    partitionIDs.resize(numElements);
    std::fill(partitionIDs.begin(), partitionIDs.end(), 0);

    // METIS要求的参数
    idx_t nvtxs  = numElements;// 顶点数量
    idx_t ncon   = 1;          // 约束数量（通常为1）
    idx_t nparts = NumParts;   // 分区数量

    // METIS选项
    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);
    options[METIS_OPTION_UFACTOR] = 200;// 0-based numbering

    // 权重数组
    std::vector<idx_t> vwgt(nvtxs, 1);// 顶点权重，默认为1
    std::vector<idx_t> adjwgt;        // 边权重

    // 转换边权重为idx_t类型
    if (!graph.AdjacencyCost.empty()) {
        adjwgt.resize(graph.AdjacencyCost.size());
        for (size_t i = 0; i < graph.AdjacencyCost.size(); i++) {
            adjwgt[i] = static_cast<idx_t>(graph.AdjacencyCost[i]);
        }
    }

    // 转换偏移数组为idx_t类型
    std::vector<idx_t> xadj(graph.AdjacencyOffset.size());
    for (size_t i = 0; i < graph.AdjacencyOffset.size(); i++) {
        xadj[i] = static_cast<idx_t>(graph.AdjacencyOffset[i]);
    }

    // 转换邻接数组为idx_t类型
    std::vector<idx_t> adjncy(graph.Adjacency.size());
    for (size_t i = 0; i < graph.Adjacency.size(); i++) {
        adjncy[i] = static_cast<idx_t>(graph.Adjacency[i]);
    }

    // 分区结果
    std::vector<idx_t> part(nvtxs);

    // 调试输出
    LOGI("METIS Input - Vertices: %d, Parts: %d, Edges: %d",
         nvtxs,
         nparts,
         graph.Adjacency.size());

    // 检查数据有效性
    if (nvtxs < nparts) {
        LOGE("Number of vertices (%d) less than number of parts (%d)", nvtxs, nparts);
        return;
    }

    // nvtxs = 0;
    int objval = 0;
    // 调用METIS
    int result = METIS_PartGraphKway(
        &nvtxs,                                  // 顶点数量
        &ncon,                                   // 约束数量
        xadj.data(),                             // 偏移数组
        adjncy.data(),                           // 邻接数组
        nullptr,                                 // 顶点权重 (nullptr表示所有权重为1)
        nullptr,            // 顶点大小 (nullptr表示所有大小为1)
        adjwgt.empty() ? nullptr : adjwgt.data(),  // 边权重
        &nparts,            // 分区数量
        nullptr,            // 目标分区大小
        nullptr,            // 分区权重
        options,            // 选项数组
        &objval,          // 输出：边切割数量
        part.data()         // 输出：分区结果
    );

    partitionIDs = part;
}
