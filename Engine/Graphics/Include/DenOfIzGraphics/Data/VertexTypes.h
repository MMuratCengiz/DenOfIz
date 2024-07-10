//--------------------------------------------------------------------------------------
// File: VertexTypes.h
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include <DenOfIzGraphics/Backends/Interface/IInputLayout.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace DenOfIz
{
    // Vertex struct holding position information.
    struct VertexPosition
    {
        VertexPosition() = default;

        VertexPosition(const VertexPosition &)            = default;
        VertexPosition &operator=(const VertexPosition &) = default;

        VertexPosition(VertexPosition &&)            = default;
        VertexPosition &operator=(VertexPosition &&) = default;

        VertexPosition(XMFLOAT3 const &iposition) noexcept : Position(iposition)
        {
        }

        VertexPosition(FXMVECTOR iposition) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
        }

        XMFLOAT3 Position;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

    // Vertex struct holding position and color information.
    struct VertexPositionColor
    {
        VertexPositionColor() = default;

        VertexPositionColor(const VertexPositionColor &)            = default;
        VertexPositionColor &operator=(const VertexPositionColor &) = default;

        VertexPositionColor(VertexPositionColor &&)            = default;
        VertexPositionColor &operator=(VertexPositionColor &&) = default;

        VertexPositionColor(XMFLOAT3 const &iposition, XMFLOAT4 const &icolor) noexcept : Position(iposition), Color(icolor)
        {
        }

        VertexPositionColor(FXMVECTOR iposition, FXMVECTOR icolor) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
            XMStoreFloat4(&this->Color, icolor);
        }

        XMFLOAT3 Position;
        XMFLOAT4 Color;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

    // Vertex struct holding position and texture mapping information.
    struct VertexPositionTexture
    {
        VertexPositionTexture() = default;

        VertexPositionTexture(const VertexPositionTexture &)            = default;
        VertexPositionTexture &operator=(const VertexPositionTexture &) = default;

        VertexPositionTexture(VertexPositionTexture &&)            = default;
        VertexPositionTexture &operator=(VertexPositionTexture &&) = default;

        VertexPositionTexture(XMFLOAT3 const &iposition, XMFLOAT2 const &itextureCoordinate) noexcept : Position(iposition), TextureCoordinate(itextureCoordinate)
        {
        }

        VertexPositionTexture(FXMVECTOR iposition, FXMVECTOR itextureCoordinate) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
            XMStoreFloat2(&this->TextureCoordinate, itextureCoordinate);
        }

        XMFLOAT3 Position;
        XMFLOAT2 TextureCoordinate;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

    // Vertex struct holding position and dual texture mapping information.
    struct VertexPositionDualTexture
    {
        VertexPositionDualTexture() = default;

        VertexPositionDualTexture(const VertexPositionDualTexture &)            = default;
        VertexPositionDualTexture &operator=(const VertexPositionDualTexture &) = default;

        VertexPositionDualTexture(VertexPositionDualTexture &&)            = default;
        VertexPositionDualTexture &operator=(VertexPositionDualTexture &&) = default;

        VertexPositionDualTexture(XMFLOAT3 const &iposition, XMFLOAT2 const &itextureCoordinate0, XMFLOAT2 const &itextureCoordinate1) noexcept :
            Position(iposition), TextureCoordinate0(itextureCoordinate0), TextureCoordinate1(itextureCoordinate1)
        {
        }

        VertexPositionDualTexture(FXMVECTOR iposition, FXMVECTOR itextureCoordinate0, FXMVECTOR itextureCoordinate1) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
            XMStoreFloat2(&this->TextureCoordinate0, itextureCoordinate0);
            XMStoreFloat2(&this->TextureCoordinate1, itextureCoordinate1);
        }

        XMFLOAT3 Position;
        XMFLOAT2 TextureCoordinate0;
        XMFLOAT2 TextureCoordinate1;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

    // Vertex struct holding position and normal vector.
    struct VertexPositionNormal
    {
        VertexPositionNormal() = default;

        VertexPositionNormal(const VertexPositionNormal &)            = default;
        VertexPositionNormal &operator=(const VertexPositionNormal &) = default;

        VertexPositionNormal(VertexPositionNormal &&)            = default;
        VertexPositionNormal &operator=(VertexPositionNormal &&) = default;

        VertexPositionNormal(XMFLOAT3 const &iposition, XMFLOAT3 const &inormal) noexcept : Position(iposition), Normal(inormal)
        {
        }

        VertexPositionNormal(FXMVECTOR iposition, FXMVECTOR inormal) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
            XMStoreFloat3(&this->Normal, inormal);
        }

        XMFLOAT3 Position;
        XMFLOAT3 Normal;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

    // Vertex struct holding position, color, and texture mapping information.
    struct VertexPositionColorTexture
    {
        VertexPositionColorTexture() = default;

        VertexPositionColorTexture(const VertexPositionColorTexture &)            = default;
        VertexPositionColorTexture &operator=(const VertexPositionColorTexture &) = default;

        VertexPositionColorTexture(VertexPositionColorTexture &&)            = default;
        VertexPositionColorTexture &operator=(VertexPositionColorTexture &&) = default;

        VertexPositionColorTexture(XMFLOAT3 const &iposition, XMFLOAT4 const &icolor, XMFLOAT2 const &itextureCoordinate) noexcept :
            Position(iposition), Color(icolor), TextureCoordinate(itextureCoordinate)
        {
        }

        VertexPositionColorTexture(FXMVECTOR iposition, FXMVECTOR icolor, FXMVECTOR itextureCoordinate) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
            XMStoreFloat4(&this->Color, icolor);
            XMStoreFloat2(&this->TextureCoordinate, itextureCoordinate);
        }

        XMFLOAT3 Position;
        XMFLOAT4 Color;
        XMFLOAT2 TextureCoordinate;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

    // Vertex struct holding position, normal vector, and color information.
    struct VertexPositionNormalColor
    {
        VertexPositionNormalColor() = default;

        VertexPositionNormalColor(const VertexPositionNormalColor &)            = default;
        VertexPositionNormalColor &operator=(const VertexPositionNormalColor &) = default;

        VertexPositionNormalColor(VertexPositionNormalColor &&)            = default;
        VertexPositionNormalColor &operator=(VertexPositionNormalColor &&) = default;

        VertexPositionNormalColor(XMFLOAT3 const &iposition, XMFLOAT3 const &inormal, XMFLOAT4 const &icolor) noexcept : Position(iposition), Normal(inormal), Color(icolor)
        {
        }

        VertexPositionNormalColor(FXMVECTOR iposition, FXMVECTOR inormal, FXMVECTOR icolor) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
            XMStoreFloat3(&this->Normal, inormal);
            XMStoreFloat4(&this->Color, icolor);
        }

        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT4 Color;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

    // Vertex struct holding position, normal vector, and texture mapping information.
    struct VertexPositionNormalTexture
    {
        VertexPositionNormalTexture() = default;

        VertexPositionNormalTexture(const VertexPositionNormalTexture &)            = default;
        VertexPositionNormalTexture &operator=(const VertexPositionNormalTexture &) = default;

        VertexPositionNormalTexture(VertexPositionNormalTexture &&)            = default;
        VertexPositionNormalTexture &operator=(VertexPositionNormalTexture &&) = default;

        VertexPositionNormalTexture(XMFLOAT3 const &iposition, XMFLOAT3 const &inormal, XMFLOAT2 const &itextureCoordinate) noexcept :
            Position(iposition), Normal(inormal), TextureCoordinate(itextureCoordinate)
        {
        }

        VertexPositionNormalTexture(FXMVECTOR iposition, FXMVECTOR inormal, FXMVECTOR itextureCoordinate) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
            XMStoreFloat3(&this->Normal, inormal);
            XMStoreFloat2(&this->TextureCoordinate, itextureCoordinate);
        }

        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT2 TextureCoordinate;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

    // Vertex struct holding position, normal vector, color, and texture mapping information.
    struct VertexPositionNormalColorTexture
    {
        VertexPositionNormalColorTexture() = default;

        VertexPositionNormalColorTexture(const VertexPositionNormalColorTexture &)            = default;
        VertexPositionNormalColorTexture &operator=(const VertexPositionNormalColorTexture &) = default;

        VertexPositionNormalColorTexture(VertexPositionNormalColorTexture &&)            = default;
        VertexPositionNormalColorTexture &operator=(VertexPositionNormalColorTexture &&) = default;

        VertexPositionNormalColorTexture(XMFLOAT3 const &iposition, XMFLOAT3 const &inormal, XMFLOAT4 const &icolor, XMFLOAT2 const &itextureCoordinate) noexcept :
            Position(iposition), Normal(inormal), Color(icolor), TextureCoordinate(itextureCoordinate)
        {
        }

        VertexPositionNormalColorTexture(FXMVECTOR iposition, FXMVECTOR inormal, FXMVECTOR icolor, CXMVECTOR itextureCoordinate) noexcept
        {
            XMStoreFloat3(&this->Position, iposition);
            XMStoreFloat3(&this->Normal, inormal);
            XMStoreFloat4(&this->Color, icolor);
            XMStoreFloat2(&this->TextureCoordinate, itextureCoordinate);
        }

        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT4 Color;
        XMFLOAT2 TextureCoordinate;

        static const InputLayoutDesc InputLayout;

    private:
        static const std::vector<InputLayoutElementDesc> InputElements;
    };

} // namespace DenOfIz
