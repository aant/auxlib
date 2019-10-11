#include "renderer.h"

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

	struct texture_t
	{
		ID3D10ShaderResourceView* view;
		i32_t width;
		i32_t height;
		i32_t level_count;
	};

	struct vertex_buffer_t
	{
		ID3D10Buffer* buffer;
		i32_t vertex_size;
		i32_t vertex_count;
		bool dynamic;
	};

	struct index_buffer_t
	{
		ID3D10Buffer* buffer;
		DXGI_FORMAT dxgi_format;
		e32_t index_format;
		i32_t index_size;
		i32_t index_count;
		bool dynamic;
	};

	struct renderer_t
	{
		HWND window;
		ID3D10Device* device;
		IDXGISwapChain* swap_chain;
		ID3D10RenderTargetView* render_target;
		ID3D10DepthStencilView* depth_stencil;
		ID3D10BlendState* blend_state_off;
		ID3D10BlendState* blend_state_add;
		ID3D10BlendState* blend_state_multiply;
		ID3D10BlendState* blend_state_alpha;
		ID3D10DepthStencilState* depth_state_off;
		ID3D10DepthStencilState* depth_state_lequal;
		ID3D10DepthStencilState* depth_state_gequal;
		ID3D10RasterizerState* rasterizer;
		ID3D10Buffer* current_vertex_buffer;
		ID3D10Buffer* current_index_buffer;
		UINT vsync_interval;
		renderer_caps_t caps;
	};

	#pragma pack()

	static renderer_t* renderer = nullptr;

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	template<typename T>
	static bool load_func(HMODULE lib, const char name[], T* func)
	{
		*func = (T)(void*)GetProcAddress(lib, name);
		return *func != nullptr;
	}

	__declspec(noreturn) static void handle_critical_error(const wchar_t message[], HRESULT result)
	{
		MessageBoxW(GetForegroundWindow(), message, L"CRITICAL ERROR", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR | MB_OK);
		ExitProcess((UINT)-1);
		(void)result;
	}

	static void check_for_critical_error(HRESULT result)
	{
		switch (result)
		{
			case E_OUTOFMEMORY:
				handle_critical_error(L"Out of video memory.", result);
				break;
			case DXGI_ERROR_REMOTE_OUTOFMEMORY:
				handle_critical_error(L"Out of video memory (Remote device).", result);
				break;
			case DXGI_ERROR_DEVICE_REMOVED:
				handle_critical_error(L"Video device lost.", result);
				break;
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				handle_critical_error(L"Video driver error.", result);
				break;
			case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
			case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
			case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
			case DXGI_ERROR_UNSUPPORTED:
				handle_critical_error(L"Video feature is not supported.", result);
				break;
			case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
				handle_critical_error(L"Video device lost (Remote device).", result);
				break;
		}
	}

	static void safe_release(IUnknown* unk)
	{
		if (unk != nullptr)
		{
			unk->Release();
		}
	}

	static bool create_device()
	{
		UINT flags = D3D10_CREATE_DEVICE_SINGLETHREADED | D3D10_CREATE_DEVICE_ALLOW_NULL_FROM_MAP;

		#if defined(AUX_DEBUG_ON)
		flags |= D3D10_CREATE_DEVICE_DEBUG;
		#endif

		HRESULT result = D3D10CreateDevice(nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, flags, D3D10_SDK_VERSION, &(renderer->device));
		check_for_critical_error(result);

		if (SUCCEEDED(result))
		{
			return true;
		}

		result = D3D10CreateDevice(nullptr, D3D10_DRIVER_TYPE_REFERENCE, nullptr, flags, D3D10_SDK_VERSION, &(renderer->device));
		check_for_critical_error(result);

		if (SUCCEEDED(result))
		{
			return true;
		}

		return false;
	}

	static bool create_swap_chain(UINT frame_width, UINT frame_height)
	{
		HMODULE lib = LoadLibraryW(L"dxgi.dll");

		if (lib == nullptr)
		{
			return false;
		}

		HRESULT(WINAPI* create_dxgi_factory)(REFIID, void**);

		if (!load_func(lib, "CreateDXGIFactory", &create_dxgi_factory))
		{
			FreeLibrary(lib);
			return false;
		}

		IDXGIFactory* factory;
		HRESULT result = create_dxgi_factory(IID_IDXGIFactory, (void**)&factory);
		FreeLibrary(lib);
		check_for_critical_error(result);

		if (FAILED(result))
		{
			return false;
		}

		DXGI_SWAP_CHAIN_DESC chain_desc = {};
		chain_desc.OutputWindow = renderer->window;
		chain_desc.Windowed = TRUE;
		chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		chain_desc.BufferDesc.Width = frame_width;
		chain_desc.BufferDesc.Height = frame_height;
		chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		chain_desc.BufferCount = 1;
		chain_desc.SampleDesc.Count = 1;
		chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		result = factory->CreateSwapChain(renderer->device, &chain_desc, &(renderer->swap_chain));
		factory->Release();
		check_for_critical_error(result);
		return SUCCEEDED(result);
	}

	static bool create_render_target()
	{
		ID3D10Texture2D* frame_buffer;
		HRESULT result = renderer->swap_chain->GetBuffer(0, IID_ID3D10Texture2D, (LPVOID*)&frame_buffer);
		check_for_critical_error(result);

		if (FAILED(result))
		{
			return false;
		}

		result = renderer->device->CreateRenderTargetView(frame_buffer, nullptr, &(renderer->render_target));
		frame_buffer->Release();
		check_for_critical_error(result);
		return SUCCEEDED(result);
	}

	static bool create_depth_stencil(UINT frame_width, UINT frame_height)
	{
		D3D10_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Usage = D3D10_USAGE_DEFAULT;
		texture_desc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		texture_desc.Width = frame_width;
		texture_desc.Height = frame_height;
		texture_desc.ArraySize = 1;
		texture_desc.MipLevels = 1;
		texture_desc.SampleDesc.Count = 1;
		ID3D10Texture2D* texture;
		HRESULT result = renderer->device->CreateTexture2D(&texture_desc, nullptr, &texture);
		check_for_critical_error(result);

		if (FAILED(result))
		{
			return false;
		}

		D3D10_DEPTH_STENCIL_VIEW_DESC view_desc = {};
		view_desc.Format = texture_desc.Format;
		view_desc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		result = renderer->device->CreateDepthStencilView(texture, &view_desc, &(renderer->depth_stencil));
		texture->Release();
		check_for_critical_error(result);
		return SUCCEEDED(result);
	}

	static bool create_blend_state(ID3D10BlendState** state, BOOL enabled, D3D10_BLEND blend_src, D3D10_BLEND blend_dst)
	{
		D3D10_BLEND_DESC desc = {};
		desc.BlendEnable[0] = enabled;
		desc.BlendOp = D3D10_BLEND_OP_ADD;
		desc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
		desc.SrcBlend = blend_src;
		desc.SrcBlendAlpha = D3D10_BLEND_ONE;
		desc.DestBlend = blend_dst;
		desc.DestBlendAlpha = D3D10_BLEND_ZERO;
		desc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
		HRESULT result = renderer->device->CreateBlendState(&desc, state);
		check_for_critical_error(result);
		return SUCCEEDED(result);
	}

	static bool create_blend_states()
	{
		if (!create_blend_state(&(renderer->blend_state_off), FALSE, D3D10_BLEND_ONE, D3D10_BLEND_ZERO))
		{
			return false;
		}

		if (!create_blend_state(&(renderer->blend_state_add), TRUE, D3D10_BLEND_ONE, D3D10_BLEND_ONE))
		{
			return false;
		}

		if (!create_blend_state(&(renderer->blend_state_multiply), TRUE, D3D10_BLEND_DEST_COLOR, D3D10_BLEND_ZERO))
		{
			return false;
		}

		if (!create_blend_state(&(renderer->blend_state_alpha), TRUE, D3D10_BLEND_SRC_ALPHA, D3D10_BLEND_INV_SRC_ALPHA))
		{
			return false;
		}

		return true;
	}

	static void set_blend_state(ID3D10BlendState* state)
	{
		renderer->device->OMSetBlendState(state, nullptr, 0xffffffff);
	}

	static bool create_depth_state(ID3D10DepthStencilState** state, BOOL enabled, bool mask, D3D10_COMPARISON_FUNC func)
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

		HRESULT result = renderer->device->CreateDepthStencilState(&desc, state);
		check_for_critical_error(result);
		return SUCCEEDED(result);
	}

	static bool create_depth_states()
	{
		if (!create_depth_state(&(renderer->depth_state_off), FALSE, true, D3D10_COMPARISON_ALWAYS))
		{
			return false;
		}

		if (!create_depth_state(&(renderer->depth_state_lequal), TRUE, false, D3D10_COMPARISON_LESS_EQUAL))
		{
			return false;
		}

		if (!create_depth_state(&(renderer->depth_state_gequal), TRUE, false, D3D10_COMPARISON_GREATER_EQUAL))
		{
			return false;
		}

		return true;
	}

	static void set_depth_state(ID3D10DepthStencilState* state)
	{
		renderer->device->OMSetDepthStencilState(state, 0);
	}

	static bool create_rasterizer()
	{
		D3D10_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D10_CULL_BACK;
		desc.FillMode = D3D10_FILL_SOLID;
		desc.FrontCounterClockwise = FALSE;
		desc.DepthClipEnable = TRUE;
		HRESULT result = renderer->device->CreateRasterizerState(&desc, &(renderer->rasterizer));
		check_for_critical_error(result);
		return SUCCEEDED(result);
	}

	static bool create_buffer(ID3D10Buffer** buffer, UINT usage, bool dynamic, UINT size, const void* data)
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
			D3D10_SUBRESOURCE_DATA sr_data = {};
			sr_data.pSysMem = data;
			result = renderer->device->CreateBuffer(&desc, &sr_data, buffer);
		}
		else
		{
			result = renderer->device->CreateBuffer(&desc, nullptr, buffer);
		}

		check_for_critical_error(result);
		return SUCCEEDED(result);
	}

	static void update_buffer(ID3D10Buffer* buffer, bool dynamic, UINT offset, UINT size, const void* data)
	{
		if (dynamic)
		{
			void* dest;
			HRESULT result = buffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &dest);

			if (SUCCEEDED(result))
			{
				copy_mem(data, (u8_t*)dest + offset, (size_t)size);
				buffer->Unmap();
			}
			else
			{
				check_for_critical_error(result);
			}
		}
		else
		{
			D3D10_BOX box = {};
			box.left = offset;
			box.right = offset + size;
			renderer->device->UpdateSubresource(buffer, 0, &box, data, 0, 0);
		}
	}

	static void set_current_vertex_buffer(ID3D10Buffer* buffer, UINT vertex_size)
	{
		if (renderer->current_vertex_buffer != buffer)
		{
			renderer->current_vertex_buffer = buffer;
			UINT offset = 0;
			renderer->device->IASetVertexBuffers(0, 1, &buffer, &vertex_size, &offset);
		}
	}

	static void set_current_index_buffer(ID3D10Buffer* buffer, DXGI_FORMAT format)
	{
		if (renderer->current_index_buffer != buffer)
		{
			renderer->current_index_buffer = buffer;
			renderer->device->IASetIndexBuffer(buffer, format, 0);
		}
	}

	static i32_t get_index_size(e32_t index_format)
	{
		switch (index_format)
		{
			case INDEX_FORMAT_U16:
				return 2;
			case INDEX_FORMAT_U32:
				return 4;
			default:
				return 0;
		}
	}

	static DXGI_FORMAT get_index_format(e32_t index_format)
	{
		switch (index_format)
		{
			case INDEX_FORMAT_U16:
				return DXGI_FORMAT_R16_UINT;
			case INDEX_FORMAT_U32:
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

	bool internal__init_graphics(HWND window)
	{
		renderer = (renderer_t*)zalloc_mem(sizeof(renderer_t));
		renderer->window = window;
		RECT client;

		if (!GetClientRect(window, &client))
		{
			return false;
		}

		UINT frame_width = (UINT)(client.right - client.left);
		UINT frame_height = (UINT)(client.bottom - client.top);

		if (!create_device())
		{
			return false;
		}

		if (!create_swap_chain(frame_width, frame_height))
		{
			return false;
		}

		if (!create_render_target())
		{
			return false;
		}

		if (!create_depth_stencil(frame_width, frame_height))
		{
			return false;
		}

		if (!create_blend_states())
		{
			return false;
		}

		if (!create_depth_states())
		{
			return false;
		}

		if (!create_rasterizer())
		{
			return false;
		}

		renderer->device->OMSetRenderTargets(1, &(renderer->render_target), renderer->depth_stencil);
		set_blend_state(renderer->blend_state_off);
		set_depth_state(renderer->depth_state_off);
		renderer->device->RSSetState(renderer->rasterizer);
		renderer->device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		renderer->vsync_interval = 1;
		renderer->caps.max_texture_dim = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		renderer->caps.max_cubemap_dim = D3D10_REQ_TEXTURECUBE_DIMENSION;
		renderer->caps.max_texture_slots = 0;
		renderer->caps.max_sampler_slots = D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT;
		return true;
	}

	void internal__free_graphics()
	{
		if (renderer != nullptr)
		{
			if (renderer->device != nullptr)
			{
				renderer->device->ClearState();
				safe_release(renderer->rasterizer);
				safe_release(renderer->depth_state_gequal);
				safe_release(renderer->depth_state_lequal);
				safe_release(renderer->depth_state_off);
				safe_release(renderer->blend_state_alpha);
				safe_release(renderer->blend_state_multiply);
				safe_release(renderer->blend_state_add);
				safe_release(renderer->blend_state_off);
				safe_release(renderer->depth_stencil);
				safe_release(renderer->render_target);
				safe_release(renderer->swap_chain);
				renderer->device->Release();
			}

			free_mem(renderer);
			renderer = nullptr;
		}
	}

	///////////////////////////////////////////////////////////
	//
	//	Graphics functions
	//
	///////////////////////////////////////////////////////////

	const renderer_caps_t& get_renderer_caps()
	{
		return renderer->caps;
	}

	void clear_frame(f32_t color_r, f32_t color_g, f32_t color_b, f32_t depth)
	{
		const FLOAT color[] = {color_r, color_g, color_b, 1.0f};
		renderer->device->ClearRenderTargetView(renderer->render_target, color);
		renderer->device->ClearDepthStencilView(renderer->depth_stencil, D3D10_CLEAR_DEPTH, depth, 0);
	}

	void clear_frame(f32_t color_r, f32_t color_g, f32_t color_b, f32_t color_a, f32_t depth)
	{
		const FLOAT color[] = {color_r, color_g, color_b, color_a};
		renderer->device->ClearRenderTargetView(renderer->render_target, color);
		renderer->device->ClearDepthStencilView(renderer->depth_stencil, D3D10_CLEAR_DEPTH, depth, 0);
	}

	void clear_frame_color(f32_t color_r, f32_t color_g, f32_t color_b)
	{
		const FLOAT color[] = {color_r, color_g, color_b, 1.0f};
		renderer->device->ClearRenderTargetView(renderer->render_target, color);
	}

	void clear_frame_color(f32_t color_r, f32_t color_g, f32_t color_b, f32_t color_a)
	{
		const FLOAT color[] = {color_r, color_g, color_b, color_a};
		renderer->device->ClearRenderTargetView(renderer->render_target, color);
	}

	void clear_frame_depth(f32_t depth)
	{
		renderer->device->ClearDepthStencilView(renderer->depth_stencil, D3D10_CLEAR_DEPTH, depth, 0);
	}

	void set_viewport(i32_t x, i32_t y, i32_t width, i32_t height, f32_t depth_near, f32_t depth_far)
	{
		AUX_DEBUG_ASSERT(x >= 0);
		AUX_DEBUG_ASSERT(y >= 0);
		AUX_DEBUG_ASSERT(width > 0);
		AUX_DEBUG_ASSERT(height > 0);

		D3D10_VIEWPORT viewport = {};
		viewport.TopLeftX = (INT)x;
		viewport.TopLeftY = (INT)y;
		viewport.Width = (UINT)width;
		viewport.Height = (UINT)height;
		viewport.MinDepth = depth_near;
		viewport.MaxDepth = depth_far;
		renderer->device->RSSetViewports(1, &viewport);
	}

	void present_frame()
	{
		renderer->swap_chain->Present(renderer->vsync_interval, 0);
	}

	void toggle_frame_sync(bool on)
	{
		renderer->vsync_interval = (UINT)(on ? 1 : 0);
	}

	void set_color_blend(e32_t blend)
	{
		switch (blend)
		{
			case COLOR_BLEND_OFF:
				set_blend_state(renderer->blend_state_off);
				break;
			case COLOR_BLEND_ADD:
				set_blend_state(renderer->blend_state_add);
				break;
			case COLOR_BLEND_MULTIPLY:
				set_blend_state(renderer->blend_state_multiply);
				break;
			case COLOR_BLEND_ALPHA:
				set_blend_state(renderer->blend_state_alpha);
				break;
		}
	}

	void set_depth_test(e32_t test)
	{
		switch (test)
		{
			case DEPTH_TEST_OFF:
				set_depth_state(renderer->depth_state_off);
				break;
			case DEPTH_TEST_LESS_EQUAL:
				set_depth_state(renderer->depth_state_lequal);
				break;
			case DEPTH_TEST_GREATER_EQUAL:
				set_depth_state(renderer->depth_state_gequal);
				break;
		}
	}

	void set_draw_mode(e32_t mode)
	{
		switch (mode)
		{
			case DRAW_MODE_LINES:
				renderer->device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
				break;
			case DRAW_MODE_TRIANGLES:
				renderer->device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				break;
		}
	}

	void draw_elements(i32_t start_vertex, i32_t vertex_count)
	{
		AUX_DEBUG_ASSERT(start_vertex >= 0);
		AUX_DEBUG_ASSERT(vertex_count > 1);

		renderer->device->Draw((UINT)vertex_count, (UINT)start_vertex);
	}

	void draw_indexed_elements(i32_t start_index, i32_t index_count)
	{
		AUX_DEBUG_ASSERT(start_index >= 0);
		AUX_DEBUG_ASSERT(index_count > 1);

		renderer->device->DrawIndexed((UINT)index_count, (UINT)start_index, 0);
	}

	texture_t* create_texture(e32_t format, i32_t width, i32_t height, i32_t level_count, const void* data)
	{
		AUX_DEBUG_ASSERT(width > 0);
		AUX_DEBUG_ASSERT(height > 0);
		AUX_DEBUG_ASSERT(level_count > 0);

		if (width > renderer->caps.max_texture_dim)
		{
			return nullptr;
		}

		if (height > renderer->caps.max_texture_dim)
		{
			return nullptr;
		}

		/*ID3D10ShaderResourceView* view = create_texture_view(format, width, height, multilevel, data);

		if (view != nullptr)
		{
			texture_t* texture = (texture_t*)alloc_mem(sizeof(texture_t));
			texture->view = view;
			texture->width = width;
			texture->height = height;
			texture->level_count = 0;
		}*/

		return nullptr;
	}

	void destroy_texture(texture_t* texture)
	{
		texture->view->Release();
		free_mem(texture);
	}

	void update_texture(texture_t* texture, i32_t level, i32_t x, i32_t y, i32_t width, i32_t height, const void* data)
	{
	}

	vertex_buffer_t* create_vertex_buffer(bool dynamic, i32_t vertex_size, i32_t vertex_count, const void* data)
	{
		AUX_DEBUG_ASSERT(vertex_size > 0);
		AUX_DEBUG_ASSERT(vertex_count > 1);

		UINT size = (UINT)vertex_size * (UINT)vertex_count;
		ID3D10Buffer* resource;

		if (create_buffer(&resource, D3D10_BIND_VERTEX_BUFFER, dynamic, size, data))
		{
			vertex_buffer_t* buffer = (vertex_buffer_t*)alloc_mem(sizeof(vertex_buffer_t));
			buffer->buffer = resource;
			buffer->vertex_size = vertex_size;
			buffer->vertex_count = vertex_count;
			buffer->dynamic = dynamic;
			return buffer;
		}

		return nullptr;
	}

	void destroy_vertex_buffer(vertex_buffer_t* buffer)
	{
		if (buffer->buffer == renderer->current_vertex_buffer)
		{
			set_current_vertex_buffer(nullptr, 0);
		}

		buffer->buffer->Release();
		free_mem(buffer);
	}

	void update_vertex_buffer(vertex_buffer_t* buffer, i32_t start_vertex, i32_t vertex_count, const void* data)
	{
		AUX_DEBUG_ASSERT(start_vertex >= 0);
		AUX_DEBUG_ASSERT(vertex_count > 0);
		AUX_DEBUG_ASSERT((start_vertex + vertex_count) <= buffer->vertex_count);
		AUX_DEBUG_ASSERT(data != nullptr);

		UINT offset = (UINT)start_vertex * (UINT)buffer->vertex_size;
		UINT size = (UINT)vertex_count * (UINT)buffer->vertex_size;
		update_buffer(buffer->buffer, buffer->dynamic, offset, size, data);
	}

	void set_vertex_buffer(vertex_buffer_t* buffer)
	{
		set_current_vertex_buffer(buffer->buffer, (UINT)buffer->vertex_size);
	}

	index_buffer_t* create_index_buffer(bool dynamic, e32_t index_format, i32_t index_count, const void* data)
	{
		i32_t index_size = get_index_size(index_format);

		AUX_DEBUG_ASSERT(index_size > 0);
		AUX_DEBUG_ASSERT(index_count > 1);

		UINT size = (UINT)index_size * (UINT)index_count;
		ID3D10Buffer* resource;

		if (create_buffer(&resource, D3D10_BIND_INDEX_BUFFER, dynamic, size, data))
		{
			index_buffer_t* buffer = (index_buffer_t*)alloc_mem(sizeof(index_buffer_t));
			buffer->buffer = resource;
			buffer->dxgi_format = get_index_format(index_format);
			buffer->index_format = index_format;
			buffer->index_size = index_size;
			buffer->index_count = index_count;
			buffer->dynamic = dynamic;
			return buffer;
		}

		return nullptr;
	}

	void destroy_index_buffer(index_buffer_t* buffer)
	{
		if (buffer->buffer == renderer->current_index_buffer)
		{
			set_current_index_buffer(nullptr, DXGI_FORMAT_UNKNOWN);
		}

		buffer->buffer->Release();
		free_mem(buffer);
	}

	void update_index_buffer(index_buffer_t* buffer, i32_t start_index, i32_t index_count, const void* data)
	{
		AUX_DEBUG_ASSERT(start_index >= 0);
		AUX_DEBUG_ASSERT(index_count > 0);
		AUX_DEBUG_ASSERT((start_index + index_count) <= buffer->index_count);
		AUX_DEBUG_ASSERT(data != nullptr);

		UINT offset = (UINT)start_index * (UINT)buffer->index_size;
		UINT size = (UINT)index_count * (UINT)buffer->index_size;
		update_buffer(buffer->buffer, buffer->dynamic, offset, size, data);
	}

	void set_index_buffer(index_buffer_t* buffer)
	{
		set_current_index_buffer(buffer->buffer, buffer->dxgi_format);
	}
}
