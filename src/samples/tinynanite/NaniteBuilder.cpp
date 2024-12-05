#include "NaniteBuilder.h"

#include "meshoptimizer.h"
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

void SaveMeshInputDataToObj(const MeshInputData& meshData, const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        LOGE("Failed to open file for writing: %s", filePath.c_str());
        return;
    }

    // Write header comment
    file << "# Exported from MeshInputData\n";
    file << "# Vertices: " << meshData.Vertices.Positions.size() << "\n";
    file << "# Triangles: " << meshData.TriangleIndices.size() / 3 << "\n\n";

    // Write vertices
    for (const auto& pos : meshData.Vertices.Positions) {
        file << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";
    }

    // Write texture coordinates
    for (const auto& uv : meshData.Vertices.UVs) {
        file << "vt " << uv.x << " " << uv.y << "\n";
    }

    // Write normals
    for (const auto& normal : meshData.Vertices.Normals) {
        file << "vn " << normal.x << " " << normal.y << " " << normal.z << "\n";
    }

    // Write faces
    // OBJ format is 1-based indexing, so we need to add 1 to all indices
    uint32_t baseTriangle = 0;
    for (uint32_t meshIndex = 0; meshIndex < meshData.TriangleCounts.size(); meshIndex++) {
        file << "\n# Mesh " << meshIndex << " (Material Index: "
             << (meshIndex < meshData.MaterialIndices.size() ? std::to_string(meshData.MaterialIndices[meshIndex]) : "N/A")
             << ")\n";

        for (uint32_t i = 0; i < meshData.TriangleCounts[meshIndex]; i++) {
            uint32_t idx0 = meshData.TriangleIndices[baseTriangle + i * 3 + 0] + 1;
            uint32_t idx1 = meshData.TriangleIndices[baseTriangle + i * 3 + 1] + 1;
            uint32_t idx2 = meshData.TriangleIndices[baseTriangle + i * 3 + 2] + 1;

            // Format: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            file << "f "
                 << idx0 << "/" << idx0 << "/" << idx0 << " "
                 << idx1 << "/" << idx1 << "/" << idx1 << " "
                 << idx2 << "/" << idx2 << "/" << idx2 << "\n";
        }
        baseTriangle += meshData.TriangleCounts[meshIndex] * 3;
    }

    file.close();
    LOGI("Successfully saved mesh to: %s", filePath.c_str());
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

        if (hashTable1.contains(hash)) {
            for (auto& anotherEdge : hashTable1[hash]) {
                callback(index, anotherEdge);
            }
        } else {
        }

        hash = Murmur32({hash1, hash0});

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

    for (uint32_t i = 0; i < clusters.size(); i++) {
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
GraphAdjancy buildClusterGroupAdjancy1(std::span<Cluster> clusters, uint32_t levelOffset) {
    GraphAdjancy graphData(clusters.size());

    // 遍历所有cluster
    for (uint32_t localId = 0; localId < clusters.size(); localId++) {
        const auto& cluster = clusters[localId];

        // 对于每个相邻的cluster
        for (const auto& globalAdjId : cluster.m_linked_cluster) {
            // 检查相邻cluster是否在当前span范围内
            if (globalAdjId >= levelOffset && globalAdjId < levelOffset + clusters.size()) {
                uint32_t localAdjId = globalAdjId - levelOffset;
                graphData.addEdge(localId, localAdjId);
            }
        }
    }

    return graphData;
}

struct Triangle {
    vec3 position[3];
};

void PrintTriangle(const Triangle& tri) {
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
                LOGE("Triangle {} and {} is not adjancy", i, adjIndex);
            }
        }
    }
}

