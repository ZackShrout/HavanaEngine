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
			const f32 cosAngle{ XMScalarCos(pi - smoothingAngle * pi / 180.0f) };
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
							f32 n{ 0.0f };
							XMVECTOR n2{ XMLoadFloat3(&m.normals[refs[k]]) };
							if (!isSoftEdge)
							{
								// NOTE: n2 is already normalized, so it's lenth is 1, so we don't divide
								// by it's length to get the cosine
								// cos(angle) = dot(n1, n2) / (||n1|| * ||n2||)
								XMStoreFloat(&n, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
							}

							if (isSoftEdge || n >= cosAngle)
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

	}
}