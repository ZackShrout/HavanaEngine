#pragma once
#include "ToolsCommon.h"

namespace Havana::Tools
{
	namespace PackedVertex
	{
		struct VertexStatic
		{
			Math::Vec3	position;
			u8			reserved[3];
			u8			tSign; // bit 0: tangent handidness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1)
			u16			normal[2];
			u16			tangent[2];
			Math::Vec2	uv;
		};
	} // namespace PackedVertex
	
	struct Vertex
	{
		Math::Vec4 tangent{};
		Math::Vec3 position{};
		Math::Vec3 normal{};
		Math::Vec2 uv{};
	};
	
	struct Mesh
	{
		// Initial data
		Utils::vector<Math::Vec3>					positions;
		Utils::vector<Math::Vec3>					normals;
		Utils::vector<Math::Vec4>					tangents;
		Utils::vector<Utils::vector<Math::Vec2>>	uvSets;
		Utils::vector<u32>							rawIndices;

		// Intermediate data
		Utils::vector<Vertex>						vertices;
		Utils::vector<u32>							indices;

		// Output data
		std::string									name;
		Utils::vector<PackedVertex::VertexStatic>	packedVerticesStatic;
		f32											lodThreshold{ -1.0f };
		u32											lodID{ U32_INVALID_ID };
	};
	
	struct LoDGroup
	{
		std::string			name;
		Utils::vector<Mesh> meshes;
	};
	
	struct Scene
	{
		std::string				name;
		Utils::vector<LoDGroup> lodGroups;
	};
	
	struct GeometryImportSettings
	{
		f32 smoothingAngle;
		u8  calculateNormals;
		u8  calculateTangents;
		u8  reverseHandedness;
		u8  importEmbededTextures;
		u8  importAnimations;
	};

	struct SceneData
	{
		u8*						buffer;
		u32						bufferSize;
		GeometryImportSettings	settings;
	};

	void ProcessScene(Scene& scene, const GeometryImportSettings& settings);
	void PackData(const Scene& scene, SceneData& data);
}