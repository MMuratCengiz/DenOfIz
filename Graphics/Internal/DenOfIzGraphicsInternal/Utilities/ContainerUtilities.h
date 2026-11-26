/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

namespace DenOfIz
{

    class ContainerUtilities
    {
    public:
        ContainerUtilities( ) = delete;

        template <typename T>
        static void EnsureSize( std::vector<T> &vec, size_t index )
        {
            if ( index >= vec.size( ) )
            {
                vec.resize( index + 1 );
            }
        }

        template <typename T>
        static T &SafeAt( std::vector<T> &vec, size_t index )
        {
            EnsureSize( vec, index );
            return vec[ index ];
        }

        template <typename T>
        static bool Contains( const std::vector<T> &vec, const T &value )
        {
            return std::find( vec.begin( ), vec.end( ), value ) != vec.end( );
        }

        template <typename T>
        static void SafeSet( std::vector<T> &vec, size_t index, const T &value )
        {
            EnsureSize( vec, index );
            vec[ index ] = value;
        }

        template <typename T>
        static void Compute( std::vector<T> &vec, size_t index, const T &newValue, std::function<void( T &existingValue )> modify )
        {
            if ( index >= vec.size( ) )
            {
                vec.resize( index + 1 );
                vec[ index ] = newValue;
            }
            else
            {
                modify( vec[ index ] );
            }
        }

        template <typename K, typename R>
        static R SafeGetMapValue( const std::unordered_map<K, R> &map, const K &key, std::string notFoundMessage = "" )
        {
            auto value = map.find( key );
            if ( value == map.end( ) )
            {
                spdlog::error( "Unable to find key [ {} ]. {}", key, notFoundMessage );
            }
            return value->second;
        }
    };

} // namespace DenOfIz
