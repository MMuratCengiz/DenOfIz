/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IShaderBindingTable.h"

#define SBT_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IShaderBindingTable, handle )
#define SHADER_LOCAL_DATA_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IShaderLocalData, handle )

extern "C"
{

    void DenOfIz_ShaderBindingTable_Resize( DenOfIz_ShaderBindingTable sbt, const DenOfIz_SBTSizeDesc *resizeDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( sbt ) || resizeDesc == NULL )
        {
            return;
        }
        SBT_IMPL( sbt )->Resize( *resizeDesc );
    }

    void DenOfIz_ShaderBindingTable_BindRayGenerationShader( DenOfIz_ShaderBindingTable sbt, const DenOfIz_RayGenerationBindingDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( sbt ) || desc == NULL )
        {
            return;
        }
        SBT_IMPL( sbt )->BindRayGenerationShader( *desc );
    }

    void DenOfIz_ShaderBindingTable_BindHitGroup( DenOfIz_ShaderBindingTable sbt, const DenOfIz_HitGroupBindingDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( sbt ) || desc == NULL )
        {
            return;
        }
        SBT_IMPL( sbt )->BindHitGroup( *desc );
    }

    void DenOfIz_ShaderBindingTable_BindMissShader( DenOfIz_ShaderBindingTable sbt, const DenOfIz_MissBindingDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( sbt ) || desc == NULL )
        {
            return;
        }
        SBT_IMPL( sbt )->BindMissShader( *desc );
    }

    void DenOfIz_ShaderBindingTable_Build( DenOfIz_ShaderBindingTable sbt )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( sbt ) )
        {
            return;
        }
        SBT_IMPL( sbt )->Build( );
    }

    void DenOfIz_ShaderBindingTable_Destroy( DenOfIz_ShaderBindingTable sbt )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( sbt ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( SBT_IMPL( sbt ) );
    }
}
