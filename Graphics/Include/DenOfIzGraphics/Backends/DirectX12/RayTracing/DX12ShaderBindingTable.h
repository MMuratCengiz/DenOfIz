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

#include <DenOfIzGraphics/Backends/DirectX12/DX12BufferResource.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12Context.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12Pipeline.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderBindingTable.h>

namespace DenOfIz
{
    class DX12ShaderBindingTable : public IShaderBindingTable
    {
    private:
        DX12Context    *m_context;
        ShaderTableDesc m_desc;
        DX12Pipeline   *m_pipeline;

        std::unique_ptr<DX12BufferResource> m_buffer;
        std::unique_ptr<DX12BufferResource> m_stagingBuffer;
        void                               *m_mappedMemory = nullptr;

    public:
        DX12ShaderBindingTable( DX12Context *context, const ShaderTableDesc &desc );
        void             Resize( const SBTSizeDesc &desc ) override;
        void             BindRayGenerationShader( const RayGenerationBindingDesc &desc ) override;
        void             BindHitGroup( const HitGroupBindingDesc &desc ) override;
        void             BindMissShader( const MissBindingDesc &desc ) override;
        void             Build( ) override;
        IBufferResource *Buffer( ) const override;
    };
} // namespace DenOfIz