void clusterTriangles2(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
    if (numTriangles == 0) {
        LOGE("No triangles to cluster");
        return;
    }

    uint32_t targetClusterCount = std::max(1u, (numTriangles + ClusterSize - 1) / ClusterSize);

    GraphPartitioner partitioner(numTriangles, targetClusterCount);
    auto             graph = partitioner.NewGraph(numTriangles * 6);

    if (!graph) {
        LOGE("Failed to create graph");
        return;
    }

    graph->AdjacencyOffset[0] = 0;

    std::vector<std::vector<uint32_t>> triangleAdjacency(numTriangles);

    for (uint32_t i = 0; i < numTriangles; i++) {
        uint32_t i0 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 0];
        uint32_t i1 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 1];
        uint32_t i2 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 2];

        for (uint32_t j = 0; j < numTriangles; j++) {
            if (i == j) continue;

            uint32_t j0 = InputMeshData.TriangleIndices[baseTriangle + j * 3 + 0];
            uint32_t j1 = InputMeshData.TriangleIndices[baseTriangle + j * 3 + 1];
            uint32_t j2 = InputMeshData.TriangleIndices[baseTriangle + j * 3 + 2];

            bool isAdjacent = false;
            isAdjacent |= (i0 == j0 && i1 == j1) || (i0 == j1 && i1 == j0);
            isAdjacent |= (i1 == j1 && i2 == j2) || (i1 == j2 && i2 == j1);
            isAdjacent |= (i2 == j2 && i0 == j0) || (i2 == j0 && i0 == j2);

            if (isAdjacent) {
                triangleAdjacency[i].push_back(j);
            }
        }
    }

    for (uint32_t i = 0; i < numTriangles; i++) {
        const auto& adjacentTris = triangleAdjacency[i];

        graph->AdjacencyOffset[i + 1] = graph->AdjacencyOffset[i] + adjacentTris.size();

        for (uint32_t adjTri : adjacentTris) {
            float weight = 1.0f;

            graph->Adjacency.push_back(adjTri);
            graph->AdjacencyCost.push_back(weight);
        }
    }

    bool isValid = true;
    for (uint32_t i = 0; i < numTriangles; i++) {
        if (graph->AdjacencyOffset[i] > graph->AdjacencyOffset[i + 1]) {
            LOGE("Invalid adjacency offset at {}", i);
            isValid = false;
            break;
        }
    }

    if (!isValid) {
        delete graph;
        return;
    }

    LOGI("Graph stats:");
    LOGI("  Triangles: {}", numTriangles);
    LOGI("  Target clusters: {}", targetClusterCount);
    LOGI("  Total edges: {}", graph->Adjacency.size());
    LOGI("  Average edges per triangle: %.2f",
         static_cast<float>(graph->Adjacency.size()) / numTriangles);

    partitioner.partition(*graph);

    delete graph;
}

Cluster InitClusterFromMeshInputData(MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
    Cluster cluster;

    for (uint32_t i = 0; i < numTriangles; i++) {
        for (int k = 0; k < 3; k++) {
            uint32_t globalIndex = InputMeshData.TriangleIndices[baseTriangle + i * 3 + k];

            if (std::find(cluster.m_indexes.begin(), cluster.m_indexes.end(), globalIndex) == cluster.m_indexes.end()) {
                cluster.m_indexes.push_back(globalIndex);
                cluster.m_positions.push_back(InputMeshData.Vertices.Positions[globalIndex]);
                cluster.m_normals.push_back(InputMeshData.Vertices.Normals[globalIndex]);
                cluster.m_uvs.push_back(InputMeshData.Vertices.UVs[globalIndex]);
            }
        }
    }

    return cluster;
}

void clusterTriangles1(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
    GraphAdjancy graphData          = buildAdjancy(std::span<uint32_t>(InputMeshData.TriangleIndices.data() + baseTriangle, numTriangles * 3), [&](uint32_t index) {
        return InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[index]];
    });
    uint32_t     targetClusterCount = numTriangles / ClusterSize;

    if (targetClusterCount == 1) {
        clusters.push_back(InitClusterFromMeshInputData(InputMeshData, baseTriangle, numTriangles));
        return;
    }

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

    SaveMeshInputDataToObj(InputMeshData, FileUtils::getFilePath("mesh", "obj"));

    partitioner.partition(*graph);

    struct ClusterAdjacency {
        std::unordered_map<uint32_t, std::unordered_set<uint32_t>> adj_clusters;

        void addEdge(uint32_t cluster1, uint32_t cluster2) {
            if (cluster1 != cluster2) {
                adj_clusters[cluster1].insert(cluster2);
                adj_clusters[cluster2].insert(cluster1);
            }
        }
    } clusterAdj;

    std::vector<std::unordered_map<uint32_t, uint32_t>> oldToNewIndexMaps(targetClusterCount);
    const uint32_t                                      oldClusterCount = clusters.size();
    clusters.resize(clusters.size() + targetClusterCount);

    for (uint32_t i = 0; i < numTriangles; i++) {
        int   partId           = partitioner.partitionIDs[i];
        auto  clusterId        = oldClusterCount + partId;
        auto& oldToNewIndexMap = oldToNewIndexMaps[partId];

        for (int k = 0; k < 3; k++) {
            uint32_t globalIndex = InputMeshData.TriangleIndices[baseTriangle + i * 3 + k];

            if (oldToNewIndexMap.find(globalIndex) == oldToNewIndexMap.end()) {
                oldToNewIndexMap[globalIndex] = clusters[clusterId].m_positions.size();
                clusters[clusterId].m_positions.push_back(InputMeshData.Vertices.Positions[globalIndex]);
                clusters[clusterId].m_normals.push_back(InputMeshData.Vertices.Normals[globalIndex]);
                clusters[clusterId].m_uvs.push_back(InputMeshData.Vertices.UVs[globalIndex]);
            }
            clusters[clusterId].m_indexes.push_back(oldToNewIndexMap[globalIndex]);
        }
    }

    for (uint32_t i = 0; i < numTriangles * 3; i++) {
        uint32_t currentCluster = partitioner.partitionIDs[i / 3] + oldClusterCount;

        graphData.forAll(i, [&](uint32_t edgeIndex0, uint32_t edgeIndex1) {
            uint32_t neighborCluster = partitioner.partitionIDs[edgeIndex1 / 3] + oldClusterCount;
            clusterAdj.addEdge(currentCluster, neighborCluster);
        });
    }

    for (const auto& [clusterId, adjClusters] : clusterAdj.adj_clusters) {
        auto& cluster            = clusters[clusterId];
        cluster.m_linked_cluster = adjClusters;

        for (auto adjClusterId : adjClusters) {
            const auto& adjCluster = clusters[adjClusterId];

            for (uint32_t i = 0; i < cluster.m_positions.size(); i++) {
                const auto& pos = cluster.m_positions[i];

                for (uint32_t j = 0; j < adjCluster.m_positions.size(); j++) {
                    if (pos == adjCluster.m_positions[j]) {
                        ClusterExternEdge edge;
                        edge.v0           = i;
                        edge.v1           = j;
                        edge.clusterIndex = adjClusterId;
                        cluster.m_external_edges.push_back(edge);
                        break;
                    }
                }
            }
        }
    }

    for (uint32_t i = oldClusterCount; i < clusters.size(); i++) {
        auto& cluster = clusters[i];
        cluster.guid  = i;

        cluster.m_bounding_box = BBox();
        cluster.m_min_pos      = glm::vec3(1e30f);
        cluster.m_max_pos      = glm::vec3(-1e30f);

        for (const auto& pos : cluster.m_positions) {
            cluster.m_bounding_box.unite(pos);
            cluster.m_min_pos = glm::min(cluster.m_min_pos, pos);
            cluster.m_max_pos = glm::max(cluster.m_max_pos, pos);
        }
    }

    delete graph;
}

