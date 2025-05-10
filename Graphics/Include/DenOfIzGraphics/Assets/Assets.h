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

#include "FileSystem/FileIO.h"
#include "FileSystem/PathResolver.h"

#include "Stream/BinaryContainer.h"
#include "Stream/BinaryReader.h"
#include "Stream/BinaryWriter.h"

#include "Import/AssetScanner.h"
#include "Import/AssimpImporter.h"
#include "Import/FontImporter.h"
#include "Import/IAssetImporter.h"
#include "Import/ShaderImporter.h"

#include "Serde/Animation/AnimationAsset.h"
#include "Serde/Animation/AnimationAssetReader.h"
#include "Serde/Animation/AnimationAssetWriter.h"
#include "Serde/Common/AssetReaderHelpers.h"
#include "Serde/Common/AssetWriterHelpers.h"
#include "Serde/Font/FontAsset.h"
#include "Serde/Font/FontAssetReader.h"
#include "Serde/Font/FontAssetWriter.h"
#include "Serde/Material/MaterialAsset.h"
#include "Serde/Material/MaterialAssetReader.h"
#include "Serde/Material/MaterialAssetWriter.h"
#include "Serde/Mesh/MeshAsset.h"
#include "Serde/Mesh/MeshAssetReader.h"
#include "Serde/Mesh/MeshAssetWriter.h"
#include "Serde/Physics/PhysicsAsset.h"
#include "Serde/Physics/PhysicsAssetReader.h"
#include "Serde/Physics/PhysicsAssetWriter.h"
#include "Serde/Shader/ShaderAsset.h"
#include "Serde/Shader/ShaderAssetReader.h"
#include "Serde/Shader/ShaderAssetWriter.h"
#include "Serde/Skeleton/SkeletonAsset.h"
#include "Serde/Skeleton/SkeletonAssetReader.h"
#include "Serde/Skeleton/SkeletonAssetWriter.h"
#include "Serde/Texture/TextureAsset.h"
#include "Serde/Texture/TextureAssetReader.h"
#include "Serde/Texture/TextureAssetWriter.h"

#include "Shaders/DxcEnumConverter.h"
#include "Shaders/DxilToMsl.h"
#include "Shaders/ReflectionDebugOutput.h"
#include "Shaders/ShaderCompiler.h"
#include "Shaders/ShaderReflectDesc.h"
#include "Shaders/ShaderReflectionHelper.h"

#include "Font/EmbeddedTextRendererShaders.h"
#include "Font/Font.h"
#include "Font/FontLibrary.h"
#include "Font/TextLayout.h"
#include "Font/TextRenderer.h"

#include "Bundle/Bundle.h"
#include "Bundle/BundleManager.h"