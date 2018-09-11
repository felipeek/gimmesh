#include <GLFW/glfw3.h>
#include "core.h"
#include "filter.h"
#include "domain_transform.h"
#include "menu.h"

#define PHONG_VERTEX_SHADER_PATH "./shaders/phong_shader.vs"
#define PHONG_FRAGMENT_SHADER_PATH "./shaders/phong_shader.fs"
#define GIM_ENTITY_COLOR (Vec4) {0.0f, 0.0f, 1.0f, 1.0f}

static GeometryImage originalGim, noisyGim, filteredGim;
static Entity gimEntity;
static Shader phongShader;
static PerspectiveCamera camera;
static Light light;

static void filterRecursiveCallback(r32 ss, s32 n)
{
	filteredGim = filterGeometryImageFilter(&noisyGim, n, ss, 0.0f, RECURSIVE_FILTER, 0);
	gimGeometryImageUpdate3D(&filteredGim);
	Mesh m = gimGeometryImageToMesh(&filteredGim, (Vec4) {0.0f, 0.0f, 1.0f, 1.0f});
	graphicsEntityMeshReplace(&gimEntity, m, false, false);
}

static void filterCurvatureCallback(r32 ss, r32 sr, s32 n, s32 blurMode, r32 blurSS, r32 blurSR)
{
	BlurInformation blurInformation = {0};
	if (blurMode == 0)
		blurInformation.useBlur = false;
	else
		blurInformation.useBlur = true;

	if (blurMode == 1)
		blurInformation.blurMode = RECURSIVE_FILTER;
	else if (blurMode == 2)
		blurInformation.blurMode = DISTANCE_FILTER;
	
	blurInformation.ss = blurSS;
	blurInformation.sr = blurSR;

	filteredGim = filterGeometryImageFilter(&noisyGim, n, ss, sr, CURVATURE_FILTER, &blurInformation);
	gimGeometryImageUpdate3D(&filteredGim);
	Mesh m = gimGeometryImageToMesh(&filteredGim, (Vec4) {0.0f, 0.0f, 1.0f, 1.0f});
	graphicsEntityMeshReplace(&gimEntity, m, false, false);
}

static void filterDistanceCallback(r32 ss, r32 sr, s32 n)
{
	filteredGim = filterGeometryImageFilter(&noisyGim, n, ss, sr, DISTANCE_FILTER, 0);
	gimGeometryImageUpdate3D(&filteredGim);
	Mesh m = gimGeometryImageToMesh(&filteredGim, (Vec4) {0.0f, 0.0f, 1.0f, 1.0f});
	graphicsEntityMeshReplace(&gimEntity, m, false, false);
}

static void textureChangeCallBack(s32 textureNumber)
{
	static u32 currentTexture = 0;
	static boolean hasTexture = false;

	if (hasTexture)
	{
		graphicsTextureDelete(currentTexture);
		hasTexture = false;
	}

	// Solid color
	if (textureNumber == 0)
	{
		graphicsMeshChangeColor(&gimEntity.mesh, GIM_ENTITY_COLOR, false);
	}
	// Curvature texture w/ blur
	else if (textureNumber == 1)
	{
		BlurInformation blurInformation;
		blurInformation.blurMode = RECURSIVE_FILTER;
		blurInformation.ss = 0.8f;
		blurInformation.sr = 1.0f;
		blurInformation.useBlur	= true;
		FloatImageData curvatureImage = dtGenerateCurvatureImage(&noisyGim, 100.0f, 1.0f, &blurInformation);
		FloatImageData normalizedCurvatureImage = gimNormalizeImageForVisualization(&curvatureImage);
		graphicsFloatImageSave("./res/curv_w_blur.bmp", &normalizedCurvatureImage);
		graphicsFloatImageFree(&curvatureImage);
		currentTexture = graphicsTextureCreateFromFloatData(&normalizedCurvatureImage);
		//gimCheckGeometryImage(&normalizedCurvatureImage);
		graphicsFloatImageFree(&normalizedCurvatureImage);
		graphicsMeshChangeDiffuseMap(&gimEntity.mesh, currentTexture, false);
		hasTexture = true;
	}
	// Curvature texture wo/ blur
	else if (textureNumber == 2)
	{
		BlurInformation blurInformation;
		blurInformation.useBlur	= false;
		FloatImageData curvatureImage = dtGenerateCurvatureImage(&noisyGim, 100.0f, 1.0f, &blurInformation);
		FloatImageData normalizedCurvatureImage = gimNormalizeImageForVisualization(&curvatureImage);
		graphicsFloatImageSave("./res/curv_wo_blur.bmp", &normalizedCurvatureImage);
		graphicsFloatImageFree(&curvatureImage);
		currentTexture = graphicsTextureCreateFromFloatData(&normalizedCurvatureImage);
		//gimCheckGeometryImage(&normalizedCurvatureImage);
		graphicsFloatImageFree(&normalizedCurvatureImage);
		graphicsMeshChangeDiffuseMap(&gimEntity.mesh, currentTexture, false);
		hasTexture = true;
	}
}

