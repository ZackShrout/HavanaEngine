#include "D3D12Helpers.h"
#include "D3D12Core.h"
#include "D3D12Upload.h"

namespace havana::graphics::d3d12::D3DX
{
	namespace
	{

	} // anonymous namespace

	void TransitionResource(
		id3d12GraphicsCommandList* cmdList,
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
		D3D12_RESOURCE_BARRIER_FLAGS flags /*= D3D12_RESOURCE_BARRIER_FLAG_NONE*/,
		u32 subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/)
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = flags;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = subresource;

		cmdList->ResourceBarrier(1, &barrier);
	}

	ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc)
	{
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc{};
		versionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		versionedDesc.Desc_1_1 = desc;

		using namespace Microsoft::WRL;
		ComPtr<ID3DBlob> signatureBlob{ nullptr };
		ComPtr<ID3DBlob> errorBlob{ nullptr };
		HRESULT hr{ S_OK };
		if (FAILED(hr = D3D12SerializeVersionedRootSignature(&versionedDesc, &signatureBlob, &errorBlob)))
		{
			DEBUG_OP(const char* errorMsg{ errorBlob ? (const char*)errorBlob->GetBufferPointer() : "" });
			DEBUG_OP(OutputDebugStringA(errorMsg));
			return nullptr;
		}

		ID3D12RootSignature* signature{ nullptr };
		DXCall(hr = Core::Device()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&signature)));

		if (FAILED(hr))
		{
			Core::Release(signature);
		}

		return signature;
	}

	ID3D12PipelineState* CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC desc)
	{
		assert(desc.pPipelineStateSubobjectStream && desc.SizeInBytes);
		ID3D12PipelineState* pso{ nullptr };
		DXCall(Core::Device()->CreatePipelineState(&desc, IID_PPV_ARGS(&pso)));
		assert(pso);
		return pso;
	}

	ID3D12PipelineState* CreatePipelineState(void* stream, u64 streamSize)
	{
		assert(stream && streamSize);
		D3D12_PIPELINE_STATE_STREAM_DESC desc{};
		desc.SizeInBytes = streamSize;
		desc.pPipelineStateSubobjectStream = stream;

		return CreatePipelineState(desc);
	}

	ID3D12Resource* CreateBuffer(const void* data, u32 buffer_size, bool isCPUAccessible/* = false*/,
								 D3D12_RESOURCE_STATES state/* = D3D12_RESOURCE_STATE_COMMON*/,
								 D3D12_RESOURCE_FLAGS flags/* = D3D12_RESOURCE_FLAG_NONE*/,
								 ID3D12Heap* heap/* = nullptr*/, u64 heapOffset/* = 0*/)
	{
		assert(buffer_size);

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = buffer_size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc = { 1,0 };
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = isCPUAccessible ? D3D12_RESOURCE_FLAG_NONE : flags;

		// The buffer will be only used for upload or as constant buffer/UAV
		assert(desc.Flags == D3D12_RESOURCE_FLAG_NONE || desc.Flags == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		ID3D12Resource* resource{ nullptr };
		const D3D12_RESOURCE_STATES resourceState{ isCPUAccessible ? D3D12_RESOURCE_STATE_GENERIC_READ : state };

		if (heap)
		{
			DXCall(Core::Device()->CreatePlacedResource(heap, heapOffset, &desc, resourceState, nullptr, IID_PPV_ARGS(&resource)));
		}
		else
		{
			DXCall(Core::Device()->CreateCommittedResource(isCPUAccessible ? &heapProperties.uploadHeap : &heapProperties.defaultHeap, D3D12_HEAP_FLAG_NONE, &desc, resourceState, nullptr, IID_PPV_ARGS(&resource)));
		}

		if (data)
		{
			// If we have initial data which we'd like to be able to change later, we set isCPUAccessible
			// to true. If we only want to upload some data once to be used by the GPU, then isCPUAccessible
			// should be set to false.
			if (isCPUAccessible)
			{
				// NOTE: range's Begin and End fields are set to 0, to indicate that
				//		 the CPU is not reading any data (i.e. write-only)
				const D3D12_RANGE range{};
				void* cpuAddress{ nullptr };
				DXCall(resource->Map(0, &range, reinterpret_cast<void**>(&cpuAddress)));
				assert(cpuAddress);
				memcpy(cpuAddress, data, buffer_size);
				resource->Unmap(0, nullptr);
			}
			else
			{
				Upload::D3D12UploadContext context{ buffer_size };
				memcpy(context.CPUAddress(), data, buffer_size);
				context.CommandList()->CopyResource(resource, context.UploadBuffer());
				context.EndUpload();
			}
		}

		assert(resource);
		return resource;
	}
}