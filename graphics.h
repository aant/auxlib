#pragma once

#include "point2.h"
#include "size2.h"
#include "color4.h"

namespace aux
{
	enum
	{
		COLOR_BLEND_BAD_ENUM = -1,

		COLOR_BLEND_OFF,
		COLOR_BLEND_ADD,
		COLOR_BLEND_MULTIPLY,
		COLOR_BLEND_ALPHA,

		COLOR_BLEND_MAX_ENUMS
	};

	enum
	{
		DEPTH_TEST_BAD_ENUM = -1,

		DEPTH_TEST_OFF,
		DEPTH_TEST_LESS_EQUAL,
		DEPTH_TEST_GREATER_EQUAL,

		DEPTH_TEST_MAX_ENUMS
	};

	enum
	{
		DRAW_MODE_BAD_ENUM = -1,

		DRAW_MODE_LINES,
		DRAW_MODE_TRIANGLES,

		DRAW_MODE_MAX_ENUMS
	};

	enum
	{
		TEXTURE_FORMAT_BAD_ENUM = -1,

		TEXTURE_FORMAT_R8,
		TEXTURE_FORMAT_R8G8,
		TEXTURE_FORMAT_R8G8B8,
		TEXTURE_FORMAT_R8G8B8A8,

		TEXTURE_FORMAT_MAX_ENUMS
	};

	enum
	{
		CUBEMAP_FACE_BAD_ENUM = -1,

		CUBEMAP_FACE_LEFT,
		CUBEMAP_FACE_RIGHT,
		CUBEMAP_FACE_TOP,
		CUBEMAP_FACE_BOTTOM,
		CUBEMAP_FACE_FRONT,
		CUBEMAP_FACE_BACK,

		CUBEMAP_FACE_MAX_ENUMS
	};

	enum
	{
		TEXTURE_FILTER_BAD_ENUM = -1,

		TEXTURE_FILTER_POINT,
		TEXTURE_FILTER_BILINEAR,
		TEXTURE_FILTER_TRILINEAR,
		TEXTURE_FILTER_ANISOTROPIC,

		TEXTURE_FILTER_MAX_ENUMS
	};

	enum
	{
		TEXTURE_WRAP_BAD_ENUM = -1,

		TEXTURE_WRAP_CLAMP,
		TEXTURE_WRAP_REPEAT,

		TEXTURE_WRAP_MAX_ENUMS
	};

	enum
	{
		INDEX_FORMAT_BAD_ENUM = -1,

		INDEX_FORMAT_UINT16,
		INDEX_FORMAT_UINT32,

		INDEX_FORMAT_MAX_ENUMS
	};

	struct texture_t;
	struct cubemap_t;
	struct sampler_t;
	struct vertex_format_t;
	struct vertex_buffer_t;
	struct index_buffer_t;
	struct shader_buffer_t;
	struct vertex_shader_t;
	struct pixel_shader_t;

	struct graphics_caps_t
	{
		i32_t max_texture_dim;
		i32_t max_cubemap_dim;
		i32_t max_texture_slots;
		i32_t max_sampler_slots;
	};

	const graphics_caps_t& get_graphics_caps();

	void clear_frame(const color4_t& color, f32_t depth);
	void clear_frame_color(const color4_t& color);
	void clear_frame_depth(f32_t depth);
	void present_frame();
	void toggle_frame_sync(bool on);

	void set_color_blend(e32_t blend);
	void set_depth_test(e32_t test);
	void set_draw_mode(e32_t mode);
	void set_viewport(const point2_t& offset, size2_t& size, f32_t depth_near, f32_t depth_far);

	void draw_elements(i32_t start_vertex, i32_t vertex_count);
	void draw_indexed_elements(i32_t start_index, i32_t index_count);

	texture_t* create_texture(e32_t format, const size2_t& size, bool multilevel, const void* data = nullptr);
	void destroy_texture(texture_t* texture);
	void update_texture(texture_t* texture, i32_t level, const point2_t& offset, const size2_t& size, const void* data);

	cubemap_t* create_cubemap(e32_t format, i32_t size, bool multilevel, const void* data = nullptr);
	void destroy_cubemap(texture_t* cubemap);
	void update_cubemap(texture_t* cubemap, i32_t level, const point2_t& offset, const size2_t& size, const void* data);

	sampler_t* create_sampler(e32_t filter, e32_t wrap_u, e32_t wrap_v);
	void destroy_sampler(sampler_t* sampler);

	vertex_buffer_t* create_vertex_buffer(bool dynamic, i32_t vertex_size, i32_t vertex_count, const void* data = nullptr);
	void destroy_vertex_buffer(vertex_buffer_t* buffer);
	void update_vertex_buffer(vertex_buffer_t* buffer, i32_t start_vertex, i32_t vertex_count, const void* data);
	void set_vertex_buffer(vertex_buffer_t* buffer);

	index_buffer_t* create_index_buffer(bool dynamic, e32_t index_format, i32_t index_count, const void* data = nullptr);
	void destroy_index_buffer(index_buffer_t* buffer);
	void update_index_buffer(index_buffer_t* buffer, i32_t start_index, i32_t index_count, const void* data);
	void set_index_buffer(index_buffer_t* buffer);

	void set_pixel_shader_resource(texture_t* texture, i32_t slot);
	void set_pixel_shader_resource(sampler_t* sampler, i32_t slot);
	void set_pixel_shader_resource(shader_buffer_t* buffer, i32_t slot);

	void set_vertex_shader_resource(shader_buffer_t* buffer, i32_t slot);
}
