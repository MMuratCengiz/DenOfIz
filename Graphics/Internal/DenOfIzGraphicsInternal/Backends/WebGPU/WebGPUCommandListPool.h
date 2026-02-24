/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandListPool.h>
#include <memory>
#include <vector>

#include "WebGPUCommandList.h"
#include "WebGPUContext.h"

namespace DenOfIz
{
    class WebGPUCommandListPool final : public ICommandListPool
    {
        WebGPUContext                                  *m_context{ };
        DenOfIz_CommandListPoolDesc                     m_desc{ };
        std::vector<std::unique_ptr<WebGPUCommandList>> m_commandLists{ };
        std::vector<DenOfIz_CommandList>                m_commandListPtrs{ };

    public:
        WebGPUCommandListPool( WebGPUContext *context, const DenOfIz_CommandListPoolDesc &desc );
        ~WebGPUCommandListPool( ) override;

        DenOfIz_CommandListArray GetCommandLists( ) override;
    };

} // namespace DenOfIz
