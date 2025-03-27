#include "Scene.h"
#include <random>

namespace RaytracingDX12
{
	Scene::Scene(RenderDeviceD3D12* device)
	{
		m_PlaneMesh = std::make_shared<Mesh>(device, "Models\\plane.fbx");
		m_PlaneMesh->Load();

		m_MainMesh = std::make_shared<Mesh>(device, "Models\\joseph.fbx");
		m_MainMesh->Load();

		m_SphereMesh = std::make_shared<Mesh>(device, "Models\\Sphere.fbx");
		m_SphereMesh->Load();

		m_WallMesh = std::make_shared<Mesh>(device, "Models\\Cube.fbx");
		m_WallMesh->Load();

		m_PlaneTexture = std::make_unique<Texture>(device, L"Textures\\tile.dds");
		m_MainTexture = std::make_unique<Texture>(device, L"Textures\\joseph_albedo.dds");
		m_WhiteTexture = std::make_unique<Texture>(device, L"Textures\\white.dds");

		m_PlaneMaterial = std::make_shared<Material>(device);
		m_PlaneMaterial->SetMainTexture(m_PlaneTexture.get());
		m_PlaneMaterial->Load();

		m_MainMaterial = std::make_shared<Material>(device);
		m_MainMaterial->SetMainTexture(m_MainTexture.get());
		m_MainMaterial->Load();

		m_WhiteMaterial = std::make_shared<Material>(device);
		m_WhiteMaterial->SetMainTexture(m_WhiteTexture.get());
		m_WhiteMaterial->Load();

		m_PlaneRenderObject = std::make_shared<RenderObject>();
		m_PlaneRenderObject->SetMaterial(m_PlaneMaterial.get());
		m_PlaneRenderObject->SetMesh(m_PlaneMesh.get());
		m_PlaneRenderObject->InstanceCount = 1;
		m_PlaneRenderObject->WorldMatrix[0] = SimpleMath::Matrix::CreateScale(0.5f) * SimpleMath::Matrix::CreateTranslation(0, -2.5, 0);
		m_PlaneRenderObject->ObjectUpload[0] = std::make_unique<UploadBufferD3D12>(device, CD3DX12_RESOURCE_DESC::Buffer(sizeof(RaytracingPass::ObjectConstants)), QueueID::Direct);
		m_PlaneRenderObject->ObjectUpload[0]->LoadData(&(m_PlaneRenderObject->WorldMatrix[0].Transpose()));

		m_MainRenderObject = std::make_shared<RenderObject>();
		m_MainRenderObject->SetMaterial(m_MainMaterial.get());
		m_MainRenderObject->SetMesh(m_MainMesh.get());
		m_MainRenderObject->InstanceCount = 1;
		m_MainRenderObject->ObjectUpload[0] = std::make_unique<UploadBufferD3D12>(device, CD3DX12_RESOURCE_DESC::Buffer(sizeof(RaytracingPass::ObjectConstants)), QueueID::Direct);

		m_SphereRenderObject = std::make_shared<RenderObject>();
		m_SphereRenderObject->SetMaterial(m_WhiteMaterial.get());
		m_SphereRenderObject->SetMesh(m_SphereMesh.get());
		m_SphereRenderObject->InstanceCount = 16;

		for (size_t i = 0; i < m_SphereRenderObject->InstanceCount; i++)
			m_SphereRenderObject->ObjectUpload[i] = std::make_unique<UploadBufferD3D12>(device, CD3DX12_RESOURCE_DESC::Buffer(sizeof(RaytracingPass::ObjectConstants)), QueueID::Direct);

		m_WallRenderObject = std::make_shared<RenderObject>();
		m_WallRenderObject->SetMaterial(m_WhiteMaterial.get());
		m_WallRenderObject->SetMesh(m_WallMesh.get());
		m_WallRenderObject->InstanceCount = 2;
		m_WallRenderObject->WorldMatrix[0] = SimpleMath::Matrix::CreateScale(4, 80, 200) * SimpleMath::Matrix::CreateTranslation(-100, 40, 0);
		m_WallRenderObject->WorldMatrix[1] = SimpleMath::Matrix::CreateScale(4, 80, 200) * SimpleMath::Matrix::CreateTranslation(+100, 40, 0);
		m_WallRenderObject->ObjectUpload[0] = std::make_unique<UploadBufferD3D12>(device, CD3DX12_RESOURCE_DESC::Buffer(sizeof(RaytracingPass::ObjectConstants)), QueueID::Direct);
		m_WallRenderObject->ObjectUpload[0]->LoadData(&(m_WallRenderObject->WorldMatrix[0].Transpose()));
		m_WallRenderObject->ObjectUpload[1] = std::make_unique<UploadBufferD3D12>(device, CD3DX12_RESOURCE_DESC::Buffer(sizeof(RaytracingPass::ObjectConstants)), QueueID::Direct);
		m_WallRenderObject->ObjectUpload[1]->LoadData(&(m_WallRenderObject->WorldMatrix[1].Transpose()));

		XMFLOAT3 sphereColors[16] = {
			{1.0f, 0.2f, 0.2f},  // Red
			{0.2f, 1.0f, 0.2f},  // Green
			{0.2f, 0.2f, 1.0f},  // Blue
			{1.0f, 1.0f, 0.2f},  // Yellow
			{1.0f, 0.5f, 0.0f},  // Orange
			{0.6f, 0.2f, 1.0f},  // Purple
			{0.2f, 1.0f, 1.0f},  // Cyan
			{1.0f, 0.2f, 1.0f},  // Pink
			{0.5f, 0.5f, 0.5f},  // Gray (metallic)
			{0.0f, 0.8f, 0.5f},  // Teal
			{0.7f, 0.3f, 0.9f},  // Lavender
			{0.9f, 0.6f, 0.3f},  // Peach
			{0.3f, 0.9f, 0.6f},  // Mint
			{0.8f, 0.1f, 0.3f},  // Raspberry
			{0.2f, 0.6f, 1.0f},  // Sky Blue
			{1.0f, 0.8f, 0.3f}   // Golden
		};

		m_BrightMaterials = new Material * [BrightMaterialsSize];
		for (size_t i = 0; i < BrightMaterialsSize; i++)
		{
			m_BrightMaterials[i] = new Material(device);
			m_BrightMaterials[i]->Constants.DiffuseColor = sphereColors[i];
			m_BrightMaterials[i]->Load();
		}
	}

