#include "Mesh.h"
#include "MeshData.h"

namespace RaytracingDX12
{
	Mesh::Mesh(RenderDeviceD3D12* device, const char* filePath) :
		m_Device(device),
		m_FilePath(filePath),
		m_Scene(nullptr),
		m_RefCount(0)
	{

	}

	Mesh::~Mesh()
	{
		m_RefCount = 0;
		m_Scene = nullptr;
		m_AssimpImporter.FreeScene();
		m_VertexBuffer.reset();
		m_IndexBuffer.reset();
	}

	void Mesh::Load()
	{
		if (m_RefCount > 0)
		{
			m_RefCount++;
			return;
		}

		m_Scene = m_AssimpImporter.ReadFile(m_FilePath, aiProcess_FlipUVs);

		MeshData meshData;
		for (int i = 0; i < m_Scene->mMeshes[0]->mNumVertices; i++)
		{
			auto aiVertex = m_Scene->mMeshes[0]->mVertices[i];
			auto aiNormal = m_Scene->mMeshes[0]->mNormals[i];
			auto aiTangents = m_Scene->mMeshes[0]->mTangents ? m_Scene->mMeshes[0]->mTangents[i] : aiVector3D();
			auto aiTexC = m_Scene->mMeshes[0]->mTextureCoords[0] ? m_Scene->mMeshes[0]->mTextureCoords[0][i] : aiVector3D();

			meshData.Vertices.push_back(Vertex(
				{ aiVertex.x, aiVertex.y, aiVertex.z },
				{ aiNormal.x, aiNormal.y, aiNormal.z },
				{ aiTangents.x, aiTangents.y, aiTangents.z },
				{ aiTexC.x, aiTexC.y }
			));
		}

		for (size_t i = 0; i < m_Scene->mMeshes[0]->mNumFaces; i++)
		{
			for (size_t k = 0; k < m_Scene->mMeshes[0]->mFaces[i].mNumIndices; k += 3)
			{
				meshData.Indices32.push_back(m_Scene->mMeshes[0]->mFaces[i].mIndices[k + 2]);
				meshData.Indices32.push_back(m_Scene->mMeshes[0]->mFaces[i].mIndices[k]);
				meshData.Indices32.push_back(m_Scene->mMeshes[0]->mFaces[i].mIndices[k + 1]);
			}
		}

		m_VertexBuffer = std::make_shared<VertexBufferD3D12>(m_Device, meshData.Vertices.data(),
			sizeof(Vertex), (UINT)meshData.Vertices.size());
		m_IndexBuffer = std::make_shared<IndexBufferD3D12>(m_Device, meshData.GetIndices16().data(),
			sizeof(uint16), (UINT)meshData.GetIndices16().size(), DXGI_FORMAT_R16_UINT);

		m_RefCount = 1;
	}

	void Mesh::Free()
	{
		if (m_RefCount == 0)
			return;

		m_RefCount--;

		if (m_RefCount == 0)
		{
			m_VertexBuffer.reset();
			m_IndexBuffer.reset();

			m_AssimpImporter.FreeScene();
			m_Scene = nullptr;
		}

	}

	void Mesh::UpdateFilePath(const char* filePath)
	{
		m_FilePath = filePath;
	}

	int Mesh::GetVertexCount()
	{
		return m_Scene->mMeshes[0]->mNumVertices;
	}

	int Mesh::GetIndexCount()
	{
		return m_Scene->mMeshes[0]->mNumFaces * 3;
	}
}