static void registerMenuCallbacks()
{
	menuRegisterRecursiveFilterCallBack(filterRecursiveCallback);
	menuRegisterCurvatureFilterCallBack(filterCurvatureCallback);
	menuRegisterDistanceFilterCallBack(filterDistanceCallback);
	menuRegisterTextureChangeCallBack(textureChangeCallBack);
}

static PerspectiveCamera createCamera()
{
	PerspectiveCamera camera;
	Vec4 cameraPosition = (Vec4) {0.0f, 0.0f, 2.0f, 1.0f};
	Vec4 cameraUp = (Vec4) {0.0f, 1.0f, 0.0f, 1.0f};
	Vec4 cameraView = (Vec4) {0.0f, 0.0f, -1.0f, 0.0f};
	r32 cameraNearPlane = -0.01f;
	r32 cameraFarPlane = -1000.0f;
	r32 cameraFov = 45.0f;
	cameraInit(&camera, cameraPosition, cameraUp, cameraView, cameraNearPlane, cameraFarPlane, cameraFov);
	return camera;
}

static Light createLight()
{
	Light light;
	Vec4 lightPosition = (Vec4) {1.0f, 0.0f, 0.0f, 1.0f};
	Vec4 ambientColor = (Vec4) {0.4f, 0.4f, 0.4f, 1.0f};
	Vec4 diffuseColor = (Vec4) {1.0f, 1.0f, 1.0f, 1.0f};
	Vec4 specularColor = (Vec4) {1.0f, 1.0f, 1.0f, 1.0f};
	graphicsLightCreate(&light, lightPosition, ambientColor, diffuseColor, specularColor);
	return light;
}

static void loadGeometryImage(const s8* gimPath)
{
	// Parse the original geometry image
	originalGim = gimParseGeometryImageFile(gimPath);
	// Update 3d information
	gimGeometryImageUpdate3D(&originalGim);
	// Copy original gim to noisy gim
	noisyGim = gimCopyGeometryImage(&originalGim);
	// Copy original gim to filtered gim
	filteredGim = gimCopyGeometryImage(&originalGim);

	// Transform filtered geometry image to a mesh
	Mesh m = gimGeometryImageToMesh(&filteredGim, GIM_ENTITY_COLOR);
	// Create filteredGim's entity
	graphicsEntityCreate(&gimEntity, m, (Vec4){0.0f, 0.0f, 0.0f, 1.0f}, (Vec3){0.0f, 0.0f, 0.0f}, (Vec3){1.0f, 1.0f, 1.0f});
}

