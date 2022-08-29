#include "ContentToEngine.h"
#include "Graphics/Renderer.h"
#include "Utilities/IOStream.h"

namespace havana::content
{
	namespace
	{
		class GeometryHierarchyStream
		{
		public:
			struct lodOffset
			{
				u16 offset;
				u16 count;
			};

			GeometryHierarchyStream(u8* const buffer, u32 lods = u32_invalid_id) : m_buffer{ buffer }
			{
				assert(buffer && lods);
				if (lods != u32_invalid_id)
				{
					*((u32*)buffer) = lods;
				}

				m_lodCount = *((u32*)buffer);
				m_thresholds = (f32*)(&buffer[sizeof(u32)]);
				m_lodOffsets = (lodOffset*)(&m_thresholds[m_lodCount]);
				m_gpuIds = (id::id_type*)(&m_lodOffsets[m_lodCount]);
			}
			DISABLE_COPY_AND_MOVE(GeometryHierarchyStream);

			void GpuIds(u32 lod, id::id_type*& ids, u32& idCount)
			{
				assert(lod < m_lodCount);
				ids = &m_gpuIds[m_lodOffsets[lod].offset];
				idCount = m_lodOffsets[lod].count;
			}

			u32 LoDFromThreshold(f32 threshold)
			{
				assert(threshold > 0);

				for (u32 i{ m_lodCount - 1 }; i > 0; i--)
				{
					if (m_thresholds[i] <= threshold) return i;
				}

				assert(false); // something bad happened if we get here.
				return 0;
			}

			[[nodiscard]] constexpr u32 LoDCount() const { return m_lodCount; }
			[[nodiscard]] constexpr f32* Thresholds() const { return m_thresholds; }
			[[nodiscard]] constexpr lodOffset* LoDOffsets() const { return m_lodOffsets; }
			[[nodiscard]] constexpr id::id_type* GpuIds() const { return m_gpuIds; }

		private:
			u8* const		m_buffer;
			f32*			m_thresholds;
			lodOffset*		m_lodOffsets;
			id::id_type*	m_gpuIds;
			u32				m_lodCount;
		};
		
		// This constant indicate that an element in geometryHierarchies is not a pointer, but a gpuId
		constexpr uintptr_t		singleMeshMarker{ (uintptr_t)0x01 };
		utl::free_list<u8*>	geometryHierarchies;
		std::mutex				geometryMutex;
		
		// NOTE: expects the same data as CreateGeometryResource()
		u32 GetGeometryHierarchySize(const void* const data)
		{
			assert(data);
			utl::blob_stream_reader blob{ (const u8*)data };
			const u32 lodCount{ blob.read<u32>() };
			assert(lodCount);
			// Add size of lodCount, thresholds, and lod offsets to the size of hierarchy
			u32 size{ sizeof(u32) + (sizeof(f32) + sizeof(GeometryHierarchyStream::lodOffset)) * lodCount };

			for (u32 lodIdx{ 0 }; lodIdx < lodCount; lodIdx++)
			{
				// skip threshold
				blob.skip(sizeof(f32));
				// add size of gpuIds (sizeof(id::id_type) * submeshCount)
				size += sizeof(id::id_type) * blob.read<u32>();
				// skip submesh data and go to the next LOD
				blob.skip(blob.read<u32>());				
			}

			return size;
		}

		// Creates a hierarchy stream for a geometry that has multiple LODs and/or multiple submeshes
		// NOTE: expects the same data as CreateGeometryResource()
		id::id_type CreateMeshHierarchy(const void* const data)
		{
			assert(data);
			const u32 size{ GetGeometryHierarchySize(data) };
			u8* const hierarchyBuffer{ (u8* const)malloc(size) };

			utl::blob_stream_reader blob{ (const u8*)data };
			const u32 lodCount{ blob.read<u32>() };
			assert(lodCount);
			GeometryHierarchyStream stream{ hierarchyBuffer, lodCount };
			u32 submeshIndex{ 0 };
			id::id_type* const gpuIds{ stream.GpuIds() };

			for (u32 lodIdx{ 0 }; lodIdx < lodCount; lodIdx++)
			{
				stream.Thresholds()[lodIdx] = blob.read<f32>();
				const u32 idCount{ blob.read<u32>() };
				assert(idCount < (1 << 16));
				stream.LoDOffsets()[lodIdx] = { (u16)submeshIndex, (u16)idCount };
				blob.skip(sizeof(u32)); // skip over sizeOfSubmeshes
				for (u32 idIdx{ 0 }; idIdx < idCount; idIdx++)
				{
					const u8* at{ blob.position() };
					gpuIds[submeshIndex++] = graphics::AddSubmesh(at);
					blob.skip((u32)(at - blob.position()));
					assert(submeshIndex < (1 << 16));
				}
			}

			assert([&]() {
				f32 previousThreshold{ stream.Thresholds()[0] };
				for (u32 i{ 1 }; i < lodCount; i++)
				{
					if (stream.Thresholds()[i] <= previousThreshold) return false;
					previousThreshold = stream.Thresholds()[i];
				}
				return true;
				}());

			static_assert(alignof(void*) > 2, "We need the least significant bit for the single mesh marker.");
			std::lock_guard lock{ geometryMutex };
			return geometryHierarchies.add(hierarchyBuffer);
		}

