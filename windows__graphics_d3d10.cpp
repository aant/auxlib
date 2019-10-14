#include "graphics.h"

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#define STRICT
#define INITGUID
#include <windows.h>
#include <d3d10.h>

#pragma warning(pop)

#pragma comment(lib, "d3d10.lib")

namespace aux
{
	#pragma pack(1)

	struct TextureImpl
	{
		ID3D10ShaderResourceView* view;
		int32 width;
		int32 height;
		int32 levelCount;
	};

	struct VertexBufferImpl
	{
		ID3D10Buffer* buffer;
		int32 vertexSize;
		int32 vertexCount;
		bool dynamic;
	};

	struct IndexBufferImpl
	{
		ID3D10Buffer* buffer;
		DXGI_FORMAT DXGIFormat;
		enum32 indexFormat;
		int32 indexSize;
		int32 indexCount;
		bool dynamic;
	};

	struct Graphics
	{
		HWND window;
		ID3D10Device* device;
		IDXGISwapChain* swapChain;
		ID3D10RenderTargetView* renderTarget;
		ID3D10DepthStencilView* depthStencil;
		ID3D10BlendState* blendStateOff;
		ID3D10BlendState* blendStateAdd;
		ID3D10BlendState* blendStateMultiply;
		ID3D10BlendState* blendStateAlpha;
		ID3D10DepthStencilState* depthStateOff;
		ID3D10DepthStencilState* depthStateLessEqual;
		ID3D10DepthStencilState* depthStateGreaterEqual;
		ID3D10RasterizerState* rasterizerState;
		ID3D10Buffer* currentVertexBuffer;
		ID3D10Buffer* currentIndexBuffer;
		UINT vsyncInterval;
		GraphicsCaps caps;
	};

	#pragma pack()

	static Graphics* graphics = nullptr;

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	template<typename T>
	static bool LL_LoadFunc(HMODULE lib, const char name[], T* func)
	{
		*func = (T)(void*)GetProcAddress(lib, name);
		return *func != nullptr;
	}

	__declspec(noreturn) static void LL_HandleCriticalError(const wchar_t message[], HRESULT result)
	{
		MessageBoxW(GetForegroundWindow(), message, L"CRITICAL ERROR", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR | MB_OK);
		ExitProcess((UINT)-1);
		(void)result;
	}

	static void LL_CheckForCriticalError(HRESULT result)
	{
		switch (result)
		{
			case E_OUTOFMEMORY:
				LL_HandleCriticalError(L"Out of video memory.", result);
				break;
			case DXGI_ERROR_REMOTE_OUTOFMEMORY:
				LL_HandleCriticalError(L"Out of video memory (Remote device).", result);
				break;
			case DXGI_ERROR_DEVICE_REMOVED:
				LL_HandleCriticalError(L"Video device lost.", result);
				break;
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				LL_HandleCriticalError(L"Video driver error.", result);
				break;
			case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
			case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
			case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
			case DXGI_ERROR_UNSUPPORTED:
				LL_HandleCriticalError(L"Video feature is not supported.", result);
				break;
			case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
				LL_HandleCriticalError(L"Video device lost (Remote device).", result);
				break;
		}
	}

	static void LL_SafeRelease(IUnknown* unk)
	{
		if (unk != nullptr)
		{
			unk->Release();
		}
	}

	static bool LL_CreateDevice()
	{
		UINT flags = D3D10_CREATE_DEVICE_SINGLETHREADED | D3D10_CREATE_DEVICE_ALLOW_NULL_FROM_MAP;

		#if defined(AUX_DEBUG_ON)
		flags |= D3D10_CREATE_DEVICE_DEBUG;
		#endif

		HRESULT result = D3D10CreateDevice(nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, flags, D3D10_SDK_VERSION, &(graphics->device));
		LL_CheckForCriticalError(result);

		if (SUCCEEDED(result))
		{
			return true;
		}

		result = D3D10CreateDevice(nullptr, D3D10_DRIVER_TYPE_REFERENCE, nullptr, flags, D3D10_SDK_VERSION, &(graphics->device));
		LL_CheckForCriticalError(result);

		if (SUCCEEDED(result))
		{
			return true;
		}

		return false;
	}

