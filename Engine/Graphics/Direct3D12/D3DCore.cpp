#include "D3DCore.h"

using namespace Microsoft::WRL;

namespace Havana::Graphics::D3D12::Core
{
	namespace
	{
		ID3D12Device8* mainDevice{ nullptr };
		IDXGIFactory7* dxgiFactory{ nullptr };
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
}