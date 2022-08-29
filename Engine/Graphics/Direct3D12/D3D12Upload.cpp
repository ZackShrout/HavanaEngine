#include "D3D12Upload.h"
#include "D3D12Core.h"

namespace havana::graphics::d3d12::Upload
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
		std::mutex			frameMutex{};
		std::mutex			queueMutex{};

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

		// NOTE: frames should be locked before this function is called.
		u32 GetAvailableUploadFrame()
		{
			u32 index{ u32_invalid_id };
			const u32 count{ uploadFrameCount };
			UploadFrame* const frames{ &uploadFrames[0] };

			for (u32 i{ 0 }; i < count; i++)
			{
				if (frames[i].IsReady())
				{
					index = i;
					break;
				}
			}

			// None of the frames were done uploading. We're the only thread here, so
			// we can iterate through the frames unril we find one that is ready
			if (index == u32_invalid_id)
			{
				index = 0;
				while (!frames[index].IsReady())
				{
					index = (index + 1) % count;
					std::this_thread::yield();
				}
			}

			return index;
		}

		bool InitFailed()
		{
			Shutdown();
			return false;
		}

	} // anonymous namespace

	D3D12UploadContext::D3D12UploadContext(u32 alignedSize)
	{
		assert(uploadCmdQueue);
		{
			// We don't want to lock this function for longer than necessary, so we scope this lock
			std::lock_guard lock{ frameMutex };
			m_frameIndex = GetAvailableUploadFrame();
			assert(m_frameIndex != u32_invalid_id);
			// Before unlocking, we prevent other threads from picking
			// this frame by making IsReady() return false.
			uploadFrames[m_frameIndex].uploadBuffer = (ID3D12Resource*)1;
		}

		UploadFrame& frame{ uploadFrames[m_frameIndex] };
		frame.uploadBuffer = D3DX::CreateBuffer(nullptr, alignedSize, true);
		NAME_D3D12_OBJECT_INDEXED(frame.uploadBuffer, alignedSize, L"Upload Buffer - size");

		const D3D12_RANGE range{};
		DXCall(frame.uploadBuffer->Map(0, &range, reinterpret_cast<void**>(&frame.cpuAddress)));
		assert(frame.cpuAddress);

		m_cmdList = frame.cmdList;
		m_uploadBuffer = frame.uploadBuffer;
		m_cpuAddress = frame.cpuAddress;
		assert(m_cmdList && m_uploadBuffer && m_cpuAddress);

		DXCall(frame.cmdAllocator->Reset());
		DXCall(frame.cmdList->Reset(frame.cmdAllocator, nullptr));
	}

	void D3D12UploadContext::EndUpload()
	{
		assert(m_frameIndex != u32_invalid_id);
		UploadFrame& frame{ uploadFrames[m_frameIndex] };
		id3d12GraphicsCommandList* const cmdList{frame.cmdList};
		DXCall(cmdList->Close());

		std::lock_guard lock{ queueMutex };

		ID3D12CommandList* const cmdLists[]{ cmdList };
		ID3D12CommandQueue* const cmdQueue{ uploadCmdQueue };
		cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		++uploadFenceValue;
		frame.fenceValue = uploadFenceValue;
		DXCall(cmdQueue->Signal(uploadFence, frame.fenceValue));

		// Wait for copy queue to finish. Then release the upload buffer.
		frame.WaitAndReset();
		// This instance of upload context is now expired. make sure we don't use it again.
		DEBUG_OP(new (this) D3D12UploadContext);
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