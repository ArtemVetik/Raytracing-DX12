#include "RenderEngine.h"

#include <dxgi1_6.h>

#include "../Core/InputSystem/InputManager.h"
#include "../Core/Graphics/DynamicUploadBuffer.h"

namespace RaytracingDX12
{
	RenderEngine* RenderEngine::m_Instance = nullptr;

	RenderEngine* RenderEngine::GetInstance()
	{
		return m_Instance;
	}

	RenderEngine::RenderEngine() :
		m_Viewport{},
		m_ScissorRect{}
	{
		assert(m_Instance == nullptr);
		m_Instance = this;
	}

	RenderEngine::~RenderEngine()
	{
		if (m_Device != nullptr)
			m_Device->FlushQueues();
	}

	bool RenderEngine::StartUp(const Window& mainWindow)
	{
#if defined(DEBUG) || defined(_DEBUG) 
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
		debugController->EnableDebugLayer();
#endif

		IDXGIFactory6* pFactory = nullptr;
		if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory6), (void**)&pFactory)))
		{
			OutputDebugStringW(L"Failed to create DXGI Factory!");
			return false;
		}

		IDXGIAdapter1* pAdapter = nullptr;

		if (FAILED(pFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter1), (void**)&pAdapter)))
		{
			OutputDebugStringW(L"Failed to get high-performance GPU!");
			pFactory->Release();
			return false;
		}

		Microsoft::WRL::ComPtr<ID3D12Device5> device;
		HRESULT hardwareResult = D3D12CreateDevice(
			pAdapter,
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(device.GetAddressOf()));

		if (FAILED(hardwareResult))
		{
			OutputDebugStringW(L"The selected GPU does not support DirectX 12!");
			pAdapter->Release();
			pFactory->Release();
			return false;
		}

		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
			throw std::runtime_error("Failed to check D3D12_FEATURE_D3D12_OPTIONS5 feature support");

		if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
			throw std::runtime_error("Raytracing not supported on device");

		pAdapter->GetDesc1(&m_DeviceDesc);

		pAdapter->Release();
		pFactory->Release();

		m_Device = std::make_unique<RenderDeviceD3D12>(device);

		m_SwapChain = std::make_unique<SwapChain>(m_Device.get(),
			mainWindow.GetClientWidth(), mainWindow.GetClientHeight(), mainWindow.GetMainWindow());

		m_Camera = std::make_unique<Camera>(m_Device.get(), mainWindow.GetClientWidth(), mainWindow.GetClientHeight());

		Resize(mainWindow.GetClientWidth(), mainWindow.GetClientHeight());

		m_Device->GetCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT).Reset();

		m_ColorPass = std::make_unique<ColorPass>(m_Device.get());

		m_Mesh = std::make_shared<Mesh>(m_Device.get(), "Models\\joseph.fbx");
		m_Mesh->Load();

		m_Texture = std::make_unique<Texture>(m_Device.get(), L"Textures\\principledshader_albedo.dds");
		m_Material = std::make_shared<Material>();
		m_Material->SetMainTexture(m_Texture.get());
		m_Material->Load();

		m_RenderObject = std::make_shared<RenderObject>();
		m_RenderObject->SetMaterial(m_Material.get());
		m_RenderObject->SetMesh(m_Mesh.get());

		m_AccelerationStructure = std::make_unique<AccelerationStructure>(m_Device.get());
		m_AccelerationStructure->CreateAccelerationStructures(m_Mesh.get());

		return true;
	}

	void RenderEngine::Update(const Timer& timer)
	{
		XMVECTOR direction = XMLoadFloat3(&m_Camera->GetLook());
		XMVECTOR lrVector = XMLoadFloat3(&m_Camera->GetRight());
		XMVECTOR upVector = XMLoadFloat3(&m_Camera->GetUp());

		float moveScale = 35.0f;
		static constexpr float rotateScale = 0.01f;
		static constexpr float rotateLerpSpeed = 20.0f;

		if (InputManager::GetInstance().IsKeyPressed(DIK_LSHIFT))
			moveScale *= 2;

		if (InputManager::GetInstance().IsKeyPressed(DIK_W))
			m_Camera->Move(direction * moveScale * timer.GetDeltaTime());
		if (InputManager::GetInstance().IsKeyPressed(DIK_S))
			m_Camera->Move(-direction * moveScale * timer.GetDeltaTime());
		if (InputManager::GetInstance().IsKeyPressed(DIK_A))
			m_Camera->Move(-lrVector * moveScale * timer.GetDeltaTime());
		if (InputManager::GetInstance().IsKeyPressed(DIK_D))
			m_Camera->Move(lrVector * moveScale * timer.GetDeltaTime());
		if (InputManager::GetInstance().IsKeyPressed(DIK_E))
			m_Camera->Move(upVector * moveScale * timer.GetDeltaTime());
		if (InputManager::GetInstance().IsKeyPressed(DIK_Q))
			m_Camera->Move(-upVector * moveScale * timer.GetDeltaTime());

		auto mouseState = InputManager::GetInstance().GetMouseState();

		static XMFLOAT2 currentDelta = { 0, 0 };
		static XMFLOAT2 targetDelta = { 0, 0 };

		if ((mouseState.rgbButtons[1] & 0x80) != 0)
		{
			targetDelta.x += mouseState.lX * rotateScale;
			targetDelta.y += mouseState.lY * rotateScale;
		}

		auto Lerp = [](float a, float b, float t) {
			return a + (b - a) * t;
			};

		float prevX = currentDelta.x;
		currentDelta.x = Lerp(currentDelta.x, targetDelta.x, timer.GetDeltaTime() * rotateLerpSpeed);
		m_Camera->RotateY(currentDelta.x - prevX);

		float prevY = currentDelta.y;
		currentDelta.y = Lerp(currentDelta.y, targetDelta.y, timer.GetDeltaTime() * rotateLerpSpeed);
		m_Camera->Pitch(currentDelta.y - prevY);

		m_Camera->Update(timer);
	}

	void RenderEngine::Render()
	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { m_Device->GetD3D12DescriptorHeap() };

		auto& dCommandContext = m_Device->GetCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto& dCommandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		dCommandContext.Reset();
		dCommandContext.ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChain->CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		dCommandContext.FlushResourceBarriers();
		dCommandContext.GetCmdList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		dCommandContext.SetRenderTargets(1, &m_SwapChain->CurrentBackBufferView(), true, &m_SwapChain->DepthStencilView());
		dCommandContext.SetViewports(&m_Viewport, 1);
		dCommandContext.SetScissorRects(&m_ScissorRect, 1);

		const float clear[4] = { 0, 1, 0, 1 };
		dCommandContext.GetCmdList()->ClearRenderTargetView(m_SwapChain->CurrentBackBufferView(), clear, 0, nullptr);
		dCommandContext.GetCmdList()->ClearDepthStencilView(m_SwapChain->DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		dCommandContext.GetCmdList()->SetPipelineState(m_ColorPass->GetD3D12PipelineState());
		dCommandContext.GetCmdList()->SetGraphicsRootSignature(m_ColorPass->GetD3D12RootSignature());
	
		ColorPass::ObjectConstants objConstants;
		objConstants.World = (SimpleMath::Matrix::CreateScale(0.1f) * SimpleMath::Matrix::CreateRotationY(XM_PI)).Transpose();

		ColorPass::PassConstants passConstants;
		XMStoreFloat4x4(&passConstants.ViewProj, XMMatrixTranspose(m_Camera->GetViewProjMatrix()));

		DynamicUploadBuffer objBuffer(m_Device.get(), QueueID::Direct);
		objBuffer.LoadData(objConstants);

		DynamicUploadBuffer passBuffer(m_Device.get(), QueueID::Direct);
		passBuffer.LoadData(passConstants);

		D3D12_GPU_DESCRIPTOR_HANDLE albedoTex = {};
		albedoTex.ptr = reinterpret_cast<UINT64>(m_Texture->GetGPUPtr());
		dCommandContext.GetCmdList()->SetGraphicsRootDescriptorTable(0, albedoTex);
		dCommandContext.GetCmdList()->SetGraphicsRootConstantBufferView(1, objBuffer.GetAllocation().GPUAddress);
		dCommandContext.GetCmdList()->SetGraphicsRootConstantBufferView(2, passBuffer.GetAllocation().GPUAddress);

		dCommandContext.GetCmdList()->IASetVertexBuffers(0, 1, &(m_RenderObject->GetVertexBuffer()->GetView()));
		dCommandContext.GetCmdList()->IASetIndexBuffer(&(m_RenderObject->GetIndexBuffer()->GetView()));
		dCommandContext.GetCmdList()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		dCommandContext.GetCmdList()->DrawIndexedInstanced(m_RenderObject->GetIndexBuffer()->GetLength(), 1, 0, 0, 0);

		dCommandContext.ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChain->CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		dCommandContext.FlushResourceBarriers();

		dCommandQueue.CloseAndExecuteCommandContext(&dCommandContext);
		dCommandContext.Reset();

		m_SwapChain->Present();
		m_Device->FinishFrame();

		// Resize should be at the end of the frame after the main ExecuteCommandList and FinishFrame.
		// Since FinishFrame also occurs inside swapChain->Resize(), and it is not desirable
		// that resources are removed before the end of rendering
		if (m_PendingResize != EmptyResize)
		{
			Resize(m_PendingResize.width, m_PendingResize.height);
			m_PendingResize = EmptyResize;
			dCommandContext.Reset();
		}
	}

	void RenderEngine::PendingResize(UINT w, UINT h)
	{
		UINT lx, ly, lw, lh;
		Window::GetInstance()->GetPosition(lx, ly, lw, lh);

		if (lw != w || lh != h)
			m_PendingResize = { (long)lx, (long)ly, (long)w, (long)h };
	}

	void RenderEngine::Resize(UINT w, UINT h)
	{
		auto& commandContext = m_Device->GetCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		commandContext.Reset();
		m_SwapChain->Resize(w, h);

		commandContext.FlushResourceBarriers();
		commandQueue.CloseAndExecuteCommandContext(&commandContext);

		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;
		m_Viewport.Width = w;
		m_Viewport.Height = h;
		m_Viewport.MinDepth = 0.0f;
		m_Viewport.MaxDepth = 1.0f;

		m_ScissorRect = { 0, 0, (int)w, (int)h };

		m_Camera->SetProjectionMatrix(w, h);
	}
}