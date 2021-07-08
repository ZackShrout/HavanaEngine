#include "Geometry.h"

namespace Havana::Tools
{
	namespace
	{
		using namespace Math;
		using namespace DirectX;

		void RecalculateNormals(Mesh& m)
		{
			const u32 numIndices{ (u32)m.rawIndices.size() };
			m.normals.reserve(numIndices);

			for (u32 i{ 0 }; i < numIndices; i++)
			{
				const u32 i0{ m.rawIndices[i] };
				const u32 i1{ m.rawIndices[++i] };
				const u32 i2{ m.rawIndices[++i] };

				XMVECTOR v0{ XMLoadFloat3(&m.positions[i0]) };
				XMVECTOR v1{ XMLoadFloat3(&m.positions[i1]) };
				XMVECTOR v2{ XMLoadFloat3(&m.positions[i2]) };

				XMVECTOR e0{ v1 - v0 };
				XMVECTOR e1{ v2 - v0 };
				XMVECTOR n{ XMVector3Normalize(XMVector3Cross(e0, e1)) };

				XMStoreFloat3(&m.normals[i], n);
				m.normals[i - 1] = m.normals[i];
				m.normals[i - 2] = m.normals[i];
			}
		}

		void ProcessNormals(Mesh& m, f32 smoothingAngle)
		{
			const f32 cosAlpha{ XMScalarCos(pi - smoothingAngle * pi / 180.0f) };
			const bool isHardEdge{ XMScalarNearEqual(smoothingAngle, 180.0f, epsilon) };
			const bool isSoftEdge{ XMScalarNearEqual(smoothingAngle, 0.0f, epsilon) };
			const u32 numIndices{ (u32)m.rawIndices.size() };
			const u32 numVertices{ (u32)m.positions.size() };

			assert(numIndices && numVertices);

			m.indices.resize(numIndices);
			Utils::vector<Utils::vector<u32>> idxRef(numVertices);
			
			for (u32 i{ 0 }; i < numIndices; i++)
			{
				idxRef[m.rawIndices[i]].emplace_back(i);
			}

			for (u32 i{ 0 }; i < numVertices; i++)
			{
				auto& refs{ idxRef[i] };
				u32 numRefs{ (u32)refs.size() };

				for (u32 j{ 0 }; j < numRefs; j++)
				{
					m.indices[refs[j]] = (u32)m.vertices.size();
					Vertex& v{ m.vertices.emplace_back() };
					v.position = m.positions[m.rawIndices[refs[j]]];

					// calculate where this is a hard edge or a soft edge
					XMVECTOR n1{ XMLoadFloat3(&m.normals[refs[j]]) };
					if (!isHardEdge)
					{
						for (u32 k{ j + 1 }; k < numRefs; k++)
						{
							// This value represents the cosine of the angle between the normals
							f32 cosTheta{ 0.0f };
							XMVECTOR n2{ XMLoadFloat3(&m.normals[refs[k]]) };
							if (!isSoftEdge)
							{
								// NOTE: n2 is already normalized, so it's lenth is 1, so we don't divide
								// by it's length to get the cosine
								// cos(angle) = dot(n1, n2) / (||n1|| * ||n2||)
								XMStoreFloat(&cosTheta, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
							}

							if (isSoftEdge || cosTheta >= cosAlpha)
							{
								n1 += n2;
								m.indices[refs[k]] = m.indices[refs[j]];
								refs.erase(refs.begin() + k);
								--numRefs;
								--k;
							}
						}
					}
					XMStoreFloat3(&v.normal, XMVector3Normalize(n1));
				}
			}
		}

		void ProcessUVs(Mesh& m)
		{
			Utils::vector<Vertex> oldVertices;
			oldVertices.swap(m.vertices);
			Utils::vector<u32> oldIndices;
			oldIndices.swap(m.indices);
			const u32 numIndices{ (u32)oldIndices.size() };
			const u32 numVertices{ (u32)oldVertices.size() };

			assert(numVertices && numIndices);

			Utils::vector<Utils::vector<u32>> idxRef(numVertices);

			for (u32 i{ 0 }; i < numIndices; i++)
			{
				idxRef[oldIndices[i]].emplace_back(i);
			}

			for (u32 i{ 0 }; i < numIndices; i++)
			{
				auto& refs{ idxRef[i] };
				u32 numRefs{ (u32)refs.size() };

				for (u32 j{ 0 }; j < numRefs; j++)
				{
					m.indices[refs[j]] = (u32)m.vertices.size();
					Vertex& v{ oldVertices[oldIndices[refs[j]]] };
					v.uv = m.uvSets[0][refs[j]];
					m.vertices.emplace_back(v);

					for (u32 k{ j + 1 }; k < numRefs; k++)
					{
						Vec2 uv1{ m.uvSets[0][refs[k]] };
						
						if (XMScalarNearEqual(v.uv.x, uv1.x, epsilon) &&
							XMScalarNearEqual(v.uv.y, uv1.y, epsilon))
						{
							m.indices[refs[k]] = m.indices[refs[j]];
							refs.erase(refs.begin() + k);
							--numRefs;
							--k;
						}
					}
				}
			}
		}

		void PackVerticesStatic(Mesh& m)
		{
			const u32 numVertices{ (u32)m.vertices.size() };

			assert(numVertices);

			m.packedVerticesStatic.reserve(numVertices);

			for (u32 i{ 0 }; i < numVertices; i++)
			{
				Vertex& v{ m.vertices[i] };
				const u8 signs{ (u8)((v.normal.z > 0.0f) << 1) };
				const u16 normalX{ (u16)PackFloat<16>(v.normal.x, -1.0f, 1.0f) };
				const u16 normalY{ (u16)PackFloat<16>(v.normal.y, -1.0f, 1.0f) };
				// TODO: pack tangents in sign and in x/y components

				m.packedVerticesStatic.emplace_back(PackedVertex::VertexStatic
					{
						v.position, {0, 0, 0}, signs, {normalX, normalY}, {}, v.uv
					});
			}
		}

		void ProcessVertices(Mesh& m, const GeometryImportSettings& settings)
		{
			assert((m.rawIndices.size() % 3) == 0);
			
			if (settings.calculateNormals || m.normals.empty())
			{
				RecalculateNormals(m);
			}

			ProcessNormals(m, settings.smoothingAngle);

			if (!m.uvSets.empty())
			{
				ProcessUVs(m);
			}

			PackVerticesStatic(m);
		}

		void PackMeshData(const Mesh& m, u8* const buffer, u64& at)
		{
			constexpr u32 su32{ sizeof(u32) };
			u32 s{ 0 };

			// Mesh name
			s = (u32)m.name.size();
			memcpy(&buffer[at], &s, su32); at += su32;
			memcpy(&buffer[at], m.name.c_str(), s); at += s;
			// LoD ID
			s = m.lodID;
			memcpy(&buffer[at], &s, su32); at += su32;
			// Vertex size
			constexpr u32 vertexSize{ sizeof(PackedVertex::VertexStatic) };
			s = vertexSize;
			memcpy(&buffer[at], &s, su32); at += su32;
			// Number of vertices
			const u32 numVertices{ (u32)m.vertices.size() };
			s = numVertices;
			memcpy(&buffer[at], &s, su32); at += su32;
			// Index size (16 bit or 32 bit)
			const u32 indexSize{ (numVertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
			s = indexSize;
			memcpy(&buffer[at], &s, su32); at += su32;
			// Number of indices
			const u32 numIndices{ (u32)m.indices.size() };
			s = numIndices;
			memcpy(&buffer[at], &s, su32); at += su32;
			// LoD threshold
			memcpy(&buffer[at], &m.lodThreshold, sizeof(f32)); at += sizeof(f32);
			// Vertex data
			s = vertexSize * numVertices;
			memcpy(&buffer[at], m.packedVerticesStatic.data(), s); at += s;
			// Index data
			s = indexSize * numIndices;
			void* data{ (void*)m.indices.data() };
			Utils::vector<u16> indices;

			if (indexSize == sizeof(u16))
			{
				indices.resize(numIndices);
				for (u32 i{ 0 }; i < numIndices; i++)
				{
					indices[i] = (u16)m.indices[i];
					data = (void*)indices.data();
				}
			}

			memcpy(&buffer[at], data, s); at += s;
		}

		u64 GetMeshSize(const Mesh& m)
		{
			const u64 numVertices{ m.vertices.size() };
			const u64 vertexBufferSize{ sizeof(PackedVertex::VertexStatic) * numVertices };
			const u64 indexSize{ (numVertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
			const u64 indexBufferSize{ indexSize * m.indices.size() };
			constexpr u64 su32{ sizeof(u32) };
			const u64 size
			{
				su32 +				// name length (number of characters)
				m.name.size() +		// room for mesh name string
				su32 +				// LoD id
				su32 +				// vertex size
				su32 +				// number of vertices
				su32 +				// index size (16 bit or 32 bit)
				su32 +				// number of indices
				sizeof(f32) +		// LoD threshold
				vertexBufferSize +	// room for the vertices
				indexBufferSize		// room for the indices
			};

			return size;
		}

		u64 GetSceneSize(const Scene& scene)
		{
			constexpr u64 su32{ sizeof(u32) };
			u64 size
			{
				su32 +				// name length (number of characters)
				scene.name.size() +	// room for scene name string
				su32				// number of LoDs
			};

			for (auto& lod : scene.lodGroups)
			{
				u64 lodSize
				{
					su32 + lod.name.size() +	// LoD name length (number of characters) and room for LoD name string
					su32						// number of meshes in this LoD
				};

				for (auto& m : lod.meshes)
				{
					lodSize += GetMeshSize(m);
				}

				size += lodSize;
			}

			return size;
		}
	} // anonymous namespace

	void ProcessScene(Scene& scene, GeometryImportSettings& settings)
	{
		for (auto& lod : scene.lodGroups)
		{
			for (auto& m : lod.meshes)
			{
				ProcessVertices(m, settings);
			}
		}
	}

	void PackData(const Scene& scene, SceneData& data)
	{
		constexpr u64 su32(sizeof(u32));
		const u64 sceneSize{ GetSceneSize(scene) };
		data.bufferSize = (u32)sceneSize;
		data.buffer = (u8*)CoTaskMemAlloc(sceneSize);
		assert(data.buffer);

		u8* const buffer{ data.buffer };
		u64 at{ 0 };
		u32 s{ 0 };

		// Scene name
		s = (u32)scene.name.size();
		memcpy(&buffer[at], &s, su32); at += su32;
		memcpy(&buffer[at], scene.name.c_str(), s); at += s;
		// Number of LoDs
		s = (u32)scene.lodGroups.size();
		memcpy(&buffer[at], &s, su32); at += su32;

		for (auto& lod : scene.lodGroups)
		{
			// LoD name
			s = (u32)lod.name.size();
			memcpy(&buffer[at], &s, su32); at += su32;
			memcpy(&buffer[at], lod.name.c_str(), s); at += s;
			// Number of meshes in this LoD
			s = (u32)lod.meshes.size();
			memcpy(&buffer[at], &s, su32); at += su32;

			for (auto& m : lod.meshes)
			{
				PackMeshData(m, buffer, at);
			}
		}

	}
}