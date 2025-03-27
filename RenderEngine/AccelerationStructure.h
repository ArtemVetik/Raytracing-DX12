#pragma once

#include <wrl/client.h>
#include <dxcapi.h>
#include <vector>

#include "framework.h"
#include "RenderObject.h"
#include "Timer.h"

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

		void CreateAccelerationStructures(RenderObject** renderObjects, int size);
		void CreateTopLevelAS(bool updateOnly = false);
		
		void Update(DirectX::XMMATRIX world);

		BufferHeapView* GetSrvView() const { return m_SrvView.get(); }

	private:
		struct AccelerationStructureBuffers
		{
			ComPtr<ID3D12Resource> pScratch;
			ComPtr<ID3D12Resource> pResult;
			ComPtr<ID3D12Resource> pInstanceDesc;
		};

		void CreateTopLevelAS(const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances, bool updateOnly = false);
		AccelerationStructureBuffers CreateBottomLevelAS(std::vector<Mesh*> meshes);

	private:
		RenderDeviceD3D12* m_Device;

		nv_helpers_dx12::TopLevelASGenerator m_TopLevelASGenerator;
		AccelerationStructureBuffers m_TopLevelASBuffers;
		std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> m_Instances;

		std::unique_ptr<BufferHeapView> m_SrvView;
	};
}