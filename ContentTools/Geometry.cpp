#include "Geometry.h"
#include "..\Engine\Utilities\IOStream.h"

namespace Havana::Tools
{
	namespace
	{
		using namespace Math;
		using namespace DirectX;

		void RecalculateNormals(Mesh& m)
		{
			const u32 numIndices{ (u32)m.rawIndices.size() };
			m.normals.resize(numIndices);

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
								numRefs--;
								k--;
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
			Utils::vector<u32> oldIndices(m.indices.size());
			oldIndices.swap(m.indices);
			const u32 numIndices{ (u32)oldIndices.size() };
			const u32 numVertices{ (u32)oldVertices.size() };

			assert(numVertices && numIndices);

			Utils::vector<Utils::vector<u32>> idxRef(numVertices);

			for (u32 i{ 0 }; i < numIndices; i++)
			{
				idxRef[oldIndices[i]].emplace_back(i);
			}

			for (u32 i{ 0 }; i < numVertices; i++)
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
							numRefs--;
							k--;
						}
					}
				}
			}
		}

		u64 GetVertexElementSize(Elements::ElementsType::Type elementsType)
		{
			using namespace Elements;

			switch (elementsType)
			{
			case ElementsType::StaticNormal:
				return sizeof(StaticNormal);
			case ElementsType::StaticNormalTexture:
				return sizeof(StaticNormalTexture);
			case ElementsType::StaticColor:
				return sizeof(StaticColor);
			case ElementsType::Skeletal:
				return sizeof(Skeletal);
			case ElementsType::SkeletalColor:
				return sizeof(SkeletalColor);
			case ElementsType::SkeletalNormal:
				return sizeof(SkeletalNormal);
			case ElementsType::SkeletalNormalColor:
				return sizeof(SkeletalNormalColor);
			case ElementsType::SkeletalNormalTexture:
				return sizeof(SkeletalNormalTexture);
			case ElementsType::SkeletalNormalTextureColor:
				return sizeof(SkeletalNormalTextureColor);
			}
			
			return 0;
		}

		void PackVertices(Mesh& m)
		{
			const u32 numVertices{ (u32)m.vertices.size() };
			assert(numVertices);

			m.positionBuffer.resize(sizeof(Math::Vec3) * numVertices);
			Math::Vec3* const positionBuffer{ (Math::Vec3* const)m.positionBuffer.data() };

			for (u32 i{ 0 }; i < numVertices; i++)
			{
				positionBuffer[i] = m.vertices[i].position;
			}

			struct u16v2 { u16 x, y; };
			struct u8v3 { u8 x, y, z; };

			Utils::vector<u8>		tSigns(numVertices);
			Utils::vector<u16v2>	normals(numVertices);
			Utils::vector<u16v2>	tangents(numVertices);
			Utils::vector<u8v3>		jointWeights(numVertices);

			if (m.elementsType & Elements::ElementsType::StaticNormal)
			{
				// normals only
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					tSigns[i] = (u8)((v.normal.z > 0.0f) << 1);
					normals[i] = { (u16)PackFloat<16>(v.normal.x, -1.0f, 1.0f), (u16)PackFloat<16>(v.normal.y, -1.0f, 1.0f) };
				}

				if (m.elementsType & Elements::ElementsType::StaticNormalTexture)
				{
					// full t-space
					for (u32 i{ 0 }; i < numVertices; i++)
					{
						Vertex& v{ m.vertices[i] };
						tSigns[i] |= (u8)((v.tangent.w > 0.0f) && (v.tangent.z > 0.0f));
						tangents[i] = { (u16)PackFloat<16>(v.tangent.x, -1.0f, 1.0f), (u16)PackFloat<16>(v.tangent.y, -1.0f, 1.0f) };
					}
				}
			}

			if (m.elementsType & Elements::ElementsType::Skeletal)
			{
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					// pack joint weights (from [0.0, 1.0] to [0...255])
					jointWeights[i] =
					{
						(u8)PackUnitFloat<8>(v.jointWeights.x),
						(u8)PackUnitFloat<8>(v.jointWeights.y),
						(u8)PackUnitFloat<8>(v.jointWeights.z)
					};
					// NOTE: w3 will be calculated in the shader since joint weights sum to one(1).
				}
			}

			m.elementBuffer.resize(GetVertexElementSize(m.elementsType) * numVertices);
			using namespace Elements;

			switch (m.elementsType)
			{
			case ElementsType::StaticColor:
			{
				StaticColor* const elementBuffer{ (StaticColor* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					elementBuffer[i] = { { v.red, v.green, v.blue }, {/*pad*/}};
				}
			}
				break;
			case ElementsType::StaticNormal:
			{
				StaticNormal* const elementBuffer{ (StaticNormal* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					elementBuffer[i] = { { v.red, v.green, v.blue }, tSigns[i], {normals[i].x, normals[i].y} };
				}
			}
				break;
			case ElementsType::StaticNormalTexture:
			{
				StaticNormalTexture* const elementBuffer{ (StaticNormalTexture* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					elementBuffer[i] = { { v.red, v.green, v.blue }, tSigns[i],
										 {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y},
										 v.uv };
				}
			}
				break;
			case ElementsType::Skeletal:
			{
				Skeletal* const elementBuffer{ (Skeletal* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.jointIndices.x, (u16)v.jointIndices.y, (u16)v.jointIndices.z, (u16)v.jointIndices.w };
					elementBuffer[i] = { {jointWeights[i].x, jointWeights[i].y, jointWeights[i].z}, {/*pad*/},
										 {indices[0], indices[1], indices[2], indices[3]} };
				}
			}
				break;
			case ElementsType::SkeletalColor:
			{
				SkeletalColor* const elementBuffer{ (SkeletalColor* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.jointIndices.x, (u16)v.jointIndices.y, (u16)v.jointIndices.z, (u16)v.jointIndices.w };
					elementBuffer[i] = { {jointWeights[i].x, jointWeights[i].y, jointWeights[i].z}, {/*pad*/},
										 {indices[0], indices[1], indices[2], indices[3]},
										 {v.red, v.green, v.blue}, {/*pad*/}};
				}
			}
				break;
			case ElementsType::SkeletalNormal:
			{
				SkeletalNormal* const elementBuffer{ (SkeletalNormal* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.jointIndices.x, (u16)v.jointIndices.y, (u16)v.jointIndices.z, (u16)v.jointIndices.w };
					elementBuffer[i] = { {jointWeights[i].x, jointWeights[i].y, jointWeights[i].z}, tSigns[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {normals[i].x, normals[i].y} };
				}
			}
				break;
			case ElementsType::SkeletalNormalColor:
			{
				SkeletalNormalColor* const elementBuffer{ (SkeletalNormalColor* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.jointIndices.x, (u16)v.jointIndices.y, (u16)v.jointIndices.z, (u16)v.jointIndices.w };
					elementBuffer[i] = { {jointWeights[i].x, jointWeights[i].y, jointWeights[i].z}, tSigns[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {normals[i].x, normals[i].y}, {v.red, v.green, v.blue}, {/*pad*/} };
				}
			}
				break;
			case ElementsType::SkeletalNormalTexture:
			{
				SkeletalNormalTexture* const elementBuffer{ (SkeletalNormalTexture* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.jointIndices.x, (u16)v.jointIndices.y, (u16)v.jointIndices.z, (u16)v.jointIndices.w };
					elementBuffer[i] = { {jointWeights[i].x, jointWeights[i].y, jointWeights[i].z}, tSigns[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y}, v.uv };
				}
			}
				break;
			case ElementsType::SkeletalNormalTextureColor:
			{
				SkeletalNormalTextureColor* const elementBuffer{ (SkeletalNormalTextureColor* const)m.elementBuffer.data() };
				for (u32 i{ 0 }; i < numVertices; i++)
				{
					Vertex& v{ m.vertices[i] };
					const u16 indices[4]{ (u16)v.jointIndices.x, (u16)v.jointIndices.y, (u16)v.jointIndices.z, (u16)v.jointIndices.w };
					elementBuffer[i] = { {jointWeights[i].x, jointWeights[i].y, jointWeights[i].z}, tSigns[i],
										 {indices[0], indices[1], indices[2], indices[3]},
										 {normals[i].x, normals[i].y}, {tangents[i].x, tangents[i].y}, v.uv,
										 {v.red, v.green, v.blue}, {/*pad*/} };
				}
			}
				break;
			}
		}

		void DetermineElementsType(Mesh& m)
		{
			using namespace Elements;

			if (m.normals.size())
			{
				if (m.uvSets.size() && m.uvSets[0].size())
				{
					m.elementsType = ElementsType::StaticNormalTexture;
				}
				else
				{
					m.elementsType = ElementsType::StaticNormal;
				}
			}
			else if (m.colors.size())
			{
				m.elementsType = ElementsType::StaticColor;
			}

			// TODO: We lack data for skeletal meshes. Expand for skeletal meshes later.
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

			DetermineElementsType(m);
			PackVertices(m);
		}

		void PackMeshData(const Mesh& m, Utils::BlobStreamWriter& blob)
		{
			// Mesh name
			blob.Write((u32)m.name.size());
			blob.Write(m.name.c_str(), m.name.size());
			// LoD ID
			blob.Write(m.lodID);
			// Vertex element size
			const u32 elementsSize{ (u32)GetVertexElementSize(m.elementsType) };
			blob.Write(elementsSize);
			// Elements type enumeration
			blob.Write((u32)m.elementsType);
			// Number of vertices
			const u32 numVertices{ (u32)m.vertices.size() };
			blob.Write(numVertices);
			// Index size (16 bit or 32 bit)
			const u32 indexSize{ (numVertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
			blob.Write(indexSize);
			// Number of indices
			const u32 numIndices{ (u32)m.indices.size() };
			blob.Write(numIndices);
			// LoD threshold
			blob.Write(m.lodThreshold);
			// Position buffer
			assert(m.positionBuffer.size() == sizeof(Math::Vec3) * numVertices);
			blob.Write(m.positionBuffer.data(), m.positionBuffer.size());
			// Element buffer
			assert(m.elementBuffer.size() == elementsSize * numVertices);
			blob.Write(m.elementBuffer.data(), m.elementBuffer.size());
			// Index data
			const u32 indexBufferSize{ indexSize * numIndices };
			const u8* data{ (const u8*)m.indices.data() };
			Utils::vector<u16> indices;

			if (indexSize == sizeof(u16))
			{
				indices.resize(numIndices);
				for (u32 i{ 0 }; i < numIndices; i++)
				{
					indices[i] = (u16)m.indices[i];
					data = (const u8*)indices.data();
				}
			}

			blob.Write(data, indexBufferSize);
		}

		u64 GetMeshSize(const Mesh& m)
		{
			const u64 numVertices{ m.vertices.size() };
			const u64 positionBufferSize{ m.positionBuffer.size() };
			assert(positionBufferSize == sizeof(Math::Vec3) * numVertices);
			const u64 elementBufferSize{ m.elementBuffer.size() };
			assert(elementBufferSize == GetVertexElementSize(m.elementsType) * numVertices);
			const u64 indexSize{ (numVertices < (1 << 16)) ? sizeof(u16) : sizeof(u32) };
			const u64 indexBufferSize{ indexSize * m.indices.size() };
			constexpr u64 su32{ sizeof(u32) };
			const u64 size
			{
				su32 +					// name length (number of characters)
				m.name.size() +			// room for mesh name string
				su32 +					// LoD id
				su32 +					// vertex element size (vertex size excluding position element)
				su32 +					// element type enumeration
				su32 +					// number of vertices
				su32 +					// index size (16 bit or 32 bit)
				su32 +					// number of indices
				sizeof(f32) +			// LoD threshold
				positionBufferSize +	// room for verticex positions
				elementBufferSize +		// room for verticex elements
				indexBufferSize			// room for the indices
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

		bool SplitMeshesByMaterial(u32 materialIdx, const Mesh& m, Mesh& submesh)
		{
			submesh.name = m.name;
			submesh.lodThreshold = m.lodThreshold;
			submesh.lodID = m.lodID;
			submesh.materialUsed.emplace_back(materialIdx);
			submesh.uvSets.resize(m.uvSets.size());

			const u32 numPolys{ (u32)m.rawIndices.size() / 3 };
			Utils::vector<u32> vertexRef(m.positions.size(), U32_INVALID_ID);

			for (u32 i{ 0 }; i < numPolys; i++)
			{
				const u32 mtlIdx{ m.materialIndices[i] };
				if (mtlIdx != materialIdx) continue;

				const u32 index{ i * 3 };
				for (u32 j = index; j < index + 3; j++)
				{
					const u32 vIdx{ m.rawIndices[j] };
					if (vertexRef[vIdx] != U32_INVALID_ID)
					{
						submesh.rawIndices.emplace_back(vertexRef[vIdx]);
					}
					else
					{
						submesh.rawIndices.emplace_back((u32)submesh.positions.size());
						vertexRef[vIdx] = submesh.rawIndices.back();
						submesh.positions.emplace_back(m.positions[vIdx]);
					}

					if (m.normals.size())
					{
						submesh.normals.emplace_back(m.normals[j]);
					}

					if (m.tangents.size())
					{
						submesh.tangents.emplace_back(m.tangents[j]);
					}

					for (u32 k{ 0 }; k < m.uvSets.size(); k++)
					{
						if (m.uvSets[k].size())
						{
							submesh.uvSets[k].emplace_back(m.uvSets[k][j]);
						}
					}
				}
			}

			assert((submesh.rawIndices.size() % 3) == 0);
			return !submesh.rawIndices.empty();
		}

		void SplitMeshesByMaterial(Scene& scene)
		{
			for (auto& lod : scene.lodGroups)
			{
				Utils::vector<Mesh> newMeshes;
				for (auto& m : lod.meshes)
				{
					// If moew than one material is used in this mesh
					// then split it into submeshes
					const u32 numMaterials{ (u32)m.materialUsed.size() };
					if (numMaterials > 1)
					{
						for (u32 i{ 0 }; i < numMaterials; i++)
						{
							Mesh submesh{};
							if (SplitMeshesByMaterial(m.materialUsed[i], m, submesh))
							{
								newMeshes.emplace_back(submesh);
							}
						}
					}
					else
					{
						newMeshes.emplace_back(m);
					}
				}

				newMeshes.swap(lod.meshes);
			}
		}
	} // anonymous namespace

	void ProcessScene(Scene& scene, const GeometryImportSettings& settings)
	{
		SplitMeshesByMaterial(scene);
		
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

		Utils::BlobStreamWriter blob{ data.buffer, data.bufferSize };

		// Scene name
		blob.Write((u32)scene.name.size());
		blob.Write(scene.name.c_str(), scene.name.size());
		// Number of LoDs
		blob.Write((u32)scene.lodGroups.size());

		for (auto& lod : scene.lodGroups)
		{
			// LoD name
			blob.Write((u32)lod.name.size());
			blob.Write(lod.name.c_str(), lod.name.size());
			// Number of meshes in this LoD
			blob.Write((u32)lod.meshes.size());

			for (auto& m : lod.meshes)
			{
				PackMeshData(m, blob);
			}
		}

		assert(sceneSize == blob.Offset());
	}
}