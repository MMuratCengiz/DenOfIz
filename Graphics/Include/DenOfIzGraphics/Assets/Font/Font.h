/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // A TrueType or OpenType face, immutable once created. DenOfIz_TextRenderer and DenOfIz_UI borrow fonts: the caller destroys a font, and
    // only once nothing has it registered.
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Font )

    // Data is copied. FaceIndex selects a face of a font collection.
    typedef struct DenOfIz_FontDesc
    {
        DenOfIz_ByteArrayView Data;
        uint32_t              FaceIndex;
    } DenOfIz_FontDesc;

    // In ems, multiply by the font size. Descent and UnderlinePos are distances below the baseline, StrikeoutPos is above it.
    typedef struct DenOfIz_FontMetrics
    {
        float Ascent;
        float Descent;
        float LineGap;
        float LineHeight;
        float UnderlinePos;
        float UnderlineThickness;
        float StrikeoutPos;
        float StrikeoutThickness;
        float CapHeight;
    } DenOfIz_FontMetrics;

    // A face synthesized from another, as browsers do for a missing bold or italic. Bold thickens outlines and widens advances by that fraction of an
    // em, around 0.03 for a bold weight. Slant shears outlines by that tangent, around 0.2 for an oblique.
    typedef struct DenOfIz_FontSynthesisDesc
    {
        float Bold;
        float Slant;
    } DenOfIz_FontSynthesisDesc;

    // NULL when the data holds no face at FaceIndex.
    DZ_API DenOfIz_Font DenOfIz_Font_Create( const DenOfIz_FontDesc *desc );
    // A new font sharing the data of font, which may be destroyed first. Destroy it like any other font.
    DZ_API DenOfIz_Font DenOfIz_Font_CreateSynthetic( DenOfIz_Font font, const DenOfIz_FontSynthesisDesc *desc );
    // A relative path is resolved against DenOfIz_FSConfig's asset path.
    DZ_API DenOfIz_Font DenOfIz_Font_CreateFromPath( DenOfIz_StringView path, uint32_t faceIndex );
    // Does nothing for the default font.
    DZ_API void DenOfIz_Font_Destroy( DenOfIz_Font font );
    // The built in font, shared and alive until the process exits.
    DZ_API DenOfIz_Font        DenOfIz_Font_GetDefault( );
    DZ_API DenOfIz_FontMetrics DenOfIz_Font_GetMetrics( DenOfIz_Font font );
    DZ_API bool                DenOfIz_Font_HasCodePoint( DenOfIz_Font font, uint32_t codePoint );

#ifdef __cplusplus
}
#endif
