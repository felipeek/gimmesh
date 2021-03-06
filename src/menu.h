#ifndef GIMMESH_MENU_H
#define GIMMESH_MENU_H
#include <GLFW/glfw3.h>
#include "common.h"

typedef void (*FilterCallback)(r32, r32, s32);
typedef void (*TextureChangeSolidCallback)();
typedef void (*TextureChangeCurvatureCallback)(r32, r32);
typedef void (*TextureChangeNormalsCallback)(r32);
typedef void (*TextureChangeCustomCallback)(char*);
typedef void (*NoiseGeneratorCallback)(r32);
typedef void (*ExportWavefrontCallback)();
typedef void (*ExportPointCloudCallback)();
typedef void (*ExportGimCallback)();

extern void menuRegisterNoiseGeneratorCallBack(NoiseGeneratorCallback f);
extern void menuRegisterFilterCallBack(FilterCallback f);
extern void menuRegisterTextureChangeSolidCallBack(TextureChangeSolidCallback f);
extern void menuRegisterTextureChangeCurvatureCallBack(TextureChangeCurvatureCallback f);
extern void menuRegisterTextureChangeNormalsCallBack(TextureChangeNormalsCallback f);
extern void menuRegisterTextureChangeCustomCallBack(TextureChangeCustomCallback f);
extern void menuRegisterExportWavefrontCallBack();
extern void menuRegisterExportPointCloudCallBack();
extern void menuRegisterExportGimCallBack(ExportGimCallback f);
extern void menuCharClickProcess(GLFWwindow* window, u32 c);
extern void menuKeyClickProcess(GLFWwindow* window, s32 key, s32 scanCode, s32 action, s32 mods);
extern void menuMouseClickProcess(GLFWwindow* window, s32 button, s32 action, s32 mods);
extern void menuScrollChangeProcess(GLFWwindow* window, s64 xoffset, s64 yoffset);
extern void menuInit(GLFWwindow* window);
extern void menuRender();
extern void menuDestroy();

#endif