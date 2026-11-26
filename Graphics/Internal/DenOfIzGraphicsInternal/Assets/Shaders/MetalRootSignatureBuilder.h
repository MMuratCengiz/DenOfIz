/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <cstdint>
#include <vector>
#include "DenOfIzGraphics/Assets/Shaders/ShaderReflectDesc.h"
#include "metal_irconverter/metal_irconverter.h"

namespace DenOfIz
{
    struct MetalRootSignatureBinding
    {
        IRDescriptorRangeType Type             = IRDescriptorRangeTypeCBV;
        uint32_t              RegisterSpace    = 0;
        uint32_t              Binding          = 0;
        uint32_t              NumDescriptors   = 1;
        uint32_t              NumConstantBytes = 0;
        bool                  IsBindless       = false;
        bool                  IsRootConstant   = false;
        bool                  IsRootDescriptor = false;
    };

    enum class MetalRootParameterKind
    {
        RootConstant,
        CbvSrvUavTable,
        SamplerTable,
        RootDescriptor
    };

    struct MetalRootParameterLocation
    {
        MetalRootParameterKind Kind;
        IRDescriptorRangeType  Type;
        uint32_t               RegisterSpace;
        uint32_t               Binding;
        uint32_t               OffsetBytes;
        uint32_t               SizeBytes;
    };

    struct MetalDescriptorTableEntry
    {
        IRDescriptorRangeType Type;
        uint32_t              RegisterSpace;
        uint32_t              Binding;
        uint32_t              NumDescriptors;
        uint32_t              TableIndex;
    };

    class MetalRootSignatureBuilder
    {
        struct SpaceParameters
        {
            std::vector<IRRootConstants>     Constants;
            std::vector<IRDescriptorRange1>  CbvSrvUavRanges;
            std::vector<IRDescriptorRange1>  SamplerRanges;
            std::vector<IRRootDescriptor1>   Descriptors;
            std::vector<IRRootParameterType> DescriptorTypes;
        };

        std::vector<SpaceParameters>            m_spaces;
        std::vector<IRRootParameter1>           m_rootParameters;
        std::vector<MetalRootParameterLocation> m_locations;
        std::vector<MetalDescriptorTableEntry>  m_tableEntries;
        uint32_t                                m_numBytes      = 0;
        IRRootSignature                        *m_rootSignature = nullptr;

    public:
        explicit MetalRootSignatureBuilder( std::vector<MetalRootSignatureBinding> bindings );
        ~MetalRootSignatureBuilder( );
        MetalRootSignatureBuilder( const MetalRootSignatureBuilder & )            = delete;
        MetalRootSignatureBuilder &operator=( const MetalRootSignatureBuilder & ) = delete;

        [[nodiscard]] IRRootSignature                               *RootSignature( ) const;
        [[nodiscard]] IRVersionedRootSignatureDescriptor             Descriptor( ) const;
        [[nodiscard]] const std::vector<IRRootParameter1>           &RootParameters( ) const;
        [[nodiscard]] const std::vector<MetalRootParameterLocation> &Locations( ) const;
        [[nodiscard]] const std::vector<MetalDescriptorTableEntry>  &TableEntries( ) const;
        [[nodiscard]] uint32_t                                       NumBytes( ) const;

        static int                   TypeOrder( IRDescriptorRangeType type );
        static IRDescriptorRangeType RangeType( DenOfIz_ResourceBindingType type );

    private:
        void BuildRootParameters( );
        void ResolveLocations( );
    };
} // namespace DenOfIz
