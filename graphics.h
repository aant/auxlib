#pragma once

#include "point2.h"
#include "size2.h"
#include "color.h"

namespace aux
{
	enum
	{
		ColorBlend_BadEnum = -1,

		ColorBlend_Off,
		ColorBlend_Add,
		ColorBlend_Multiply,
		ColorBlend_Alpha,

		ColorBlend_MaxEnums
	};

	enum
	{
		DepthTest_BadEnum = -1,

		DepthTest_Off,
		DepthTest_LessEqual,
		DepthTest_GreaterEqual,

		DepthTest_MaxEnums
	};

	enum
	{
		DrawMode_BadEnum = -1,

		DrawMode_Lines,
		DrawMode_Triangles,

		DrawMode_MaxEnums
	};

	enum
	{
		TextureFormat_BadEnum = -1,

		TextureFormat_R8,
		TextureFormat_R8G8,
		TextureFormat_R8G8B8,
		TextureFormat_R8G8B8A8,

		TextureFormat_MaxEnums
	};

	enum
	{
		TextureFilter_BadEnum = -1,

		TextureFilter_Point,
		TextureFilter_Bilinear,
		TextureFilter_Trilinear,
		TextureFilter_Anisotropic,

		TextureFilter_MaxEnums
	};

	enum
	{
		TextureWrap_BadEnum = -1,

		TextureWrap_Clamp,
		TextureWrap_Repeat,

		TextureWrap_MaxEnums
	};

	enum
	{
		CubemapFace_BadEnum = -1,

		CubemapFace_Left,
		CubemapFace_Right,
		CubemapFace_Top,
		CubemapFace_Bottom,
		CubemapFace_Front,
		CubemapFace_Back,

		CubemapFace_MaxEnums
	};

	enum
	{
		IndexFormat_BadEnum = -1,

		IndexFormat_Uint16,
		IndexFormat_Uint32,

		IndexFormat_MaxEnums
	};

	typedef struct TextureImpl* TextureHandle;
	typedef struct CubemapImpl* CubemapHandle;
	typedef struct SamplerImpl* SamplerHandle;
	typedef struct VertexFormatImpl* VertexFormatHandle;
	typedef struct VertexBufferImpl* VertexBufferHandle;
	typedef struct IndexBufferImpl* IndexBufferHandle;
	typedef struct ShaderBufferImpl* ShaderBufferHandle;
	typedef struct VertexShaderImpl* VertexShaderHandle;
	typedef struct PixelShaderImpl* PixelShaderHandle;

	struct GraphicsCaps
	{
		int32 maxTextureDimension;
		int32 maxTextureSlots;
		int32 maxCubemapDimension;
		int32 maxSamplerSlots;
	};

	const GraphicsCaps& Graphics_GetCaps();

	void Graphics_ClearFrame(const Color& color, float32 depth);
	void Graphics_ClearFrameColor(const Color& color);
	void Graphics_ClearFrameDepth(float32 depth);
	void Graphics_PresentFrame();
	void Graphics_ToggleFrameSync(bool on);

	void Graphics_SetColorBlend(enum32 blend);
	void Graphics_SetDepthTest(enum32 test);
	void Graphics_SetDrawMode(enum32 mode);
	void Graphics_SetViewport(const Point2& origin, const Size2& extents, float32 depthNear, float32 depthFar);

	void Graphics_DrawElements(int32 startVertex, int32 vertexCount);
	void Graphics_DrawIndexedElements(int32 startIndex, int32 indexCount);

	TextureHandle Graphics_CreateTexture(enum32 format, int32 width, int32 height, int32 levelCount = 1, const void* data = nullptr);
	TextureHandle Graphics_CreateTexture(enum32 format, const Size2& size, int32 levelCount = 1, const void* data = nullptr);
	void Graphics_DestroyTexture(TextureHandle texture);
	void Graphics_UpdateTexture(TextureHandle texture, int32 level, int32 x, int32 y, int32 width, int32 height, const void* data);
	void Graphics_UpdateTexture(TextureHandle texture, int32 level, const Point2& offset, const Size2& size, const void* data);

	CubemapHandle Graphics_CreateCubemap(enum32 format, int32 size, int32 levelCount = 1, const void* data = nullptr);
	void Graphics_DestroyCubemap(CubemapHandle cubemap);
	void Graphics_UpdateCubemap(CubemapHandle cubemap, int32 level, int32 x, int32 y, int32 width, int32 height, const void* data);

	SamplerHandle Graphics_CreateSampler(enum32 filter, enum32 wrapU, enum32 wrapV);
	void Graphics_DestroySampler(SamplerHandle sampler);

	VertexBufferHandle Graphics_CreateVertexBuffer(bool dynamic, int32 vertexSize, int32 vertexCount, const void* data = nullptr);
	void Graphics_DestroyVertexBuffer(VertexBufferHandle buffer);
	void Graphics_UpdateVertexBuffer(VertexBufferHandle buffer, int32 startVertex, int32 vertexCount, const void* data);
	void Graphics_SetVertexBuffer(VertexBufferHandle buffer);

	IndexBufferHandle Graphics_CreateIndexBuffer(bool dynamic, enum32 indexFormat, int32 indexCount, const void* data = nullptr);
	void Graphics_DestroyIndexBuffer(IndexBufferHandle buffer);
	void Graphics_UpdateIndexBuffer(IndexBufferHandle buffer, int32 startIndex, int32 indexCount, const void* data);
	void Graphics_SetIndexBuffer(IndexBufferHandle buffer);

	void Graphics_SetPixelShaderResource(TextureHandle texture, int32 slot);
	void Graphics_SetPixelShaderResource(SamplerHandle sampler, int32 slot);
	void Graphics_SetPixelShaderResource(ShaderBufferHandle buffer, int32 slot);

	void Graphics_SetVertexShaderResource(ShaderBufferHandle buffer, int32 slot);
}
