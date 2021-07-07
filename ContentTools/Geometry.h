#pragma once
#include "ToolsCommon.h"

namespace Havana::Tools
{
	struct Mesh
	{
		// Initial data
		Utils::vector<Math::Vec3>					positions;
		Utils::vector<Math::Vec3>					normals;
		Utils::vector<Math::Vec4>					tangents;
		Utils::vector<Utils::vector<Math::Vec2>>	uvSets;
		Utils::vector<u32>							rawIndices;

		// Intermediate data

		// Output data
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
}