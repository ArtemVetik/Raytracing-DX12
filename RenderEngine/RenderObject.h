#pragma once

#include "framework.h"
#include "Mesh.h"
#include "Material.h"

#include "../Core/Graphics/BufferD3D12.h"
#include "../Core/EduMath/SimpleMath.h"

using namespace EduEngine;

namespace RaytracingDX12
{
	class RenderObject
	{
	public:
		RenderObject();
		~RenderObject();

		void SetMesh(Mesh* mesh);
		void SetMaterial(Material* material);

		VertexBufferD3D12* GetVertexBuffer() const;
		IndexBufferD3D12* GetIndexBuffer() const;

		Mesh* GetMesh() const { return m_Mesh; }
		Material* GetMaterial() const { return m_Material; }

		static constexpr int MaxInstances = 16;

		int InstanceCount = 1;
		std::unique_ptr<UploadBufferD3D12> ObjectUpload[MaxInstances] = {};
		DirectX::SimpleMath::Matrix WorldMatrix[MaxInstances] = { DirectX::SimpleMath::Matrix::Identity };
		DirectX::SimpleMath::Matrix TextureTransform = DirectX::SimpleMath::Matrix::Identity;

	private:
		Mesh* m_Mesh;
		Material* m_Material;
	};
}