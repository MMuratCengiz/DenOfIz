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

#include <DenOfIzExamples/IExample.h>

#include "../../../../Graphics/Include/DenOfIzGraphics/Assets/Vector2d/QuadRenderer.h"
#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetReader.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"
#include "DenOfIzGraphics/Utilities/Time.h"

#include <DirectXMath.h>
#include <memory>

namespace DenOfIz
{
    class VGExample final : public IExample
    {
        static constexpr auto               m_folderSVG    = "Assets/SVG/folder.svg";
        static constexpr auto               m_folderAsset  = "Assets/Textures/folder.dztex";
        static constexpr auto               m_milkTeaSVG   = "Assets/SVG/milk-tea.svg";
        static constexpr auto               m_milkTeaAsset = "Assets/Textures/milk-tea.dztex";
        std::unique_ptr<FrameDebugRenderer> m_debugRenderer;
        Time                                m_time;

        std::unique_ptr<QuadRenderer> m_quadRenderer;
        std::unique_ptr<TextRenderer> m_textRenderer;

        std::unique_ptr<ITextureResource> m_starTexture;
        std::unique_ptr<ITextureResource> m_folderTexture;
        std::unique_ptr<ITextureResource> m_milkTeaTexture;

        std::vector<std::unique_ptr<BinaryReader>>       m_textureReaders;
        std::vector<std::unique_ptr<TextureAssetReader>> m_assetReaders;

        uint32_t m_redMaterialId     = 0;
        uint32_t m_folderMaterialId  = 0;
        uint32_t m_milkTeaMaterialId = 0;
        uint32_t m_starMaterialId    = 0;

        uint32_t m_folderQuadIndex  = 0;
        uint32_t m_milkTeaQuadIndex = 0;
        uint32_t m_starQuadIndex    = 0;
        bool     m_textLabelsAdded  = false;

        float      m_animationTime;
        XMFLOAT4X4 m_projectionMatrix;

        float m_rotationAngle;
        float m_scaleAnimTime;
        float m_colorAnimTime;

    public:
        VGExample( )           = default;
        ~VGExample( ) override = default;

        void              Init( ) override;
        void              ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void              HandleEvent( Event &event ) override;
        void              Update( ) override;
        void              Render( uint32_t frameIndex, ICommandList *commandList ) override;
        ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = ExampleWindowDesc( );
            windowDesc.Title  = "Vector Graphics Example - 2D Drawing Framework";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void UpdateProjectionMatrix( );
        void InitializeRenderers( );
        void LoadSvgTextures( );
        void InitializeMaterials( );
        void CreateStarTexture( );

        void AddQuads( ) const;
        void ImportSvgIfNeeded( const InteropString &svgPath, const InteropString &targetPath ) const;
    };
} // namespace DenOfIz
