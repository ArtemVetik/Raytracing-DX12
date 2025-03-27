#pragma once

#include <memory>
#include <string>

#include "framework.h"
#include "Timer.h"
#include "Window.h"

namespace RaytracingDX12
{
	class RENDERENGINE_API IRenderEngine
	{
	public:
		virtual void Update(const Timer& timer) = 0;
		virtual void Render() = 0;

		virtual std::wstring GetAdapterInfo() const = 0;

		static std::shared_ptr<IRenderEngine> Create(const Window& mainWindow);
	};
}