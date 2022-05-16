#include "D3D12Upload.h"
#include "D3D12Core.h"

namespace Havana::Graphics::D3D12::Upload
{
	namespace
	{
		struct UploadFrame
		{
			ID3D12CommandAllocator*		cmdAllocator{ nullptr };
			id3d12GraphicsCommandList*	cmdList{ nullptr };
			ID3D12Resource*				uploadBuffer{ nullptr };
			void*						cpuAddress{ nullptr };
			u64							fenceValue{ 0 };

			void WaitAndReset();

			void Release()
			{
				WaitAndReset();
				Core::Release(cmdAllocator);
				Core::Release(cmdList);
			}

			constexpr bool IsReady() const { return uploadBuffer == nullptr; }
		};

		constexpr u32		uploadFrameCount{ 4 };
		UploadFrame			uploadFrames[uploadFrameCount]{};
		ID3D12CommandQueue*	uploadCmdQueue{ nullptr };
		ID3D12Fence1*		uploadFence{ nullptr };
		u64					uploadFenceValue{ 0 };
		HANDLE				fenceEvent{};

		void UploadFrame::WaitAndReset()
		{
			assert(uploadFence && fenceEvent);
			if (uploadFence->GetCompletedValue() < fenceValue)
			{
				DXCall(uploadFence->SetEventOnCompletion(fenceValue, fenceEvent));
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			Core::Release(uploadBuffer);
			cpuAddress = nullptr;
		}

		bool InitFailed()
		{
			Shutdown();
			return false;
		}

	} // anonymous namespace

	D3D12UploadContext::D3D12UploadContext(u32 alignedSize)
	{
	}

	void D3D12UploadContext::EndUpload()
	{
	}
	
	bool Initialize()
	{
		id3d12Device* const device{ Core::Device() };
		assert(device && !uploadCmdQueue);

		HRESULT hr{ S_OK };

		for (u32 i{ 0 }; i < uploadFrameCount; i++)
		{
			UploadFrame& frame{ uploadFrames[i] };
			DXCall(hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&frame.cmdAllocator)));
			if (FAILED(hr)) return InitFailed();

			DXCall(hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, frame.cmdAllocator, nullptr, IID_PPV_ARGS(&frame.cmdList)));
			if (FAILED(hr)) return InitFailed();

			DXCall(frame.cmdList->Close());

			NAME_D3D12_OBJECT_INDEXED(frame.cmdAllocator, i, L"Upload Command Allocator");
			NAME_D3D12_OBJECT_INDEXED(frame.cmdList, i, L"Upload Command List");
		}

		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

		DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&uploadCmdQueue)));
		if (FAILED(hr)) return InitFailed();
		NAME_D3D12_OBJECT(uploadCmdQueue, L"Upload Copy Queue");

		DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence)));
		if (FAILED(hr)) return InitFailed();
		NAME_D3D12_OBJECT(uploadFence, L"Upload Fence");

		fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		assert(fenceEvent);
		if (!fenceEvent) return InitFailed();

		return true;
	}

	void Shutdown()
	{
		for (u32 i{ 0 }; i < uploadFrameCount; i++)
		{
			uploadFrames[i].Release();
		}

		if (fenceEvent)
		{
			CloseHandle(fenceEvent);
			fenceEvent = nullptr;
		}

		Core::Release(uploadCmdQueue);
		Core::Release(uploadFence);
		uploadFenceValue = 0;
	}
}