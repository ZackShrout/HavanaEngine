#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace Havana::Tools
{
	namespace
	{
		using primitive_mesh_creator = void(*)(Scene&, const PrimitiveInitInfo& info);

		void CreatePlane(Scene&, const PrimitiveInitInfo& info);
		void CreateCube(Scene&, const PrimitiveInitInfo& info);
		void CreateUVSphere(Scene&, const PrimitiveInitInfo& info);
		void CreateICOSphere(Scene&, const PrimitiveInitInfo& info);
		void CreateCylinder(Scene&, const PrimitiveInitInfo& info);
		void CreateCapsule(Scene&, const PrimitiveInitInfo& info);

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
	} // anonymous namespace

	EDITOR_INTERFACE void CreatePrimitiveMesh(SceneData* data, PrimitiveInitInfo* info)
	{
		assert(data && info);
		assert(info->type < PrimitiveMeshType::Count);
		Scene scene{};
	}
}