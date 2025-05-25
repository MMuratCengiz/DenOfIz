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

#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include <DenOfIzGraphics/Renderer/Sync/ResourceTracking.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>
#include <memory>
#include <vector>

#include <DirectXMath.h>

namespace DenOfIz
{
    struct DZ_API QuadRendererDesc
    {
        ILogicalDevice *LogicalDevice      = nullptr;
        Format          RenderTargetFormat = Format::B8G8R8A8Unorm;
        uint32_t        NumFrames          = 3;
        uint32_t        BatchSize          = 1024;
        uint32_t        MaxNumMaterials    = 32;
        uint32_t        MaxNumQuads        = 1024;
    };

    struct DZ_API QuadMaterialDesc
    {
        uint32_t          MaterialId = 0;
        ITextureResource *Texture    = nullptr;
        Float_4           Color      = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    struct DZ_API QuadDataDesc
    {
        uint32_t QuadId         = 0;
        Float_2  Position       = { 0.0f, 0.0f };
        Float_2  Size           = { 100.0f, 100.0f };
        uint32_t MaterialId     = 0;
        float    Rotation       = 0.0f;
        Float_2  RotationCenter = { 0.0f, 0.0f };
        Float_2  Scale          = { 1.0f, 1.0f };
        Float_2  UV0            = { 0.0f, 0.0f };
        Float_2  UV1            = { 1.0f, 1.0f };
    };

    class QuadRenderer
    {
        struct QuadVertex
        {
            DirectX::XMFLOAT3 Position;
            DirectX::XMFLOAT2 TexCoord;
        };

        struct QuadInstance
        {
            DirectX::XMFLOAT4X4 Transform;
            DirectX::XMFLOAT4   UVScaleOffset; // xy: scale, zw: offset
            uint32_t            MaterialId;
            Float_3             _Pad0;
            Float_4             _Pad1; // Fully align with 256, DX12 Constant buffer offset
        };

        struct FrameConstants
        {
            DirectX::XMFLOAT4X4 Projection;
        };

        struct RootConstants
        {
            uint32_t StartInstance;
            uint32_t HasTexture;
            float    Padding[ 2 ];
        };

        struct MaterialShaderData
        {
            Float_4 Color;
        };

        struct MaterialData
        {
            ITextureResource *Texture = nullptr;
        };

        struct DrawBatch
        {
            uint32_t StartInstance;
            uint32_t InstanceCount = 1;
            uint32_t MaterialId;
        };

        QuadRendererDesc m_desc;
        ILogicalDevice  *m_logicalDevice = nullptr;

        std::unique_ptr<ShaderProgram>  m_shaderProgram;
        std::unique_ptr<IRootSignature> m_rootSignature;
        std::unique_ptr<IInputLayout>   m_inputLayout;
        std::unique_ptr<IPipeline>      m_rasterPipeline;
        std::unique_ptr<ISampler>       m_sampler;

        std::unique_ptr<IBufferResource> m_vertexBuffer;
        std::unique_ptr<IBufferResource> m_indexBuffer;

        struct FrameData
        {
            std::unique_ptr<IResourceBindGroup>                               InstanceBindGroup;
            std::unordered_map<uint32_t, std::unique_ptr<IResourceBindGroup>> MaterialBindGroups;
            std::unique_ptr<IResourceBindGroup>                               RootConstantsBindGroup;
        };
        std::vector<FrameData>        m_frameData;
        std::vector<QuadMaterialDesc> m_materialDescs;

        uint32_t                                m_materialBatchIndex = 0;
        std::unordered_map<uint32_t, DrawBatch> m_drawBatches; // key = MaterialId
        std::unique_ptr<IBufferResource>        m_instanceBuffer = nullptr;
        QuadInstance                           *m_instances      = nullptr;
        std::unique_ptr<IBufferResource>        m_materialBuffer = nullptr;
        Byte                                   *m_materialData   = nullptr;
        std::unique_ptr<IBufferResource>        m_constantsBuffer;
        std::unique_ptr<ITextureResource>       m_nullTexture;

        DirectX::XMFLOAT4X4 m_projectionMatrix;

    public:
        DZ_API explicit QuadRenderer( const QuadRendererDesc &desc );
        DZ_API ~QuadRenderer( );

        DZ_API void Initialize( );
        DZ_API void SetCanvas( uint32_t width, uint32_t height );

        DZ_API void AddMaterial( const QuadMaterialDesc &desc );
        DZ_API void UpdateMaterial( uint32_t frameIndex, const QuadMaterialDesc &desc ) const;

        DZ_API void AddQuad( const QuadDataDesc &desc );
        DZ_API void UpdateQuad( uint32_t frameIndex, const QuadDataDesc &desc ) const;

        DZ_API void Render( uint32_t frameIndex, ICommandList *commandList );

    private:
        void                CreateShaderResources( );
        void                CreateStaticQuadGeometry( );
        DirectX::XMFLOAT4X4 CalculateTransform( const QuadDataDesc &desc ) const;
    };

} // namespace DenOfIz
