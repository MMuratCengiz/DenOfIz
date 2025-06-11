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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    struct DZ_API PositionKey
    {
        float   Timestamp; // Time in seconds
        Float_3 Value;
    };

    struct DZ_API PositionKeyArray
    {
        PositionKey *Elements;
        uint32_t     NumElements;

        DZ_ARRAY_METHODS( PositionKeyArray, PositionKey )
    };

    struct DZ_API RotationKey
    {
        float   Timestamp; // Time in seconds
        Float_4 Value;     // Quaternion
    };

    struct DZ_API RotationKeyArray
    {
        RotationKey *Elements;
        uint32_t     NumElements;

        DZ_ARRAY_METHODS( RotationKeyArray, RotationKey )
    };

    struct DZ_API ScaleKey
    {
        float   Timestamp;
        Float_3 Value;
    };

    struct DZ_API ScaleKeyArray
    {
        ScaleKey *Elements;
        uint32_t  NumElements;

        DZ_ARRAY_METHODS( ScaleKeyArray, ScaleKey )
    };

    struct DZ_API MorphKeyframe
    {
        float Timestamp;
        float Weight;
    };

    struct DZ_API MorphKeyframeArray
    {
        MorphKeyframe *Elements;
        uint32_t       NumElements;

        DZ_ARRAY_METHODS( MorphKeyframeArray, MorphKeyframe )
    };

    struct DZ_API MorphAnimTrack
    {
        InteropString      Name;
        MorphKeyframeArray Keyframes;
    };

    struct DZ_API MorphAnimTrackArray
    {
        MorphAnimTrack *Elements;
        uint32_t        NumElements;

        DZ_ARRAY_METHODS( MorphAnimTrackArray, MorphAnimTrack )
    };

    struct DZ_API JointAnimTrack
    {
        InteropString    JointName;
        PositionKeyArray PositionKeys;
        RotationKeyArray RotationKeys;
        ScaleKeyArray    ScaleKeys;

        void Dispose( ) const
        {
            PositionKeys.Dispose( );
            RotationKeys.Dispose( );
            ScaleKeys.Dispose( );
        }
    };

    struct DZ_API JointAnimTrackArray
    {
        JointAnimTrack *Elements;
        uint32_t        NumElements;

        static JointAnimTrackArray Create( const size_t numElements )
        {
            JointAnimTrackArray Array{ };
            Array.Elements    = new JointAnimTrack[ numElements ];
            Array.NumElements = numElements;
            return Array;
        }

        void Dispose( ) const
        {
            for ( uint32_t i = 0; i < NumElements; ++i )
            {
                Elements[ i ].Dispose( );
            }
            delete[] Elements;
        }
    };

    struct DZ_API AnimationClip
    {
        InteropString       Name;
        float               Duration{ };
        JointAnimTrackArray Tracks;
        MorphAnimTrackArray MorphTracks;

        void Dispose( ) const
        {
            Tracks.Dispose( );
            MorphTracks.Dispose( );
        }
    };

    struct DZ_API AnimationClipArray
    {
        AnimationClip *Elements;
        uint32_t       NumElements;

        static AnimationClipArray Create( const size_t numElements )
        {
            AnimationClipArray Array{ };
            Array.Elements    = new AnimationClip[ numElements ];
            Array.NumElements = numElements;
            return Array;
        }

        void Dispose( ) const
        {
            for ( uint32_t i = 0; i < NumElements; ++i )
            {
                Elements[ i ].Dispose( );
            }
            delete[] Elements;
        }
    };

    struct DZ_API AnimationAsset : AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        InteropString      Name;
        AssetUri           SkeletonRef;
        AnimationClipArray Animations;

        AnimationAsset( ) : AssetHeader( 0x445A414E494D /*DZANIM*/, Latest, 0 )
        {
        }

        static InteropString Extension( )
        {
            return "dzanim";
        }
    };
} // namespace DenOfIz
