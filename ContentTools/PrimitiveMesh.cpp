#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace Havana::Tools
{
	namespace
	{
		using namespace Math;
		using namespace DirectX;
		using primitive_mesh_creator = void(*)(Scene&, const PrimitiveInitInfo& info);

		void CreatePlane(Scene& scene, const PrimitiveInitInfo& info);
		void CreateCube(Scene& scene, const PrimitiveInitInfo& info);
		void CreateUVSphere(Scene& scene, const PrimitiveInitInfo& info);
		void CreateICOSphere(Scene& scene, const PrimitiveInitInfo& info);
		void CreateCylinder(Scene& scene, const PrimitiveInitInfo& info);
		void CreateCapsule(Scene& scene, const PrimitiveInitInfo& info);

		primitive_mesh_creator creators[]
		{
			CreatePlane,
			CreateCube,
			CreateUVSphere,
			CreateICOSphere,
			CreateCylinder,
			CreateCapsule
		};
		
		static_assert(_countof(creators) == PrimitiveMeshType::Count);

		struct Axis
		{
			enum: u32 
			{
				x = 0,
				y = 1,
				z = 2
			};
		};

		Mesh CreatePlane(const PrimitiveInitInfo& info,
			u32 horizontalIndex = Axis::x, u32 verticalIndex = Axis::z, bool flipWinding = false,
			Vec3 offset = { -0.5f, 0.0f, -0.5f }, Vec2 uRange = { 0.0f, 1.0f }, Vec2 vRange = { 0.0f, 1.0f })
		{
			assert(horizontalIndex < 3 && verticalIndex < 3);
			assert(horizontalIndex != verticalIndex);

			const u32 horizontalCount{ clamp(info.segments[horizontalIndex], 1u, 10u) };
			const u32 verticalCount{ clamp(info.segments[verticalIndex], 1u, 10u) };
			const f32 horizontalStep{ 1.0f / horizontalCount };
			const f32 verticalStep{ 1.0f / verticalCount };
			const f32 uStep{ (uRange.y - uRange.x) / horizontalCount };
			const f32 vStep{ (vRange.y - vRange.x) / verticalCount };

			Mesh m{};
			Utils::vector<Vec2> uvs;

			for (u32 j{ 0 }; j <= verticalCount; j++)
			{
				for (u32 i{ 0 }; i <= horizontalCount; i++)
				{
					Vec3 position{ offset };
					f32* const as_array{ &position.x };
					as_array[horizontalIndex] += i * horizontalStep;
					as_array[verticalIndex] += j * verticalStep;
					m.positions.emplace_back(position.x * info.size.x, position.y * info.size.y, position.z * info.size.z);

					Vec2 uv{ uRange.x, 1.0f - vRange.x };
					uv.x += i * uStep;
					uv.y -= j * vStep;
					uvs.emplace_back(uv);
				}
			}

			assert(m.positions.size() == (((u64)horizontalCount + 1)*((u64)verticalCount + 1)));
			
			const u32 rowLength{ horizontalCount + 1 }; // number of vertices in a row
			
			for (u32 j{ 0 }; j < verticalCount; j++)
			{
				u32 k{ 0 };
				for (u32 i{ k }; i < horizontalCount; i++)
				{
					const u32 index[4]
					{
						i + j * rowLength,
						i + (j + 1) * rowLength,
						(i + 1) + j * rowLength,
						(i + 1) + (j + 1) * rowLength,
					};

					// Triangle 1
					m.rawIndices.emplace_back(index[0]);
					m.rawIndices.emplace_back(index[flipWinding ? 2 : 1]);
					m.rawIndices.emplace_back(index[flipWinding ? 1 : 2]);

					// Triangle 2
					m.rawIndices.emplace_back(index[2]);
					m.rawIndices.emplace_back(index[flipWinding ? 3 : 1]);
					m.rawIndices.emplace_back(index[flipWinding ? 1 : 3]);
				}
				++k;
			}

			const u32 numIndices{ 3 * 2 * horizontalCount * verticalCount };
			assert(m.rawIndices.size() == numIndices);

			m.uvSets.resize(1);

			for (u32 i{ 0 }; i < numIndices; i++)
			{
				m.uvSets[0].emplace_back(uvs[m.rawIndices[i]]);
			}

			return m;
		}

		Mesh CreateUVSphere(const PrimitiveInitInfo& info)
		{
			const u32 phiCount{ clamp(info.segments[Axis::x], 3u, 64u) };
			const u32 thetaCount{ clamp(info.segments[Axis::y], 2u, 64u) };
			const f32 thetaStep{ pi / thetaCount };
			const f32 phiStep{ twoPi / phiCount };
			const u32 numVertices{ 2 + phiCount * (thetaCount - 1) };
			const u32 numIndices{ 2 * 3 * phiCount + 2 * 3 * phiCount * (thetaCount - 2) };

			Mesh m{};
			m.name = "uvSphere";
			m.positions.resize(numVertices);

			// Add top vertex
			u32 c{ 0 };
			m.positions[c++] = { 0.0f, info.size.y, 0.0f };

			// Add the body of the vertices
			for (u32 j{ 1 }; j < thetaCount; j++)
			{
				const f32 theta{ j * thetaStep };
				for (u32 i{ 0 }; i < phiCount; i++)
				{
					const f32 phi{ i * phiStep };
					m.positions[c++] =
					{
						info.size.x * XMScalarSin(theta) * XMScalarCos(phi),
						info.size.y * XMScalarCos(theta),
						-info.size.z * XMScalarSin(theta) * XMScalarSin(phi)
					};
				}
			}

			// Add bottom vertex
			m.positions[c++] = { 0.0f, -info.size.y, 0.0f };
			
			assert(numVertices == c);

			c = 0;
			m.rawIndices.resize(numIndices);
			Utils::vector<Vec2> uvs(numIndices);
			const f32 inverseThetaCount{ 1.0f / thetaCount };
			const f32 inversePhiCount{ 1.0f / phiCount };

			// Indices for top cap, connecting the north pole to the first ring
			// UV Coords at the same time
			for (u32 i{ 0 }; i < (phiCount - 1); i++)
			{
				uvs[c] = { (2 * i + 1) * 0.5f * inversePhiCount, 1.0f };
				m.rawIndices[c++] = 0;
				uvs[c] = { i * inversePhiCount, 1.0f - inverseThetaCount };
				m.rawIndices[c++] = i + 1;
				uvs[c] = { (i + 1) * inversePhiCount, 1.0f - inverseThetaCount };
				m.rawIndices[c++] = i + 2;
			}
			
			uvs[c] = { 1.0f - 0.5f * inversePhiCount, 1.0f };
			m.rawIndices[c++] = 0;
			uvs[c] = { 1.0f - inversePhiCount, 1.0f - inverseThetaCount };
			m.rawIndices[c++] = phiCount;
			uvs[c] = { 1.0f, 1.0f - inverseThetaCount };
			m.rawIndices[c++] = 1;

			// Indices for the section between the top and bottom rings
			// UV Coords at the same time
			for (u32 j{ 0 }; j < (thetaCount - 2); j++)
			{
				for (u32 i{ 0 }; i < (phiCount - 1); i++)
				{
					const u32 index[4]
					{
						1 + i + j * phiCount,
						1 + i + (j + 1) * phiCount,
						1 + (i + 1) + (j + 1) * phiCount,
						1 + (i + 1) + j * phiCount
					};

					// Triangle 1
					uvs[c] = { i * inversePhiCount, 1.0f - (j + 1) * inverseThetaCount };
					m.rawIndices[c++] = index[0];
					uvs[c] = { i * inversePhiCount, 1.0f - (j + 2) * inverseThetaCount };
					m.rawIndices[c++] = index[1];
					uvs[c] = { (i + 1) * inversePhiCount, 1.0f - (j + 2) * inverseThetaCount };
					m.rawIndices[c++] = index[2];

					// Triangle 2
					uvs[c] = { i * inversePhiCount, 1.0f - (j + 1) * inverseThetaCount };
					m.rawIndices[c++] = index[0];
					uvs[c] = { (i + 1) * inversePhiCount, 1.0f - (j + 2) * inverseThetaCount };
					m.rawIndices[c++] = index[2];
					uvs[c] = { (i + 1) * inversePhiCount, 1.0f - (j + 1) * inverseThetaCount };
					m.rawIndices[c++] = index[3];
				}
				
				const u32 index[4]
				{
					phiCount + j * phiCount,
					phiCount + (j + 1) * phiCount,
					1 + (j + 1) * phiCount,
					1 + j * phiCount
				};
				
				// Triangle 1
				uvs[c] = { 1.0f - inversePhiCount, 1.0f - (j + 1) * inverseThetaCount };
				m.rawIndices[c++] = index[0];
				uvs[c] = { 1.0f - inversePhiCount, 1.0f - (j + 2) * inverseThetaCount };
				m.rawIndices[c++] = index[1];
				uvs[c] = { 1.0f, 1.0f - (j + 2) * inverseThetaCount };
				m.rawIndices[c++] = index[2];

				// Triangle 2
				uvs[c] = { 1.0f - inversePhiCount, 1.0f - (j + 1) * inverseThetaCount };
				m.rawIndices[c++] = index[0];
				uvs[c] = { 1.0f, 1.0f - (j + 2) * inverseThetaCount };
				m.rawIndices[c++] = index[2];
				uvs[c] = { 1.0f, 1.0f - (j + 1) * inverseThetaCount };
				m.rawIndices[c++] = index[3];
			}

			// Indices for bottom cap, connecting the south pole to the last ring
			// UV Coords at the same time
			const u32 southPoleIndex{ (u32)m.positions.size() - 1 };
			for (u32 i{ 0 }; i < (phiCount - 1); i++)
			{
				uvs[c] = { (2 * i + 1) * 0.5f * inversePhiCount, 0.0f };
				m.rawIndices[c++] = southPoleIndex;
				uvs[c] = { (i + 1) * inversePhiCount, inverseThetaCount };
				m.rawIndices[c++] = southPoleIndex - phiCount + i + 1;
				uvs[c] = { i * inversePhiCount, inverseThetaCount };
				m.rawIndices[c++] = southPoleIndex - phiCount + i;
			}

			uvs[c] = { 1.0f - 0.5f * inversePhiCount, 0.0f };
			m.rawIndices[c++] = southPoleIndex;
			uvs[c] = { 1.0f, inverseThetaCount };
			m.rawIndices[c++] = southPoleIndex - phiCount;
			uvs[c] = { 1.0f - inversePhiCount, inverseThetaCount };
			m.rawIndices[c++] = southPoleIndex - 1;

			assert(c == numIndices);

			m.uvSets.emplace_back(uvs);

			return m;
		}

		void CreatePlane(Scene& scene, const PrimitiveInitInfo& info)
		{
			LoDGroup lod{};
			lod.name = "plane";
			lod.meshes.emplace_back(CreatePlane(info));
			scene.lodGroups.emplace_back(lod);
		}

		void CreateCube(Scene& scene, const PrimitiveInitInfo& info)
		{}

		void CreateUVSphere(Scene& scene, const PrimitiveInitInfo& info)
		{
			LoDGroup lod{};
			lod.name = "uvSphere";
			lod.meshes.emplace_back(CreateUVSphere(info));
			scene.lodGroups.emplace_back(lod);
		}

		void CreateICOSphere(Scene& scene, const PrimitiveInitInfo& info)
		{}

		void CreateCylinder(Scene& scene, const PrimitiveInitInfo& info)
		{}

		void CreateCapsule(Scene& scene, const PrimitiveInitInfo& info)
		{}

	} // anonymous namespace

	EDITOR_INTERFACE void CreatePrimitiveMesh(SceneData* data, PrimitiveInitInfo* info)
	{
		assert(data && info);
		assert(info->type < PrimitiveMeshType::Count);
		Scene scene{};
		creators[info->type](scene, *info);

		// TODO: process scene and pack to be sent to the Editor
		data->settings.calculateNormals = 1;
		ProcessScene(scene, data->settings);
		PackData(scene, *data);
	}
}