		// Creates geometry stream for the GPU that has a single submesh with a single LOD
		// Creates a single submesh
		// NOTE: expects the same data as CreateGeometryResource()
		id::id_type CreateSingleSubmesh(const void* const data)
		{
			assert(data);
			utl::blob_stream_reader blob{ (const u8*)data };
			// skip lodCount, lod_threshold, submeshCount, and sizeOfSubmeshes
			blob.skip(sizeof(u32) + sizeof(f32) + sizeof(u32) + sizeof(u32));
			const u8* at{ blob.position() };
			const id::id_type gpuId{ graphics::AddSubmesh(at) };

			// Create a fake pointer and put it in the geometryHierarchies
			static_assert(sizeof(uintptr_t) > sizeof(id::id_type));
			constexpr u8 shiftBits{ (sizeof(uintptr_t) - sizeof(id::id_type)) << 3 };
			u8* const fakePointer{ (u8* const)((((uintptr_t)gpuId) << shiftBits) | singleMeshMarker) };
			std::lock_guard lock{ geometryMutex };
			return geometryHierarchies.add(fakePointer);
		}

		// Determine if this geometry has a single lod with a single submesh
		// NOTE: expects the same data as CreateGeometryResource()
		bool IsSingleMesh(const void* const data)
		{
			assert(data);
			utl::blob_stream_reader blob{ (const u8*)data };
			const u32 lodCount{ blob.read<u32>() };
			assert(lodCount);
			if (lodCount > 1) return false;

			// Skip over threshold
			blob.skip(sizeof(f32));
			const u32 submeshCount{ blob.read<u32>() };
			assert(submeshCount);
			return submeshCount == 1;
		}

		id::id_type GpuIdFromFakePointer(u8* const pointer)
		{
			assert((uintptr_t)pointer & singleMeshMarker);
			static_assert(sizeof(uintptr_t) > sizeof(id::id_type));
			constexpr u8 shiftBits{ (sizeof(uintptr_t) - sizeof(id::id_type)) << 3 };
			return (((uintptr_t)pointer) >> shiftBits) & (uintptr_t)id::invalid_id;
		}

		// NOTE: Expects 'data' to contain (in order):
		// struct
		// {
		//     u32 lodCount
		//     struct
		//     {
		//         f32 lod_threshold,
		//         u32 submeshCount,
		//         u32 sizeOfSubmeshes,
		//         struct
		//         {
		//             u32 elementSize, u32 vertexCount,
		//             u32 indexCount, u32 elements_type, u32 primitiveTopology
		//		       u8 positions[sizeof(f32) * 3 * vertextCount],		// sizeof(positions) must be a multiple of 4 bytes. Pad if needed.
		//		       u8 elements[sizeof(elementSize) * vertextCount],	// sizeof(elements) must be a multiple of 4 bytes. Pad if needed.
		//		       u8 indices[indexSize * indexCount],
		//         } submeshes[submeshCount]
		//     } meshLods[lodCount]
		// } geometry;
		//
		// Output format:
		// 
		// If geometry has more than one LOD or submesh:
		// struct
		// {
		//     u32 lodCount,
		//     f32 thresholds[lodCount],
		//     struct
		//     {
		//         u16 offset,
		//         u16 count,
		//     } lodOffsets[lodCount],
		//     id::id_type gpuIds[totalNumberOfSubmeshes]
		// } geometryHierarchy;
		//
		// If geometry has a single LOD and submesh
		// 
		// (gpu_id << 32) | 0x01
		//

		id::id_type CreateGeometryResource(const void* const data)
		{
			assert(data);
			return IsSingleMesh(data) ? CreateSingleSubmesh(data) : CreateMeshHierarchy(data);
		}

		void DestroyGeometryResource(id::id_type id)
		{
			std::lock_guard lock{ geometryMutex };
			u8* const pointer{ geometryHierarchies[id] };
			if ((uintptr_t)pointer & singleMeshMarker)
			{
				graphics::RemoveSubmesh(GpuIdFromFakePointer(pointer));
			}
			else
			{
				GeometryHierarchyStream stream{ pointer };
				const u32 lodCount{ stream.LoDCount() };
				u32 idIndex{ 0 };
				for (u32 lod{ 0 }; lod < lodCount; lod++)
				{
					for (u32 i{ 0 }; i < stream.LoDOffsets()[lod].count; i++)
					{
						graphics::RemoveSubmesh(stream.GpuIds()[idIndex++]);
					}
				}
				
				free(pointer);
			}

			geometryHierarchies.remove(id);
		}
	} // anonymous namespaces

	id::id_type CreateResource(const void* const data, AssetType::type type)
	{
		assert(data);
		id::id_type id{ id::invalid_id };

		switch (type)
		{
		case AssetType::Unknown:
			break;
		case AssetType::Animation:
			break;
		case AssetType::Audio:
			break;
		case AssetType::Material:
			break;
		case AssetType::Mesh:
			id = CreateGeometryResource(data);
			break;
		case AssetType::Skeleton:
			break;
		case AssetType::Texture:
			break;
		}

		assert(id::is_valid(id));
		return id;
	}

	void DestroyResource(id::id_type id, AssetType::type type)
	{
		assert(id::is_valid(id));

		switch (type)
		{
		case AssetType::Unknown:
			break;
		case AssetType::Animation:
			break;
		case AssetType::Audio:
			break;
		case AssetType::Material:
			break;
		case AssetType::Mesh:
			DestroyGeometryResource(id);
			break;
		case AssetType::Skeleton:
			break;
		case AssetType::Texture:
			break;
		default:
			assert(false);
			break;
		}
	}
}