#include "D3DCore.h"

using namespace Microsoft::WRL;

namespace Havana::Graphics::D3D12::Core
{
	namespace
	{
		class D3D12Command
		{
		public:
			D3D12Command() = default;
			DISABLE_COPY_AND_MOVE(D3D12Command)
			explicit D3D12Command(ID3D12Device8* const device, D3D12_COMMAND_LIST_TYPE type)
			{
				HRESULT hr{ S_OK };
				D3D12_COMMAND_QUEUE_DESC description{};
				description.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				description.NodeMask = 0;
				description.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
				description.Type = type;
				
				// Command Queue
				DXCall(hr = device->CreateCommandQueue(&description, IID_PPV_ARGS(&commandQueue)));
				if (FAILED(hr))
				{
					Release();
					return;
				}
				NAME_D3D12_OBJECT(commandQueue, type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
												L"Graphics Command Queue" :
												type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
												L"Compute Command Queue" : L"Command Queue");

				// Command allocators
				for (u32 i{ 0 }; i < frameBufferCount; i++)
				{
					CommandFrame& frame{ commandFrames[i] };
					DXCall(hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame.commandAllocator)));
					if (FAILED(hr))
					{
						Release();
						return;
					}
					NAME_D3D12_OBJECT_INDEXED(frame.commandAllocator, i, 
											  type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
											  L"GFX Command Allocator" :
											  type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
											  L"Compute Command Allocator" : L"Command Allocator");
				}

				// Command list
				DXCall(hr = device->CreateCommandList(0, type, commandFrames[0].commandAllocator , nullptr, IID_PPV_ARGS(&commandList)));
				if (FAILED(hr))
				{
					Release();
					return;
				}
				DXCall(commandList->Close());
				NAME_D3D12_OBJECT(commandList, type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
											   L"Graphics Command List" :
											   type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
											   L"Compute Command List" : L"Command List");

				// Fence
				DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
				if (FAILED(hr))
				{
					Release();
					return;
				}
				NAME_D3D12_OBJECT(fence, L"D3D12 Fence");

				fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
				assert(fenceEvent);
			}

			~D3D12Command()
			{
				assert(!commandQueue && !commandList && !fence);
			}

			// Wait for the current frame to be signaled and reset the command list/allocator
			void BeginFrame()
			{
				CommandFrame& frame{ commandFrames[frameIndex] };
				frame.Wait(fenceEvent, fence);
				DXCall(frame.commandAllocator->Reset());
				DXCall(commandList->Reset(frame.commandAllocator, nullptr));
			}

			// Singal the fence with the new fence value
			void EndFrame()
			{
				DXCall(commandList->Close());
				ID3D12CommandList* const commandLists[]{ commandList };
				commandQueue->ExecuteCommandLists(_countof(commandLists), &commandLists[0]);

				u64& newFenceValue{ fenceValue };
				newFenceValue++;
				CommandFrame& frame{ commandFrames[frameIndex] };
				frame.fenceValue = newFenceValue;
				commandQueue->Signal(fence, newFenceValue);

				frameIndex = (frameIndex + 1) % frameBufferCount;
			}

			// Complete all work on GPU for all frames
			void Flush()
			{
				for (u32 i{ 0 }; i < frameBufferCount; i++)
				{
					commandFrames[i].Wait(fenceEvent, fence);
				}
				frameIndex = 0;
			}
			
			void Release()
			{
				Flush();
				Core::Release(fence);
				fenceValue = 0;

				CloseHandle(fenceEvent);
				fenceEvent = nullptr;

				Core::Release(commandQueue);
				Core::Release(commandList);

				for (u32 i{ 0 }; i < frameBufferCount; i++)
				{
					commandFrames[i].Release();
				}
			}

			constexpr ID3D12CommandQueue* const CommandQueue() const { return commandQueue; }
			constexpr ID3D12GraphicsCommandList6* const CommandList() const { return commandList; }
			constexpr u32 FrameIndex() const { return frameIndex; }

		private:
			struct CommandFrame
			{
				ID3D12CommandAllocator* commandAllocator{ nullptr };
				u64						fenceValue{ 0 };

				void Wait(HANDLE fenceEvent, ID3D12Fence1* fence)
				{
					assert(fence && fenceEvent);
					// If the current fence value is still less than "fenceValue"
					// then we know the GPU hasn't finished executing the command list
					// since it has not reached the commandQueue->Signel() command.
					if (fence->GetCompletedValue() < fenceValue)
					{
						// We have the fence create an event which is signaled once the fence's current value equals "fenceValue"
						DXCall(fence->SetEventOnCompletion(fenceValue, fenceEvent));
						// Wait until the fence creates the above event, signaling that the command queue has finished executing
						WaitForSingleObject(fenceEvent, INFINITE);
					}
				}

				void Release()
				{
					Core::Release(commandAllocator);
				}
			};

