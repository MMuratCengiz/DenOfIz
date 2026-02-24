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

#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"

#include <cstring>
#include <deque>
#include <string>
#include <vector>

#define SHADER_ASSET_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ShaderAsset, handle )

namespace DenOfIz
{
    class ShaderAsset
    {
        std::string                           m_path;
        std::vector<DenOfIz_ShaderStageAsset> m_stages;
        std::vector<std::string>              m_entryPoints;
        std::deque<std::string>               m_strings;
        DenOfIz_ShaderReflectDesc             m_reflectDesc;
        DenOfIz_ShaderRayTracingDesc          m_rayTracing;

        uint64_t m_magic    = 0x44414853445A;
        uint32_t m_version  = 1;
        uint64_t m_numBytes = 0;

    public:
        static constexpr uint32_t Latest = 1;

        ShaderAsset( )
        {
            m_reflectDesc = { };
            m_rayTracing  = { };
        }
        ~ShaderAsset( ) = default;

        DenOfIz_AssetHeader Header( ) const
        {
            DenOfIz_AssetHeader header{ };
            header.Magic    = m_magic;
            header.Version  = m_version;
            header.NumBytes = m_numBytes;
            header.Path     = { m_path.c_str( ), m_path.size( ) };
            return header;
        }

        uint64_t Magic( ) const
        {
            return m_magic;
        }
        uint32_t Version( ) const
        {
            return m_version;
        }
        uint64_t NumBytes( ) const
        {
            return m_numBytes;
        }
        DenOfIz_StringView Path( ) const
        {
            return { m_path.c_str( ), m_path.size( ) };
        }

        size_t NumStages( ) const
        {
            return m_stages.size( );
        }

        DenOfIz_ShaderStageAsset *GetStage( size_t index )
        {
            if ( index >= m_stages.size( ) )
            {
                return nullptr;
            }
            return &m_stages[ index ];
        }

        DenOfIz_ShaderStageAsset *AddStage( )
        {
            m_stages.push_back( DenOfIz_ShaderStageAsset{ } );
            m_entryPoints.emplace_back( );
            return &m_stages.back( );
        }

        void SetStageEntryPoint( size_t index, DenOfIz_StringView entryPoint )
        {
            if ( index >= m_entryPoints.size( ) )
            {
                return;
            }
            m_entryPoints[ index ]                = std::string( entryPoint.Chars, entryPoint.NumChars );
            m_stages[ index ].EntryPoint.Chars    = m_entryPoints[ index ].c_str( );
            m_stages[ index ].EntryPoint.NumChars = m_entryPoints[ index ].size( );
        }

        void ReserveStages( size_t capacity )
        {
            m_stages.reserve( capacity );
            m_entryPoints.reserve( capacity );
        }

        void ClearStages( )
        {
            m_stages.clear( );
            m_entryPoints.clear( );
        }

        DenOfIz_ShaderReflectDesc *GetReflectDesc( )
        {
            return &m_reflectDesc;
        }
        void SetReflectDesc( const DenOfIz_ShaderReflectDesc &desc )
        {
            m_reflectDesc = desc;
        }

        DenOfIz_ShaderRayTracingDesc *GetRayTracing( )
        {
            return &m_rayTracing;
        }

        void SetVersion( uint32_t version )
        {
            m_version = version;
        }
        void SetNumBytes( uint64_t numBytes )
        {
            m_numBytes = numBytes;
        }
        void SetPath( DenOfIz_StringView path )
        {
            m_path = std::string( path.Chars, path.NumChars );
        }

        DenOfIz_StringView StoreString( DenOfIz_StringView str )
        {
            m_strings.emplace_back( str.Chars, str.NumChars );
            const std::string &stored = m_strings.back( );
            return { stored.c_str( ), stored.size( ) };
        }
    };
} // namespace DenOfIz

