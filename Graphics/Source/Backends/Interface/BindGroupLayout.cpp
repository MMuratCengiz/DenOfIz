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

#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroupLayout.h"

using namespace DenOfIz;

IBindGroupLayout::IBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc ) : m_storedDesc( desc )
{
    if ( desc.Bindings.NumElements > 0 && desc.Bindings.Elements != NULL )
    {
        m_bindingsCopy.assign( desc.Bindings.Elements, desc.Bindings.Elements + desc.Bindings.NumElements );
        m_storedDesc.Bindings.Elements = m_bindingsCopy.data( );
    }
}

const DenOfIz_BindGroupLayoutDesc &IBindGroupLayout::Desc( ) const
{
    return m_storedDesc;
}

#define BIND_GROUP_LAYOUT_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IBindGroupLayout, handle )

extern "C"
{

    void DenOfIz_BindGroupLayout_Destroy( DenOfIz_BindGroupLayout bindGroupLayout )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroupLayout ) )
        {
            return;
        }
        delete BIND_GROUP_LAYOUT_IMPL( bindGroupLayout );
    }

    const DenOfIz_BindGroupLayoutDesc *DenOfIz_BindGroupLayout_GetDesc( DenOfIz_BindGroupLayout bindGroupLayout )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroupLayout ) )
        {
            return NULL;
        }
        return &BIND_GROUP_LAYOUT_IMPL( bindGroupLayout )->Desc( );
    }
}