	static bool LL_CreateSwapChain(UINT frameWidth, UINT frameHeight)
	{
		HMODULE lib = LoadLibraryW(L"dxgi.dll");

		if (lib == nullptr)
		{
			return false;
		}

		HRESULT(WINAPI* createDXGIFactory)(REFIID, void**);

		if (!LL_LoadFunc(lib, "CreateDXGIFactory", &createDXGIFactory))
		{
			FreeLibrary(lib);
			return false;
		}

		IDXGIFactory* factory;
		HRESULT result = createDXGIFactory(IID_IDXGIFactory, (void**)&factory);
		FreeLibrary(lib);
		LL_CheckForCriticalError(result);

		if (FAILED(result))
		{
			return false;
		}

		DXGI_SWAP_CHAIN_DESC chainDesc = {};
		chainDesc.OutputWindow = graphics->window;
		chainDesc.Windowed = TRUE;
		chainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		chainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		chainDesc.BufferDesc.Width = frameWidth;
		chainDesc.BufferDesc.Height = frameHeight;
		chainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		chainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		chainDesc.BufferCount = 1;
		chainDesc.SampleDesc.Count = 1;
		chainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		result = factory->CreateSwapChain(graphics->device, &chainDesc, &(graphics->swapChain));
		factory->Release();
		LL_CheckForCriticalError(result);
		return SUCCEEDED(result);
	}

	static bool LL_CreateRenderTarget()
	{
		ID3D10Texture2D* frameBuffer;
		HRESULT result = graphics->swapChain->GetBuffer(0, IID_ID3D10Texture2D, (LPVOID*)&frameBuffer);
		LL_CheckForCriticalError(result);

		if (FAILED(result))
		{
			return false;
		}

		result = graphics->device->CreateRenderTargetView(frameBuffer, nullptr, &(graphics->renderTarget));
		frameBuffer->Release();
		LL_CheckForCriticalError(result);
		return SUCCEEDED(result);
	}

	static bool LL_CreateDepthStencil(UINT frameWidth, UINT frameHeight)
	{
		D3D10_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Usage = D3D10_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		textureDesc.Width = frameWidth;
		textureDesc.Height = frameHeight;
		textureDesc.ArraySize = 1;
		textureDesc.MipLevels = 1;
		textureDesc.SampleDesc.Count = 1;
		ID3D10Texture2D* texture;
		HRESULT result = graphics->device->CreateTexture2D(&textureDesc, nullptr, &texture);
		LL_CheckForCriticalError(result);

		if (FAILED(result))
		{
			return false;
		}

		D3D10_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = textureDesc.Format;
		viewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		result = graphics->device->CreateDepthStencilView(texture, &viewDesc, &(graphics->depthStencil));
		texture->Release();
		LL_CheckForCriticalError(result);
		return SUCCEEDED(result);
	}

	static bool LL_CreateBlendState(ID3D10BlendState** state, BOOL enabled, D3D10_BLEND blendSrc, D3D10_BLEND blendDst)
	{
		D3D10_BLEND_DESC desc = {};
		desc.BlendEnable[0] = enabled;
		desc.BlendOp = D3D10_BLEND_OP_ADD;
		desc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
		desc.SrcBlend = blendSrc;
		desc.SrcBlendAlpha = D3D10_BLEND_ONE;
		desc.DestBlend = blendDst;
		desc.DestBlendAlpha = D3D10_BLEND_ZERO;
		desc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
		HRESULT result = graphics->device->CreateBlendState(&desc, state);
		LL_CheckForCriticalError(result);
		return SUCCEEDED(result);
	}

	static bool LL_CreateBlendStates()
	{
		if (!LL_CreateBlendState(&(graphics->blendStateOff), FALSE, D3D10_BLEND_ONE, D3D10_BLEND_ZERO))
		{
			return false;
		}

		if (!LL_CreateBlendState(&(graphics->blendStateAdd), TRUE, D3D10_BLEND_ONE, D3D10_BLEND_ONE))
		{
			return false;
		}

		if (!LL_CreateBlendState(&(graphics->blendStateMultiply), TRUE, D3D10_BLEND_DEST_COLOR, D3D10_BLEND_ZERO))
		{
			return false;
		}

		if (!LL_CreateBlendState(&(graphics->blendStateAlpha), TRUE, D3D10_BLEND_SRC_ALPHA, D3D10_BLEND_INV_SRC_ALPHA))
		{
			return false;
		}

		return true;
	}

