#include "IRenderEngine.h"

#include "RenderEngine.h"

namespace RaytracingDX12
{
    std::shared_ptr<IRenderEngine> IRenderEngine::IRenderEngine::Create(const Window& mainWindow)
    {
        auto renderEngine = std::make_shared<RenderEngine>();
        renderEngine->StartUp(mainWindow);

        return renderEngine;
    }
}