void clusterTriangles3(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
    if (numTriangles == 0) return;

    uint32_t         targetClusterCount = std::max(1u, (numTriangles + ClusterSize - 1) / ClusterSize);
    GraphPartitioner partitioner(numTriangles, targetClusterCount);
    auto             graph = partitioner.NewGraph(numTriangles * 6);

    struct Edge {
        uint32_t v0, v1;

        Edge(uint32_t a, uint32_t b) {
            v0 = std::min(a, b);
            v1 = std::max(a, b);
        }

        bool operator==(const Edge& other) const {
            return (v0 == other.v0 && v1 == other.v1);
        }
    };

    struct EdgeHash {
        size_t operator()(const Edge& edge) const {
            return (static_cast<size_t>(edge.v0) << 32) | edge.v1;
        }
    };

    std::unordered_map<Edge, std::vector<uint32_t>, EdgeHash> edgeToTriangles;
    edgeToTriangles.reserve(numTriangles * 3);

    for (uint32_t triIdx = 0; triIdx < numTriangles; triIdx++) {
        uint32_t idx = baseTriangle + triIdx * 3;
        uint32_t i0  = InputMeshData.TriangleIndices[idx + 0];
        uint32_t i1  = InputMeshData.TriangleIndices[idx + 1];
        uint32_t i2  = InputMeshData.TriangleIndices[idx + 2];

        edgeToTriangles[Edge(i0, i1)].push_back(triIdx);
        edgeToTriangles[Edge(i1, i2)].push_back(triIdx);
        edgeToTriangles[Edge(i2, i0)].push_back(triIdx);
    }

    std::vector<std::vector<std::pair<uint32_t, float>>> triangleAdjacency(numTriangles);

    for (const auto& pair : edgeToTriangles) {
        const auto& triangles = pair.second;
        for (size_t i = 0; i < triangles.size(); ++i) {
            for (size_t j = i + 1; j < triangles.size(); ++j) {
                triangleAdjacency[triangles[i]].push_back(std::make_pair(triangles[j], 1.0f));
                triangleAdjacency[triangles[j]].push_back(std::make_pair(triangles[i], 1.0f));
            }
        }
    }

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

    partitioner.partition(*graph);

    const uint32_t oldClusterCount = clusters.size();
    clusters.resize(oldClusterCount + targetClusterCount);
    std::vector<std::unordered_map<uint32_t, uint32_t>> oldToNewIndexMaps(targetClusterCount);

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

    for (uint32_t i = 0; i < numTriangles; i++) {
        int   partId   = partitioner.partitionIDs[i];
        auto& cluster  = clusters[oldClusterCount + partId];
        auto& indexMap = oldToNewIndexMaps[partId];

        for (int k = 0; k < 3; k++) {
            uint32_t globalIndex = InputMeshData.TriangleIndices[baseTriangle + i * 3 + k];
            cluster.m_indexes.push_back(indexMap[globalIndex]);
        }
    }

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

    std::unordered_map<ClusterEdge, std::vector<uint32_t>, ClusterEdgeHash> clusterEdges;

    for (uint32_t i = 0; i < numTriangles; i++) {
        int      partId    = partitioner.partitionIDs[i];
        uint32_t clusterId = oldClusterCount + partId;
        auto&    cluster   = clusters[clusterId];
        auto&    indexMap  = oldToNewIndexMaps[partId];

        uint32_t i0 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 0];
        uint32_t i1 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 1];
        uint32_t i2 = InputMeshData.TriangleIndices[baseTriangle + i * 3 + 2];

        clusterEdges[ClusterEdge(i0, i1)].push_back(clusterId);
        clusterEdges[ClusterEdge(i1, i2)].push_back(clusterId);
        clusterEdges[ClusterEdge(i2, i0)].push_back(clusterId);
    }

    for (const auto& [edge, clusterIds] : clusterEdges) {
        if (clusterIds.size() > 1) {
            for (size_t i = 0; i < clusterIds.size(); ++i) {
                uint32_t clusterId = clusterIds[i];
                auto&    cluster   = clusters[clusterId];
                auto&    indexMap  = oldToNewIndexMaps[clusterId - oldClusterCount];

                uint32_t localV0 = indexMap[edge.v0];
                uint32_t localV1 = indexMap[edge.v1];

                for (size_t j = 0; j < clusterIds.size(); ++j) {
                    if (i != j) {
                        cluster.m_external_edges.push_back({localV0,
                                                            localV1,
                                                            clusterIds[j]});
                    }
                }
            }
        }
    }

    for (uint32_t i = oldClusterCount; i < clusters.size(); ++i) {
        auto& cluster = clusters[i];

        for (const auto& pos : cluster.m_positions) {
            cluster.m_min_pos = glm::min(cluster.m_min_pos, pos);
            cluster.m_max_pos = glm::max(cluster.m_max_pos, pos);
            cluster.m_bounding_box.unite(pos);
        }

        cluster.guid = i;

        for (const auto& edge : cluster.m_external_edges) {
            cluster.m_linked_cluster.insert(edge.clusterIndex);
        }
    }

    delete graph;
}

