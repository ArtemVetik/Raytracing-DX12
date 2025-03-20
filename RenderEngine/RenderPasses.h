#pragma once

#include "framework.h"

#include "../Core/Graphics/PipelineStateD3D12.h"
#include "../Core/Graphics/ComputePipelineStateD3D12.h"
#include "../Core/Graphics/CommandSignatureD3D12.h"
#include "../Core/EduMath/SimpleMath.h"

using namespace EduEngine;

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
			m_VertexShader(L"Shaders\\Color.hlsl", EDU_SHADER_TYPE_VERTEX, macros, L"VS", L"vs_6_0"),
			m_PixelShader(L"Shaders\\Color.hlsl", EDU_SHADER_TYPE_PIXEL, macros, L"PS", L"ps_6_0")
		{
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
}