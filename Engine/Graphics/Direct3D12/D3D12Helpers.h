#pragma once
#include "D3D12CommonHeaders.h"

namespace Havana::Graphics::D3D12::D3DX
{
	
	constexpr struct
	{
		D3D12_HEAP_PROPERTIES defaultHeap
		{
			D3D12_HEAP_TYPE_DEFAULT,					// Type
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,			// CPUPageProperty
			D3D12_MEMORY_POOL_UNKNOWN,					// MemoryPoolPreference
			0,											// CreationNodeMask
			0											// VisibleNodeMask
		};

		D3D12_HEAP_PROPERTIES uploadHeap
		{
			D3D12_HEAP_TYPE_UPLOAD,						// Type
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,			// CPUPageProperty
			D3D12_MEMORY_POOL_UNKNOWN,					// MemoryPoolPreference
			0,											// CreationNodeMask
			0											// VisibleNodeMask
		};

	} heapProperties;

	constexpr struct
	{
		const D3D12_RASTERIZER_DESC noCull
		{
			D3D12_FILL_MODE_SOLID,						// FillMode
			D3D12_CULL_MODE_NONE,						// CullMode
			0,											// FrontCounterClockwise
			0,											// DepthBias
			0,											// DepthBiasClamp
			0,											// SlopeScaledDepthBias
			1,											// DepthClipEnable
			1,											// MultisampleEnable
			0,											// AntialiasedLineEnable
			0,											// ForcedSampleCount
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
		};

		const D3D12_RASTERIZER_DESC backfaceCull
		{
			D3D12_FILL_MODE_SOLID,						// FillMode
			D3D12_CULL_MODE_BACK,						// CullMode
			0,											// FrontCounterClockwise
			0,											// DepthBias
			0,											// DepthBiasClamp
			0,											// SlopeScaledDepthBias
			1,											// DepthClipEnable
			1,											// MultisampleEnable
			0,											// AntialiasedLineEnable
			0,											// ForcedSampleCount
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
		};
		
		const D3D12_RASTERIZER_DESC frontfaceCull
		{
			D3D12_FILL_MODE_SOLID,						// FillMode
			D3D12_CULL_MODE_FRONT,						// CullMode
			0,											// FrontCounterClockwise
			0,											// DepthBias
			0,											// DepthBiasClamp
			0,											// SlopeScaledDepthBias
			1,											// DepthClipEnable
			1,											// MultisampleEnable
			0,											// AntialiasedLineEnable
			0,											// ForcedSampleCount
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
		};

		const D3D12_RASTERIZER_DESC wireframe
		{
			D3D12_FILL_MODE_WIREFRAME,					// FillMode
			D3D12_CULL_MODE_NONE,						// CullMode
			0,											// FrontCounterClockwise
			0,											// DepthBias
			0,											// DepthBiasClamp
			0,											// SlopeScaledDepthBias
			1,											// DepthClipEnable
			1,											// MultisampleEnable
			0,											// AntialiasedLineEnable
			0,											// ForcedSampleCount
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF	// ConservativeRaster
		};
	} rasterizerState;

	constexpr struct
	{
		const D3D12_DEPTH_STENCIL_DESC1 disabled
		{
			0,											// DepthEnable
			D3D12_DEPTH_WRITE_MASK_ZERO,				// DepthWriteMask
			D3D12_COMPARISON_FUNC_LESS_EQUAL,			// DepthFunc
			0,											// StencilEnable
			0,											// StencilReadMask
			0,											// StencilWriteMask
			{},											// FrontFace
			{},											// BackFace
			0											// DepthBoundsTestEnable
		};
	} depthState;

	class ResourceBarrier
	{
	public:
		constexpr static u32 maxResourceBarriers{ 32 };

		// Add a transition barrier to the list of barriers
		constexpr void Add(
			ID3D12Resource* resource,
			D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
			D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
			u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			assert(resource);
			assert(m_offset < maxResourceBarriers);
			
			D3D12_RESOURCE_BARRIER& barrier{ m_barriers[m_offset] };
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = flags;
			barrier.Transition.pResource = resource;
			barrier.Transition.StateBefore = before;
			barrier.Transition.StateAfter = after;
			barrier.Transition.Subresource = subresource;

			m_offset++;
		}

