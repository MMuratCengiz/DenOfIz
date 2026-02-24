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

#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
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
    class DefaultRenderPipeline
    {
        DenOfIz_ShaderProgram                  m_program       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                       m_pipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                  m_rootSignature = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                    m_inputLayout   = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout>   m_bindGroupLayouts;
        std::unordered_map<uint32_t, uint32_t> m_spaceToLayoutIndex;

        std::unique_ptr<PerDrawBinding>     m_perDrawBinding;
        std::unique_ptr<PerFrameBinding>    m_perFrameBinding;
        std::unique_ptr<PerMaterialBinding> m_perMaterialBinding;

    public:
        DefaultRenderPipeline( DenOfIz_GraphicsApi graphicsApi, DenOfIz_LogicalDevice logicalDevice );
        ~DefaultRenderPipeline( );
        [[nodiscard]] PerMaterialBinding *GetPerMaterialBinding( ) const;
        void                              Render( DenOfIz_CommandList commandList, const WorldData &worldData ) const;
    };
} // namespace DenOfIz
