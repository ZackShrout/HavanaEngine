#include "D3D12Core.h"
#include "D3D12Resources.h"
#include "D3D12Surface.h"

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
				DXCall(hr = device->CreateCommandQueue(&description, IID_PPV_ARGS(&m_commandQueue)));
				if (FAILED(hr))
				{
					Release();
					return;
				}
				NAME_D3D12_OBJECT(m_commandQueue, type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
												L"Graphics Command Queue" :
												type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
												L"Compute Command Queue" : L"Command Queue");

				// Command allocators
				for (u32 i{ 0 }; i < frameBufferCount; i++)
				{
					CommandFrame& frame{ m_commandFrames[i] };
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
				DXCall(hr = device->CreateCommandList(0, type, m_commandFrames[0].commandAllocator , nullptr, IID_PPV_ARGS(&m_commandList)));
				if (FAILED(hr))
				{
					Release();
					return;
				}
				DXCall(m_commandList->Close());
				NAME_D3D12_OBJECT(m_commandList, type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
											   L"Graphics Command List" :
											   type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
											   L"Compute Command List" : L"Command List");

				// Fence
				DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
				if (FAILED(hr))
				{
					Release();
					return;
				}
				NAME_D3D12_OBJECT(m_fence, L"D3D12 Fence");

				m_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
				assert(m_fenceEvent);
			}

			~D3D12Command()
			{
				assert(!m_commandQueue && !m_commandList && !m_fence);
			}

			// Wait for the current frame to be signaled and reset the command list/allocator
			void BeginFrame()
			{
				CommandFrame& frame{ m_commandFrames[m_frameIndex] };
				frame.Wait(m_fenceEvent, m_fence);
				DXCall(frame.commandAllocator->Reset());
				DXCall(m_commandList->Reset(frame.commandAllocator, nullptr));
			}

			// Singal the fence with the new fence value
			void EndFrame()
			{
				DXCall(m_commandList->Close());
				ID3D12CommandList* const commandLists[]{ m_commandList };
				m_commandQueue->ExecuteCommandLists(_countof(commandLists), &commandLists[0]);

				u64& fenceValue{ m_fenceValue };
				fenceValue++;
				CommandFrame& frame{ m_commandFrames[m_frameIndex] };
				frame.fenceValue = fenceValue;
				m_commandQueue->Signal(m_fence, fenceValue);

				m_frameIndex = (m_frameIndex + 1) % frameBufferCount;
			}

			// Complete all work on GPU for all frames
			void Flush()
			{
				for (u32 i{ 0 }; i < frameBufferCount; i++)
				{
					m_commandFrames[i].Wait(m_fenceEvent, m_fence);
				}
				m_frameIndex = 0;
			}
			
			void Release()
			{
				Flush();
				Core::Release(m_fence);
				m_fenceValue = 0;

				CloseHandle(m_fenceEvent);
				m_fenceEvent = nullptr;

				Core::Release(m_commandQueue);
				Core::Release(m_commandList);

				for (u32 i{ 0 }; i < frameBufferCount; i++)
				{
					m_commandFrames[i].Release();
				}
			}

			constexpr ID3D12CommandQueue* const CommandQueue() const { return m_commandQueue; }
			constexpr ID3D12GraphicsCommandList6* const CommandList() const { return m_commandList; }
			constexpr u32 FrameIndex() const { return m_frameIndex; }

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
					fenceValue = 0;
				}
			};

			ID3D12CommandQueue*			m_commandQueue{ nullptr };
			ID3D12GraphicsCommandList6* m_commandList{ nullptr };
			ID3D12Fence1*				m_fence{ nullptr };
			HANDLE						m_fenceEvent{ nullptr };
			u64							m_fenceValue{ 0 };
			CommandFrame				m_commandFrames[frameBufferCount]{};
			u32							m_frameIndex{ 0 };
		};

		ID3D12Device8*				mainDevice{ nullptr };
		IDXGIFactory7*				dxgiFactory{ nullptr };
		D3D12Command				gfxCommand;
		Utils::vector<D3D12Surface> surfaces;
		DescriptorHeap				rtvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
		DescriptorHeap				dsvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
		DescriptorHeap				srvDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
		DescriptorHeap				uavDescHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
		Utils::vector<IUnknown*>	deferredReleases[frameBufferCount]{};
		u32							deferredReleasesFlag[frameBufferCount]{};
		std::mutex					deferredReleasesMutex{};

		constexpr D3D_FEATURE_LEVEL minimumFeatureLevel{ D3D_FEATURE_LEVEL_11_0 };
		constexpr DXGI_FORMAT renderTargetFormat{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };

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

		// This wont be called often so we shouldn't have this inlined
		void __declspec(noinline) ProcessDeferredReleases(u32 frameIdx)
		{
			std::lock_guard lock{ deferredReleasesMutex };

			// This flag is cleared at the beginning becuase otherwise it might
			// overwrite some other thread that was trying to set it. Its ok if
			// the overwrite happens before processing the items.
			deferredReleasesFlag[frameIdx] = 0;

			rtvDescHeap.ProcessDeferredFree(frameIdx);
			dsvDescHeap.ProcessDeferredFree(frameIdx);
			srvDescHeap.ProcessDeferredFree(frameIdx);
			uavDescHeap.ProcessDeferredFree(frameIdx);
			
			Utils::vector<IUnknown*>& resources{ deferredReleases[frameIdx] };
			if (!resources.empty())
			{
				for (auto& resource : resources)
					Release(resource);
				resources.clear();
			}
		}
	} // anonymous namespace

	namespace Detail
	{
		void DeferredRelease(IUnknown* resource)
		{
			const u32 frameIdx{ CurrentFrameIndex() };
			std::lock_guard lock{ deferredReleasesMutex };
			deferredReleases[frameIdx].push_back(resource);
			SetDeferredReleasesFlag();
		}
	} // Detail namespace

	bool Initialize()
	{
		if (mainDevice) Shutdown();

		u32 dxgiFactoryFlags{ 0 };

#ifdef _DEBUG
		// Enable debugging layer. Requires "Graphics Tools" optional feature
		{
			ComPtr<ID3D12Debug3> debugInterface;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
			{
				debugInterface->EnableDebugLayer();
			}
			else
			{
				OutputDebugStringA("Warning: D3D12 debug interface is not available. Verify that the Graphics Tools optional feature is installed in this device.\n");
			}
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

#ifdef _DEBUG
		{
			ComPtr<ID3D12InfoQueue> infoQueue;
			DXCall(mainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		}
#endif // _DEBUG

		// Initialize Descriptor Heaps
		bool result{ true };
		result &= rtvDescHeap.Initialize(512, false);
		result &= dsvDescHeap.Initialize(512, false);
		result &= srvDescHeap.Initialize(4896, true);
		result &= uavDescHeap.Initialize(512, false);
		if (!result) return FailedInit();

		// There is nothing in D3D12Command that would cause a memory leak by calling 
		// new here, but care must be taken.
		new (&gfxCommand) D3D12Command(mainDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);
		if (!gfxCommand.CommandQueue()) return FailedInit();

		NAME_D3D12_OBJECT(mainDevice, L"Main D3D Device");
		NAME_D3D12_OBJECT(rtvDescHeap.Heap(), L"RTV Descriptor Heap");
		NAME_D3D12_OBJECT(dsvDescHeap.Heap(), L"DSV Descriptor Heap");
		NAME_D3D12_OBJECT(srvDescHeap.Heap(), L"SRV Descriptor Heap");
		NAME_D3D12_OBJECT(uavDescHeap.Heap(), L"USV Descriptor Heap");

		return true;
	}

	void Shutdown()
	{
		gfxCommand.Release();

		// This is not called at the end because some resources
		// (like swap chains) can't be released before their
		// depending resources are released.
		for (u32 i{ 0 }; i < frameBufferCount; i++)
		{
			ProcessDeferredReleases(i);
		}

		Release(dxgiFactory);

		rtvDescHeap.Release();
		dsvDescHeap.Release();
		srvDescHeap.Release();
		uavDescHeap.Release();

		// Some types only use deferred releases for their resources
		// during shutdown/reset/clear. To release these resources,
		// this function is called one last time.
		ProcessDeferredReleases(0);

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

	ID3D12Device* const Device()
	{
		return mainDevice;
	}
	
	DescriptorHeap& RTVHeap() { return rtvDescHeap; }

	DescriptorHeap& DSVHeap() { return dsvDescHeap; }

	DescriptorHeap& SRVHeap() { return uavDescHeap; }

	DescriptorHeap& UAVHeap() { return uavDescHeap; }

	DXGI_FORMAT DefaultRenderTargetFormat() { return renderTargetFormat; }
	
	u32 CurrentFrameIndex()
	{
		return gfxCommand.FrameIndex();
	}
	
	void SetDeferredReleasesFlag()
	{
		deferredReleasesFlag[CurrentFrameIndex()] = 1;
	}

	Surface CreateSurface(Platform::Window window)
	{
		surfaces.emplace_back(window);
		surface_id id{ (u32)surfaces.size() - 1 };
		surfaces[id].CreateSwapChain(dxgiFactory, gfxCommand.CommandQueue(), renderTargetFormat);
		return Surface{ id };
	}

	void RemoveSurface(surface_id id)
	{
		gfxCommand.Flush();
		// TODO: use proper removal of surfaces.
		surfaces[id].~D3D12Surface();
	}

	void ResizeSurface(surface_id id, u32, u32)
	{
		gfxCommand.Flush();
		surfaces[id].Resize();
	}
	
	u32 SurfaceWidth(surface_id id)
	{
		return surfaces[id].Width();
	}

	u32 SurfaceHeight(surface_id id)
	{
		return surfaces[id].Height();
	}

	void RenderSurface(surface_id id)
	{
		// Wait for the GPU to finish with the command allocator and
		// reset the allocator once the GPU is done with it.
		// This frees the memory that was used to store commands.
		gfxCommand.BeginFrame();
		ID3D12GraphicsCommandList6* commandList{ gfxCommand.CommandList() };

		// Check to see if there are deferred releases to handle
		const u32 frameIdx{ CurrentFrameIndex() };
		if (deferredReleasesFlag[frameIdx])
		{
			(ProcessDeferredReleases(frameIdx));
		}
		
		const D3D12Surface& surface{ surfaces[id] };
		
		// Presenting swap chain buffers happens in lockstep with frame buffers.
		surface.Present();

		// Record commands
		// ......
		//
		// Done recording commands, now execute them,
		// signal and incriment fence value for next frame.
		gfxCommand.EndFrame();
	}

}