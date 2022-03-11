#include "FbxImporter.h"
#include "Geometry.h"

// If you get any compilation or linker errors, make sure that:
// 1) FBX SDK 2020.2 or later is installed on your system
// 2) The include path to fbxsdk.h is added to "Additional Include Directories" (compiler settins)
// 3) The library paths in the following section point to the correct location
#if _DEBUG
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.2.1/lib/vs2019/x64/debug/libfbxsdk-md.lib")
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.2.1/lib/vs2019/x64/debug/libxml2-md.lib")
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.2.1/lib/vs2019/x64/debug/zlib-md.lib")
#else
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.2.1/lib/vs2019/x64/release/libfbxsdk-md.lib")
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.2.1/lib/vs2019/x64/release/libxml2-md.lib")
#pragma comment(lib, "C:/Program Files/Autodesk/FBX/FBX SDK/2020.2.1/lib/vs2019/x64/release/zlib-md.lib")
#endif

namespace Havana::Tools
{
	namespace
	{
		std::mutex fbxMutex{};
	} // anonymous namespace

	bool FbxContext::InitializeFbx()
	{
		assert(!IsValid());

		m_fbxManager = FbxManager::Create();
		if (!m_fbxManager)
		{
			return false;
		}

		FbxIOSettings* ios{ FbxIOSettings::Create(m_fbxManager, IOSROOT) };
		assert(ios);
		m_fbxManager->SetIOSettings(ios);

		return true;
	}

	void FbxContext::LoadFbxFile(const char* file)
	{
		assert(m_fbxManager && !m_fbxScene);
		m_fbxScene = FbxScene::Create(m_fbxManager, "Importer Scene");
		if (!m_fbxScene)
		{
			return;
		}

		FbxImporter* importer{ FbxImporter::Create(m_fbxManager, "Importer") };
		if (!(importer &&
			importer->Initialize(file, -1, m_fbxManager->GetIOSettings()) &&
			importer->Import(m_fbxScene)))
		{
			return;
		}

		importer->Destroy();

		// Get scene scale in meters
		m_sceneScale = (f32)m_fbxScene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m);
	}

	void FbxContext::GetScene(FbxNode* root /*= nullptr*/)
	{
		assert(IsValid());

		if (!root)
		{
			root = m_fbxScene->GetRootNode();
			if (!root) return;
		}

		const s32 numNodes{ root->GetChildCount() };
		for (s32 i{ 0 }; i < numNodes; i++)
		{
			FbxNode* node{ root->GetChild(i) };
			if (!node) continue;

			if (node->GetMesh())
			{
				LoDGroup lod{};
				GetMesh(node, lod.meshes);
				if (lod.meshes.size())
				{
					lod.name = lod.meshes[0].name;
					m_scene->lodGroups.emplace_back(lod);
				}
			}
			else if (node->GetLodGroup())
			{
				GetLoDGroup(node);
			}
			else
			{
				// See if there is a mesh somewhere further down the hierarchy
				GetScene(node);
			}
		}
	}

	void FbxContext::GetMesh(FbxNode* node, Utils::vector<Mesh>& meshes)
	{
		assert(node);

		if (FbxMesh* fbxMesh{ node->GetMesh() })
		{
			if (fbxMesh->RemoveBadPolygons() < 0) return;

			// Triangulate the mesh if needed
			FbxGeometryConverter gc{ m_fbxManager };
			fbxMesh = static_cast<FbxMesh*>(gc.Triangulate(fbxMesh, true));
			if (!fbxMesh || fbxMesh->RemoveBadPolygons() < 0) return;

			Mesh m;
			m.lodID = (u32)meshes.size();
			m.lodThreshold = -1.0f;
			m.name = (node->GetName()[0] != '\0') ? node->GetName() : fbxMesh->GetName();

			if (GetMeshData(fbxMesh, m))
			{
				meshes.emplace_back(m);
			}
		}
	}

	void FbxContext::GetLoDGroup(FbxNode* node)
	{
	}

	EDITOR_INTERFACE void ImportFbx(const char* file, SceneData* data)
	{
		assert(file && data);
		Scene scene{};

		// NOTE: anything that involves using the FBX SDK should be single-threaded
		{
			std::lock_guard lock{ fbxMutex };
			FbxContext fbxContext{ file, &scene, data };
			if (fbxContext.IsValid())
			{
				// Get scene
			}
			else
			{
				// TODO: send failure log message to editor
				return;
			}
		}

		ProcessScene(scene, data->settings);
		PackData(scene, *data);

	}
}