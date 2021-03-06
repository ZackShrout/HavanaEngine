#include "FbxImporter.h"
#include "Geometry.h"

// If you get any compilation or linker errors, make sure that:
// 1) FBX SDK 2020.2 or later is installed on your system
// 2) The include path to fbxsdk.h is added to "Additional Include Directories" (compiler settings)
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
// LNK4899 PDB not found warnings can be resolved either by installing FBX SDK PDBs (separate download) or
// by disabling this warning in linker options (Linker command line: /ignore:4899).

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

			LoDGroup lod{};
			GetMeshes(node, lod.meshes, 0, -1.0f);
			if (lod.meshes.size())
			{
				lod.name = lod.meshes[0].name;
				m_scene->lodGroups.emplace_back(lod);
			}
		}
	}

	void FbxContext::GetMeshes(FbxNode* node, Utils::vector<Mesh>& meshes, u32 lodID, f32 lodThreshold)
	{
		assert(node && lodID != U32_INVALID_ID);
		bool isLODGroup{ false };

		if (const s32 numAttributes{ node->GetNodeAttributeCount() })
		{
			for (s32 i{ 0 }; i < numAttributes; i++)
			{
				FbxNodeAttribute* attribute{ node->GetNodeAttributeByIndex(i) };
				const FbxNodeAttribute::EType attributeType{ attribute->GetAttributeType() };
				if (attributeType == FbxNodeAttribute::eMesh)
				{
					GetMesh(attribute, meshes, lodID, lodThreshold);
				}
				else if (attributeType == FbxNodeAttribute::eLODGroup)
				{
					GetLoDGroup(attribute);
					isLODGroup = true;
				}
			}
		}

		if (!isLODGroup)
		{
			if (const s32 numChildren{ node->GetChildCount() })
			{
				for (s32 i{ 0 }; i < numChildren; i++)
				{
					GetMeshes(node->GetChild(i), meshes, lodID, lodThreshold);
				}
			}
		}
	}

	void FbxContext::GetMesh(FbxNodeAttribute* attribute, Utils::vector<Mesh>& meshes, u32 lodID, f32 lodThreshold)
	{
		assert(attribute);

		FbxMesh* fbxMesh{ (FbxMesh*)attribute };
		if (fbxMesh->RemoveBadPolygons() < 0) return;

		// Triangulate the mesh if needed
		FbxGeometryConverter gc{ m_fbxManager };
		fbxMesh = (FbxMesh*)gc.Triangulate(fbxMesh, true);
		if (!fbxMesh || fbxMesh->RemoveBadPolygons() < 0) return;

		FbxNode* const node{ fbxMesh->GetNode() };

		Mesh m;
		m.lodID = lodID;
		m.lodThreshold = lodThreshold;
		m.name = (node->GetName()[0] != '\0') ? node->GetName() : fbxMesh->GetName();

		if (GetMeshData(fbxMesh, m))
		{
			meshes.emplace_back(m);
		}

		// See if there is a mesh somewhere further down the hierarchy
		GetScene(node);
	}

	void FbxContext::GetLoDGroup(FbxNodeAttribute* attribute)
	{
		assert(attribute);

		FbxLODGroup* lodGrp{ (FbxLODGroup*)attribute };
		FbxNode* const node{ lodGrp->GetNode() };
		LoDGroup lod{};
		lod.name = (node->GetName()[0] != '\0' ? node->GetName() : lodGrp->GetName());
		// NOTE: number of LODs is exlusive the base mesh (LOD 0)
		const s32 numNodes{ node->GetChildCount() };
		assert(numNodes > 0 && lodGrp->GetNumThresholds() == (numNodes - 1));

		for (s32 i{ 0 }; i < numNodes; i++)
		{
			f32 lodThreshold{ -1.0f };
			if (i > 0)
			{
				FbxDistance threshold;
				lodGrp->GetThreshold(i - 1, threshold);
				lodThreshold = threshold.value() * m_sceneScale;
			}
			
			GetMeshes(node->GetChild(i), lod.meshes, (u32)lod.meshes.size(), lodThreshold);
		}

		if (lod.meshes.size()) m_scene->lodGroups.emplace_back(lod);
	}

	bool FbxContext::GetMeshData(FbxMesh* fbxMesh, Mesh& m)
	{
		assert(fbxMesh);

		FbxNode* const node{ fbxMesh->GetNode() };
		FbxAMatrix geometricTransform;

		geometricTransform.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
		geometricTransform.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
		geometricTransform.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));

		FbxAMatrix transform{ node->EvaluateGlobalTransform() * geometricTransform };
		FbxAMatrix inverseTranspose{ transform.Inverse().Transpose() };

		const s32 numPolys{ fbxMesh->GetPolygonCount() };
		if (numPolys <= 0) return false;

		// Get vertices
		const s32 numVertices{ fbxMesh->GetControlPointsCount() };
		FbxVector4* vertices{ fbxMesh->GetControlPoints() };
		const s32 numIndices{ fbxMesh->GetPolygonVertexCount() };
		s32* indices{ fbxMesh->GetPolygonVertices() };

		assert(numVertices > 0 && vertices && numIndices > 0 && indices);
		if (!(numVertices > 0 && vertices && numIndices > 0 && indices)) return false;

		m.rawIndices.resize(numIndices);
		Utils::vector vertexRef(numVertices, U32_INVALID_ID);

		for (s32 i{ 0 }; i < numIndices; i++)
		{
			const u32 vIdx{ (u32)indices[i] };
			// Did we encounter this vertex before? If so, just add it's index.
			// If not, add the vertex and a new index.
			if (vertexRef[vIdx] != U32_INVALID_ID)
			{
				m.rawIndices[i] = vertexRef[vIdx];
			}
			else
			{
				FbxVector4 v = transform.MultT(vertices[vIdx]) * m_sceneScale;
				m.rawIndices[i] = (u32)m.positions.size();
				vertexRef[vIdx] = m.rawIndices[i];
				m.positions.emplace_back((f32)v[0], (f32)v[1], (f32)v[2]);
			}
		}

		assert(m.rawIndices.size() % 3 == 0);

		// Get material index per polygon
		assert(numPolys > 0);
		FbxLayerElementArrayTemplate<s32>* mtlIndices;
		if (fbxMesh->GetMaterialIndices(&mtlIndices))
		{
			for (s32 i{ 0 }; i < numPolys; i++)
			{
				const s32 mtlIndex{ mtlIndices->GetAt(i) };
				assert(mtlIndex >= 0);
				m.materialIndices.emplace_back((u32)mtlIndex);
				if (std::find(m.materialUsed.begin(), m.materialUsed.end(), (u32)mtlIndex) == m.materialUsed.end())
				{
					m.materialUsed.emplace_back((u32)mtlIndex);
				}
			}
		}

		// Importing normals in ON by default
		const bool importNormals{ !m_sceneData->settings.calculateNormals };
		// Importing tangents is OFF by default
		const bool importTangents{ !m_sceneData->settings.calculateTangents };

		// Import normals
		if (importNormals)
		{
			FbxArray<FbxVector4> normals;
			// Calculate normals using FBX's built-in method, but only if no normal data is already there
			if (fbxMesh->GenerateNormals() && fbxMesh->GetPolygonVertexNormals(normals) && normals.Size() > 0)
			{
				const s32 numNormals{ normals.Size() };
				for (s32 i{ 0 }; i < numNormals; i++)
				{
					FbxVector4 n{ inverseTranspose.MultT(normals[i]) };
					n.Normalize();
					m.normals.emplace_back((f32)n[0], (f32)n[1], (f32)n[2]);
				}
			}
			else
			{
				// Something went wrong with importing normals from FBX.
				// Fall back to our own calculation method. This will cause us to lose
				// information regarding which edges are hard and soft
				m_sceneData->settings.calculateNormals = true;
			}
		}

		// Import tangents
		if (importTangents)
		{
			FbxLayerElementArrayTemplate<FbxVector4>* tangents{ nullptr };
			// Calculate tangents using FBX's built-in method, but only if no tangent data is already there.
			if (fbxMesh->GenerateTangentsData() && fbxMesh->GetTangents(&tangents) && tangents && tangents->GetCount() > 0)
			{
				const s32 numTangents{ tangents->GetCount() };
				for (s32 i{0}; i < numTangents; i++)
				{
					// TODO: not sure if this transformation is correct.
					FbxVector4 t{ tangents->GetAt(i) };
					const f32 handedness{ (f32)t[3] };
					t[3] = 0.0;
					t = transform.MultT(t);
					t.Normalize();
					m.tangents.emplace_back((f32)t[0], (f32)t[1], (f32)t[2], handedness);
				}
			}
			else
			{
				// Something went wrong with importing tangents from FBX.
				// Fall back to our own calculation method.
				m_sceneData->settings.calculateTangents = true;
			}
		}

		// Get UV coordinates
		FbxStringList uvNames;
		fbxMesh->GetUVSetNames(uvNames);
		const s32 uvSetCount{ uvNames.GetCount() };
		// NOTE: it's OK if we don't have a uv set. For example, some emissive objects don't need a uv map
		m.uvSets.resize(uvSetCount);

		for (s32 i{ 0 }; i < uvSetCount; i++)
		{
			FbxArray<FbxVector2> uvs;
			if (fbxMesh->GetPolygonVertexUVs(uvNames.GetStringAt(i), uvs))
			{
				const s32 numUVs{ uvs.Size() };
				for (s32 j{ 0 }; j < numUVs; j++)
				{
					m.uvSets[i].emplace_back((f32)uvs[j][0], (f32)uvs[j][1]);
				}
			}
		}

		return true;
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
				fbxContext.GetScene();
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
} // namespace Havana::Tools