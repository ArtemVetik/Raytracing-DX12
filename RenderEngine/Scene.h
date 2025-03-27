#pragma once

#include "framework.h"

#include "RenderPasses.h"
#include "RenderObject.h"
#include "Timer.h"

namespace RaytracingDX12
{
	class Scene
	{
	public:
		Scene(RenderDeviceD3D12* device);
		~Scene();

		void Update(const Timer& timer);

		RenderObject* GetPlane() const { return m_PlaneRenderObject.get(); }
		RenderObject* GetMain() const { return m_MainRenderObject.get(); }
		RenderObject* GetSphere() const { return m_SphereRenderObject.get(); }
		RenderObject* GetWall() const { return m_WallRenderObject.get(); }

		Material** GetBrightMaterials() const { return m_BrightMaterials; }

		static constexpr int BrightMaterialsSize = 16;

	private:
		XMFLOAT3 GenerateBrightColor();

	private:
		std::shared_ptr<Texture> m_PlaneTexture;
		std::shared_ptr<Texture> m_MainTexture;
		std::shared_ptr<Texture> m_WhiteTexture;
		std::shared_ptr<Mesh> m_PlaneMesh;
		std::shared_ptr<Mesh> m_MainMesh;
		std::shared_ptr<Mesh> m_SphereMesh;
		std::shared_ptr<Mesh> m_WallMesh;
		std::shared_ptr<Material> m_PlaneMaterial;
		std::shared_ptr<Material> m_MainMaterial;
		std::shared_ptr<Material> m_WhiteMaterial;
		std::shared_ptr<RenderObject> m_PlaneRenderObject;
		std::shared_ptr<RenderObject> m_MainRenderObject;
		std::shared_ptr<RenderObject> m_SphereRenderObject;
		std::shared_ptr<RenderObject> m_WallRenderObject;

		Material** m_BrightMaterials;
	};
}