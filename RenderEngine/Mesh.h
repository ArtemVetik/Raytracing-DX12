#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "framework.h"

#include "../Core/Graphics/BufferD3D12.h"

using namespace EduEngine;

namespace RaytracingDX12
{
	class Mesh
	{
	public:
		Mesh(RenderDeviceD3D12* device, const char* filePath);
		~Mesh();

		void Load();
		void Free();

		void UpdateFilePath(const char* filePath);

		int GetRefCount() const { return m_RefCount; }
		const char* GetFilePath() const { return m_FilePath; }

		int GetVertexCount();
		int GetIndexCount();

		VertexBufferD3D12* GetVertexBuffer() const { return m_VertexBuffer.get(); }
		IndexBufferD3D12* GetIndexBuffer() const { return m_IndexBuffer.get(); }

		const aiMesh* GetAiMesh() const { return m_Scene->mMeshes[0]; }

	private:
		RenderDeviceD3D12* m_Device;
		Assimp::Importer m_AssimpImporter;
		const aiScene* m_Scene;
		std::shared_ptr<VertexBufferD3D12> m_VertexBuffer;
		std::shared_ptr<IndexBufferD3D12> m_IndexBuffer;

		const char* m_FilePath;
		int m_RefCount;
	};
}