	static void LL_SetCurrentBlendState(ID3D10BlendState* state)
	{
		graphics->device->OMSetBlendState(state, nullptr, 0xffffffff);
	}

	static bool LL_CreateDepthState(ID3D10DepthStencilState** state, BOOL enabled, bool mask, D3D10_COMPARISON_FUNC func)
	{
		D3D10_DEPTH_STENCIL_DESC desc = {};
		desc.DepthFunc = func;
		desc.DepthEnable = enabled;

		if (mask)
		{
			desc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
		}
		else
		{
			desc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
		}

		HRESULT result = graphics->device->CreateDepthStencilState(&desc, state);
		LL_CheckForCriticalError(result);
		return SUCCEEDED(result);
	}

	static bool LL_CreateDepthStates()
	{
		if (!LL_CreateDepthState(&(graphics->depthStateOff), FALSE, true, D3D10_COMPARISON_ALWAYS))
		{
			return false;
		}

		if (!LL_CreateDepthState(&(graphics->depthStateLessEqual), TRUE, false, D3D10_COMPARISON_LESS_EQUAL))
		{
			return false;
		}

		if (!LL_CreateDepthState(&(graphics->depthStateGreaterEqual), TRUE, false, D3D10_COMPARISON_GREATER_EQUAL))
		{
			return false;
		}

		return true;
	}

	static void LL_SetCurrentDepthState(ID3D10DepthStencilState* state)
	{
		graphics->device->OMSetDepthStencilState(state, 0);
	}

	static bool LL_CreateRasterizerState()
	{
		D3D10_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D10_CULL_BACK;
		desc.FillMode = D3D10_FILL_SOLID;
		desc.FrontCounterClockwise = FALSE;
		desc.DepthClipEnable = TRUE;
		HRESULT result = graphics->device->CreateRasterizerState(&desc, &(graphics->rasterizerState));
		LL_CheckForCriticalError(result);
		return SUCCEEDED(result);
	}

	static bool LL_CreateBuffer(ID3D10Buffer** buffer, UINT usage, bool dynamic, UINT size, const void* data)
	{
		D3D10_BUFFER_DESC desc = {};
		desc.BindFlags = usage;
		desc.ByteWidth = size;

		if (dynamic)
		{
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
			desc.Usage = D3D10_USAGE_DYNAMIC;
		}
		else
		{
			desc.Usage = D3D10_USAGE_DEFAULT;
		}

		HRESULT result;

		if (data != nullptr)
		{
			D3D10_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pSysMem = data;
			result = graphics->device->CreateBuffer(&desc, &subresourceData, buffer);
		}
		else
		{
			result = graphics->device->CreateBuffer(&desc, nullptr, buffer);
		}

		LL_CheckForCriticalError(result);
		return SUCCEEDED(result);
	}

