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

#include "DenOfIzGraphicsInternal/Assets/Shaders/ReflectionDebugOutput.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#ifdef __linux__
#include <iomanip>
#endif

using namespace DenOfIz;

void ReflectionDebugOutput::DumpIRRootParameters( const std::vector<IRRootParameter1> &rootParameters, const char *prefix )
{
    std::stringstream output;
    output << "\n=== " << prefix << " IR Root Parameters ===\n";
    output << "Total Parameters: " << rootParameters.size( ) << "\n";

    for ( size_t i = 0; i < rootParameters.size( ); ++i )
    {
        const auto &param = rootParameters[ i ];
        output << "\nParameter[" << i << "]:\n";
        output << "  Type: " << [ & ]( )
        {
            switch ( param.ParameterType )
            {
            case IRRootParameterTypeDescriptorTable:
                return "Descriptor Table";
            case IRRootParameterType32BitConstants:
                return "32 Bit Constants";
            case IRRootParameterTypeCBV:
                return "CBV";
            case IRRootParameterTypeSRV:
                return "SRV";
            case IRRootParameterTypeUAV:
                return "UAV";
            default:
                return "Unknown";
            }
        }( ) << "\n";

        output << "  Shader Visibility: " << [ & ]( )
        {
            switch ( param.ShaderVisibility )
            {
            case IRShaderVisibilityAll:
                return "All";
            case IRShaderVisibilityVertex:
                return "Vertex";
            case IRShaderVisibilityPixel:
                return "Pixel";
            case IRShaderVisibilityGeometry:
                return "Geometry";
            case IRShaderVisibilityHull:
                return "Hull";
            case IRShaderVisibilityDomain:
                return "Domain";
            default:
                return "Unknown";
            }
        }( ) << "\n";

        // Log specific data based on parameter type
        switch ( param.ParameterType )
        {
        case IRRootParameterTypeDescriptorTable:
            {
                output << "  Descriptor Table:\n";
                output << "    NumDescriptorRanges: " << param.DescriptorTable.NumDescriptorRanges << "\n";

                for ( uint32_t j = 0; j < param.DescriptorTable.NumDescriptorRanges; j++ )
                {
                    const auto &range = param.DescriptorTable.pDescriptorRanges[ j ];
                    output << "    Range[" << j << "]: \n";
                    output << "      RangeType: " << [ & ]( )
                    {
                        switch ( range.RangeType )
                        {
                        case IRDescriptorRangeTypeSRV:
                            return "SRV";
                        case IRDescriptorRangeTypeUAV:
                            return "UAV";
                        case IRDescriptorRangeTypeCBV:
                            return "CBV";
                        case IRDescriptorRangeTypeSampler:
                            return "Sampler";
                        default:
                            return "Unknown";
                        }
                    }( ) << "\n";
                    output << "      NumDescriptors: " << range.NumDescriptors << "\n";
                    output << "      BaseShaderRegister: " << range.BaseShaderRegister << "\n";
                    output << "      RegisterSpace: " << range.RegisterSpace << "\n";
                    output << "      Offset: " << range.OffsetInDescriptorsFromTableStart << "\n";
                }
                break;
            }
        case IRRootParameterType32BitConstants:
            {
                output << "  32-Bit Constants: \n";
                output << "    ShaderRegister: " << param.Constants.ShaderRegister << "\n";
                output << "    RegisterSpace: " << param.Constants.RegisterSpace << "\n";
                output << "    Num32BitValues: " << param.Constants.Num32BitValues << "\n";
                break;
            }
        case IRRootParameterTypeCBV:
        case IRRootParameterTypeSRV:
        case IRRootParameterTypeUAV:
            {
                output << "  Descriptor:\n";
                output << "    ShaderRegister: " << param.Descriptor.ShaderRegister << "\n";
                output << "    RegisterSpace: " << param.Descriptor.RegisterSpace << "\n";
                break;
            }
        }
    }

    spdlog::info( "{}", output.str( ) );
}

