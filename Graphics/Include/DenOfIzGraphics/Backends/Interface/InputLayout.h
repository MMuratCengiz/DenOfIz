/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "CommonData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** Read by pipelines at creation only. DenOfIz_ShaderProgram_Reflect fills a matching DenOfIz_InputLayoutDesc. */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_InputLayout )

    typedef enum DenOfIz_StepRate
    {
        DENOFIZ_STEP_RATE_PER_VERTEX,
        DENOFIZ_STEP_RATE_PER_INSTANCE
    } DenOfIz_StepRate;

    typedef enum DenOfIz_Semantic
    {
        DENOFIZ_SEMANTIC_POSITION,
        DENOFIZ_SEMANTIC_NORMAL,
        DENOFIZ_SEMANTIC_COLOR,
        DENOFIZ_SEMANTIC_TANGENT,
        DENOFIZ_SEMANTIC_BINORMAL,
        DENOFIZ_SEMANTIC_BITANGENT,
        DENOFIZ_SEMANTIC_BLEND_INDICES,
        DENOFIZ_SEMANTIC_BLEND_WEIGHT,
        DENOFIZ_SEMANTIC_TEXTURE_COORDINATE
    } DenOfIz_Semantic;

    typedef struct DenOfIz_InputLayoutElementDesc
    {
        DenOfIz_StringView Semantic; /**< HLSL semantic name without the index, e.g. "TEXCOORD". */
        uint32_t           SemanticIndex;
        DenOfIz_Format     Format;
    } DenOfIz_InputLayoutElementDesc;

    typedef struct DenOfIz_InputLayoutElementDescArray
    {
        DenOfIz_InputLayoutElementDesc *Elements;
        size_t                          NumElements;
    } DenOfIz_InputLayoutElementDescArray;

    /** One vertex buffer binding: the group's index is the vertex buffer slot. Elements are packed in order with no padding. */
    typedef struct DenOfIz_InputGroupDesc
    {
        DenOfIz_StepRate                    StepRate;
        DenOfIz_InputLayoutElementDescArray Elements;
        uint32_t                            Stride; /**< Bytes, zero is the sum of the element sizes. */
    } DenOfIz_InputGroupDesc;

    typedef struct DenOfIz_InputGroupDescArray
    {
        DenOfIz_InputGroupDesc *Elements;
        size_t                  NumElements;
    } DenOfIz_InputGroupDescArray;

    typedef struct DenOfIz_InputLayoutDesc
    {
        DenOfIz_InputGroupDescArray InputGroups;
    } DenOfIz_InputLayoutDesc;

    /** Release is deferred until the GPU is done with it, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. */
    DZ_API void DenOfIz_InputLayout_Destroy( DenOfIz_InputLayout inputLayout );

#ifdef __cplusplus
}
#endif
