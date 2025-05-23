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

#include <DenOfIzGraphics/Utilities/InteropMath.h>
#include <DirectXMath.h>
#include <vector>

namespace DenOfIz
{
    class VGTransform
    {
        DirectX::XMFLOAT4X4 m_projection{ };
        DirectX::XMMATRIX m_transform{ };
        std::vector<DirectX::XMMATRIX> m_transformStack;

    public:
        DZ_API VGTransform( uint32_t width, uint32_t height );
        DZ_API void      SetSize( uint32_t width, uint32_t height );
        
        // Transform operations
        DZ_API void      SetTransform( const Float_4x4 &transform );
        DZ_API void      ResetTransform( );
        DZ_API void      Transform( const Float_4x4 &matrix );
        DZ_API void      Translate( const Float_2 &offset );
        DZ_API void      Scale( const Float_2 &scale );
        DZ_API void      Scale( float scale );
        DZ_API void      Rotate( float angleRadians );
        DZ_API void      Rotate( float angleRadians, const Float_2 &center );
        DZ_API void      Skew( const Float_2 &skew );
        
        // Transform stack
        DZ_API void      PushTransform( );
        DZ_API void      PushTransform( const Float_4x4 &transform );
        DZ_API void      PopTransform( );
        
        // Matrix getters
        DZ_API Float_4x4 GetMatrix( ) const;
        DZ_API Float_4x4 GetProjectionMatrix( ) const;
        DZ_API Float_4x4 GetCombinedMatrix( ) const; // projection * transform
        
    private:
        void ApplyTransform( const DirectX::XMMATRIX &transform );
    };
} // namespace DenOfIz