inline float calculateEdgeWeight(const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& normal1, const glm::vec3& normal2) {
    float distWeight   = glm::length2(pos1 - pos2);
    float normalWeight = 1.0f - glm::dot(normal1, normal2);
    return distWeight * (1.0f + normalWeight * 2.0f);
}

void clusterTriangles(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
}

// void BuildDAG(std::vector<ClusterGroup>& groups, std::vector<Cluster>& clusters, uint32_t ClusterStart, uint32_t clusterRangeNum, uint32_t MeshIndex, BBox MeshBounds) {
//     bool bFirstLevel = true;
//     std::atomic<uint32_t> numClusters = 0;
//     uint32_t levelOffset = ClusterStart;
//
//     while (true) {
//         std::span<Cluster> levelClusters(&clusters[levelOffset], bFirstLevel ? clusterRangeNum : clusters.size() - levelOffset);
//         bFirstLevel = false;
//
//         if (levelClusters.size() < 2) {
//             break;
//         }
//
//         if (levelClusters.size() <= MaxClusterGroupSize) {
//             std::vector<uint32_t> children;
//             uint32_t numGroupElements = 0;
//             for (uint32_t i = 0; i < levelClusters.size(); i++) {
//                 children.push_back(levelOffset + i);
//                 numGroupElements += levelClusters[i].m_indexes.size();
//             }
//             uint32_t maxParents = numGroupElements / (ClusterSize * 2);
//             DAGReduce(groups, clusters, numClusters, children, maxParents, groups.size() - 1, MeshIndex);
//         } else {
//             GraphAdjancy adjancy = buildClusterGroupAdjancy(levelClusters);
//
//             uint32_t targetGroupCount = (levelClusters.size() + MinClusterGroupSize - 1) / MinClusterGroupSize;
//             GraphPartitioner partitioner(levelClusters.size(), targetGroupCount);
//
//             auto graph = partitioner.NewGraph(levelClusters.size() * 6);
//             if (!graph) {
//                 LOGE("Failed to create graph for cluster grouping");
//                 return;
//             }
//
//             graph->AdjacencyOffset[0] = 0;
//             for (uint32_t i = 0; i < levelClusters.size(); i++) {
//                 graph->AdjacencyOffset[i + 1] = graph->Adjacency.size();
//                 for (const auto& adjClusterId : levelClusters[i].m_linked_cluster) {
//                     if (adjClusterId >= levelOffset && adjClusterId < levelOffset + levelClusters.size()) {
//                         float weight = 1.0f;
//                         partitioner.addAdjacency(graph, adjClusterId - levelOffset, weight);
//                     }
//                 }
//             }
//
//             partitioner.partition(*graph);
//             delete graph;
//
//             groups.resize(groups.size() + partitioner.ranges.size());
//             for (auto& Range : partitioner.ranges) {
//                 std::vector<uint32_t> children;
//                 uint32_t numGroupElements = 0;
//
//                 for (uint32_t i = Range.start; i < Range.end; i++) {
//                     uint32_t clusterId = levelOffset + partitioner.indexes[i];
//                     children.push_back(clusterId);
//                     numGroupElements += clusters[clusterId].m_indexes.size();
//                 }
//
//                 uint32_t maxParents = numGroupElements / (ClusterSize * 2);
//                 uint32_t clusterGroupIndex = groups.size() - partitioner.ranges.size();
//                 DAGReduce(groups, clusters, numClusters, children, maxParents, clusterGroupIndex, MeshIndex);
//             }
//         }
//
//         levelOffset = clusters.size() - numClusters;
//     }
// }

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
        m_bounding_box = m_bounding_box + cluster->m_bounding_box;
        m_min_pos      = glm::min(m_min_pos, cluster->m_min_pos);
        m_max_pos      = glm::max(m_max_pos, cluster->m_max_pos);
    }

    GraphAdjancy adjancy;

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
            }
        }
    }
}
GraphAdjancy Cluster::buildAdjacency() {
    return GraphAdjancy();
}

