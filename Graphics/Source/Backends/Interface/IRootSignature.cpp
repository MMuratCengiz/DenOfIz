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

#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <algorithm>

using namespace DenOfIz;

InteropArray<ResourceBindingDesc> DenOfIz::SortResourceBindings( const InteropArray<ResourceBindingDesc> &bindings )
{
    std::vector<ResourceBindingDesc> resourceBindings;
    resourceBindings.reserve( bindings.NumElements( ) );

    for ( size_t i = 0; i < bindings.NumElements( ); ++i )
    {
        resourceBindings.push_back( bindings.GetElement( i ) );
    }

    std::sort( resourceBindings.begin( ), resourceBindings.end( ),
               []( const ResourceBindingDesc &a, const ResourceBindingDesc &b )
               {
                   if ( a.RegisterSpace != b.RegisterSpace )
                   {
                       return a.RegisterSpace < b.RegisterSpace;
                   }
                   if ( a.Binding != b.Binding )
                   {
                       return a.Binding < b.Binding;
                   }
                   return static_cast<int>( a.BindingType ) < static_cast<int>( b.BindingType );
               } );

    InteropArray<ResourceBindingDesc> sortedBindings( resourceBindings.size( ) );

    for ( size_t i = 0; i < resourceBindings.size( ); ++i )
    {
        ResourceBindingDesc &copy = sortedBindings.GetElement( i );
        copy.Name                 = resourceBindings[ i ].Name;
        copy.BindingType          = resourceBindings[ i ].BindingType;
        copy.Binding              = resourceBindings[ i ].Binding;
        copy.RegisterSpace        = resourceBindings[ i ].RegisterSpace;
        copy.Descriptor           = resourceBindings[ i ].Descriptor;
        copy.Stages               = resourceBindings[ i ].Stages;
        copy.ArraySize            = resourceBindings[ i ].ArraySize;

        ReflectionDesc &reflectionCopy = copy.Reflection;
        reflectionCopy.Name            = resourceBindings[ i ].Reflection.Name;
        reflectionCopy.Type            = resourceBindings[ i ].Reflection.Type;
        reflectionCopy.NumBytes        = resourceBindings[ i ].Reflection.NumBytes;

        reflectionCopy.Fields = InteropArray<ReflectionResourceField>( resourceBindings[ i ].Reflection.Fields.NumElements( ) );
        for ( size_t j = 0; j < resourceBindings[ i ].Reflection.Fields.NumElements( ); ++j )
        {
            ReflectionResourceField &field    = reflectionCopy.Fields.GetElement( j );
            const auto              &srcField = resourceBindings[ i ].Reflection.Fields.GetElement( j );

            field.Name        = srcField.Name;
            field.Type        = srcField.Type;
            field.NumColumns  = srcField.NumColumns;
            field.NumRows     = srcField.NumRows;
            field.Elements    = srcField.Elements;
            field.Offset      = srcField.Offset;
            field.Level       = srcField.Level;
            field.ParentIndex = srcField.ParentIndex;
        }
    }

    return sortedBindings;
}
