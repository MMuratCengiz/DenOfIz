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

#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <memory>

#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h"

namespace DenOfIz
{
    struct DZ_API VGPipelineDesc
    {
        ILogicalDevice    *LogicalDevice{ };
        InteropArray<Byte> VertexShaderOverride; // Default is used if 0 elements are provided
        InteropArray<Byte> PixelShaderOverride;  // Default is used if 0 elements are provided
        uint32_t           NumFrames = 3;
        bool               SetupData = true; // this sets up the projection data, only set to true if using default version of VGPipelineDesc
    };

    /// Defines the style for `VectorGraphics`
    /// There are various preset pipelines with various styling options, this is here for flexibility
    /// By default a project
    class VGPipeline
    {
        std::unique_ptr<IPipeline>      m_pipeline{ };
        std::unique_ptr<IRootSignature> m_rootSignature{ };
        std::unique_ptr<IInputLayout>   m_inputLayout{ };
        std::unique_ptr<ShaderProgram>  m_program{ };

        // for clarity purposes
        struct BindGroupsPerFrame
        {
            std::vector<std::unique_ptr<IResourceBindGroup>> BindGroups{ };
        };
        std::vector<BindGroupsPerFrame>  m_bindGroupsPerFrame;
        PipelineDesc                     m_pipelineDesc{ };
        std::unique_ptr<IBufferResource> m_data{ };
        Byte                            *m_dataMappedMemory = nullptr;
        uint32_t                         m_alignedElementNumBytes;

    public:
        DZ_API explicit VGPipeline( const VGPipelineDesc &desc );
        DZ_API ~VGPipeline( ) = default;
        // Do not use if customizing VGPipeline, binding slot for the projection matrix is now different
        DZ_API void                UpdateProjection( const uint32_t &frameIndex, const Float_4x4 &projection ) const;
        DZ_API IResourceBindGroup *GetBindGroup( const uint32_t &frameIndex, const uint32_t &registerSpace ) const;
        DZ_API IPipeline          *GetPipeline( ) const;
        DZ_API IInputLayout       *GetInputLayout( ) const;

    private:
        static InteropArray<Byte> GetVertexShader( );
        static InteropArray<Byte> GetPixelShader( );
    };
} // namespace DenOfIz