// ClusterGroup 结构体定义
struct ClusterGroup {
    uint32_t startIndex; // 在clusters数组中的起始索引
    uint32_t count;      // 包含的cluster数量
    BBox     boundingBox;// 组的包围盒
    float    errorMetric;// LOD误差度量

    ClusterGroup()
        : startIndex(0), count(0), boundingBox(), errorMetric(0.0f) {}

    // 从一组clusters初始化
    void initFromClusters(const std::vector<Cluster>& clusters, uint32_t start, uint32_t clusterCount) {
        startIndex = start;
        count      = clusterCount;

        // 计算包围盒
        boundingBox = BBox();
        errorMetric = 0.0f;

        for (uint32_t i = 0; i < count; i++) {
            const auto& cluster = clusters[startIndex + i];
            boundingBox         = boundingBox + cluster.m_bounding_box;
        }
    }

    // 获取组内的clusters
    std::span<Cluster> getClusters(std::vector<Cluster>& clusters) {
        return std::span<Cluster>(&clusters[startIndex], count);
    }

    std::span<const Cluster> getClusters(const std::vector<Cluster>& clusters) const {
        return std::span<const Cluster>(&clusters[startIndex], count);
    }
};

float simplifyMeshData1(MeshInputData& inputData, uint32_t targetNumTris) {
    if (inputData.TriangleIndices.empty() || targetNumTris >= inputData.TriangleIndices.size() / 3) {
        return 0.0f;
    }
    SaveMeshInputDataToObj(inputData, "before.obj");

    // 准备顶点位置数据
    size_t             vertexCount = inputData.Vertices.Positions.size();
    std::vector<float> vertex_positions;
    vertex_positions.reserve(vertexCount * 3);
    for (const auto& pos : inputData.Vertices.Positions) {
        vertex_positions.push_back(pos.x);
        vertex_positions.push_back(pos.y);
        vertex_positions.push_back(pos.z);
    }

    // 准备顶点属性数据 (normals + uvs)
    std::vector<float> vertex_attributes;
    vertex_attributes.reserve(vertexCount * (3 + 2));// 3 for normal, 2 for uv
    for (size_t i = 0; i < vertexCount; ++i) {
        // Add normal
        vertex_attributes.push_back(inputData.Vertices.Normals[i].x);
        vertex_attributes.push_back(inputData.Vertices.Normals[i].y);
        vertex_attributes.push_back(inputData.Vertices.Normals[i].z);
        // Add UV
        vertex_attributes.push_back(inputData.Vertices.UVs[i].x);
        vertex_attributes.push_back(inputData.Vertices.UVs[i].y);
    }

    // 设置属性权重
    std::vector<float> attribute_weights = {1.0f, 1.0f};// normal和uv的权重

    // 准备索引数据
    std::vector<unsigned int> indices(inputData.TriangleIndices.begin(), inputData.TriangleIndices.end());
    std::vector<unsigned int> destination_indices(indices.size());

    // 简化参数
    float target_error = 0.1f;
    float lod_error    = 0.0f;
    uint  max_iter     = 10;
    uint  iteration    = 0;

    size_t simplified_index_count = 0;
    do {
        simplified_index_count = meshopt_simplifyWithAttributes(
            destination_indices.data(),// destination indices
            indices.data(),            // source indices
            indices.size(),            // index count
            vertex_positions.data(),   // vertex positions
            vertexCount,               // vertex count
            sizeof(float) * 3,         // vertex position stride
            vertex_attributes.data(),  // vertex attributes
            sizeof(float) * 5,         // vertex attribute stride (3 for normal + 2 for uv)
            attribute_weights.data(),  // attribute weights
            2,                         // attribute count (normal and uv)
            nullptr,                   // vertex lock (optional)
            targetNumTris * 3,         // target index count
            target_error,              // target error
            0,                         // options
            &lod_error                 // result error
        );

        target_error *= 1.5f;
        target_error = std::min(target_error, 3.0f);
        iteration++;
    } while (float(simplified_index_count) > float(targetNumTris * 3) * 1.1f && iteration < max_iter);

    if (iteration >= max_iter) {
        LOGE("simplify iteration exceed max iteration %u", max_iter);
        return 0.0f;
    }

    // 重新映射顶点
    std::vector<unsigned int> remap(vertexCount);
    size_t                    unique_vertex_count = meshopt_generateVertexRemap(
        remap.data(),
        destination_indices.data(),
        simplified_index_count,
        vertex_positions.data(),
        vertexCount,
        sizeof(float) * 3);

    if (unique_vertex_count == 0) {
        LOGE("meshopt_generateVertexRemap failed");
        return 0.0f;
    }

    // 创建新的顶点数据
    std::vector<glm::vec3> new_positions;
    std::vector<glm::vec3> new_normals;
    std::vector<glm::vec2> new_uvs;
    new_positions.reserve(unique_vertex_count);
    new_normals.reserve(unique_vertex_count);
    new_uvs.reserve(unique_vertex_count);

    // 创建重映射后的顶点缓冲区
    std::vector<unsigned int> remapped_indices(simplified_index_count);
    meshopt_remapIndexBuffer(remapped_indices.data(), destination_indices.data(), simplified_index_count, remap.data());

    // 重新映射顶点属性
    std::vector<float> remapped_vertices(unique_vertex_count * 3);
    std::vector<float> remapped_attributes(unique_vertex_count * 5);

    meshopt_remapVertexBuffer(remapped_vertices.data(), vertex_positions.data(), vertexCount, sizeof(float) * 3, remap.data());
    meshopt_remapVertexBuffer(remapped_attributes.data(), vertex_attributes.data(), vertexCount, sizeof(float) * 5, remap.data());

    // 转换回glm类型
    for (size_t i = 0; i < unique_vertex_count; ++i) {
        new_positions.push_back(glm::vec3(
            remapped_vertices[i * 3 + 0],
            remapped_vertices[i * 3 + 1],
            remapped_vertices[i * 3 + 2]));
        new_normals.push_back(glm::vec3(
            remapped_attributes[i * 5 + 0],
            remapped_attributes[i * 5 + 1],
            remapped_attributes[i * 5 + 2]));
        new_uvs.push_back(glm::vec2(
            remapped_attributes[i * 5 + 3],
            remapped_attributes[i * 5 + 4]));
    }

    // 更新输入数据
    inputData.Vertices.Positions = std::move(new_positions);
    inputData.Vertices.Normals   = std::move(new_normals);
    inputData.Vertices.UVs       = std::move(new_uvs);

    // 更新索引
    inputData.TriangleIndices = std::move(remapped_indices);

    // 更新三角形计数
    inputData.TriangleCounts[0] = simplified_index_count / 3;

    SaveMeshInputDataToObj(inputData, "after.obj");
    return lod_error;
}

