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

#include <memory>
#include <vector>
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Backends/Interface/ICommandList.h"
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/IPipeline.h"
#include "DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/IRootSignature.h"
#include "DenOfIzGraphics/Backends/Interface/ITextureResource.h"
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    struct DZ_API QuadRendererDesc
    {
        ILogicalDevice *LogicalDevice      = nullptr;
        Format          RenderTargetFormat = Format::B8G8R8A8Unorm;
        uint32_t        NumFrames          = 3;
        uint32_t        MaxNumTextures     = 64;
        uint32_t        MaxNumQuads        = 10240;
    };

    struct DZ_API QuadMaterialDesc
    {
        uint32_t MaterialId   = 0;
        uint32_t TextureIndex = 0; // Index into the bindless texture array
        Float_4  Color        = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    struct DZ_API QuadDataDesc
    {
        uint32_t QuadId         = 0;
        Float_2  Position       = { 0.0f, 0.0f };
        Float_2  Size           = { 100.0f, 100.0f };
        uint32_t TextureIndex   = 0; // Index into the bindless texture array
        Float_4  Color          = { 1.0f, 1.0f, 1.0f, 1.0f };
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
            Float_3 Position;
            Float_2 TexCoord;
        };

        struct QuadInstance
        {
            Float_4x4 Transform;
            Float_4   UVScaleOffset; // xy: scale, zw: offset
            uint32_t  TextureIndex;
            Float_4   Color;
            Float_3   _Pad0;
        };

        struct FrameConstants
        {
            Float_4x4 Projection;
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
            std::unique_ptr<IResourceBindGroup> InstanceBindGroup;
            std::unique_ptr<IResourceBindGroup> TextureBindGroup;
        };
        std::vector<FrameData>            m_frameData;
        std::vector<ITextureResource *>   m_textures;
        std::vector<uint32_t>             m_freeTextureIndices;
        uint32_t                          m_currentQuadCount = 0;
        std::unique_ptr<IBufferResource>  m_instanceBuffer   = nullptr;
        QuadInstance                     *m_instances        = nullptr;
        std::unique_ptr<IBufferResource>  m_constantsBuffer;
        std::unique_ptr<ITextureResource> m_nullTexture;

        Float_4x4 m_projectionMatrix;

    public:
        DZ_API explicit QuadRenderer( const QuadRendererDesc &desc );
        DZ_API ~QuadRenderer( );

        DZ_API void SetCanvas( uint32_t width, uint32_t height );

        DZ_API uint32_t RegisterTexture( ITextureResource *texture );
        DZ_API void     UnregisterTexture( uint32_t textureIndex );

        DZ_API void AddQuad( const QuadDataDesc &desc );
        DZ_API void UpdateQuad( uint32_t frameIndex, const QuadDataDesc &desc ) const;

        DZ_API void ClearQuads( );

        DZ_API void Render( uint32_t frameIndex, ICommandList *commandList ) const;

    private:
        void      Initialize( );
        void      CreateShaderResources( );
        void      CreateStaticQuadGeometry( );
        void      UpdateTextureBindings( uint32_t frameIndex ) const;
        Float_4x4 CalculateTransform( const QuadDataDesc &desc ) const;
    };

} // namespace DenOfIz