		// Add a UAV barrier to the list of barries
		constexpr void Add(
			ID3D12Resource* resource,
			D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
		{
			assert(resource);
			assert(m_offset < maxResourceBarriers);
			
			D3D12_RESOURCE_BARRIER& barrier{ m_barriers[m_offset] };
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.Flags = flags;
			barrier.UAV.pResource = resource;

			m_offset++;
		}

		// Add an aliasing barrier to the list of barriers
		constexpr void Add(
			ID3D12Resource* before, ID3D12Resource* after,
			D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
		{
			assert(before && after);
			assert(m_offset < maxResourceBarriers);

			D3D12_RESOURCE_BARRIER& barrier{ m_barriers[m_offset] };
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
			barrier.Flags = flags;
			barrier.Aliasing.pResourceBefore = before;
			barrier.Aliasing.pResourceAfter = after;

			m_offset++;
		}

		void Apply(ID3D12GraphicsCommandList* cmdList)
		{
			assert(m_offset);
			cmdList->ResourceBarrier(m_offset, m_barriers);
			m_offset = 0;
		}
	private:
		D3D12_RESOURCE_BARRIER	m_barriers[maxResourceBarriers]{};
		u32						m_offset{ 0 };
	};

	void TransitionResource(
		id3d12GraphicsCommandList* cmdList,
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
		D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

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

#pragma warning(push)
#pragma warning(disable : 4324)
	template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type, typename T>
	class alignas(void*) D3D12_Pipeline_State_Subobject
	{
	public:
		D3D12_Pipeline_State_Subobject() = default;
		constexpr explicit D3D12_Pipeline_State_Subobject(T subobject) : m_type{ type }, m_subobject{ subobject }{}
		D3D12_Pipeline_State_Subobject& operator=(const T& subobject) { m_subobject = subobject; return *this; }
	private:
		const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE m_type{ type };
		T m_subobject{};
	};
#pragma warning(pop)

// Pipeline State Subobject (PSS) macro
#define PSS(name, ...) using D3D12_Pipeline_State_Subobject_##name = D3D12_Pipeline_State_Subobject<__VA_ARGS__>;

	PSS(rootSignature, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*);
	PSS(vs, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE);
	PSS(ps, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS, D3D12_SHADER_BYTECODE);
	PSS(ds, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS, D3D12_SHADER_BYTECODE);
	PSS(hs, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS, D3D12_SHADER_BYTECODE);
	PSS(gs, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS, D3D12_SHADER_BYTECODE);
	PSS(cs, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS, D3D12_SHADER_BYTECODE);
	PSS(streamOutput, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT, D3D12_STREAM_OUTPUT_DESC);
	PSS(blend, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, D3D12_BLEND_DESC);
	PSS(sampleMask, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK, u32);
	PSS(rasterizer, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, D3D12_RASTERIZER_DESC);
	PSS(depthStencil, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL, D3D12_DEPTH_STENCIL_DESC);
	PSS(inputLayer, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT, D3D12_INPUT_LAYOUT_DESC);
	PSS(ibStripCutValue, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE, D3D12_INDEX_BUFFER_STRIP_CUT_VALUE);
	PSS(primitiveTopology, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE);
	PSS(renderTargetFormats, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, D3D12_RT_FORMAT_ARRAY);
	PSS(depthStencilFormat, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT, DXGI_FORMAT);
	PSS(sampleDesc, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC, DXGI_SAMPLE_DESC);
	PSS(nodeMask, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK, u32);
	PSS(cachedPSO, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO, D3D12_CACHED_PIPELINE_STATE);
	PSS(flags, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS, D3D12_PIPELINE_STATE_FLAGS);
	PSS(depthStencil1, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1, D3D12_DEPTH_STENCIL_DESC1);
	PSS(viewInstancing, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING, D3D12_VIEW_INSTANCING_DESC);
	PSS(as, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS, D3D12_SHADER_BYTECODE);
	PSS(ms, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS, D3D12_SHADER_BYTECODE);

#undef PSS

	ID3D12PipelineState* CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC desc);
	ID3D12PipelineState* CreatePipelineState(void* stream, u64 streamSize);

	ID3D12Resource* CreateBuffer(const void* data, u32 bufferSize, bool isCPUAccessible = false,
								 D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
								 D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
								 ID3D12Heap* heap = nullptr, u64 heapOffset = 0);
}