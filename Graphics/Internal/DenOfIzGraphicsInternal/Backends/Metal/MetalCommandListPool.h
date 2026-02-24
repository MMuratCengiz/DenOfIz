#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandListPool.h>
#include "MetalCommandList.h"

namespace DenOfIz
{
    class MetalCommandListPool final : public ICommandListPool
    {
        MetalContext                              *m_context{ };
        DenOfIz_CommandListPoolDesc                m_desc;
        std::vector<std::unique_ptr<ICommandList>> m_commandLists;
        std::vector<DenOfIz_CommandList>           m_commandListPtrs;

    public:
        MetalCommandListPool( MetalContext *context, DenOfIz_CommandListPoolDesc desc );
        DenOfIz_CommandListArray GetCommandLists( ) override;
        ~MetalCommandListPool( ) override = default;
    };
} // namespace DenOfIz
