#pragma once
#include "D3D12Content.h"
#include "D3D12Core.h"
#include "Utilities/IOStream.h"
#include "Content/ContentToEngine.h"

namespace havana::graphics::d3d12::content
{
	namespace
	{
		struct submesh_view
		{
			D3D12_VERTEX_BUFFER_VIEW			position_buffer_view{};
			D3D12_VERTEX_BUFFER_VIEW			element_buffer_view{};
			D3D12_INDEX_BUFFER_VIEW				index_buffer_view{};
			D3D_PRIMITIVE_TOPOLOGY				primitive_topology;
			u32									elements_type{};
		};

		utl::free_list<ID3D12Resource*>			submesh_buffers{};
		utl::free_list<submesh_view>			submesh_views{};
		std::mutex								submesh_mutex{};
		
		utl::free_list<d3d12_texture>			textures;
		std::mutex								texture_mutex{};

		utl::vector<ID3D12RootSignature*>		root_signatures;
		std::unordered_map<u64, id::id_type>	mtl_rs_map; // maps a material's type and shader flags to an index in the array of root signatures
		utl::free_list<std::unique_ptr<u8[]>>	materials;
		std::mutex								material_mutex{};

		class d3d12_material_stream
		{
		public:
			DISABLE_COPY_AND_MOVE(d3d12_material_stream);
			explicit d3d12_material_stream(u8* const material_buffer) : _buffer{ material_buffer }
			{
				initialize();
			}

			explicit d3d12_material_stream(std::unique_ptr<u8[]>& material_buffer, material_init_info info)
			{
				assert(!material_buffer);

				u32 shader_count{ 0 };
				u32 flags{ 0 };
				for (u32 i{ 0 }; i < shader_type::count; ++i)
				{
					if (id::is_valid(info.shader_ids[i]))
					{
						++shader_count;
						flags |= (1 << i);
					}
				}

				assert(shader_count && flags);

				const u32 buffer_size{
					sizeof(material_type::type) +								// material type
					sizeof(shader_flags::flags) +								// shader flags	
					sizeof(id::id_type) +										// root signature id
					sizeof(u32) +												// texture count
					sizeof(id::id_type) * shader_count +						// shader ids
					(sizeof(id::id_type) + sizeof(u32)) * info.texture_count	// texture ids and descriptor indices (may be 0 if no textures used)
				};

				material_buffer = std::make_unique<u8[]>(buffer_size);
				_buffer = material_buffer.get();
				u8* const buffer{ _buffer };

				*(material_type::type*)buffer = info.type;
				*(shader_flags::flags*)(&buffer[shader_flags_index]) = (shader_flags::flags)flags;
				*(id::id_type*)(&buffer[root_signature_index]) = create_root_signature(info.type, (shader_flags::flags)flags);
				*(u32*)(&buffer[texture_count_index]) = info.texture_count;

				initialize();

				if (info.texture_count)
				{
					memcpy(_texture_ids, info.texture_ids, info.texture_count * sizeof(id::id_type));
					texture::get_descriptor_indices(_texture_ids, info.texture_count, _descriptor_indices);
				}

				u32 shader_index{ 0 };
				for (u32 i{ 0 }; i < shader_type::count; ++i)
				{
					if (id::is_valid(info.shader_ids[i]))
					{
						_shader_ids[shader_index] = info.shader_ids[i];
						++shader_index;
					}
				}

				assert(shader_index == (u32)_mm_popcnt_u32(_shader_flags));
			}
		private:
			void initialize()
			{
				assert(_buffer);
				u8* const buffer{ _buffer };

				_type = *(material_type::type*)buffer;
				_shader_flags = *(shader_flags::flags*)(&buffer[shader_flags_index]);
				_root_signature_id = *(id::id_type*)(&buffer[root_signature_index]);
				_texture_count = *(u32*)(&buffer[texture_count_index]);

				_shader_ids = (id::id_type*)(&buffer[texture_count_index + sizeof(u32)]);
				_texture_ids = _texture_count ? &_shader_ids[_mm_popcnt_u32(_shader_flags)] : nullptr;
				_descriptor_indices = _texture_count ? (u32*)(&_texture_ids[_texture_count]) : nullptr;
			}

			constexpr static u32	shader_flags_index{ sizeof(material_type::type) };
			constexpr static u32	root_signature_index{ shader_flags_index + sizeof(shader_flags::flags) };
			constexpr static u32	texture_count_index{ root_signature_index + sizeof(id::id_type) };

			u8*						_buffer;
			id::id_type*			_texture_ids;
			u32*					_descriptor_indices;
			id::id_type*			_shader_ids;
			id::id_type				_root_signature_id;
			u32						_texture_count;
			material_type::type		_type;
			shader_flags::flags		_shader_flags;
		};

