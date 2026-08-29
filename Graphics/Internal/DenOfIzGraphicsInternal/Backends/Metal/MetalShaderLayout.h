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
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Assets/Shaders/ShaderReflectDesc.h"
#include "DenOfIzGraphicsInternal/Assets/Shaders/MetalRootSignatureBuilder.h"
#include "MetalContext.h"

namespace DenOfIz
{
    struct MetalSpaceLayout
    {
        uint32_t                               RegisterSpace        = 0;
        uint32_t                               CbvSrvUavTableOffset = UINT32_MAX;
        uint32_t                               CbvSrvUavTableSize   = 0;
        uint32_t                               SamplerTableOffset   = UINT32_MAX;
        uint32_t                               SamplerTableSize     = 0;
        std::unordered_map<uint64_t, uint32_t> TableIndices;
        std::unordered_map<uint64_t, uint32_t> RootDescriptorOffsets;
        uint64_t                               Hash = 0;

        [[nodiscard]] const uint32_t *TableIndex( DenOfIz_ResourceBindingType type, uint32_t binding ) const;
        [[nodiscard]] const uint32_t *RootDescriptorOffset( DenOfIz_ResourceBindingType type, uint32_t binding ) const;
    };

    struct MetalRootConstantLayout
    {
        uint32_t RegisterSpace;
        uint32_t Binding;
        uint32_t Offset;
        uint32_t NumBytes;
    };

    class MetalShaderLayout
    {
        std::vector<std::unique_ptr<MetalSpaceLayout>> m_spaces;
        std::vector<MetalRootConstantLayout>           m_rootConstants;
        uint32_t                                       m_numBytes = 0;
        uint64_t                                       m_hash     = 0;

    public:
        explicit MetalShaderLayout( const std::vector<MetalRootSignatureBinding> &bindings );

        static std::vector<MetalRootSignatureBinding> GlobalBindings( const DenOfIz_ShaderReflectDesc &reflectDesc );
        static std::vector<MetalRootSignatureBinding> LocalBindings( const DenOfIz_LocalRootSignatureDescArray &localRootSignatures );
        static IRDescriptorRangeType                  RangeType( DenOfIz_ResourceBindingType type );
        static uint64_t                               BindingKey( DenOfIz_ResourceBindingType type, uint32_t binding );

        [[nodiscard]] const MetalSpaceLayout                                *Space( uint32_t registerSpace ) const;
        [[nodiscard]] const std::vector<std::unique_ptr<MetalSpaceLayout>>  &Spaces( ) const;
        [[nodiscard]] const MetalRootConstantLayout                         *RootConstant( uint32_t binding ) const;
        [[nodiscard]] const MetalRootConstantLayout                         *RootConstant( uint32_t registerSpace, uint32_t binding ) const;
        [[nodiscard]] const MetalSpaceLayout                                *FindTable( DenOfIz_ResourceBindingType type, uint32_t binding, uint32_t &outIndex ) const;
        [[nodiscard]] uint32_t                                               NumBytes( ) const;
        [[nodiscard]] uint64_t                                               Hash( ) const;
        [[nodiscard]] const std::vector<MetalRootConstantLayout>            &RootConstants( ) const;
    };
} // namespace DenOfIz
