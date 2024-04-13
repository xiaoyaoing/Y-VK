#ifndef  PATH_COMMONS_H
#define PATH_COMMONS_H
#include "../commons.h"

struct PCPath
{
    vec3 sky_col;
    uint frame_num;
    uint light_num;
    uint max_depth;
    uint min_depth;
    uint enable_sample_bsdf;
    uint enable_sample_light;
};
#endif