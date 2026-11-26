/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "BottomLevelAS.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** Built on the GPU with DenOfIz_CommandList_BuildTopLevelAS. The BLASes it instances must outlive it. */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_TopLevelAS )

    typedef struct DenOfIz_ASInstanceDesc
    {
        DenOfIz_BottomLevelAS BLAS;
        DenOfIz_Float4x4      Transform;                   /**< First three rows as a row major 3x4 matrix, translation in the fourth column. Zero hides the instance. */
        uint32_t              ContributionToHitGroupIndex; /**< Hit group record offset in the shader binding table. */
        uint32_t              ID;                          /**< Shader visible instance id (InstanceID()). */
        uint32_t              Mask;                        /**< Low 8 bits, zero is never hit. */
    } DenOfIz_ASInstanceDesc;

    typedef struct DenOfIz_ASInstanceDescArray
    {
        DenOfIz_ASInstanceDesc *Elements;
        uint32_t                NumElements;
    } DenOfIz_ASInstanceDescArray;

    typedef struct DenOfIz_TopLevelASDesc
    {
        DenOfIz_ASInstanceDescArray Instances;
        uint32_t                    BuildFlags; /**< DenOfIz_ASBuildFlagBits. */
    } DenOfIz_TopLevelASDesc;

    typedef struct DenOfIz_UpdateTransformsDesc
    {
        DenOfIz_Float4x4Array Transforms; /**< One per instance, in creation order, at most the instance count. */
    } DenOfIz_UpdateTransformsDesc;

    /**
     * Sets instance transforms, applied by the next DenOfIz_CommandList_UpdateTopLevelAS or BuildTopLevelAS. DX12 and Vulkan write the
     * instance buffer right away, so no build or update of this TLAS may be executing.
     */
    DZ_API void DenOfIz_TopLevelAS_UpdateInstanceTransforms( DenOfIz_TopLevelAS topLevelAS, const DenOfIz_UpdateTransformsDesc *desc );
    /** Scratch bytes DenOfIz_CommandList_BuildTopLevelAS needs, in a HEAP_TYPE_GPU buffer with STORAGE usage. */
    DZ_API void DenOfIz_TopLevelAS_BuildNumBytes( DenOfIz_TopLevelAS topLevelAS, size_t *outNumBytes );
    /** Release is deferred until the GPU is done with it, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. */
    DZ_API void DenOfIz_TopLevelAS_Destroy( DenOfIz_TopLevelAS topLevelAS );

#ifdef __cplusplus
}
#endif
