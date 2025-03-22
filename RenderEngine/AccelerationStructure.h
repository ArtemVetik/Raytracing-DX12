#pragma once

#include <wrl/client.h>
#include <dxcapi.h>
#include <vector>

#include "framework.h"
#include "Mesh.h"

#include "../Core/Graphics/RenderDeviceD3D12.h"
#include "DXRHelpers/nv_helpers_dx12/TopLevelASGenerator.h"

namespace RaytracingDX12
{
	using namespace Microsoft::WRL;
	using namespace EduEngine;

	class AccelerationStructure
	{
	public:
		AccelerationStructure(RenderDeviceD3D12* device);

		/// Create all acceleration structures, bottom and top
		void CreateAccelerationStructures(Mesh* mesh);

	private:
		struct AccelerationStructureBuffers
		{
			ComPtr<ID3D12Resource> pScratch;
			ComPtr<ID3D12Resource> pResult;
			ComPtr<ID3D12Resource> pInstanceDesc;
		};

		/// Create the acceleration structure of an instance
		///
		/// \param     vVertexBuffers : pair of buffer and vertex count
		/// \return    AccelerationStructureBuffers for TLAS
		AccelerationStructureBuffers CreateBottomLevelAS(std::vector<Mesh*> meshes);

		/// Create the main acceleration structure that holds
		/// all instances of the scene
		/// \param     instances : pair of BLAS and transform
		void CreateTopLevelAS(const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances);

	private:
		RenderDeviceD3D12* m_Device;
		ComPtr<ID3D12Resource> m_bottomLevelAS;

		nv_helpers_dx12::TopLevelASGenerator m_topLevelASGenerator;
		AccelerationStructureBuffers m_topLevelASBuffers;
		std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> m_instances;
	};
}