			ID3D12CommandQueue*			commandQueue{ nullptr };
			ID3D12GraphicsCommandList6* commandList{ nullptr };
			ID3D12Fence1*				fence{ nullptr };
			HANDLE						fenceEvent{ nullptr };
			u64							fenceValue{ 0 };
			CommandFrame				commandFrames[frameBufferCount]{};
			u32							frameIndex{ 0 };
		};

		ID3D12Device8*	mainDevice{ nullptr };
		IDXGIFactory7*	dxgiFactory{ nullptr };
		D3D12Command	gfxCommand;

		constexpr D3D_FEATURE_LEVEL minimumFeatureLevel{ D3D_FEATURE_LEVEL_11_0 };

		bool FailedInit()
		{
			Shutdown();
			return false;
		}

		// Get the first most performing adapter that supports the minimum feature level.
		// NOTE: this function can be expanded in functionality with, for example, checking if any
		//       output devices (i.e. screens) are attached, enumerate the supported resolutions, provide
		//       a means for the user to choose which adapter to use in a multi-adapter setting, etc.
		IDXGIAdapter4* DetermineMainAdapter()
		{
			IDXGIAdapter4* adapter{ nullptr };

			// Get adapters in decending order of performance
			for (u32 i{ 0 };
				dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
				i++)
			{
				// Pick the first adapter that supports the minimum feature level
				if (SUCCEEDED(D3D12CreateDevice(adapter, minimumFeatureLevel, __uuidof(ID3D12Device), nullptr)))
				{
					return adapter;
				}
				Release(adapter);
			}
			
			return nullptr;
		}

		/// <summary>
		/// Get the maximum supported feature level of the given adapter
		/// </summary>
		/// <param name="adapter"> - Adapter to check feature levels of.</param>
		/// <returns>D3D_FEATURE_LEVEL containing max feature level.</returns>
		D3D_FEATURE_LEVEL GetMaxFeatureLevel(IDXGIAdapter4* adapter)
		{
			constexpr D3D_FEATURE_LEVEL featureLevels[4]
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_12_1
			};

			D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo{};
			featureLevelInfo.NumFeatureLevels = _countof(featureLevels);
			featureLevelInfo.pFeatureLevelsRequested = featureLevels;

			ComPtr<ID3D12Device> device;
			DXCall(D3D12CreateDevice(adapter, minimumFeatureLevel, IID_PPV_ARGS(&device)));
			DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo)));
			return featureLevelInfo.MaxSupportedFeatureLevel;
		}
	} // anonymous namespace

	bool Initialize()
	{
		if (mainDevice) Shutdown();

		u32 dxgiFactoryFlags{ 0 };

#ifdef _DEBUG
		// Enable debugging layer. Requires "Graphics Tools" optional feature
		{
			ComPtr<ID3D12Debug3> debugInterface;
			DXCall(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
			debugInterface->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif // _DEBUG

		HRESULT hr{ S_OK };
		DXCall(hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
		if (FAILED(hr)) return FailedInit();
		
		// Determine which adapter (i.e. graphics card) to use, if any
		ComPtr<IDXGIAdapter4> mainAdapter;
		mainAdapter.Attach(DetermineMainAdapter());
		if (!mainAdapter) return FailedInit();
		
		// Determine what the max feature level supported is
		D3D_FEATURE_LEVEL maxFeatureLevel{ GetMaxFeatureLevel(mainAdapter.Get()) };
		assert(maxFeatureLevel >= minimumFeatureLevel);
		if (maxFeatureLevel < minimumFeatureLevel) return FailedInit();
		
		// Create an ID3D12Device (virtual adapter)
		DXCall(hr = D3D12CreateDevice(mainAdapter.Get(), maxFeatureLevel, IID_PPV_ARGS(&mainDevice)));
		if (FAILED(hr)) return FailedInit();

		new (&gfxCommand) D3D12Command(mainDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);
		if (!gfxCommand.CommandQueue()) return FailedInit();

		NAME_D3D12_OBJECT(mainDevice, L"Main D3D Device");

#ifdef _DEBUG
		{
			ComPtr<ID3D12InfoQueue> infoQueue;
			DXCall(mainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		}
#endif // _DEBUG

		return true;
	}

	void Shutdown()
	{
		gfxCommand.Release();
		Release(dxgiFactory);

#ifdef _DEBUG
		{
			{
				ComPtr<ID3D12InfoQueue> infoQueue;
				DXCall(mainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			}

			ComPtr<ID3D12DebugDevice2> debugDevice;
			DXCall(mainDevice->QueryInterface(IID_PPV_ARGS(&debugDevice)));
			Release(mainDevice);
			DXCall(debugDevice->ReportLiveDeviceObjects
			(
				D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL
			));

		}
#endif // _DEBUG
		
		Release(mainDevice);
	}

	void Render()
	{
		// Wait for the GPU to finish with the command allocator and
		// reset the allocator once the GPU is done with it.
		// This frees the memory that was used to store commands.
		gfxCommand.BeginFrame();
		ID3D12GraphicsCommandList6* commandList{ gfxCommand.CommandList() };

		// Record commands
		// ......
		//
		// Done recording commands, now execute them,
		// signal and incriment fence value for next frame.
		gfxCommand.EndFrame();
	}
}