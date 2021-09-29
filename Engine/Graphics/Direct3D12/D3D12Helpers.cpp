#include "D3D12Helpers.h"
#include "D3D12Core.h"

namespace Havana::Graphics::D3D12::D3DX
{
	namespace
	{

	} // anonymous namespace


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

}