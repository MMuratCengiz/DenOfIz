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

#include <vector>
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroupLayout.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/InputLayout.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Pipeline.h"
#include "DenOfIzGraphics/Backends/Interface/RootSignature.h"
#include "Interop.h"
#include "PerDrawBinding.h"
#include "PerFrameBinding.h"
#include "PerMaterialBinding.h"
#include "WorldData.h"

namespace DenOfIz
{

    class QuadPipeline final
    {
        DenOfIz_ShaderProgram                m_program       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_pipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_rootSignature = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  m_inputLayout   = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;
        DenOfIz_BindGroup                    m_bindGroups[ 15 ]{ };

    public:
        DZ_EXAMPLES_API QuadPipeline( DenOfIz_GraphicsApi graphicsApi, DenOfIz_LogicalDevice logicalDevice, const char *pixelShader );
        DZ_EXAMPLES_API ~QuadPipeline( );
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Pipeline      Pipeline( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_RootSignature RootSignature( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_BindGroup     BindGroup( uint32_t frame, uint32_t registerSpace = 0 ) const;
        DZ_EXAMPLES_API void                                Render( DenOfIz_CommandList commandList, uint32_t frame ) const;
    };
} // namespace DenOfIz
