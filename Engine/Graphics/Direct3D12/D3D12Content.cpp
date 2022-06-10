#pragma once
#include "D3D12Content.h"
#include "D3D12Core.h"
#include "Utilities/IOStream.h"
#include "Content/ContentToEngine.h"

namespace Havana::Graphics::D3D12::Content
{
	namespace
	{
		struct PositionView
		{
			D3D12_VERTEX_BUFFER_VIEW		positionBufferView{};
			D3D12_INDEX_BUFFER_VIEW			indexBufferView{};
		};

		struct ElementView
		{
			D3D12_VERTEX_BUFFER_VIEW		elementBufferView{};
			u32								elementsType{};
			D3D_PRIMITIVE_TOPOLOGY			primitiveTopology;
		};

		Utils::free_list<ID3D12Resource*>	submeshBuffers{};
		Utils::free_list<PositionView>		positionViews{};
		Utils::free_list<ElementView>		elementViews{};
		std::mutex							submeshMutex{};

		D3D_PRIMITIVE_TOPOLOGY GetD3DPrimitiveTopology(Havana::Content::PrimitiveTopology::Type type)
		{
			using namespace Havana::Content;

			assert(type < PrimitiveTopology::Count);

			switch (type)
			{
			case PrimitiveTopology::PointList:
				return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			case PrimitiveTopology::LineList:
				return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case PrimitiveTopology::LineStrip:
				return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case PrimitiveTopology::TriangleList:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case PrimitiveTopology::TriangleStrip:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			}

			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

	} // anonymous namespace
	
	namespace Submesh
	{
		// NOTE: Expects 'data' to contain (in order):
		//		u32 elementSize, u32 vertexCount,
		//		u32 indexCount, u32 elementsType, u32 primitiveTopology
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
		Id::id_type Add(const u8*& data)
		{
			Utils::BlobStreamReader blob{ (const u8*)data };

			const u32 elementSize{ blob.Read<u32>() };
			const u32 vertexCount{ blob.Read<u32>() };
			const u32 indexCount{ blob.Read<u32>() };
			const u32 elementsType{ blob.Read<u32>() };
			const u32 primitiveTopology{ blob.Read<u32>() };
			const u32 indexSize{ (vertexCount < (1 << 16)) ? sizeof(u16) : sizeof(u32) };

			// NOTE: element size may be 0, for position-only vertex formats.
			const u32 positionBufferSize{ sizeof(Math::Vec3) * vertexCount };
			const u32 elementBufferSize{ elementSize * vertexCount };
			const u32 indexBufferSize{ indexSize * indexCount};

			constexpr u32 alignment{ D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE };
			const u32 alignedPositionBufferSize{ (u32)Math::AlignSizeUp<alignment>(positionBufferSize) };
			const u32 alignedElementBufferSize{ (u32)Math::AlignSizeUp<alignment>(elementBufferSize) };
			const u32 totalBufferSize{ alignedPositionBufferSize + alignedElementBufferSize + indexBufferSize };

			ID3D12Resource* resource{ D3DX::CreateBuffer(blob.Position(), totalBufferSize) };
			
			blob.Skip(totalBufferSize);
			data = blob.Position();

			PositionView positionView{};
			positionView.positionBufferView.BufferLocation = resource->GetGPUVirtualAddress();
			positionView.positionBufferView.SizeInBytes = positionBufferSize;
			positionView.positionBufferView.StrideInBytes = sizeof(Math::Vec3);

			positionView.indexBufferView.BufferLocation = resource->GetGPUVirtualAddress() + alignedPositionBufferSize + alignedElementBufferSize;
			positionView.indexBufferView.Format = (indexSize == sizeof(u16)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
			positionView.indexBufferView.SizeInBytes = indexBufferSize;

			ElementView elementView{};
			if (elementSize)
			{
				elementView.elementBufferView.BufferLocation = resource->GetGPUVirtualAddress() + alignedPositionBufferSize;
				elementView.elementBufferView.SizeInBytes = elementBufferSize;
				elementView.elementBufferView.StrideInBytes = elementSize;
			}

			elementView.elementsType = elementsType;
			elementView.primitiveTopology = GetD3DPrimitiveTopology((Havana::Content::PrimitiveTopology::Type)primitiveTopology);

			std::lock_guard lock{ submeshMutex };
			submeshBuffers.add(resource);
			positionViews.add(positionView);
			return elementViews.add(elementView);
		}

		void Remove(Id::id_type id)
		{
			std::lock_guard lock{ submeshMutex };
			positionViews.remove(id);
			elementViews.remove(id);

			Core::DeferredRelease(submeshBuffers[id]);
			submeshBuffers.remove(id);
		}
	}
}