	Scene::~Scene()
	{
		for (size_t i = 0; i < BrightMaterialsSize; i++)
			delete m_BrightMaterials[i];

		delete[] m_BrightMaterials;
	}

	void Scene::Update(const Timer& timer)
	{
		m_MainRenderObject->WorldMatrix[0] = SimpleMath::Matrix::CreateScale(8.0f) * SimpleMath::Matrix::CreateRotationY(timer.GetTotalTime());
		m_MainRenderObject->ObjectUpload[0]->LoadData(&(m_MainRenderObject->WorldMatrix[0].Transpose()));

		float angleStep = XM_2PI / m_SphereRenderObject->InstanceCount;
		float radius = 70;
		for (size_t i = 0; i < m_SphereRenderObject->InstanceCount; i++)
		{
			float angle = i * angleStep;

			float x = radius * cos(angle);
			float z = radius * sin(angle);

			float y = 15 + sin(angle + timer.GetTotalTime()) * 5.0f;

			m_SphereRenderObject->WorldMatrix[i] = SimpleMath::Matrix::CreateScale(8) * SimpleMath::Matrix::CreateTranslation(x, y, z);
			m_SphereRenderObject->ObjectUpload[i]->LoadData(&(m_SphereRenderObject->WorldMatrix[0].Transpose()));
		}
	}

	XMFLOAT3 Scene::GenerateBrightColor()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> brightDist(0.5f, 1.0f);
		std::uniform_real_distribution<float> fullDist(0.0f, 1.0f);

		float r = fullDist(gen);
		float g = fullDist(gen);
		float b = fullDist(gen);

		float brightnessComponent = brightDist(gen);
		switch (gen() % 3) {
		case 0: r = brightnessComponent; break;
		case 1: g = brightnessComponent; break;
		case 2: b = brightnessComponent; break;
		}

		return { r, g, b };
	}
}