// DAGReduce 函数定义
void DAGReduce(std::vector<ClusterGroup>& groups, std::vector<Cluster>& clusters, std::atomic<uint32_t>& numClusters, std::span<uint32_t> children, uint32_t maxParents, uint32_t groupIndex, uint32_t MeshIndex) {
    // 合并提供的clusters
    // 合并提供的clusters
    MeshInputData mergedInputData;

    // 创建顶点重映射表
    std::unordered_map<uint32_t, uint32_t> vertexRemap;
    uint32_t                               nextVertexIndex = 0;

    // 第遍：收集所有唯一顶点并建立重映射
    for (uint32_t childId : children) {
        const auto& cluster = clusters[childId];
        for (uint32_t i = 0; i < cluster.m_positions.size(); i++) {
            const auto& pos    = cluster.m_positions[i];
            const auto& normal = cluster.m_normals[i];
            const auto& uv     = cluster.m_uvs[i];

            // 使用位置作为唯一标识符
            uint64_t hash = HashPosition(pos);

            if (vertexRemap.find(hash) == vertexRemap.end()) {
                vertexRemap[hash] = nextVertexIndex++;
                mergedInputData.Vertices.Positions.push_back(pos);
                mergedInputData.Vertices.Normals.push_back(normal);
                mergedInputData.Vertices.UVs.push_back(uv);
            }
        }
    }

    // 第二遍：重建三角形索引
    for (uint32_t childId : children) {
        const auto& cluster = clusters[childId];
        for (uint32_t i = 0; i < cluster.m_indexes.size(); i += 3) {
            uint32_t idx0 = cluster.m_indexes[i];
            uint32_t idx1 = cluster.m_indexes[i + 1];
            uint32_t idx2 = cluster.m_indexes[i + 2];

            const auto& pos0 = cluster.m_positions[idx0];
            const auto& pos1 = cluster.m_positions[idx1];
            const auto& pos2 = cluster.m_positions[idx2];

            uint64_t hash0 = HashPosition(pos0);
            uint64_t hash1 = HashPosition(pos1);
            uint64_t hash2 = HashPosition(pos2);

            mergedInputData.TriangleIndices.push_back(vertexRemap[hash0]);
            mergedInputData.TriangleIndices.push_back(vertexRemap[hash1]);
            mergedInputData.TriangleIndices.push_back(vertexRemap[hash2]);
        }
    }

    mergedInputData.TriangleCounts.push_back(mergedInputData.TriangleIndices.size() / 3);

    // ... 后续代码保持不变 ...

    // 简化合并后的数据
    uint32_t targetNumTris = maxParents * ClusterSize;
    float    error         = simplifyMeshData1(mergedInputData, targetNumTris);

    // 使用clusterTriangles1重新划分
    std::vector<Cluster> newClusters;
    clusterTriangles1(newClusters, mergedInputData, 0, mergedInputData.TriangleCounts[0]);

    // 更新groups和clusters
    uint32_t newClusterStart = clusters.size();
    clusters.insert(clusters.end(), newClusters.begin(), newClusters.end());

    // 更新group信息
    ClusterGroup newGroup;
    newGroup.startIndex = newClusterStart;
    newGroup.count      = newClusters.size();
    groups[groupIndex]  = std::move(newGroup);
}

