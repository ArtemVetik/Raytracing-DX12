#pragma once

#include "framework.h"

#include "../Core/Graphics/PipelineStateD3D12.h"
#include "../Core/Graphics/ComputePipelineStateD3D12.h"
#include "../Core/Graphics/CommandSignatureD3D12.h"
#include "../Core/EduMath/SimpleMath.h"

#include "DXRHelpers/nv_helpers_dx12/RaytracingPipelineGenerator.h"

using namespace EduEngine;
using namespace Microsoft::WRL;
using namespace DirectX;

namespace RaytracingDX12
{
	class ColorPass
	{
	public:
		struct ObjectConstants
		{
			DirectX::XMFLOAT4X4 World;
		};

		struct PassConstants
		{
			DirectX::XMFLOAT4X4 ViewProj;
		};

	private:
		ShaderD3D12 m_VertexShader;
		ShaderD3D12 m_PixelShader;
		RootSignatureD3D12 m_RootSignature;
		PipelineStateD3D12 m_Pso;

	public:
		ColorPass(RenderDeviceD3D12* device, const LPCWSTR* macros = nullptr) :
			m_VertexShader(L"Shaders\\Opaque.hlsl", EDU_SHADER_TYPE_VERTEX, macros, L"VS", L"vs_6_0"),
			m_PixelShader(L"Shaders\\Opaque.hlsl", EDU_SHADER_TYPE_PIXEL, macros, L"PS", L"ps_6_0")
		{
			CD3DX12_DESCRIPTOR_RANGE albedoTex;
			albedoTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			m_RootSignature.AddDescriptorParameter(1, &albedoTex); // albedo

			m_RootSignature.AddConstantBufferView(0); // object constants
			m_RootSignature.AddConstantBufferView(1); // pass constants

			m_RootSignature.Build(device, QueueID::Direct);

			std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			m_Pso.SetInputLayout({ mInputLayout.data(), (UINT)mInputLayout.size() });
			m_Pso.SetRootSignature(&m_RootSignature);
			m_Pso.SetShader(&m_VertexShader);
			m_Pso.SetShader(&m_PixelShader);
			m_Pso.SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
			m_Pso.Build(device);
			m_Pso.SetName(L"ColorPSO");
		}

		ID3D12RootSignature* GetD3D12RootSignature() const { return m_RootSignature.GetD3D12RootSignature(); }
		ID3D12PipelineState* GetD3D12PipelineState() const { return m_Pso.GetD3D12PipelineState(); }
	};

	class RaytracingPass
	{
	public:
		struct PassConstants
		{
			XMFLOAT3 LightPos;
			UINT Padding;
			XMFLOAT3 CamPos;
			UINT Padding1;
		};

		struct ObjectConstants
		{
			XMMATRIX World;
		};

		struct CameraConstants
		{
			XMFLOAT4 CamPos;
			XMMATRIX ViewProjInv;
		};

	private:
		ShaderD3D12 m_RayGenShader;
		ShaderD3D12 m_HitShader;
		ShaderD3D12 m_MissShader;
		ShaderD3D12 m_ShadowShader;
		RootSignatureD3D12 m_RayGenSignature;
		RootSignatureD3D12 m_HitSignature;
		RootSignatureD3D12 m_MissSignature;

		ComPtr<ID3D12StateObject> m_RtStateObject;
		ComPtr<ID3D12StateObjectProperties> m_RtStateObjectProps;

	public:
		RaytracingPass(RenderDeviceD3D12* device, const LPCWSTR* macros = nullptr) :
			m_RayGenShader(L"Shaders\\RayGen.hlsl", EDU_SHADER_TYPE_LIB, macros, L"", L"lib_6_3"),
			m_HitShader(L"Shaders\\Hit.hlsl", EDU_SHADER_TYPE_LIB, macros, L"", L"lib_6_3"),
			m_MissShader(L"Shaders\\Miss.hlsl", EDU_SHADER_TYPE_LIB, macros, L"", L"lib_6_3"),
			m_ShadowShader(L"Shaders\\ShadowRay.hlsl", EDU_SHADER_TYPE_LIB, macros, L"", L"lib_6_3")
		{
			CD3DX12_DESCRIPTOR_RANGE output;
			output.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
			m_RayGenSignature.AddDescriptorParameter(1, &output); // output

			CD3DX12_DESCRIPTOR_RANGE tlas;
			tlas.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			m_RayGenSignature.AddDescriptorParameter(1, &tlas); // TLAS

			m_RayGenSignature.AddConstantBufferView(0); // camera

			m_RayGenSignature.Build(device, QueueID::Direct, true);
			m_RayGenSignature.SetName(L"RayGenSignature");

			m_HitSignature.AddShaderResourceView(0); // vertex buffer
			m_HitSignature.AddShaderResourceView(1); // index buffer

			CD3DX12_DESCRIPTOR_RANGE albedoTex;
			albedoTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
			m_HitSignature.AddDescriptorParameter(1, &albedoTex); // albedo texture

			CD3DX12_DESCRIPTOR_RANGE tlasHit;
			tlasHit.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
			m_HitSignature.AddDescriptorParameter(1, &tlasHit); // TLAS

			m_HitSignature.AddConstantBufferView(0); // pass
			m_HitSignature.AddConstantBufferView(1); // object
			m_HitSignature.AddConstantBufferView(2); // material

			m_HitSignature.Build(device, QueueID::Direct, true);
			m_HitSignature.SetName(L"HitSignature");

			m_MissSignature.Build(device, QueueID::Direct, true);
			m_MissSignature.SetName(L"MissSignature");

			nv_helpers_dx12::RayTracingPipelineGenerator pipeline(device->GetD3D12Device());

			pipeline.AddLibrary(m_RayGenShader.GetShaderBlob().Get(), { L"RayGen" });
			pipeline.AddLibrary(m_MissShader.GetShaderBlob().Get(), { L"Miss" });
			pipeline.AddLibrary(m_HitShader.GetShaderBlob().Get(), { L"ClosestHit" });
			pipeline.AddLibrary(m_ShadowShader.GetShaderBlob().Get(), { L"ShadowClosestHit", L"ShadowMiss" });

			pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");
			pipeline.AddHitGroup(L"ShadowHitGroup", L"ShadowClosestHit");

			pipeline.AddRootSignatureAssociation(m_RayGenSignature.GetD3D12RootSignature(), { L"RayGen" });
			pipeline.AddRootSignatureAssociation(m_MissSignature.GetD3D12RootSignature(), { L"Miss", L"ShadowMiss" });
			pipeline.AddRootSignatureAssociation(m_HitSignature.GetD3D12RootSignature(), { L"HitGroup", L"ShadowHitGroup" });

			pipeline.SetMaxPayloadSize(4 * sizeof(float) + sizeof(UINT)); // RGB + distance + recursion depth
			pipeline.SetMaxAttributeSize(2 * sizeof(float)); // barycentric coordinates
			pipeline.SetMaxRecursionDepth(5);

			m_RtStateObject = pipeline.Generate();

			if (FAILED(m_RtStateObject->QueryInterface(IID_PPV_ARGS(&m_RtStateObjectProps))))
				throw std::runtime_error("Failed to query ID3D12StateObjectProperties");

			m_RtStateObject->SetName(L"rtStateObject");
		}

		ID3D12StateObjectProperties* GetRtStateObjectProps() const { return m_RtStateObjectProps.Get(); }
		ID3D12StateObject* GetRtStateObject() const { return m_RtStateObject.Get(); }
	};
}