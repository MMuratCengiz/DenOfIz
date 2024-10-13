#pragma once

#include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
#include "MetalCommandList.h"

namespace DenOfIz
{
    class MetalCommandListPool final : public ICommandListPool
    {
    private:
        MetalContext                              *m_context{ };
        CommandListPoolDesc                        m_desc;
        std::vector<std::unique_ptr<ICommandList>> m_commandLists;

    public:
        MetalCommandListPool( MetalContext *context, CommandListPoolDesc desc );
        CommandLists GetCommandLists( ) override;
        ~MetalCommandListPool( ) override = default;
    };
} // namespace DenOfIz