void BuildDAG(std::vector<ClusterGroup>& groups, std::vector<Cluster>& clusters, uint32_t ClusterStart, uint32_t clusterRangeNum, uint32_t MeshIndex, BBox MeshBounds) {
    bool                  bFirstLevel = true;
    std::atomic<uint32_t> numClusters = 0;
    uint32_t              levelOffset = ClusterStart;

    while (true) {
        numClusters = clusters.size();
        std::span<Cluster> levelClusters(&clusters[levelOffset], bFirstLevel ? clusterRangeNum : clusters.size() - levelOffset);
        bFirstLevel = false;

        if (levelClusters.size() < 2) {
            break;
        }

        if (levelClusters.size() <= MaxClusterGroupSize) {
            std::vector<uint32_t> children;
            uint32_t              numGroupElements = 0;
            for (uint32_t i = 0; i < levelClusters.size(); i++) {
                children.push_back(levelOffset + i);
                numGroupElements += levelClusters[i].m_indexes.size() / 3;
            }
            uint32_t maxParents = numGroupElements / (ClusterSize * 2);
            DAGReduce(groups, clusters, numClusters, children, maxParents, groups.size() - 1, MeshIndex);
        } else {
            GraphAdjancy adjancy = buildClusterGroupAdjancy1(levelClusters, levelOffset);

            uint32_t         targetGroupCount = (levelClusters.size() + MinClusterGroupSize - 1) / MinClusterGroupSize;
            GraphPartitioner partitioner(levelClusters.size(), targetGroupCount);

            auto graph = partitioner.NewGraph(levelClusters.size() * 6);
            if (!graph) {
                LOGE("Failed to create graph for cluster grouping");
                return;
            }

            for (uint32_t i = 0; i < levelClusters.size(); i++) {
                graph->AdjacencyOffset[i] = graph->Adjacency.size();
                for (const auto& adjClusterId : levelClusters[i].m_linked_cluster) {
                    if (adjClusterId >= levelOffset && adjClusterId < levelOffset + levelClusters.size()) {
                        float weight = 1.0f;
                        partitioner.addAdjacency(graph, adjClusterId - levelOffset, weight);
                    }
                }
            }
            graph->AdjacencyOffset[levelClusters.size()] = graph->Adjacency.size();

            partitioner.partition(*graph);
            delete graph;

            // 根据partition结果重组clusters
            std::unordered_map<int, std::vector<uint32_t>> partitionGroups;

            // 将clusters按照partition ID分组
            for (uint32_t i = 0; i < levelClusters.size(); i++) {
                int partitionId = partitioner.partitionIDs[i];
                partitionGroups[partitionId].push_back(levelOffset + i);
            }

            // 为每个partition创建新的group
            groups.resize(groups.size() + partitionGroups.size());
            uint32_t groupStartIndex = groups.size() - partitionGroups.size();

            // 处理每个partition组
            uint32_t groupIndex = 0;
            for (auto& [partitionId, children] : partitionGroups) {
                // 计算组内所有元素数量
                uint32_t numGroupElements = 0;
                for (uint32_t clusterId : children) {
                    numGroupElements += clusters[clusterId].m_indexes.size() / 3;
                }

                // 计算最大父节点数量
                uint32_t maxParents = numGroupElements / (ClusterSize * 2);

                // 为这个partition创建新的group
                DAGReduce(groups, clusters, numClusters, std::span<uint32_t>(children.data(), children.size()), maxParents, groupStartIndex + groupIndex, MeshIndex);

                groupIndex++;
            }
        }

        levelOffset = numClusters;
    }
}

