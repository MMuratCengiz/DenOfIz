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

    LOG( INFO ) << output.str( );
}


void ReflectionDebugOutput::DumpReflectionInfo( const ShaderReflectDesc &reflection )
{
    std::stringstream output;

    output << "\n\n=== Global Root Signature ===\n";
    DumpRootSignature( output, reflection.RootSignature );

    output << "\n=== Local Root Signatures ===\n";
    for ( int i = 0; i < reflection.LocalRootSignatures.NumElements( ); ++i )
    {
        if ( auto localRootSignatureDesc = reflection.LocalRootSignatures.GetElement( i ); localRootSignatureDesc.ResourceBindings.NumElements( ) > 0 )
        {
            output << "\nLocal Root Signature " << i << "\n";
            DumpResourceBindings( output, localRootSignatureDesc.ResourceBindings );
        }
    }

    output << "\n\n";
    LOG( INFO ) << output.str( );
}

void ReflectionDebugOutput::DumpResourceBindings( std::stringstream &output, const InteropArray<ResourceBindingDesc> &resourceBindings )
{
    if ( resourceBindings.NumElements( ) == 0 )
    {
        return;
    }

    output << "\n=== Resource Bindings ===\n";
    output << std::string( 100, '=' ) << '\n';
    output << std::setw( 40 ) << std::left << "Name" << std::setw( 15 ) << "Type" << std::setw( 10 ) << "Space" << std::setw( 10 ) << "Binding" << std::setw( 10 ) << "Size"
           << "Stages\n";
    output << std::string( 100, '-' ) << '\n';

    for ( int i = 0; i < resourceBindings.NumElements( ); ++i )
    {
        const auto &binding = resourceBindings.GetElement( i );

        output << std::setw( 40 ) << std::left << binding.Name.Get( ) << std::setw( 15 ) << DxcEnumConverter::GetBindingTypeString( binding.BindingType ) << std::setw( 10 )
               << binding.RegisterSpace << std::setw( 10 ) << binding.Binding << std::setw( 10 ) << binding.Reflection.NumBytes
               << DxcEnumConverter::GetStagesString( binding.Stages ) << '\n';

        if ( binding.Reflection.Fields.NumElements( ) > 0 )
        {
            output << std::string( 100, '-' ) << '\n';
            output << "  Fields for " << binding.Name.Get( ) << ":\n";
            output << "  " << std::string( 90, '-' ) << '\n';
            output << "  " << std::setw( 38 ) << std::left << "Field Name" << std::setw( 15 ) << "Type" << std::setw( 12 ) << "Columns"
                   << "Rows\n";
            output << "  " << std::string( 90, '-' ) << '\n';
            DumpStructFields( output, binding.Reflection.Fields );
            output << std::string( 100, '=' ) << '\n';
        }
    }
}

void ReflectionDebugOutput::DumpRootSignature( std::stringstream &output, const RootSignatureDesc &sig )
{
    DumpResourceBindings( output, sig.ResourceBindings );

    output << "\n--- Root Constants --- \n";
    for ( int i = 0; i < sig.RootConstants.NumElements( ); ++i )
    {
        const auto &constant = sig.RootConstants.GetElement( i );
        output << std::setw( 40 ) << constant.Name.Get( ) << std::setw( 10 ) << constant.Binding << std::setw( 10 ) << constant.NumBytes << " "
               << DxcEnumConverter::GetStagesString( constant.Stages ) << "\n";
    }
}

void ReflectionDebugOutput::DumpStructFields( std::stringstream &output, const InteropArray<ReflectionResourceField> &fields )
{
    for ( int i = 0; i < fields.NumElements( ); ++i )
    {
        const auto &field = fields.GetElement( i );

        std::string indent( 2 * field.Level, ' ' );
        output << indent << std::setw( 38 - indent.length( ) ) << std::left << field.Name.Get( ) << std::setw( 15 ) << DxcEnumConverter::GetFieldTypeString( field.Type )
               << std::setw( 12 ) << field.NumColumns << std::setw( 10 ) << field.NumRows << "offset:" << std::setw( 6 ) << field.Offset;

        if ( field.Elements > 0 )
        {
            output << " [" << field.Elements << "]";
        }
        if ( field.ParentIndex != UINT32_MAX )
        {
            output << " (parent: " << field.ParentIndex << ")";
        }
        output << '\n';
    }
}