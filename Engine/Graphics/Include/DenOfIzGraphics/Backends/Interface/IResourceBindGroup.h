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

#include <algorithm>
#include "IBufferResource.h"
#include "IRootSignature.h"
#include "ITextureResource.h"

namespace DenOfIz
{
    struct ResourceBindGroupDesc
    {
        IRootSignature *RootSignature;
        uint32_t        RegisterSpace;
        uint32_t        NumBuffers;
        uint32_t        NumTextures;
        uint32_t        NumSamplers;
    };

    template <typename T>
    struct UpdateDescItem
    {
        ResourceBindingSlot Slot;
        T                  *Resource;
    };

    struct UpdateDesc
    {
        std::vector<UpdateDescItem<IBufferResource>>  Buffers;
        std::vector<UpdateDescItem<ITextureResource>> Textures;
        std::vector<UpdateDescItem<ISampler>>         Samplers;

        UpdateDesc &Buffer( const ResourceBindingSlot &slot, IBufferResource *resource )
        {
            Buffers.push_back( { slot, resource } );
            return *this;
        }

        UpdateDesc &Texture( const ResourceBindingSlot &slot, ITextureResource *resource )
        {
            Textures.push_back( { slot, resource } );
            return *this;
        }

        UpdateDesc &Sampler( const ResourceBindingSlot &slot, ISampler *sampler )
        {
            Samplers.push_back( { slot, sampler } );
            return *this;
        }
    };

    class IResourceBindGroup
    {
    protected:
        ResourceBindGroupDesc m_desc;
        IRootSignature       *m_rootSignature;

    public:
        explicit IResourceBindGroup( const ResourceBindGroupDesc &desc ) : m_desc( desc ), m_rootSignature( desc.RootSignature )
        {
        }

        [[nodiscard]] uint32_t RegisterSpace( ) const
        {
            return m_desc.RegisterSpace;
        }

        virtual ~    IResourceBindGroup( ) = default;
        virtual void Update( const UpdateDesc &desc )
        {
            DZ_ASSERTM( desc.Buffers.size( ) == m_desc.NumBuffers, "Number of buffers being updated do not match." );
            DZ_ASSERTM( desc.Textures.size( ) == m_desc.NumTextures, "Number of textures being updated do not match." );
            DZ_ASSERTM( desc.Samplers.size( ) == m_desc.NumSamplers, "Number of sampler being updated do not match." );

            for ( auto item : desc.Buffers )
            {
                BindBuffer( item.Slot, item.Resource );
            }
            for ( auto item : desc.Textures )
            {
                BindTexture( item.Slot, item.Resource );
            }
            for ( auto item : desc.Samplers )
            {
                BindSampler( item.Slot, item.Resource );
            }
        }

    protected:
        virtual void BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource ) = 0;
        virtual void BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )   = 0;
        virtual void BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )          = 0;
    };

} // namespace DenOfIz
