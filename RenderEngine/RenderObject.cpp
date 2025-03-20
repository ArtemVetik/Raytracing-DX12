#include "RenderObject.h"

namespace RaytracingDX12
{
	RenderObject::RenderObject() :
		m_Mesh(nullptr),
		m_Material(nullptr)
	{
	}

	RenderObject::~RenderObject()
	{
		if (m_Mesh)
			m_Mesh->Free();

		if (m_Material)
			m_Material->Free();
	}

	void RenderObject::SetMesh(Mesh* mesh)
	{
		if (m_Mesh == mesh)
			return;

		if (m_Mesh)
			m_Mesh->Free();

		m_Mesh = mesh;

		if (m_Mesh)
			m_Mesh->Load();
	}

	void RenderObject::SetMaterial(Material* material)
	{
		if (m_Material == material)
			return;

		if (m_Material)
			m_Material->Free();

		m_Material = material;

		if (m_Material)
			m_Material->Load();
	}

	VertexBufferD3D12* RenderObject::GetVertexBuffer() const
	{
		if (m_Mesh)
			return m_Mesh->GetVertexBuffer();

		return nullptr;
	}

	IndexBufferD3D12* RenderObject::GetIndexBuffer() const
	{
		if (m_Mesh)
			return m_Mesh->GetIndexBuffer();

		return nullptr;
	}
}