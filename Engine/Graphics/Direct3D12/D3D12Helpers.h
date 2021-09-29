#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12::D3DX
{
	
	constexpr struct
	{
		D3D12_HEAP_PROPERTIES defaultHeap
		{
			D3D12_HEAP_TYPE_DEFAULT,			// Type;	
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,	// CPUPageProperty;
			D3D12_MEMORY_POOL_UNKNOWN,			// MemoryPoolPreference;
			0,									// CreationNodeMask;
			0									// VisibleNodeMask;
		};

	} heapProperties;

	ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc);

	struct D3D12_Descriptor_Range : public D3D12_DESCRIPTOR_RANGE1
	{
		constexpr explicit D3D12_Descriptor_Range(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, u32 descriptorCount, u32 shaderRegister, u32 space = 0,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE, u32 offsetFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) 
			: D3D12_DESCRIPTOR_RANGE1{ rangeType, descriptorCount, shaderRegister, space, flags, offsetFromTableStart }
		{}
	};

	struct D3D12_Root_Parameter : public D3D12_ROOT_PARAMETER1
	{
		constexpr void AsConstants(u32 numConstants, D3D12_SHADER_VISIBILITY visibility, u32 shaderRegister, u32 space = 0)
		{
			ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			ShaderVisibility = visibility;
			Constants.Num32BitValues = numConstants;
			Constants.ShaderRegister = shaderRegister;
			Constants.RegisterSpace = space;
		}

		constexpr void AsCBV(D3D12_SHADER_VISIBILITY visibility, u32 shaderRegister, u32 space = 0, 
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shaderRegister, space, flags);
		}

		constexpr void AsSRV(D3D12_SHADER_VISIBILITY visibility, u32 shaderRegister, u32 space = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shaderRegister, space, flags);
		}

		constexpr void AsUAV(D3D12_SHADER_VISIBILITY visibility, u32 shaderRegister, u32 space = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, visibility, shaderRegister, space, flags);
		}

		constexpr void AsDescriptorTable(D3D12_SHADER_VISIBILITY visibility, const D3D12_Descriptor_Range* ranges, u32 rangeCount)
		{
			ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			ShaderVisibility = visibility;
			DescriptorTable.NumDescriptorRanges = rangeCount;
			DescriptorTable.pDescriptorRanges = ranges;
		}

	private:
		constexpr void AsDescriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility, u32 shaderRegister, u32 space,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags)
		{
			ParameterType = type;
			ShaderVisibility = visibility;
			Descriptor.ShaderRegister = shaderRegister;
			Descriptor.RegisterSpace = space;
			Descriptor.Flags = flags;
		}
	};

	// Maximum 64 DWORDs (u32's) divided up amongst all root parameters:
	// Root constants = 1 DWORD per 32-bit constant
	// Root descriptor (CBV, SRV, UAV) = 2 DWORDs each
	// Descriptor table pointer = 1 DWORD
	// Static samplers = 0 DWORDs (compiled into shader)
	struct D3D12_Root_Signature_Desc : public D3D12_ROOT_SIGNATURE_DESC1
	{
		constexpr explicit D3D12_Root_Signature_Desc(const D3D12_Root_Parameter* parameters, u32 parameterCount,
			const D3D12_STATIC_SAMPLER_DESC* staticSamplers = nullptr, u32 samplerCount = 0,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS)
			: D3D12_ROOT_SIGNATURE_DESC1{ parameterCount, parameters, samplerCount, staticSamplers, flags }
		{}

		ID3D12RootSignature* Create() const
		{
			return CreateRootSignature(*this);
		}
	};
}