extern void coreInit(const s8* gimPath)
{
	// Register menu callbacks
	registerMenuCallbacks();
	// Create shader
	phongShader = graphicsShaderCreate(PHONG_VERTEX_SHADER_PATH, PHONG_FRAGMENT_SHADER_PATH);
	// Create camera
	camera = createCamera();
	// Load geometry image	
	loadGeometryImage(gimPath);
	// Create light
	light = createLight();
}

extern void coreDestroy()
{
	gimFreeGeometryImage(&originalGim);
	gimFreeGeometryImage(&noisyGim);
	gimFreeGeometryImage(&filteredGim);
}

extern void coreUpdate(r32 deltaTime)
{

}

extern void coreRender()
{
	graphicsEntityRenderPhongShader(phongShader, &camera, &gimEntity, &light);
}

extern void coreInputProcess(boolean* keyState, r32 deltaTime)
{
	r32 movementSpeed = 3.0f;
	r32 rotationSpeed = 3.0f;

	if (keyState[GLFW_KEY_LEFT_SHIFT])
		movementSpeed = 0.5f;
	if (keyState[GLFW_KEY_RIGHT_SHIFT])
		movementSpeed = 0.1f;

	if (keyState[GLFW_KEY_W])
		cameraSetPosition(&camera, gmAddVec4(camera.position, gmScalarProductVec4(movementSpeed * deltaTime, gmNormalizeVec4(camera.view))));
	if (keyState[GLFW_KEY_S])
		cameraSetPosition(&camera, gmAddVec4(camera.position, gmScalarProductVec4(-movementSpeed * deltaTime, gmNormalizeVec4(camera.view))));
	if (keyState[GLFW_KEY_A])
		cameraSetPosition(&camera, gmAddVec4(camera.position, gmScalarProductVec4(-movementSpeed * deltaTime, gmNormalizeVec4(camera.xAxis))));
	if (keyState[GLFW_KEY_D])
		cameraSetPosition(&camera, gmAddVec4(camera.position, gmScalarProductVec4(movementSpeed * deltaTime, gmNormalizeVec4(camera.xAxis))));
	if (keyState[GLFW_KEY_X])
	{
		if (keyState[GLFW_KEY_LEFT_SHIFT] || keyState[GLFW_KEY_RIGHT_SHIFT])
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.x -= rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
		else
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.x += rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
	}
	if (keyState[GLFW_KEY_Y])
	{
		if (keyState[GLFW_KEY_LEFT_SHIFT] || keyState[GLFW_KEY_RIGHT_SHIFT])
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.y += rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
		else
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.y -= rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
	}
	if (keyState[GLFW_KEY_Z])
	{
		if (keyState[GLFW_KEY_LEFT_SHIFT] || keyState[GLFW_KEY_RIGHT_SHIFT])
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.z += rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
		else
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.z -= rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
	}
	if (keyState[GLFW_KEY_L])
	{
		static boolean wireframe = false;

		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		wireframe = !wireframe;
		keyState[GLFW_KEY_L] = false;
	}
}

extern void coreMouseChangeProcess(boolean reset, r64 xPos, r64 yPos)
{
	static r64 xPosOld, yPosOld;
	// This constant is basically the mouse sensibility.
	// @TODO: Allow mouse sensibility to be configurable.
	static const r32 cameraMouseSpeed = 0.001f;

	if (!reset)
	{
		r64 xDifference = xPos - xPosOld;
		r64 yDifference = yPos - yPosOld;

		r32 pitchAngle = -cameraMouseSpeed * (float)xDifference;
		r32 yawAngle = cameraMouseSpeed * (float)yDifference;

		cameraIncPitch(&camera, pitchAngle);
		cameraIncYaw(&camera, yawAngle);
	}

	xPosOld = xPos;
	yPosOld = yPos;
}

extern void coreMouseClickProcess(s32 button, s32 action, r64 xPos, r64 yPos)
{

}

extern void coreScrollChangeProcess(r64 xOffset, r64 yOffset)
{

}

extern void coreWindowResizeProcess(s32 width, s32 height)
{

}