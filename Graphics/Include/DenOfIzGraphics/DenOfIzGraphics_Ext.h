/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

// Include all graphics headers, and supporting code such as animation and font
#include "Assets/Assets.h"
#include "DenOfIzGraphics.h"
#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphics/Assets/Shaders/ShaderIncludeHandler.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Backends/Interface/NativeInterop.h"
#include "DenOfIzGraphics/Backends/MetalCapture.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Data/Geometry.h"

#include "DenOfIzGraphics/Input/Clipboard.h"
#include "DenOfIzGraphics/Support/FrameSync.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphics/UI/UI.h"
#include "DenOfIzGraphics/Utilities/Engine.h"
#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"
#include "DenOfIzGraphics/Utilities/StepTimer.h"
