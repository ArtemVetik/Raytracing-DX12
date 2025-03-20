#pragma once

#include "../Core/Graphics/TextureD3D12.h"

using namespace EduEngine;

namespace RaytracingDX12
{
	class Texture
	{
	public:
		Texture(RenderDeviceD3D12* device, const wchar_t* filePath);
		~Texture();

		void UpdateFilePath(const wchar_t* filePath);
		void* GetGPUPtr();

		void Load(D3D12_SHADER_RESOURCE_VIEW_DESC* overrideDesc = nullptr);
		void Free();

		const wchar_t* GetFilePath() const { return m_FilePath; }
		int GetRefCount() const { return m_RefCount; }

	private:
		RenderDeviceD3D12* m_Device;
		const wchar_t* m_FilePath;
		std::shared_ptr<TextureD3D12> m_Texture;
		int m_RefCount;
	};
}