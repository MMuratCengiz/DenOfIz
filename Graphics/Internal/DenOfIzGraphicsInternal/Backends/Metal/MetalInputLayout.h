/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IInputLayout.h>
#include <deque>
#include <vector>
#include "MetalContext.h"

namespace DenOfIz
{

    class MetalInputLayout final : public IInputLayout
    {
        MetalContext                                            *m_context;
        std::deque<std::string>                                  m_semanticStrings;
        std::vector<DenOfIz_InputGroupDesc>                      m_inputGroups;
        std::vector<std::vector<DenOfIz_InputLayoutElementDesc>> m_inputElements;
        MTLVertexDescriptor                                     *m_vertexDescriptor;

    public:
        MetalInputLayout( MetalContext *context, const DenOfIz_InputLayoutDesc &desc );
        [[nodiscard]] MTLVertexDescriptor *GetVertexDescriptor( ) const;
        ~MetalInputLayout( ) override;
    };

} // namespace DenOfIz
