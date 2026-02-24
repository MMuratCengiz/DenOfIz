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

#include <DirectXMath.h>
#include <unordered_map>
#include <vector>
#include "DenOfIzExamples/Interop.h"
#include "DenOfIzExamples/PerDrawBinding.h"
#include "DenOfIzExamples/PerFrameBinding.h"
#include "DenOfIzExamples/WorldData.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Pipeline.h"
#include "DenOfIzGraphics/Backends/Interface/RootSignature.h"

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
        float    refractionIndex;
        float    fresnelPower;
        float    padding[ 2 ];
    };

    struct AlphaData
    {
        float alphaValue;
        float padding[ 3 ];
    };

    class ColoredSpherePipeline
    {
        DenOfIz_ShaderProgram                  m_program       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                       m_pipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                  m_rootSignature = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                    m_inputLayout   = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout>   m_bindGroupLayouts;
        std::unordered_map<uint32_t, uint32_t> m_spaceToLayoutIndex;

        DenOfIz_BindGroup              m_viewProjBindGroup = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroup> m_modelBindGroups;
        std::vector<DenOfIz_BindGroup> m_materialBindGroups;

        DenOfIz_Buffer      m_viewProjBuffer     = DENOFIZ_NULL_HANDLE;
        ViewProjectionData *m_viewProjMappedData = nullptr;

        DenOfIz_Buffer m_modelBuffer     = DENOFIZ_NULL_HANDLE;
        Byte          *m_modelMappedData = nullptr;

        DenOfIz_Buffer m_materialBuffer     = DENOFIZ_NULL_HANDLE;
        Byte          *m_materialMappedData = nullptr;

        DenOfIz_Buffer m_alphaBuffer     = DENOFIZ_NULL_HANDLE;
        Byte          *m_alphaMappedData = nullptr;

        DenOfIz_LogicalDevice m_device;
        bool                  m_isTransparent;
        uint32_t              m_numSpheres = 1;

    public:
        ColoredSpherePipeline( DenOfIz_GraphicsApi graphicsApi, DenOfIz_LogicalDevice device, bool isTransparent = false, uint32_t numSpheres = 1 );
        ~ColoredSpherePipeline( );

        void UpdateViewProjection( const FreeCamera *camera ) const;
        void UpdateModel( uint32_t sphereIndex, const XMFLOAT4X4 &modelMatrix ) const;
        void UpdateMaterialColor( uint32_t sphereIndex, const XMFLOAT4 &color ) const;
        void UpdateAlphaValue( uint32_t sphereIndex, float alphaValue ) const;
        void Render( uint32_t sphereIndex, DenOfIz_CommandList commandList, const AssetData *assetData ) const;

        DenOfIz_RootSignature GetRootSignature( ) const
        {
            return m_rootSignature;
        }
    };
} // namespace DenOfIz
