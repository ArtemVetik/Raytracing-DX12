#pragma once

#include "framework.h"
#include "Texture.h"

namespace RaytracingDX12
{
	class Material
	{
	public:
		Material();
		~Material();

		void SetMainTexture(Texture* texture);
		Texture* GetMainTexture();

		void Load();
		void Free();

		int GetRefCount() const { return m_RefCount; }

	private:
		Texture* m_MainTexture;

		DirectX::XMFLOAT4 m_DiffuseAlbedo;
		DirectX::XMFLOAT3 m_FresnelR0;
		float m_Roughness;

		int m_RefCount;
	};
}