/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
        DenOfIz::DeferredDestroyQueue::Destroy( BIND_GROUP_LAYOUT_IMPL( bindGroupLayout ) );
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
