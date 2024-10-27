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

#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderBindingTable.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipeline.h>

namespace DenOfIz
{
    class VulkanShaderBindingTable final : public IShaderBindingTable
    {
        VulkanContext         *m_context;
        VulkanPipeline        *m_pipeline;
        ShaderBindingTableDesc m_desc;

    public:
                                       VulkanShaderBindingTable( VulkanContext *context, const ShaderBindingTableDesc &desc );
        void                           Resize( const SBTSizeDesc                           &) override;
        void                           BindRayGenerationShader( const RayGenerationBindingDesc &desc ) override;
        void                           BindHitGroup( const HitGroupBindingDesc &desc ) override;
        void                           BindMissShader( const MissBindingDesc &desc ) override;
        void                           Build( ) override;
        [[nodiscard]] IBufferResource *Buffer( ) const override;

    private:
        [[nodiscard]] uint32_t AlignRecord( uint32_t size ) const;
    };
} // namespace DenOfIz
