struct _GlobalFrameUniform
{
    matrix view_proj;
    matrix inv_view_proj;
    matrix proj;
    matrix view;
    matrix inv_proj;
    matrix inv_view;

    float3 camera_pos;
    uint light_count;

    int2 resolution;
    int2 inv_resolution;

    float roughness_scale;
    float normal_scale;
    float roughness_override;
    int use_roughness_override;
};