void ReflectionDebugOutput::DumpReflectionInfo( const DenOfIz_ShaderReflectDesc &reflection )
{
    std::stringstream output;

    output << "\n\n=== Bind Group Layouts ===\n";
    DumpBindGroupLayouts( output, reflection.BindGroupLayouts );

    output << "\n=== Root Constants ===\n";
    DumpRootConstants( output, reflection.RootConstants );

    output << "\n=== Local Root Signatures ===\n";
    for ( uint32_t i = 0; i < reflection.LocalRootSignatures.NumElements; ++i )
    {
        if ( auto localRootSignatureDesc = reflection.LocalRootSignatures.Elements[ i ]; localRootSignatureDesc.ResourceBindings.NumElements > 0 )
        {
            output << "\nLocal Root Signature " << i << "\n";
            DumpLocalResourceBindings( output, localRootSignatureDesc.ResourceBindings );
        }
    }

    output << "\n\n";
    spdlog::info( "{}", output.str( ) );
}

void ReflectionDebugOutput::DumpBindGroupLayouts( std::stringstream &output, const DenOfIz_BindGroupLayoutDescArray &bindGroupLayouts )
{
    if ( bindGroupLayouts.NumElements == 0 )
    {
        output << "  (none)\n";
        return;
    }

    for ( uint32_t i = 0; i < bindGroupLayouts.NumElements; ++i )
    {
        const auto &layout = bindGroupLayouts.Elements[ i ];
        output << "\n--- Bind Group Layout (Space " << layout.RegisterSpace << ") ---\n";
        output << std::string( 70, '=' ) << '\n';
        output << std::setw( 15 ) << std::left << "Type" << std::setw( 10 ) << "Binding" << std::setw( 10 ) << "ArraySize" << std::setw( 10 ) << "Bindless" << "Stages\n";
        output << std::string( 70, '-' ) << '\n';

        for ( uint32_t j = 0; j < layout.Bindings.NumElements; ++j )
        {
            const auto                       &binding     = layout.Bindings.Elements[ j ];
            const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );

            output << std::setw( 15 ) << std::left << DxcEnumConverter::GetBindingTypeString( bindingType ) << std::setw( 10 ) << binding.Binding << std::setw( 10 )
                   << binding.ArraySize << std::setw( 10 ) << ( binding.IsBindless ? "Yes" : "No" ) << DxcEnumConverter::GetStagesString( binding.Stages ) << '\n';
        }
    }
}

void ReflectionDebugOutput::DumpLocalResourceBindings( std::stringstream &output, const DenOfIz_LocalResourceBindingDescArray &resourceBindings )
{
    if ( resourceBindings.NumElements == 0 )
    {
        return;
    }

    output << "\n=== Local Resource Bindings ===\n";
    output << std::string( 90, '=' ) << '\n';
    output << std::setw( 15 ) << std::left << "Type" << std::setw( 10 ) << "Space" << std::setw( 10 ) << "Binding" << std::setw( 10 ) << "ArraySize" << std::setw( 12 )
           << "NumBytes"
           << "Stages\n";
    output << std::string( 90, '-' ) << '\n';

    for ( uint32_t i = 0; i < resourceBindings.NumElements; ++i )
    {
        const auto                       &binding     = resourceBindings.Elements[ i ];
        const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );

        output << std::setw( 15 ) << std::left << DxcEnumConverter::GetBindingTypeString( bindingType ) << std::setw( 10 ) << binding.RegisterSpace << std::setw( 10 )
               << binding.Binding << std::setw( 10 ) << binding.ArraySize << std::setw( 12 ) << binding.NumBytes << DxcEnumConverter::GetStagesString( binding.Stages ) << '\n';
    }
}

void ReflectionDebugOutput::DumpRootConstants( std::stringstream &output, const DenOfIz_RootConstantBindingDescArray &rootConstants )
{
    if ( rootConstants.NumElements == 0 )
    {
        output << "  (none)\n";
        return;
    }

    output << std::string( 80, '=' ) << '\n';
    output << std::setw( 40 ) << std::left << "Name" << std::setw( 10 ) << "Binding" << std::setw( 12 ) << "NumBytes" << "Stages\n";
    output << std::string( 80, '-' ) << '\n';

    for ( uint32_t i = 0; i < rootConstants.NumElements; ++i )
    {
        const auto &constant = rootConstants.Elements[ i ];
        std::string constantName( constant.Name.Chars, constant.Name.NumChars );
        output << std::setw( 40 ) << std::left << constantName << std::setw( 10 ) << constant.Binding << std::setw( 12 ) << constant.NumBytes
               << DxcEnumConverter::GetStagesString( constant.Stages ) << "\n";
    }
}
