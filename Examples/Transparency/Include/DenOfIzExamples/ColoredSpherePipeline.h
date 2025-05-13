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

#include <DenOfIzExamples/Interop.h>
#include <DenOfIzExamples/PerDrawBinding.h>
#include <DenOfIzExamples/PerFrameBinding.h>
#include <DenOfIzExamples/WorldData.h>
#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include <DenOfIzGraphics/Backends/GraphicsApi.h>
#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DirectXMath.h>

namespace DenOfIz
{
    struct ViewProjectionData
    {
        XMFLOAT4X4 viewProjection;
    };

    struct ModelMatrixData
    {
        XMFLOAT4X4 model;
    };

    struct SphereMaterialData
    {
        XMFLOAT4 color;
        float             refractionIndex;
        float             fresnelPower;
        float             padding[ 2 ];
    };

    struct AlphaData
    {
        float alphaValue;
        float padding[ 3 ];
    };

    class ColoredSpherePipeline
    {
        std::unique_ptr<ShaderProgram>  m_program;
        std::unique_ptr<IPipeline>      m_pipeline;
        std::unique_ptr<IRootSignature> m_rootSignature;
        std::unique_ptr<IInputLayout>   m_inputLayout;

        std::unique_ptr<IResourceBindGroup>              m_viewProjBindGroup;  // Register space 0
        std::vector<std::unique_ptr<IResourceBindGroup>> m_modelBindGroups;    // Register space 30
        std::vector<std::unique_ptr<IResourceBindGroup>> m_materialBindGroups; // Register space 1

        std::unique_ptr<IBufferResource> m_viewProjBuffer;
        ViewProjectionData              *m_viewProjMappedData = nullptr;

        std::unique_ptr<IBufferResource> m_modelBuffer;
        Byte                            *m_modelMappedData = nullptr;

        std::unique_ptr<IBufferResource> m_materialBuffer;
        Byte                            *m_materialMappedData = nullptr;

        std::unique_ptr<IBufferResource> m_alphaBuffer;
        Byte                            *m_alphaMappedData = nullptr;

        ILogicalDevice *m_device;
        bool            m_isTransparent;
        uint32_t        m_numSpheres = 1;

    public:
        ColoredSpherePipeline( const GraphicsApi *graphicsApi, ILogicalDevice *device, bool isTransparent = false, uint32_t numSpheres = 1 );
        ~ColoredSpherePipeline( );

        void UpdateViewProjection( const Camera *camera ) const;
        void UpdateModel( uint32_t sphereIndex, const XMFLOAT4X4 &modelMatrix ) const;
        void UpdateMaterialColor( uint32_t sphereIndex, const XMFLOAT4 &color ) const;
        void UpdateAlphaValue( uint32_t sphereIndex, float alphaValue ) const;
        void Render( uint32_t sphereIndex, ICommandList *commandList, const AssetData *assetData ) const;

        IRootSignature *GetRootSignature( ) const
        {
            return m_rootSignature.get( );
        }
    };
} // namespace DenOfIz
