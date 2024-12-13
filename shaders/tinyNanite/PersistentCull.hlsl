#include "tinyNanite.h"

groupshared uint group_node_batch_start_index;
groupshared uint group_clsuter_batch_start_index;
groupshared uint group_node_mask;
groupshared uint group_node_count;
groupshared uint group_cluster_batch_ready_size;
struct QueuePassState {
    uint ClusterBatchReadOffset;// Offset in batches
    uint ClusterWriteOffset;    // Offset in individual clusters
    uint NodeReadOffset;
    uint NodeWriteOffset;
    int  NodeCount;
};

static uint MAXNodes;

bool IsNodeDataReady(uint2 RawData) {
    return RawData.x != 0xFFFFFFFFu && RawData.y != 0xFFFFFFFFu;
}

uint GetMaxClusterBatches() {
    return 0;
}

void AddToClusterBatch(uint BatchIndex, uint num) {
}

void LoadClusterBatch(uint batch_index) {
}

StructuredBuffer<QueuePassState> queue_pass_state : register(t0);
RWByteAddressBuffer              node_task_queue : register(u0);
RWByteAddressBuffer              cluster_task_queue : register(u0);

void ProcessNode(uint InstanceIndex, uint NodeIndex,uint group_index) {
    // Do something with the node
    uint local_node_index = group_index >> 

}

void ProcessCluster(uint cluster_index, uint cluster_batch_start_index, uint group_index){
    // Do something with the cluster
}

    [numthreads(64, 1, 1)] void NodeAndClusterCull(uint GroupID : SV_GroupID, uint group_index : SV_GroupIndex) {
    bool process_nodes             = true;
    uint node_batch_start_index    = 0;
    uint cluster_batch_start_index = 0xFFFFFFFFu;
    uint node_batch_ready_offset   = NANITE_MAX_BVH_NODES_PER_GROUP;
    // uint cluster_batch_ready_offset = NANITE_MAX_CLUSTERS_PER_GROUP;
    while (true) {
        if (group_index == 0) {
        }

        if (process_nodes) {
            if (node_batch_ready_offset == NANITE_MAX_BVH_NODES_PER_GROUP) {
                if (group_index == 0) {
                    InterlockedAdd(queue_pass_state[0].NodeReadOffset, NANITE_MAX_BVH_NODES_PER_GROUP, group_node_batch_start_index);
                }
                GroupMemoryBarrierWithGroupSync();
                node_batch_ready_offset = 0;
                node_batch_start_index  = group_node_batch_start_index;

                if (node_batch_start_index >= MAXNodes) {
                    process_nodes = false;
                    continue;
                }
            }

            const uint node_index = node_batch_start_index + node_batch_ready_offset + group_index;
            bool       node_ready = node_batch_ready_offset + group_index < NANITE_MAX_BVH_NODES_PER_GROUP;
            uint2      node_task  = node_task_queue.Load2(node_index * 8);
            node_ready            = node_ready && IsNodeDataReady(node_task);

            if (node_ready) {
                InterlockedOr(group_node_mask, 1u << group_index);
            }

            AllMemoryBarrierWithGroupSync();

            if (group_node_mask & 1u) {
                int batch_node_size = firstbitlow(~group_node_mask);
                batch_node_size     = (node_batch_ready_offset == 0xFFFFFFFFu) ? 64 : batch_node_size;
                if (group_index < batch_node_size) {
                    ProcessNode(node_task.x, node_task.y);
                    node_task_queue.Store2(node_batch_start_index + group_index * 8, uint2(0, 0));
                }
                node_batch_ready_offset += batch_node_size;
                continue;
            }
        }

        if (cluster_batch_start_index == 0xFFFFFFFFu) {
            if (group_index == 0) {
                InterlockedAdd(queue_pass_state[0].ClusterBatchReadOffset, 1, group_clsuter_batch_start_index);
            }
            GroupMemoryBarrierWithGroupSync();
            cluster_batch_start_index = group_clsuter_batch_start_index;
        }

        GroupMemoryBarrierWithGroupSync();

        if (!process_nodes && group_clsuter_batch_start_index >= GetMaxClusterBatches())
            break;
    }

    if (group_index == 0) {
        group_node_count               = queue_pass_state[0].NodeCount;
        group_cluster_batch_ready_size = cluster_task_queue.Load(cluster_batch_start_index * 4);
    }

    GroupMemoryBarrierWithGroupSync();
    uint cluster_batch_ready_size = group_cluster_batch_ready_size;
    if ((process_nodes && cluster_batch_ready_size == NANITE_PERSISTENT_CLUSTER_CULLING_GROUP_SIZE) || (!process_nodes && group_node_count > 0)) {
        ProcessCluster(cluster_task_queue.Load(cluster_batch_start_index * 4), cluster_batch_start_index, group_index);
    }

    if (process_nodes && group_node_count == 0) {
        process_nodes = false;
    }
}