uint jenkinsHash(uint a) {
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23cu) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09u) ^ (a >> 16);
    return a;
}

vec3 pseudocolor(uint value) {
    uint h = jenkinsHash(value);
    return glm::vec3(glm::uvec3(h, h >> 8, h >> 16) & 0xffu) / 255.f;
}

GraphPartitioner::FGraphData* GraphPartitioner::NewGraph(uint32_t maxEdges) const {
    auto graph = new FGraphData();

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

    graph->Adjacency.push_back(toVertex);
    graph->AdjacencyCost.push_back(weight);
}
void NaniteBuilder::Build(MeshInputData& InputMeshData, MeshOutputData* OutFallbackMeshData, const MeshNaniteSettings& Settings) {
    LOGI("NaniteBuilder::Build Vertex Count: {}", InputMeshData.Vertices.Positions.size());

    std::vector<Cluster>  clusters;
    std::vector<uint32_t> clusterPerMesh;
    {
        uint32_t baseTriangle = 0;
        for (uint32_t numTriangles : InputMeshData.TriangleCounts) {
            uint32_t clusterOffset = clusters.size();
            clusterTriangles1(clusters, InputMeshData, baseTriangle, numTriangles);
            clusterPerMesh.push_back(clusters.size() - clusterOffset);
            baseTriangle += numTriangles;
        }
    }

    std::vector<ClusterGroup> groups;
    BuildDAG(groups, clusters, 0, clusters.size(), 0, BBox());
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
        {0.957f, 0.263f, 0.212f, 1.0f},// Red
        {0.133f, 0.545f, 0.133f, 1.0f},// Green
        {0.231f, 0.455f, 0.969f, 1.0f},// Blue
        {0.945f, 0.769f, 0.059f, 1.0f},// Yellow
        {0.608f, 0.349f, 0.714f, 1.0f},// Purple
        {0.004f, 0.588f, 0.533f, 1.0f},// Teal
        {0.957f, 0.643f, 0.376f, 1.0f},// Orange
        {0.741f, 0.718f, 0.420f, 1.0f},// Olive
        {0.404f, 0.227f, 0.718f, 1.0f},// Indigo
        {0.914f, 0.118f, 0.388f, 1.0f},// Pink
        {0.475f, 0.333f, 0.282f, 1.0f},// Brown
        {0.612f, 0.153f, 0.690f, 1.0f},// Deep Purple
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
    float delta  = maxVal - minVal;

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
    clusterTriangles1(clusters, *meshInputData, 0, meshInputData->TriangleIndices.size() / 3);

    std::unordered_map<uint32_t, glm::vec3> idToColors;
    for (int i = 0; i < meshInputData->TriangleIndices.size(); i++) {
        auto index = meshInputData->TriangleIndices[i];
        auto part  = 0;
        if (idToColors.find(index) == idToColors.end()) {
            idToColors[index] = getClusterColor(part);
        }
    }

    std::vector<glm::vec3> colors(idToColors.size());
    for (auto& [id, color] : idToColors) {
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

    return SceneLoaderInterface::loadPrimitiveFromPritiveData(g_context->getDevice(), primData.get());
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
    LOGI("METIS Input - Vertices: {}, Parts: {}, Edges: {}",
         nvtxs,
         nparts,
         graph.Adjacency.size());

    // 查数据有效性
    if (nvtxs < nparts) {
        LOGE("Number of vertices ({}) less than number of parts ({})", nvtxs, nparts);
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
