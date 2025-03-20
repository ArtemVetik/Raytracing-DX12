#include "Material.h"

namespace RaytracingDX12
{
	Material::Material() :
		m_MainTexture(nullptr),
		m_RefCount(0),
		m_DiffuseAlbedo(1.0f, 1.0f, 1.0f, 1.0f),
		m_FresnelR0(0.01f, 0.01f, 0.01f),
		m_Roughness(0.25f)
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