#pragma once

#include "framework.h"
#include "Texture.h"

#include "../Core/Graphics/BufferD3D12.h"
#include "../Core/EduMath/SimpleMath.h"

namespace RaytracingDX12
{
	struct MaterialConstants
	{
		DirectX::XMFLOAT3 DiffuseColor = { 1, 1, 1 };
		float DiffuseCoef = 0.9;
		float SpecularCoef = 0.7;
		float SpecularPower = 50;
		float ReflectanceCoef = 0.9;
		float InShadowRadiance = 0.35f;
	};

	class Material
	{
	public:
		Material(RenderDeviceD3D12* device);
		~Material();

		void SetMainTexture(Texture* texture);
		Texture* GetMainTexture();

		void Load();
		void Free();

		int GetRefCount() const { return m_RefCount; }
		ID3D12Resource* GetMaterialBuffer() const { return m_MaterialBuffer->GetD3D12Resource(); }

		MaterialConstants Constants;

	private:
		RenderDeviceD3D12* m_Device;
		Texture* m_MainTexture;

		std::unique_ptr<BufferD3D12> m_MaterialBuffer;
		int m_RefCount;
	};
}