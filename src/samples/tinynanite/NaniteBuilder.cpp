#include "NaniteBuilder.h"

#include <unordered_map>
#include <unordered_set>
#include <metis.h>
struct BuildCluster {
    
};

struct AdjVertex {
    std::unordered_set<uint32_t> adjVertices;
};

struct GraphData {
    std::unordered_map<uint32_t, uint32_t> vertexIndexMap;
    std::vector<AdjVertex> adjVertices;
    void addEdge(uint32_t v0, uint32_t v1) {
        if(vertexIndexMap.find(v0) == vertexIndexMap.end()) {
            vertexIndexMap[v0] = adjVertices.size();
            adjVertices.push_back(AdjVertex());
        }
        if(vertexIndexMap.find(v1) == vertexIndexMap.end()) {
            vertexIndexMap[v1] = adjVertices.size();
            adjVertices.push_back(AdjVertex());
        }
        adjVertices[vertexIndexMap[v0]].adjVertices.insert(v1);
        adjVertices[vertexIndexMap[v1]].adjVertices.insert(v0);
    }
};

static __forceinline  uint32_t MurmurFinalize32(uint32_t Hash)
{
    Hash ^= Hash >> 16;
    Hash *= 0x85ebca6b;
    Hash ^= Hash >> 13;
    Hash *= 0xc2b2ae35;
    Hash ^= Hash >> 16;
    return Hash;
}


static __forceinline  uint32_t Murmur32( std::initializer_list< uint32_t > InitList )
{
    uint32_t Hash = 0;
    for( auto Element : InitList )
    {
        Element *= 0xcc9e2d51;
        Element = ( Element << 15 ) | ( Element >> (32 - 15) );
        Element *= 0x1b873593;
    
        Hash ^= Element;
        Hash = ( Hash << 13 ) | ( Hash >> (32 - 13) );
        Hash = Hash * 5 + 0xe6546b64;
    }

    return MurmurFinalize32( Hash );
}

uint32_t HashPosition(const glm::vec3& Position) {
    union { float f; uint32_t i; } x;
    union { float f; uint32_t i; } y;
    union { float f; uint32_t i; } z;

    x.f = Position.x;
    y.f = Position.y;
    z.f = Position.z;

    return Murmur32( {
        Position.x == 0.0f ? 0u : x.i,
        Position.y == 0.0f ? 0u : y.i,
        Position.z == 0.0f ? 0u : z.i
    } );
}


void clusterTriangles(std::vector<Cluster>& clusters, MeshInputData& InputMeshData, uint32_t baseTriangle, uint32_t numTriangles) {
    GraphData graphData;
    for(uint32_t i = 0; i < numTriangles; i++) {
        glm::vec3 position0 = InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[baseTriangle + i * 3 + 0]];
        glm::vec3 position1 = InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[baseTriangle + i * 3 + 1]];
        glm::vec3 position2 = InputMeshData.Vertices.Positions[InputMeshData.TriangleIndices[baseTriangle + i * 3 + 2]];

        uint32_t hash0 = HashPosition(position0);
        uint32_t hash1 = HashPosition(position1);
        uint32_t hash2 = HashPosition(position2);

        graphData.addEdge(hash0, hash1);
        graphData.addEdge(hash1, hash2);
        graphData.addEdge(hash2, hash0);
    }

    std::vector<idx_t> adjOffsets;
    std::vector<idx_t> adjVertices;
    for(uint32_t i = 0; i < graphData.adjVertices.size(); i++) {
        adjOffsets.push_back(adjVertices.size());
        for(uint32_t adjVertex:graphData.adjVertices[i].adjVertices) {
            adjVertices.push_back(adjVertex);
        }
    }

    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);
    options[METIS_OPTION_UFACTOR] = 200;
    
    int numVertices = graphData.adjVertices.size();
    idx_t nWeights = 1;
    idx_t nParts = (numVertices + 127) / 128;
    std::vector<idx_t> partResult;
    partResult.resize(numVertices);
    METIS_PartGraphKway(&numVertices, &nWeights, adjOffsets.data(), adjVertices.data(), nullptr, nullptr, nullptr, &nParts, nullptr, nullptr, nullptr, nullptr, partResult.data());

    std::vector<Cluster> clusters;
    clusters.resize(nParts);
    for(uint32_t i = 0; i < numVertices; i++) {
        
    }
}

void NaniteBuilder::Build(MeshInputData& InputMeshData, MeshOutputData* OutFallbackMeshData, const MeshNaniteSettings& Settings) {
    std::vector<Cluster> clusters;
    std::vector<uint32_t> clusterPerMesh;
    {
        uint32_t baseTriangle = 0;
        for(uint32_t numTriangles:InputMeshData.TriangleCounts) {
            uint32_t clusterOffset = clusters.size();
            clusterTriangles(clusters, InputMeshData, baseTriangle, numTriangles,clusterOffset);
            clusterPerMesh.push_back(clusters.size() - clusterOffset);
            baseTriangle += numTriangles;
        }
    }
    
}
