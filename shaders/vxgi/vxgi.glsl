layout( set = 0, binding = 5) uniform VoxelizationDesc {
    uint clip_map_resoultion;
    uint clipmap_level;
    float voxel_size;
    float max_extent_world;
    vec3 clipmap_min_pos;
    vec3 clipmap_max_pos;
};


ivec3 computeImageCoords(vec3 world_pos){
    vec3 clip_pos = fract(world_pos / max_extent_world);
    ivec3 imageCoords = ivec3(clip_pos * clip_map_resoultion) & ivec3(clip_map_resoultion - 1);

    // Target the correct clipmap level
    imageCoords.y += int(clip_map_resoultion * clipmap_level);
    return imageCoords;
}

bool pos_in_clipmap(vec3 world_pos){
    return all(lessThan(world_pos, clipmap_max_pos)) && all(greaterThan(world_pos, clipmap_min_pos));
}