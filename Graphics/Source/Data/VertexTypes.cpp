//--------------------------------------------------------------------------------------
// File: VertexTypes.cpp
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#include <DenOfIzGraphics/Data/VertexTypes.h>

using namespace DenOfIz;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// Vertex struct holding position information.
const std::vector<InputLayoutElementDesc> VertexPosition::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
};

static_assert(sizeof(VertexPosition) == 12, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPosition::InputLayout = { .InputGroups = { { .Elements = VertexPosition::InputElements } } };

//--------------------------------------------------------------------------------------
// Vertex struct holding position and color information.
const std::vector<InputLayoutElementDesc> VertexPositionColor::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::Color, .SemanticIndex = 0, .Format = Format::R32G32B32A32Float },
};

static_assert(sizeof(VertexPositionColor) == 28, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPositionColor::InputLayout = { .InputGroups = { { .Elements = VertexPositionColor::InputElements } } };

//--------------------------------------------------------------------------------------
// Vertex struct holding position and texture mapping information.
const std::vector<InputLayoutElementDesc> VertexPositionTexture::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::TextureCoordinate, .SemanticIndex = 0, .Format = Format::R32G32Float },
};

static_assert(sizeof(VertexPositionTexture) == 20, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPositionTexture::InputLayout = { .InputGroups = { { .Elements = VertexPositionTexture::InputElements } } };

//--------------------------------------------------------------------------------------
// Vertex struct holding position and dual texture mapping information.
const std::vector<InputLayoutElementDesc> VertexPositionDualTexture::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::TextureCoordinate, .SemanticIndex = 0, .Format = Format::R32G32Float },
    { .Semantic = Semantic::TextureCoordinate, .SemanticIndex = 1, .Format = Format::R32G32Float },
};

static_assert(sizeof(VertexPositionDualTexture) == 28, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPositionDualTexture::InputLayout = { .InputGroups = { { .Elements = VertexPositionDualTexture::InputElements } } };

//--------------------------------------------------------------------------------------
// Vertex struct holding position and normal vector.
const std::vector<InputLayoutElementDesc> VertexPositionNormal::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::Normal, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
};

static_assert(sizeof(VertexPositionNormal) == 24, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPositionNormal::InputLayout = { .InputGroups = { { .Elements = VertexPositionNormal::InputElements } } };

//--------------------------------------------------------------------------------------
// Vertex struct holding position, color, and texture mapping information.
const std::vector<InputLayoutElementDesc> VertexPositionColorTexture::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::Color, .SemanticIndex = 0, .Format = Format::R32G32B32A32Float },
    { .Semantic = Semantic::TextureCoordinate, .SemanticIndex = 0, .Format = Format::R32G32Float },
};

static_assert(sizeof(VertexPositionColorTexture) == 36, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPositionColorTexture::InputLayout = { .InputGroups = { { .Elements = VertexPositionColorTexture::InputElements } } };

//--------------------------------------------------------------------------------------
// Vertex struct holding position, normal vector, and color information.
const std::vector<InputLayoutElementDesc> VertexPositionNormalColor::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::Normal, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::Color, .SemanticIndex = 0, .Format = Format::R32G32B32A32Float },
};

static_assert(sizeof(VertexPositionNormalColor) == 40, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPositionNormalColor::InputLayout = { .InputGroups = { { .Elements = VertexPositionNormalColor::InputElements } } };

//--------------------------------------------------------------------------------------
// Vertex struct holding position, normal vector, and texture mapping information.
const std::vector<InputLayoutElementDesc> VertexPositionNormalTexture::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::Normal, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::TextureCoordinate, .SemanticIndex = 0, .Format = Format::R32G32Float },
};

static_assert(sizeof(VertexPositionNormalTexture) == 32, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPositionNormalTexture::InputLayout = { .InputGroups = { { .Elements = VertexPositionNormalTexture::InputElements } } };

//--------------------------------------------------------------------------------------
// Vertex struct holding position, normal vector, color, and texture mapping information.
const std::vector<InputLayoutElementDesc> VertexPositionNormalColorTexture::InputElements = {
    { .Semantic = Semantic::Position, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::Normal, .SemanticIndex = 0, .Format = Format::R32G32B32Float },
    { .Semantic = Semantic::Color, .SemanticIndex = 0, .Format = Format::R32G32B32A32Float },
    { .Semantic = Semantic::TextureCoordinate, .SemanticIndex = 0, .Format = Format::R32G32Float },
};

static_assert(sizeof(VertexPositionNormalColorTexture) == 48, "Vertex struct/layout mismatch");

const InputLayoutDesc VertexPositionNormalColorTexture::InputLayout = { .InputGroups = { { .Elements = VertexPositionNormalColorTexture::InputElements } } };