	static void LL_UpdateBuffer(ID3D10Buffer* buffer, bool dynamic, UINT offset, UINT size, const void* data)
	{
		if (dynamic)
		{
			void* dest;
			HRESULT result = buffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &dest);

			if (SUCCEEDED(result))
			{
				Memory_Copy(data, (uint8*)dest + offset, (size_t)size);
				buffer->Unmap();
			}
			else
			{
				LL_CheckForCriticalError(result);
			}
		}
		else
		{
			D3D10_BOX box = {};
			box.left = offset;
			box.right = offset + size;
			graphics->device->UpdateSubresource(buffer, 0, &box, data, 0, 0);
		}
	}

	static void LL_SetCurrentVertexBuffer(ID3D10Buffer* buffer, UINT vertexSize)
	{
		if (graphics->currentVertexBuffer != buffer)
		{
			graphics->currentVertexBuffer = buffer;
			UINT offset = 0;
			graphics->device->IASetVertexBuffers(0, 1, &buffer, &vertexSize, &offset);
		}
	}

	static void LL_SetCurrentIndexBuffer(ID3D10Buffer* buffer, DXGI_FORMAT format)
	{
		if (graphics->currentIndexBuffer != buffer)
		{
			graphics->currentIndexBuffer = buffer;
			graphics->device->IASetIndexBuffer(buffer, format, 0);
		}
	}

	static int32 LL_GetIndexSize(enum32 indexFormat)
	{
		switch (indexFormat)
		{
			case IndexFormat_Uint16:
				return 2;
			case IndexFormat_Uint32:
				return 4;
			default:
				return 0;
		}
	}

	static DXGI_FORMAT LL_GetIndexFormat(enum32 indexFormat)
	{
		switch (indexFormat)
		{
			case IndexFormat_Uint16:
				return DXGI_FORMAT_R16_UINT;
			case IndexFormat_Uint32:
				return DXGI_FORMAT_R32_UINT;
			default:
				return DXGI_FORMAT_UNKNOWN;
		}
	}

	///////////////////////////////////////////////////////////
	//
	//	Internal functions
	//
	///////////////////////////////////////////////////////////

	bool Internal__InitGraphics(HWND window)
	{
		graphics = (Graphics*)Memory_AllocateAndZero(sizeof(Graphics));
		graphics->window = window;
		RECT client;

		if (!GetClientRect(window, &client))
		{
			return false;
		}

		UINT frameWidth = (UINT)(client.right - client.left);
		UINT frameHeight = (UINT)(client.bottom - client.top);

		if (!LL_CreateDevice())
		{
			return false;
		}

		if (!LL_CreateSwapChain(frameWidth, frameHeight))
		{
			return false;
		}

		if (!LL_CreateRenderTarget())
		{
			return false;
		}

		if (!LL_CreateDepthStencil(frameWidth, frameHeight))
		{
			return false;
		}

		if (!LL_CreateBlendStates())
		{
			return false;
		}

		if (!LL_CreateDepthStates())
		{
			return false;
		}

		if (!LL_CreateRasterizerState())
		{
			return false;
		}

		graphics->device->OMSetRenderTargets(1, &(graphics->renderTarget), graphics->depthStencil);
		LL_SetCurrentBlendState(graphics->blendStateOff);
		LL_SetCurrentDepthState(graphics->depthStateOff);
		graphics->device->RSSetState(graphics->rasterizerState);
		graphics->device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics->vsyncInterval = 1;
		graphics->caps.maxTextureDimension = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		graphics->caps.maxCubemapDimension = D3D10_REQ_TEXTURECUBE_DIMENSION;
		graphics->caps.maxTextureSlots = 0;
		graphics->caps.maxSamplerSlots = D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT;
		return true;
	}

	void Internal__FreeGraphics()
	{
		if (graphics != nullptr)
		{
			if (graphics->device != nullptr)
			{
				graphics->device->ClearState();
				LL_SafeRelease(graphics->rasterizerState);
				LL_SafeRelease(graphics->depthStateGreaterEqual);
				LL_SafeRelease(graphics->depthStateLessEqual);
				LL_SafeRelease(graphics->depthStateOff);
				LL_SafeRelease(graphics->blendStateAlpha);
				LL_SafeRelease(graphics->blendStateMultiply);
				LL_SafeRelease(graphics->blendStateAdd);
				LL_SafeRelease(graphics->blendStateOff);
				LL_SafeRelease(graphics->depthStencil);
				LL_SafeRelease(graphics->renderTarget);
				LL_SafeRelease(graphics->swapChain);
				graphics->device->Release();
			}

			Memory_Free(graphics);
			graphics = nullptr;
		}
	}

	///////////////////////////////////////////////////////////
	//
	//	Graphics functions
	//
	///////////////////////////////////////////////////////////

	const GraphicsCaps& Graphics_GetCaps()
	{
		return graphics->caps;
	}

	void Graphics_ClearFrame(const Color& color, float32 depth)
	{
		graphics->device->ClearRenderTargetView(graphics->renderTarget, (const FLOAT*)&color);
		graphics->device->ClearDepthStencilView(graphics->depthStencil, D3D10_CLEAR_DEPTH, depth, 0);
	}

	void Graphics_ClearFrameColor(const Color& color)
	{
		graphics->device->ClearRenderTargetView(graphics->renderTarget, (const FLOAT*)&color);
	}

	void Graphics_ClearFrameDepth(float32 depth)
	{
		graphics->device->ClearDepthStencilView(graphics->depthStencil, D3D10_CLEAR_DEPTH, depth, 0);
	}

	void Graphics_PresentFrame()
	{
		graphics->swapChain->Present(graphics->vsyncInterval, 0);
	}

	void Graphics_ToggleFrameSync(bool on)
	{
		graphics->vsyncInterval = (UINT)(on ? 1 : 0);
	}

	void Graphics_SetColorBlend(enum32 blend)
	{
		switch (blend)
		{
			case ColorBlend_Off:
				LL_SetCurrentBlendState(graphics->blendStateOff);
				break;
			case ColorBlend_Add:
				LL_SetCurrentBlendState(graphics->blendStateAdd);
				break;
			case ColorBlend_Multiply:
				LL_SetCurrentBlendState(graphics->blendStateMultiply);
				break;
			case ColorBlend_Alpha:
				LL_SetCurrentBlendState(graphics->blendStateAlpha);
				break;
		}
	}

	void Graphics_SetDepthTest(enum32 test)
	{
		switch (test)
		{
			case DepthTest_Off:
				LL_SetCurrentDepthState(graphics->depthStateOff);
				break;
			case DepthTest_LessEqual:
				LL_SetCurrentDepthState(graphics->depthStateLessEqual);
				break;
			case DepthTest_GreaterEqual:
				LL_SetCurrentDepthState(graphics->depthStateGreaterEqual);
				break;
		}
	}

	void Graphics_SetDrawMode(enum32 mode)
	{
		switch (mode)
		{
			case DrawMode_Lines:
				graphics->device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
				break;
			case DrawMode_Triangles:
				graphics->device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				break;
		}
	}

	void Graphics_SetViewport(int32 x, int32 y, int32 width, int32 height, float32 depthNear, float32 depthFar)
	{
		AUX_DEBUG_ASSERT(x >= 0);
		AUX_DEBUG_ASSERT(y >= 0);
		AUX_DEBUG_ASSERT(width > 0);
		AUX_DEBUG_ASSERT(height > 0);

		D3D10_VIEWPORT viewport;
		viewport.TopLeftX = (INT)x;
		viewport.TopLeftY = (INT)y;
		viewport.Width = (UINT)width;
		viewport.Height = (UINT)height;
		viewport.MinDepth = depthNear;
		viewport.MaxDepth = depthFar;
		graphics->device->RSSetViewports(1, &viewport);
	}

	void Graphics_DrawElements(int32 start_vertex, int32 vertexCount)
	{
		AUX_DEBUG_ASSERT(start_vertex >= 0);
		AUX_DEBUG_ASSERT(vertexCount > 1);

		graphics->device->Draw((UINT)vertexCount, (UINT)start_vertex);
	}

	void Graphics_DrawIndexedElements(int32 start_index, int32 indexCount)
	{
		AUX_DEBUG_ASSERT(start_index >= 0);
		AUX_DEBUG_ASSERT(indexCount > 1);

		graphics->device->DrawIndexed((UINT)indexCount, (UINT)start_index, 0);
	}

	TextureHandle Graphics_CreateTexture(enum32 format, int32 width, int32 height, int32 levelCount, const void* data)
	{
		AUX_DEBUG_ASSERT(width > 0);
		AUX_DEBUG_ASSERT(height > 0);
		AUX_DEBUG_ASSERT(levelCount > 0);

		if (width > graphics->caps.maxTextureDimension)
		{
			return nullptr;
		}

		if (height > graphics->caps.maxTextureDimension)
		{
			return nullptr;
		}

		/*ID3D10ShaderResourceView* view = create_texture_view(format, width, height, multilevel, data);

		if (view != nullptr)
		{
			TextureImpl* texture = (TextureImpl*)Memory_Allocate(sizeof(TextureImpl));
			texture->view = view;
			texture->width = width;
			texture->height = height;
			texture->levelCount = 0;
		}*/

		return nullptr;
	}

	void Graphics_DestroyTexture(TextureHandle texture)
	{
		texture->view->Release();
		Memory_Free(texture);
	}

	void Graphics_UpdateTexture(TextureHandle texture, int32 level, int32 x, int32 y, int32 width, int32 height, const void* data)
	{
	}

	VertexBufferHandle Graphics_CreateVertexBuffer(bool dynamic, int32 vertexSize, int32 vertexCount, const void* data)
	{
		AUX_DEBUG_ASSERT(vertexSize > 0);
		AUX_DEBUG_ASSERT(vertexCount > 1);

		UINT size = (UINT)vertexSize * (UINT)vertexCount;
		ID3D10Buffer* resource;

		if (LL_CreateBuffer(&resource, D3D10_BIND_VERTEX_BUFFER, dynamic, size, data))
		{
			VertexBufferHandle buffer = (VertexBufferHandle)Memory_Allocate(sizeof(VertexBufferImpl));
			buffer->buffer = resource;
			buffer->vertexSize = vertexSize;
			buffer->vertexCount = vertexCount;
			buffer->dynamic = dynamic;
			return buffer;
		}

		return nullptr;
	}

	void Graphics_DestroyVertexBuffer(VertexBufferHandle buffer)
	{
		if (buffer->buffer == graphics->currentVertexBuffer)
		{
			LL_SetCurrentVertexBuffer(nullptr, 0);
		}

		buffer->buffer->Release();
		Memory_Free(buffer);
	}

	void Graphics_UpdateVertexBuffer(VertexBufferHandle buffer, int32 startVertex, int32 vertexCount, const void* data)
	{
		AUX_DEBUG_ASSERT(startVertex >= 0);
		AUX_DEBUG_ASSERT(vertexCount > 0);
		AUX_DEBUG_ASSERT((startVertex + vertexCount) <= buffer->vertexCount);
		AUX_DEBUG_ASSERT(data != nullptr);

		UINT offset = (UINT)startVertex * (UINT)buffer->vertexSize;
		UINT size = (UINT)vertexCount * (UINT)buffer->vertexSize;
		LL_UpdateBuffer(buffer->buffer, buffer->dynamic, offset, size, data);
	}

	void Graphics_SetVertexBuffer(VertexBufferHandle buffer)
	{
		LL_SetCurrentVertexBuffer(buffer->buffer, (UINT)buffer->vertexSize);
	}

	IndexBufferHandle Graphics_CreateIndexBuffer(bool dynamic, enum32 indexFormat, int32 indexCount, const void* data)
	{
		int32 indexSize = LL_GetIndexSize(indexFormat);

		AUX_DEBUG_ASSERT(indexSize > 0);
		AUX_DEBUG_ASSERT(indexCount > 1);

		UINT size = (UINT)indexSize * (UINT)indexCount;
		ID3D10Buffer* resource;

		if (LL_CreateBuffer(&resource, D3D10_BIND_INDEX_BUFFER, dynamic, size, data))
		{
			IndexBufferHandle buffer = (IndexBufferHandle)Memory_Allocate(sizeof(IndexBufferImpl));
			buffer->buffer = resource;
			buffer->DXGIFormat = LL_GetIndexFormat(indexFormat);
			buffer->indexFormat = indexFormat;
			buffer->indexSize = indexSize;
			buffer->indexCount = indexCount;
			buffer->dynamic = dynamic;
			return buffer;
		}

		return nullptr;
	}

	void Graphics_DestroyIndexBuffer(IndexBufferHandle buffer)
	{
		if (buffer->buffer == graphics->currentIndexBuffer)
		{
			LL_SetCurrentIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN);
		}

		buffer->buffer->Release();
		Memory_Free(buffer);
	}

	void Graphics_UpdateIndexBuffer(IndexBufferHandle buffer, int32 startIndex, int32 indexCount, const void* data)
	{
		AUX_DEBUG_ASSERT(startIndex >= 0);
		AUX_DEBUG_ASSERT(indexCount > 0);
		AUX_DEBUG_ASSERT((startIndex + indexCount) <= buffer->indexCount);
		AUX_DEBUG_ASSERT(data != nullptr);

		UINT offset = (UINT)startIndex * (UINT)buffer->indexSize;
		UINT size = (UINT)indexCount * (UINT)buffer->indexSize;
		LL_UpdateBuffer(buffer->buffer, buffer->dynamic, offset, size, data);
	}

	void Graphics_SetIndexBuffer(IndexBufferHandle buffer)
	{
		LL_SetCurrentIndexBuffer(buffer->buffer, buffer->DXGIFormat);
	}
}
