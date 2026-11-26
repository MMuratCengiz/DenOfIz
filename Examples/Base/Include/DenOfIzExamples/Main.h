/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#pragma once

#include "IExample.h"

namespace DenOfIz
{
    DZ_EXAMPLES_API int Main( IExample *example );
} // namespace DenOfIz

#define DZ_EXAMPLE_MAIN( ExampleType )                                                                                                                                             \
    int main( )                                                                                                                                                                    \
    {                                                                                                                                                                              \
        ExampleType *example = new ExampleType( );                                                                                                                                 \
        return DenOfIz::Main( example );                                                                                                                                           \
    }
