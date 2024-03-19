layout(std140, set = 3, binding = 5) uniform VoxelizationDesc {
    vec3 prev_clipmap_min_world_pos;
    int clipmap_level;

    vec3 prev_clipmap_max_world_pos;
    float voxel_size;

    vec3 clipmap_min_world_pos;
    int clip_map_resoultion;
    vec3 clipmap_max_world_pos;
    float max_extent_world;
};


ivec3 computeImageCoords(vec3 world_pos){
    vec3 clip_pos = fract(world_pos / max_extent_world);
    ivec3 imageCoords = ivec3(clip_pos * clip_map_resoultion) & ivec3(clip_map_resoultion - 1);

    // Target the correct clipmap level
    imageCoords.y += int(clip_map_resoultion * clipmap_level);
    return imageCoords;
}

bool pos_in_clipmap(vec3 world_pos){
    return all(lessThan(world_pos, clipmap_max_world_pos)) && all(greaterThan(world_pos, clipmap_min_world_pos));
}