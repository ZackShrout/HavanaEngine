#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace Havana::Tools
{
	namespace
	{
		using namespace Math;
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
				k++;
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
		{}

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