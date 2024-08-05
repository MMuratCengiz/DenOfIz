#pragma once

#include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
#include "MetalCommandList.h"

namespace DenOfIz
{
    class MetalCommandListPool : public ICommandListPool
    {
    private:
        MetalContext       *m_context;
        CommandListPoolDesc m_desc;

    public:
        MetalCommandListPool( MetalContext *context, CommandListPoolDesc desc );
        ~MetalCommandListPool( ) override = default;
    };
} // namespace DenOfIz
