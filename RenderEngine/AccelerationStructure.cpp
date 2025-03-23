#include "AccelerationStructure.h"
#include "MeshData.h"

#include "DXRHelpers/DXRHelper.h"
#include "DXRHelpers/nv_helpers_dx12/BottomLevelASGenerator.h"

namespace RaytracingDX12
{
	AccelerationStructure::AccelerationStructure(RenderDeviceD3D12* device) :
		m_Device(device)
	{
	}

	void AccelerationStructure::CreateAccelerationStructures(Mesh* mesh)
	{
		AccelerationStructureBuffers bottomLevelBuffers = CreateBottomLevelAS({ mesh });

		m_instances = { {bottomLevelBuffers.pResult, DirectX::XMMatrixIdentity()} };
		CreateTopLevelAS(m_instances);

		auto& commandContext = m_Device->GetCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		commandQueue.CloseAndExecuteCommandContext(&commandContext);
		commandQueue.Flush();

		commandContext.Reset();

		m_bottomLevelAS = bottomLevelBuffers.pResult;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
		
		auto allocation = m_Device->AllocateGPUDescriptor(QueueID::Direct, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
		m_Device->GetD3D12Device()->CreateShaderResourceView(nullptr, &srvDesc, allocation.GetCpuHandle());
		m_SrvView = std::make_unique<BufferHeapView>(std::move(allocation));
	}

	AccelerationStructure::AccelerationStructureBuffers AccelerationStructure::CreateBottomLevelAS(std::vector<Mesh*> meshes)
	{
		nv_helpers_dx12::BottomLevelASGenerator bottomLevelAS;
		
		for (const auto& mesh : meshes)
		{
			bottomLevelAS.AddVertexBuffer(mesh->GetVertexBuffer()->GetD3D12Resource(), 0, mesh->GetVertexCount(), sizeof(Vertex),
				mesh->GetIndexBuffer()->GetD3D12Resource(), 0, mesh->GetIndexCount(), 0, 0);
		}

		UINT64 scratchSizeInBytes = 0;
		UINT64 resultSizeInBytes = 0;

		bottomLevelAS.ComputeASBufferSizes(m_Device->GetD3D12Device(), false, &scratchSizeInBytes, &resultSizeInBytes);

		AccelerationStructureBuffers buffers;
		buffers.pScratch = nv_helpers_dx12::CreateBuffer(
			m_Device->GetD3D12Device(), scratchSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
			nv_helpers_dx12::kDefaultHeapProps);
		buffers.pResult = nv_helpers_dx12::CreateBuffer(
			m_Device->GetD3D12Device(), resultSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nv_helpers_dx12::kDefaultHeapProps);

		bottomLevelAS.Generate(m_Device->GetCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT).GetCmdList(), buffers.pScratch.Get(),
			buffers.pResult.Get(), false, nullptr);

		return buffers;
	}

	void AccelerationStructure::CreateTopLevelAS(const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances)
	{
		for (size_t i = 0; i < instances.size(); i++)
		{
			m_topLevelASGenerator.AddInstance(
				instances[i].first.Get(),
				instances[i].second,
				static_cast<UINT>(i),
				static_cast<UINT>(0)
			);
		}

		UINT64 scratchSize, resultSize, instanceDescsSize;

		m_topLevelASGenerator.ComputeASBufferSizes(m_Device->GetD3D12Device(), true, &scratchSize, &resultSize, &instanceDescsSize);

		m_topLevelASBuffers.pScratch = nv_helpers_dx12::CreateBuffer(
			m_Device->GetD3D12Device(), scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nv_helpers_dx12::kDefaultHeapProps);
		m_topLevelASBuffers.pResult = nv_helpers_dx12::CreateBuffer(
			m_Device->GetD3D12Device(), resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nv_helpers_dx12::kDefaultHeapProps);

		m_topLevelASBuffers.pInstanceDesc = nv_helpers_dx12::CreateBuffer(
			m_Device->GetD3D12Device(), instanceDescsSize, D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);

		m_topLevelASGenerator.Generate(m_Device->GetCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT).GetCmdList(),
			m_topLevelASBuffers.pScratch.Get(),
			m_topLevelASBuffers.pResult.Get(),
			m_topLevelASBuffers.pInstanceDesc.Get());
	}
}