		D3D_PRIMITIVE_TOPOLOGY
		get_d3d_primitive_topology(primitive_topology::type type)
		{
			assert(type < primitive_topology::count);

			switch (type)
			{
			case primitive_topology::point_list:
				return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			case primitive_topology::line_list:
				return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case primitive_topology::line_strip:
				return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case primitive_topology::triangle_list:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case primitive_topology::triangle_strip:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			}

			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

	} // anonymous namespace
	
	namespace submesh
	{
		// NOTE: Expects 'data' to contain (in order):
		//		u32 element_size, u32 vertex_count,
		//		u32 index_count, u32 elements_type, u32 primitive_topology
		//		u8 positions[sizeof(f32) * 3 * vertext_count],		// sizeof(positions) must be a multiple of 4 bytes. Pad if needed.
		//		u8 elements[sizeof(element_size) * vertext_count],	// sizeof(elements) must be a multiple of 4 bytes. Pad if needed.
		//		u8 indices[index_size * index_count],
		/// <summary>
		/// Advances the data pointer.
		/// Position and element buffers should be padded to be a multiple of 4 bytes in length.
		/// This 4 bytes is defined as D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE.
		/// </summary>
		/// <param name="data"></param>
		/// <returns></returns>
		id::id_type
		add(const u8*& data)
		{
			utl::blob_stream_reader blob{ (const u8*)data };

			const u32 element_size{ blob.read<u32>() };
			const u32 vertex_count{ blob.read<u32>() };
			const u32 index_count{ blob.read<u32>() };
			const u32 elements_type{ blob.read<u32>() };
			const u32 primitive_topology{ blob.read<u32>() };
			const u32 index_size{ (vertex_count < (1 << 16)) ? sizeof(u16) : sizeof(u32) };

			// NOTE: element size may be 0, for position-only vertex formats.
			const u32 position_buffer_size{ sizeof(math::v3) * vertex_count };
			const u32 element_buffer_size{ element_size * vertex_count };
			const u32 index_buffer_size{ index_size * index_count};

			constexpr u32 alignment{ D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE };
			const u32 aligned_position_buffer_size{ (u32)math::align_size_up<alignment>(position_buffer_size) };
			const u32 aligned_element_buffer_size{ (u32)math::align_size_up<alignment>(element_buffer_size) };
			const u32 total_buffer_size{ aligned_position_buffer_size + aligned_element_buffer_size + index_buffer_size };

			ID3D12Resource* resource{ d3dx::create_buffer(blob.position(), total_buffer_size) };
			
			blob.skip(total_buffer_size);
			data = blob.position();

			submesh_view view{};
			view.position_buffer_view.BufferLocation = resource->GetGPUVirtualAddress();
			view.position_buffer_view.SizeInBytes = position_buffer_size;
			view.position_buffer_view.StrideInBytes = sizeof(math::v3);

			if (element_size)
			{
				view.element_buffer_view.BufferLocation = resource->GetGPUVirtualAddress() + aligned_position_buffer_size;
				view.element_buffer_view.SizeInBytes = element_buffer_size;
				view.element_buffer_view.StrideInBytes = element_size;
			}

			view.index_buffer_view.BufferLocation = resource->GetGPUVirtualAddress() + aligned_position_buffer_size + aligned_element_buffer_size;
			view.index_buffer_view.SizeInBytes = index_buffer_size;
			view.index_buffer_view.Format = (index_size == sizeof(u16)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

			view.primitive_topology = get_d3d_primitive_topology((primitive_topology::type)primitive_topology);
			view.elements_type = elements_type;

			std::lock_guard lock{ submesh_mutex };
			submesh_buffers.add(resource);
			return submesh_views.add(view);
		}

		void
		remove(id::id_type id)
		{
			std::lock_guard lock{ submesh_mutex };
			submesh_views.remove(id);

			core::deferred_release(submesh_buffers[id]);
			submesh_buffers.remove(id);
		}
	} // namespace submesh

	namespace texture
	{
		void
		get_descriptor_indices(const id::id_type* const texture_ids, u32 id_count, u32* const indices)
		{
			assert(texture_ids && id_count && indices);
			std::lock_guard lock{ texture_mutex };
			for (u32 i{ 0 }; i < id_count; ++i)
			{
				indices[i] = textures[i].srv().index;
			}
		}
	} // namespace texture

	namespace material
	{
		// Output format:
		// struct {
		// material_type::type	type;
		// shaer_flags::flags	flags;
		// id::id_type			root_signature_id;
		// u32					texture_count;
		// id::id_type			shader_ids[shader_count];
		// id::id_type			texture_ids[texture_count;
		// u32*					descriptor_indices[texture_count];
		// } d3d12_material
		id::id_type
		add(material_init_info info)
		{
			std::unique_ptr<u8[]> buffer;
			std::lock_guard lock{ material_mutex };
			// Create material form info
			assert(buffer);
			return materials.add(std::move(buffer));
		}

		void
		remove(id::id_type id)
		{
			std::lock_guard lock{ material_mutex };
			materials.remove(id);
		}
	} // namespace material
}