extern "C"
{
    DenOfIz_ShaderAsset DenOfIz_ShaderAsset_Create( )
    {
        auto *shaderAsset = new DenOfIz::ShaderAsset( );
        return DENOFIZ_TO_HANDLE( shaderAsset );
    }

    void DenOfIz_ShaderAsset_Destroy( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return;
        }
        delete SHADER_ASSET_IMPL( shaderAsset );
    }

    DenOfIz_AssetHeader DenOfIz_ShaderAsset_Header( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return { };
        }
        return SHADER_ASSET_IMPL( shaderAsset )->Header( );
    }

    uint64_t DenOfIz_ShaderAsset_Magic( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return 0;
        }
        return SHADER_ASSET_IMPL( shaderAsset )->Magic( );
    }

    uint32_t DenOfIz_ShaderAsset_Version( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return 0;
        }
        return SHADER_ASSET_IMPL( shaderAsset )->Version( );
    }

    uint64_t DenOfIz_ShaderAsset_NumBytes( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return 0;
        }
        return SHADER_ASSET_IMPL( shaderAsset )->NumBytes( );
    }

    DenOfIz_StringView DenOfIz_ShaderAsset_Path( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return { };
        }
        return SHADER_ASSET_IMPL( shaderAsset )->Path( );
    }

    size_t DenOfIz_ShaderAsset_NumStages( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return 0;
        }
        return SHADER_ASSET_IMPL( shaderAsset )->NumStages( );
    }

    DenOfIz_ShaderStageAsset *DenOfIz_ShaderAsset_GetStage( DenOfIz_ShaderAsset shaderAsset, size_t index )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return nullptr;
        }
        return SHADER_ASSET_IMPL( shaderAsset )->GetStage( index );
    }

    DenOfIz_ShaderStageAsset *DenOfIz_ShaderAsset_AddStage( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return nullptr;
        }
        return SHADER_ASSET_IMPL( shaderAsset )->AddStage( );
    }

    void DenOfIz_ShaderAsset_SetStageEntryPoint( DenOfIz_ShaderAsset shaderAsset, size_t index, DenOfIz_StringView entryPoint )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return;
        }
        SHADER_ASSET_IMPL( shaderAsset )->SetStageEntryPoint( index, entryPoint );
    }

    void DenOfIz_ShaderAsset_ReserveStages( DenOfIz_ShaderAsset shaderAsset, size_t capacity )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return;
        }
        SHADER_ASSET_IMPL( shaderAsset )->ReserveStages( capacity );
    }

    void DenOfIz_ShaderAsset_ClearStages( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return;
        }
        SHADER_ASSET_IMPL( shaderAsset )->ClearStages( );
    }

    DenOfIz_ShaderReflectDesc *DenOfIz_ShaderAsset_GetReflectDesc( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return nullptr;
        }
        return SHADER_ASSET_IMPL( shaderAsset )->GetReflectDesc( );
    }

    void DenOfIz_ShaderAsset_SetReflectDesc( DenOfIz_ShaderAsset shaderAsset, const DenOfIz_ShaderReflectDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) || desc == NULL )
        {
            return;
        }
        SHADER_ASSET_IMPL( shaderAsset )->SetReflectDesc( *desc );
    }

    DenOfIz_ShaderRayTracingDesc *DenOfIz_ShaderAsset_GetRayTracing( DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return nullptr;
        }
        return SHADER_ASSET_IMPL( shaderAsset )->GetRayTracing( );
    }

    void DenOfIz_ShaderAsset_SetVersion( DenOfIz_ShaderAsset shaderAsset, uint32_t version )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return;
        }
        SHADER_ASSET_IMPL( shaderAsset )->SetVersion( version );
    }

    void DenOfIz_ShaderAsset_SetNumBytes( DenOfIz_ShaderAsset shaderAsset, uint64_t numBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return;
        }
        SHADER_ASSET_IMPL( shaderAsset )->SetNumBytes( numBytes );
    }

    void DenOfIz_ShaderAsset_SetPath( DenOfIz_ShaderAsset shaderAsset, DenOfIz_StringView path )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return;
        }
        SHADER_ASSET_IMPL( shaderAsset )->SetPath( path );
    }

    DenOfIz_StringView DenOfIz_ShaderAsset_StoreString( DenOfIz_ShaderAsset shaderAsset, DenOfIz_StringView str )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return { };
        }
        return SHADER_ASSET_IMPL( shaderAsset )->StoreString( str );
    }

    DenOfIz_StringView DenOfIz_ShaderAsset_Extension( )
    {
        return DENOFIZ_STRING( "dzshd" );
    }
}
