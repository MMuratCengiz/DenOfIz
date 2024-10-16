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

#include "Interop.h"
#include <DenOfIzGraphics/Backends/GraphicsApi.h>
#include "PerDrawBinding.h"
#include "PerFrameBinding.h"
#include "PerMaterialBinding.h"
#include "WorldData.h"

namespace DenOfIz
{

    class QuadPipeline final
    {
        std::unique_ptr<IPipeline>          m_pipeline;
        std::unique_ptr<IRootSignature>     m_rootSignature;
        std::unique_ptr<IInputLayout>       m_inputLayout;
        std::unique_ptr<IResourceBindGroup> m_bindGroups[ 15 ];

    public:
        DZ_EXAMPLES_API                                   QuadPipeline( const GraphicsApi *graphicsApi, ILogicalDevice *logicalDevice, const char *pixelShader );
        [[nodiscard]] DZ_EXAMPLES_API IPipeline          *Pipeline( ) const;
        [[nodiscard]] DZ_EXAMPLES_API IRootSignature     *RootSignature( ) const;
        [[nodiscard]] DZ_EXAMPLES_API IResourceBindGroup *BindGroup( uint32_t frame, uint32_t registerSpace = 0 ) const;
        DZ_EXAMPLES_API void                              Render( ICommandList *commandList, uint32_t frame ) const;
    };
} // namespace DenOfIz
