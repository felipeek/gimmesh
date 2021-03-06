#ifndef GIMMESH_GIM_H
#define GIMMESH_GIM_H
#include "graphics.h"
#include "dynamic_array.h"

typedef struct GeometryImage GeometryImage;

struct GeometryImage
{
	FloatImageData img;
	Vertex* vertices; // Vertices have the same order as the img
	u32* indexes;
	Vec4* normals;
};

extern int gimParseGeometryImageFile(GeometryImage* gim, const u8* path);
extern void gimGeometryImageUpdate3D(GeometryImage* gim);
extern Mesh gimGeometryImageToMesh(const GeometryImage* gim, Vec4 color);
extern void gimExportToObjFile(const GeometryImage* gim, const s8* objPath);
extern FloatImageData gimNormalizeImageForVisualization(const FloatImageData* gimImage);
extern void gimNormalizeAndSave(const GeometryImage* gim, const s8* imagePath);
extern void gimFreeGeometryImage(GeometryImage* gim);
extern void gimCheckGeometryImage(const FloatImageData* gimImage);
extern GeometryImage gimCopyGeometryImage(const GeometryImage* gim, boolean copy3d);
extern GeometryImage gimAddNoise(const GeometryImage* gim, r32 noiseIntensity);
extern void gimExportToPointCloudFile(const GeometryImage* gim, const s8* asciiFilePath);
extern int gimExportToGimFile(const GeometryImage* gim, const s8* filePath);

#endif