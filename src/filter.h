#ifndef DT_MESH_SMOOTHING_FILTER_H
#define DT_MESH_SMOOTHING_FILTER_H
#include "gim.h"

typedef enum FilterMode FilterMode;
typedef struct BlurInformation BlurInformation;

enum FilterMode
{
	RECURSIVE_FILTER = 0,
	DISTANCE_FILTER = 1,
	CURVATURE_FILTER = 2,
	NOISE_GENERATOR = 3,
};

struct BlurInformation
{
	boolean useBlur;
	FilterMode blurMode;
	r32 ss;
	r32 sr;
};

extern GeometryImage filterGeometryImageFilter(
	const GeometryImage* originalGim,
	s32 numIterations,
	r32 spatialFactor,
	r32 rangeFactor,
	FilterMode filterMode,
	const BlurInformation* blurInformation);

#endif