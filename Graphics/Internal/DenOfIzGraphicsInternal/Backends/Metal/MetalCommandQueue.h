/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandQueue.h>
#include "MetalCommandList.h"
#include "MetalContext.h"

namespace DenOfIz
{
    class MetalCommandQueue : public ICommandQueue
    {
    private:
        MetalContext            *m_context;
        DenOfIz_CommandQueueDesc m_desc;
        id<MTLCommandQueue>      m_queue;

    public:
        MetalCommandQueue( MetalContext *context, const DenOfIz_CommandQueueDesc &desc );
        ~MetalCommandQueue( ) override;

        void WaitIdle( ) override;
        void ExecuteCommandLists( const DenOfIz_ExecuteCommandListsDesc &executeCommandListsDesc ) override;

        id<MTLCommandQueue> GetQueue( ) const
        {
            return m_queue;
        }
        DenOfIz_QueueType GetQueueType( ) const
        {
            return m_desc.QueueType;
        }
    };
} // namespace DenOfIz
