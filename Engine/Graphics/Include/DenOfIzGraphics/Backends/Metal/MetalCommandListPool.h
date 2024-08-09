#pragma once

#include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
#include "MetalCommandList.h"

namespace DenOfIz
{
    class MetalCommandListPool : public ICommandListPool
    {
    private:
        MetalContext                              *m_context{ };
        CommandListPoolDesc                        m_desc;
        std::vector<std::unique_ptr<ICommandList>> m_commandLists;

    public:
        MetalCommandListPool( MetalContext *context, CommandListPoolDesc desc );
        std::vector<ICommandList *> GetCommandLists( ) override;
        ~MetalCommandListPool( ) override = default;
    };
} // namespace DenOfIz
