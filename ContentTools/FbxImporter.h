#pragma once
#include "ToolsCommon.h"
#include <fbxsdk.h>

namespace Havana::Tools
{
	struct SceneData;
	struct Scene;
	struct Mesh;
	struct GeometryImportSettings;
	
	class FbxContext
	{
	public:
		FbxContext(const char* file, Scene* scene, SceneData* data) : m_scene{ scene }, m_sceneData{ data }
		{
			assert(file&& m_scene&& m_sceneData);
			if (InitializeFbx())
			{
				LoadFbxFile(file);
				assert(IsValid());
			}
		}
		~FbxContext()
		{
			m_fbxScene->Destroy();
			m_fbxManager->Destroy();
			ZeroMemory(this, sizeof(FbxContext));
		}
		void GetScene(FbxNode* root = nullptr);
		constexpr bool IsValid() const { return m_fbxManager && m_fbxScene; }
		constexpr f32 SceneScale() const { return m_sceneScale; }
	private:
		bool InitializeFbx();
		void LoadFbxFile(const char* file);
		void GetMesh(FbxNode* node, Utils::vector<Mesh>& meshes);
		void GetLoDGroup(FbxNode* node);
		bool GetMeshData(FbxMesh* fbxMesh, Mesh& m);

		Scene*			m_scene{ nullptr };
		SceneData*		m_sceneData{ nullptr };
		FbxManager*		m_fbxManager{ nullptr };
		FbxScene*		m_fbxScene{ nullptr };
		f32				m_sceneScale{ 1.0f };
	};
}