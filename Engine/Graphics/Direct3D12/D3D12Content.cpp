#pragma once
#include "D3D12Content.h"
#include "D3D12Core.h"
#include "Utilities/IOStream.h"
#include "Content/ContentToEngine.h"

namespace havana::graphics::d3d12::content
{
	namespace
	{
		struct SubmeshView
		{
			D3D12_VERTEX_BUFFER_VIEW		positionBufferView{};
			D3D12_VERTEX_BUFFER_VIEW		elementBufferView{};
			D3D12_INDEX_BUFFER_VIEW			indexBufferView{};
			D3D_PRIMITIVE_TOPOLOGY			primitiveTopology;
			u32								elements_type{};
		};

		utl::free_list<ID3D12Resource*>	submeshBuffers{};
		utl::free_list<SubmeshView>		submeshViews{};
		std::mutex							submeshMutex{};

		D3D_PRIMITIVE_TOPOLOGY GetD3DPrimitiveTopology(havana::content::primitive_topology::type type)
		{
			using namespace havana::content;

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
	
	namespace Submesh
	{
		// NOTE: Expects 'data' to contain (in order):
		//		u32 elementSize, u32 vertexCount,
		//		u32 indexCount, u32 elements_type, u32 primitiveTopology
		//		u8 positions[sizeof(f32) * 3 * vertextCount],		// sizeof(positions) must be a multiple of 4 bytes. Pad if needed.
		//		u8 elements[sizeof(elementSize) * vertextCount],	// sizeof(elements) must be a multiple of 4 bytes. Pad if needed.
		//		u8 indices[indexSize * indexCount],
		/// <summary>
		/// Advances the data pointer.
		/// Position and element buffers should be padded to be a multiple of 4 bytes in length.
		/// This 4 bytes is defined as D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE.
		/// </summary>
		/// <param name="data"></param>
		/// <returns></returns>
		id::id_type Add(const u8*& data)
		{
			utl::blob_stream_reader blob{ (const u8*)data };

			const u32 elementSize{ blob.read<u32>() };
			const u32 vertexCount{ blob.read<u32>() };
			const u32 indexCount{ blob.read<u32>() };
			const u32 elements_type{ blob.read<u32>() };
			const u32 primitiveTopology{ blob.read<u32>() };
			const u32 indexSize{ (vertexCount < (1 << 16)) ? sizeof(u16) : sizeof(u32) };

			// NOTE: element size may be 0, for position-only vertex formats.
			const u32 positionBufferSize{ sizeof(math::v3) * vertexCount };
			const u32 elementBufferSize{ elementSize * vertexCount };
			const u32 indexBufferSize{ indexSize * indexCount};

			constexpr u32 alignment{ D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE };
			const u32 alignedPositionBufferSize{ (u32)math::AlignSizeUp<alignment>(positionBufferSize) };
			const u32 alignedElementBufferSize{ (u32)math::AlignSizeUp<alignment>(elementBufferSize) };
			const u32 totalBufferSize{ alignedPositionBufferSize + alignedElementBufferSize + indexBufferSize };

			ID3D12Resource* resource{ D3DX::CreateBuffer(blob.position(), totalBufferSize) };
			
			blob.skip(totalBufferSize);
			data = blob.position();

			SubmeshView view{};
			view.positionBufferView.BufferLocation = resource->GetGPUVirtualAddress();
			view.positionBufferView.SizeInBytes = positionBufferSize;
			view.positionBufferView.StrideInBytes = sizeof(math::v3);

			if (elementSize)
			{
				view.elementBufferView.BufferLocation = resource->GetGPUVirtualAddress() + alignedPositionBufferSize;
				view.elementBufferView.SizeInBytes = elementBufferSize;
				view.elementBufferView.StrideInBytes = elementSize;
			}

			view.indexBufferView.BufferLocation = resource->GetGPUVirtualAddress() + alignedPositionBufferSize + alignedElementBufferSize;
			view.indexBufferView.SizeInBytes = indexBufferSize;
			view.indexBufferView.Format = (indexSize == sizeof(u16)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

			view.primitiveTopology = GetD3DPrimitiveTopology((havana::content::primitive_topology::type)primitiveTopology);
			view.elements_type = elements_type;

			std::lock_guard lock{ submeshMutex };
			submeshBuffers.add(resource);
			return submeshViews.add(view);
		}

		void Remove(id::id_type id)
		{
			std::lock_guard lock{ submeshMutex };
			submeshViews.remove(id);

			Core::DeferredRelease(submeshBuffers[id]);
			submeshBuffers.remove(id);
		}
	}
}

