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

#include <DenOfIzExamples/Camera.h>
#include <DenOfIzExamples/DefaultRenderPipeline.h>
#include <DenOfIzExamples/IExample.h>
#include <DenOfIzExamples/PerFrameBinding.h>
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Utilities/StepTimer.h>
#include <DenOfIzGraphics/Utilities/Time.h>

namespace DenOfIz
{
    // Constants for the mesh shader pipeline
    struct GrassConstants
    {
        // View and projection matrices
        DirectX::XMMATRIX ViewProjection;
        DirectX::XMMATRIX Model;

        // Grass parameters
        DirectX::XMFLOAT4 WindDirection; // xyz = direction, w = strength
        DirectX::XMFLOAT4 GrassColor;
        DirectX::XMFLOAT4 GrassColorVariation;

        // Time and animation
        float Time;
        float DensityFactor;
        float HeightScale;
        float WidthScale;

        // Parameters for the LOD system
        float MaxDistance;
        float Padding[ 3 ];
    };

    class MeshShaderGrassExample : public IExample
    {
        std::unique_ptr<ShaderProgram>      m_meshShaderProgram;
        std::unique_ptr<IPipeline>          m_meshPipeline;
        std::unique_ptr<IRootSignature>     m_meshRootSignature;
        std::unique_ptr<IResourceBindGroup> m_meshBindGroup;
        std::unique_ptr<ITextureResource>   m_grassTexture;
        std::unique_ptr<IBufferResource>    m_grassConstantsBuffer;
        std::unique_ptr<ISampler>           m_grassSampler;

        GrassConstants *m_grassConstants = nullptr;

        StepTimer m_stepTimer;
        Time      m_time;

        std::unique_ptr<IBufferResource> m_planeVertexBuffer;
        std::unique_ptr<IBufferResource> m_planeIndexBuffer;

        float m_elapsedTime = 0.0f;
        bool  m_animateWind = true;

    public:
        void Init( ) override;
        void ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void HandleEvent( SDL_Event &event ) override;
        void Quit( ) override;

    private:
        void CreateMeshShaderPipeline( );
        void CreateConstantsBuffer( );
        void LoadGrassTexture( );
        void UpdateConstants( );
    };
} // namespace DenOfIz
