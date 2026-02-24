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

#include <webgpu/webgpu.h>
#include "DenOfIzGraphicsInternal/Backends/Interface/IPipeline.h"
#include "WebGPUContext.h"
#include "WebGPURootSignature.h"

namespace DenOfIz
{
    class WebGPUPipeline final : public IPipeline
    {
        WebGPUContext       *m_context;
        WebGPURootSignature *m_rootSignature = nullptr;
        DenOfIz_PipelineDesc m_desc;
        WGPURenderPipeline   m_renderPipeline  = nullptr;
        WGPUComputePipeline  m_computePipeline = nullptr;
        WGPUPipelineLayout   m_pipelineLayout  = nullptr;

    public:
        WebGPUPipeline( WebGPUContext *context, const DenOfIz_PipelineDesc &desc );
        ~WebGPUPipeline( ) override;

        [[nodiscard]] WGPURenderPipeline   GetRenderPipeline( ) const;
        [[nodiscard]] WGPUComputePipeline  GetComputePipeline( ) const;
        [[nodiscard]] DenOfIz_BindPoint    GetBindPoint( ) const;
        [[nodiscard]] WebGPURootSignature *RootSignature( ) const;

    private:
        void             CreateRenderPipeline( );
        void             CreateComputePipeline( );
        bool             HasCompatibleShader( const DenOfIz_CompiledShaderStage *stage );
        WGPUShaderModule CreateModule( const DenOfIz_CompiledShaderStage *stage );
    };

} // namespace DenOfIz
