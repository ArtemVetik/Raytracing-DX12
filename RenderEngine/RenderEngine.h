#pragma once

#include "IRenderEngine.h"
#include "RenderPasses.h"
#include "Timer.h"
#include "Camera.h"
#include "Window.h"
#include "RenderObject.h"
#include "AccelerationStructure.h"

#include "../Core/Graphics/SwapChain.h"
#include "DXRHelpers/nv_helpers_dx12/ShaderBindingTableGenerator.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace RaytracingDX12
{
	class RenderEngine : public IRenderEngine
	{
	public:
		RenderEngine();
		~RenderEngine();

		RenderEngine(const RenderEngine& rhs) = delete;
		RenderEngine& operator=(const RenderEngine& rhs) = delete;

		bool StartUp(const Window& mainWindow);

		void Update(const Timer& timer) override;
		void Render() override;

		bool IsRaster() const override { return m_Raster; }

		void PendingResize(UINT w, UINT h);

		static RenderEngine* GetInstance();

	private:
		void Resize(UINT w, UINT h);
		void ResizeOutputBuffer();

	private:
		static RenderEngine* m_Instance;

		std::unique_ptr<RenderDeviceD3D12> m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;

		std::unique_ptr<Camera> m_Camera;

		std::unique_ptr<ColorPass> m_ColorPass;
		std::unique_ptr<RaytracingPass> m_RaytracingPass;

		std::shared_ptr<Texture> m_Texture;
		std::shared_ptr<Mesh> m_Mesh;
		std::shared_ptr<Material> m_Material;
		std::shared_ptr<Mesh> m_PlaneMesh;
		std::shared_ptr<Texture> m_PlaneTexture;
		std::shared_ptr<Material> m_PlaneMaterial;
		std::shared_ptr<RenderObject> m_RenderObject;
		std::shared_ptr<RenderObject> m_PlaneRenderObject;

		std::unique_ptr<AccelerationStructure> m_AccelerationStructure;
		std::unique_ptr<BufferD3D12> m_MissPadding;
		std::unique_ptr<TextureD3D12> m_OutputBuffer;
		nv_helpers_dx12::ShaderBindingTableGenerator m_SbtHelper;
		std::unique_ptr<UploadBufferD3D12> m_SbtStorage;
		std::unique_ptr<UploadBufferD3D12> m_PassUpload;
		std::unique_ptr<UploadBufferD3D12> m_CamUpload;

		XMFLOAT3 m_LightPos;
		bool m_Raster;
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;
		DXGI_ADAPTER_DESC1 m_DeviceDesc;

		static constexpr DirectX::SimpleMath::Rectangle EmptyResize = { -1, -1, -1, -1 };
		DirectX::SimpleMath::Rectangle m_PendingResize = EmptyResize;
	};
}