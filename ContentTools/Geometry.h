#pragma once
#include "ToolsCommon.h"

namespace Havana::Tools
{
	//namespace PackedVertex
	//{
	//	struct VertexStatic
	//	{
	//		Math::Vec3	position;
	//		u8			reserved[3];
	//		u8			tSign; // bit 0: tangent handidness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1)
	//		u16			normal[2];
	//		u16			tangent[2];
	//		Math::Vec2	uv;
	//	};
	//} // namespace PackedVertex
	
	struct Vertex
	{
		Math::Vec4		tangent{};
		Math::Vec4		jointWeights{};
		Math::Vec4u32	jointIndices{ U32_INVALID_ID, U32_INVALID_ID , U32_INVALID_ID , U32_INVALID_ID };
		Math::Vec3		position{};
		Math::Vec3		normal{};
		Math::Vec2		uv{};
		u8				red{}, green{}, blue{};
		u8				pad{};
	};

	namespace Elements
	{
		struct ElementsType
		{
			enum Type : u32
			{
				PositionOnly = 0x00,
				StaticNormal = 0x01,
				StaticNormalTexture = 0x03,
				StaticColor = 0x04,
				Skeletal = 0x08,
				SkeletalColor = Skeletal | StaticColor,
				SkeletalNormal = Skeletal | StaticNormal,
				SkeletalNormalColor = SkeletalNormal | StaticColor,
				SkeletalNormalTexture = Skeletal | StaticNormalTexture,
				SkeletalNormalTextureColor = SkeletalNormalTexture | StaticColor,
			};
		};

		struct StaticColor
		{
			u8			color[3];
			u8			pad;
		};

		struct StaticNormal
		{
			u8			color[3];
			u8			tSign; // bit 0: tangent handidness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1)
			u16			normal[2];
		};

		struct StaticNormalTexture
		{
			u8			color[3];
			u8			tSign; // bit 0: tangent handidness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1)
			u16			normal[2];
			u16			tangent[2];
			Math::Vec2	uv;
		};

		struct Skeletal
		{
			u8			joinWeights[3]; // normalized joint weights for up to 4 joints
			u8			pad;
			u16			jointIndices[4];
		};

		struct SkeletalColor
		{
			u8			joinWeights[3]; // normalized joint weights for up to 4 joints
			u8			pad;
			u16			jointIndices[4];
			u8			color[3];
			u8			pad2;
		};

		struct SkeletalNormal
		{
			u8			joinWeights[3]; // normalized joint weights for up to 4 joints
			u8			tSign; // bit 0: tangent handidness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1)
			u16			jointIndices[4];
			u16			normal[2];
		};

		struct SkeletalNormalColor
		{
			u8			joinWeights[3]; // normalized joint weights for up to 4 joints
			u8			tSign; // bit 0: tangent handidness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1)
			u16			jointIndices[4];
			u16			normal[2];
			u8			color[3];
			u8			pad;
		};

		struct SkeletalNormalTexture
		{
			u8			joinWeights[3]; // normalized joint weights for up to 4 joints
			u8			tSign; // bit 0: tangent handidness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1)
			u16			jointIndices[4];
			u16			normal[2];
			u16			tangent[2];
			Math::Vec2	uv;
		};

		struct SkeletalNormalTextureColor
		{
			u8			joinWeights[3]; // normalized joint weights for up to 4 joints
			u8			tSign; // bit 0: tangent handidness * (tangent.z sign), bit 1: normal.z sign (0 means -1, 1 means +1)
			u16			jointIndices[4];
			u16			normal[2];
			u16			tangent[2];
			Math::Vec2	uv;
			u8			color[3];
			u8			pad;
		};
	} // namespace Elements
	
	struct Mesh
	{
		// Initial data
		Utils::vector<Math::Vec3>					positions;
		Utils::vector<Math::Vec3>					normals;
		Utils::vector<Math::Vec4>					tangents;
		Utils::vector<Math::Vec3>					colors;
		Utils::vector<Utils::vector<Math::Vec2>>	uvSets;
		Utils::vector<u32>							rawIndices;
		Utils::vector<u32>							materialIndices;
		Utils::vector<u32>							materialUsed;

		// Intermediate data
		Utils::vector<Vertex>						vertices;
		Utils::vector<u32>							indices;

		// Output data
		std::string									name;
		Elements::ElementsType::Type				elementsType;
		Utils::vector<u8>							positionBuffer;
		Utils::vector<u8>							elementBuffer;
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