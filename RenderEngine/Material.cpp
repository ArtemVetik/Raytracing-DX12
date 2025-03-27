#include "Material.h"

namespace RaytracingDX12
{
	Material::Material(RenderDeviceD3D12* device) :
		m_Device(device),
		m_MainTexture(nullptr),
		m_RefCount(0),
		Constants{}
	{
	}

	Material::~Material()
	{
		if (m_MainTexture)
			m_MainTexture->Free();
	}

	void Material::SetMainTexture(Texture* texture)
	{
		if (m_MainTexture && m_RefCount > 0)
			m_MainTexture->Free();

		m_MainTexture = texture;

		if (m_MainTexture && m_RefCount > 0)
			m_MainTexture->Load();
	}

	Texture* Material::GetMainTexture()
	{
		return m_MainTexture;
	}

	void Material::Load()
	{
		if (m_RefCount > 0)
		{
			m_RefCount++;
			return;
		}

		if (m_MainTexture)
			m_MainTexture->Load();

		m_MaterialBuffer = std::make_unique<BufferD3D12>(m_Device, CD3DX12_RESOURCE_DESC::Buffer(sizeof(MaterialConstants)), QueueID::Direct);
		m_MaterialBuffer->SetName(L"MaterialBuffer");
		m_MaterialBuffer->LoadData(&Constants);

		m_RefCount = 1;
	}

	void Material::Free()
	{
		if (m_RefCount <= 0)
			return;

		m_RefCount--;

		if (m_RefCount == 0)
		{
			if (m_MainTexture)
				m_MainTexture->Free();
		}
	}
}