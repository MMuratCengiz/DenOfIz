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

#include "DenOfIzBindings/Emitters/CSharpEmitter.h"
#include "DenOfIzBindings/Core/DzirTypeVisitor.h"
#include "DenOfIzBindings/Core/NativeConvention.h"

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

using namespace DenOfIz;

static const std::unordered_map<std::string, std::string> NumericsTypeMap = {
    { "Float2", "Vector2" },
    { "Float3", "Vector3" },
    { "Float4", "Vector4" },
    { "Float4x4", "Matrix4x4" },
};

static bool IsNumericsType( const std::string &shortName )
{
    return NumericsTypeMap.find( shortName ) != NumericsTypeMap.end( );
}

static std::string GetNumericsType( const std::string &shortName )
{
    auto it = NumericsTypeMap.find( shortName );
    if ( it != NumericsTypeMap.end( ) )
    {
        return it->second;
    }
    return shortName;
}

static const EmitterStreamFormattingDesc CSharpFormatting = {
    .LineEndIndicator        = ";",
    .IncludeFormat           = "using {};",
    .IncludeFormatVariant    = "using {};",
    .AssignmentFormat        = "{} = {}",
    .NamespaceOpenFormat     = "namespace {}",
    .NamespaceCloseFormat    = "}}",
    .ClassOpenFormat         = "public partial class {}",
    .ClassCloseFormat        = "}}",
    .FunctionDeclFormat      = "{} {}({});",
    .FunctionDefOpenFormat   = "{} {}({})",
    .BlockCommentOpenFormat  = "/*",
    .BlockCommentLineFormat  = " * {}",
    .BlockCommentCloseFormat = " */",
    .InlineCommentFormat     = "// {}",
};

CSharpEmitter::CSharpEmitter( const DzirLibrary *library, const CSharpEmitterConfig &config ) : m_library( library ), m_config( config )
{
}

void CSharpEmitter::SetOutputDirectory( const std::filesystem::path &outputDir )
{
    m_outputDir = outputDir;
    std::filesystem::create_directories( m_outputDir );
}

void CSharpEmitter::Emit( )
{
    if ( m_config.GenerateNative )
    {
        EmitNativeFile( );
    }

    if ( m_config.GenerateEnums )
    {
        EmitEnumsFile( );
    }

    if ( m_config.GenerateStructs )
    {
        EmitStructsFile( );
        EmitHelpersFile( );
    }

    if ( m_config.GenerateWrappers )
    {
        const auto &ownerTypes = m_library->GetOwnerTypes( );
        for ( const auto &[ name, ownerType ] : ownerTypes )
        {
            EmitWrapperFile( ownerType );
        }

        EmitOrphanHandleWrappers( );
        EmitGlobalFunctionsFile( );
    }
}

void CSharpEmitter::EmitNativeFile( )
{
    EmitterStreamDesc desc;
    desc.FilePath    = m_outputDir / "Native.g.cs";
    desc.IndentWidth = 4;
    desc.Formatting  = &CSharpFormatting;

    EmitterStream stream( desc );

    stream.WriteLine( "// Auto-generated file - do not edit" );
    stream.WriteLine( "using System;" );
    stream.WriteLine( "using System.Numerics;" );
    stream.WriteLine( "using System.Runtime.InteropServices;" );
    stream.WriteLine( );

    stream.WriteLine( "namespace {}.Native", m_config.Namespace );
    stream.OpenBlock( );

    stream.WriteLine( "internal static partial class Methods" );
    stream.OpenBlock( );

    stream.Write( "#if BROWSER_WASM\n" );
    stream.WriteLine( "private const string LibraryName = \"__Internal\";" );
    stream.Write( "#else\n" );
    stream.WriteLine( "private const string LibraryName = \"{}\";", m_config.LibraryName );
    stream.Write( "#endif\n" );
    stream.WriteLine( );

    for ( const DzirHeader *header : m_library->GetHeaders( ) )
    {
        for ( const DzirFunction *func : header->Functions )
        {
            EmitNativeFunction( stream, func );
        }
    }

    stream.CloseBlock( );
    stream.CloseBlock( );
}

void CSharpEmitter::EmitEnumsFile( )
{
    EmitterStreamDesc desc;
    desc.FilePath    = m_outputDir / "Enums.g.cs";
    desc.IndentWidth = 4;
    desc.Formatting  = &CSharpFormatting;

    EmitterStream stream( desc );

    stream.WriteLine( "// Auto-generated file - do not edit" );
    stream.WriteLine( "using System;" );
    stream.WriteLine( );

    stream.WriteLine( "namespace {}", m_config.Namespace );
    stream.OpenBlock( );

    for ( const DzirHeader *header : m_library->GetHeaders( ) )
    {
        for ( const DzirEnum *enumType : header->Enums )
        {
            EmitEnum( stream, enumType );
            stream.WriteLine( );
        }
    }

    stream.CloseBlock( );
}

void CSharpEmitter::EmitStructsFile( )
{
    EmitterStreamDesc desc;
    desc.FilePath    = m_outputDir / "Structs.g.cs";
    desc.IndentWidth = 4;
    desc.Formatting  = &CSharpFormatting;

    EmitterStream stream( desc );

    stream.WriteLine( "// Auto-generated file - do not edit" );
    stream.WriteLine( "using System;" );
    stream.WriteLine( "using System.Numerics;" );
    stream.WriteLine( "using System.Runtime.InteropServices;" );
    stream.WriteLine( );

    stream.WriteLine( "namespace {}", m_config.Namespace );
    stream.OpenBlock( );

    for ( const DzirHeader *header : m_library->GetHeaders( ) )
    {
        for ( const DzirStruct *structType : header->Structs )
        {
            if ( IsNumericsType( structType->ShortName ) )
            {
                continue;
            }
            EmitStruct( stream, structType );
            stream.WriteLine( );
        }
    }

    stream.CloseBlock( );
}

void CSharpEmitter::EmitHelpersFile( )
{
    EmitterStreamDesc desc;
    desc.FilePath    = m_outputDir / "ArrayExtensions.g.cs";
    desc.IndentWidth = 4;
    desc.Formatting  = &CSharpFormatting;

    EmitterStream stream( desc );

    stream.WriteLine( "// Auto-generated file - do not edit" );
    stream.WriteLine( "using System;" );
    stream.WriteLine( "using System.Numerics;" );
    stream.WriteLine( "using System.Runtime.InteropServices;" );
    stream.WriteLine( "using System.Text;" );
    stream.WriteLine( );

    stream.WriteLine( "namespace {}", m_config.Namespace );
    stream.OpenBlock( );

    stream.WriteLine( "public class DenOfIzException : Exception" );
    stream.OpenBlock( );
    stream.WriteLine( "public Result Result { get; }" );
    stream.WriteLine( );
    stream.WriteLine( "public DenOfIzException(Result result) : base($\"DenOfIz operation failed with result: {{result}}\")" );
    stream.OpenBlock( );
    stream.WriteLine( "Result = result;" );
    stream.CloseBlock( );
    stream.CloseBlock( );
    stream.WriteLine( );

    stream.WriteLine( "public partial struct StringView" );
    stream.OpenBlock( );
    stream.WriteLine( "public override string ToString()" );
    stream.OpenBlock( );
    stream.WriteLine( "if (Chars == IntPtr.Zero || NumChars == 0) return string.Empty;" );
    stream.WriteLine( "unsafe { return Encoding.UTF8.GetString((byte*)Chars, (int)NumChars); }" );
    stream.CloseBlock( );
    stream.WriteLine( );
    stream.WriteLine( "public static implicit operator string(StringView sv) => sv.ToString();" );
    stream.WriteLine( );
    stream.WriteLine( "public static Pinned Create(string s) => new Pinned(s);" );
    stream.WriteLine( );
    stream.WriteLine( "public sealed class Pinned : IDisposable" );
    stream.OpenBlock( );
    stream.WriteLine( "private GCHandle _handle;" );
    stream.WriteLine( "public StringView Value { get; }" );
    stream.WriteLine( );
    stream.WriteLine( "public Pinned(string s)" );
    stream.OpenBlock( );
    stream.WriteLine( "if (string.IsNullOrEmpty(s)) { Value = default; return; }" );
    stream.WriteLine( "var bytes = Encoding.UTF8.GetBytes(s);" );
    stream.WriteLine( "_handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);" );
    stream.WriteLine( "Value = new StringView { Chars = _handle.AddrOfPinnedObject(), NumChars = (ulong)bytes.Length };" );
    stream.CloseBlock( );
    stream.WriteLine( );
    stream.WriteLine( "public static implicit operator StringView(Pinned p) => p.Value;" );
    stream.WriteLine( "~Pinned() => Dispose();" );
    stream.WriteLine( "public void Dispose() { if (_handle.IsAllocated) { _handle.Free(); GC.SuppressFinalize(this); } }" );
    stream.CloseBlock( );
    stream.WriteLine( );
    stream.WriteLine(
        "private static readonly System.Collections.Generic.Dictionary<string, Pinned> _internedStrings = new System.Collections.Generic.Dictionary<string, Pinned>();" );
    stream.WriteLine( );
    stream.WriteLine( "public static StringView Intern(string s)" );
    stream.OpenBlock( );
    stream.WriteLine( "if (string.IsNullOrEmpty(s)) return default;" );
    stream.WriteLine( "if (!_internedStrings.TryGetValue(s, out var pinned))" );
    stream.OpenBlock( );
    stream.WriteLine( "pinned = new Pinned(s);" );
    stream.WriteLine( "_internedStrings[s] = pinned;" );
    stream.CloseBlock( );
    stream.WriteLine( "return pinned.Value;" );
    stream.CloseBlock( );
    stream.CloseBlock( );
    stream.WriteLine( );

    stream.WriteLine( "public sealed class PinnedArray<T> : IDisposable where T : unmanaged" );
    stream.OpenBlock( );
    stream.WriteLine( "private readonly T[] _array;" );
    stream.WriteLine( "private GCHandle _handle;" );
    stream.WriteLine( );
    stream.WriteLine( "public PinnedArray(int capacity)" );
    stream.OpenBlock( );
    stream.WriteLine( "_array = new T[capacity];" );
    stream.WriteLine( "_handle = GCHandle.Alloc(_array, GCHandleType.Pinned);" );
    stream.CloseBlock( );
    stream.WriteLine( );
    stream.WriteLine( "public PinnedArray(T[] source)" );
    stream.OpenBlock( );
    stream.WriteLine( "_array = source;" );
    stream.WriteLine( "_handle = GCHandle.Alloc(_array, GCHandleType.Pinned);" );
    stream.CloseBlock( );
    stream.WriteLine( );
    stream.WriteLine( "public T[] Array => _array;" );
    stream.WriteLine( "public IntPtr Pointer => _handle.AddrOfPinnedObject();" );
    stream.WriteLine( "public GCHandle Handle => _handle;" );
    stream.WriteLine( "public int Length => _array.Length;" );
    stream.WriteLine( "public ref T this[int index] => ref _array[index];" );
    stream.WriteLine( );
    stream.WriteLine( "~PinnedArray() => Dispose();" );
    stream.WriteLine( "public void Dispose() { if (_handle.IsAllocated) { _handle.Free(); GC.SuppressFinalize(this); } }" );
    stream.CloseBlock( );
    stream.WriteLine( );

    const auto &structMap = m_library->GetStructMap( );
    for ( const auto &[ name, structType ] : structMap )
    {
        std::string shortName = structType->ShortName;
        std::string elementType;
        bool        isArrayLike = false;

        if ( shortName.length( ) > 5 && shortName.substr( shortName.length( ) - 5 ) == "Array" )
        {
            elementType = shortName.substr( 0, shortName.length( ) - 5 );
            isArrayLike = true;
        }
        else if ( shortName.length( ) > 9 && shortName.substr( shortName.length( ) - 9 ) == "ArrayView" )
        {
            elementType = shortName.substr( 0, shortName.length( ) - 9 );
            isArrayLike = true;
        }

        if ( isArrayLike )
        {

            bool        hasElements     = false;
            bool        hasNumElements  = false;
            std::string numElementsType = "ulong";
            for ( const DzirField *field : structType->Fields )
            {
                if ( field->Name == "Elements" )
                {
                    hasElements = true;
                }
                if ( field->Name == "NumElements" )
                {
                    hasNumElements = true;
                    if ( const auto *prim = std::get_if<PrimitiveKind>( &field->Type ) )
                    {
                        numElementsType = NativeConvention::PrimitiveTypeToString( *prim );
                    }
                }
            }

            if ( !hasElements || !hasNumElements )
            {
                continue;
            }

            bool        isKnownType  = false;
            bool        isHandleType = false;
            std::string csElementType;
            std::string wrapperType;

            if ( elementType == "Float" )
            {
                csElementType = "float";
                isKnownType   = true;
            }
            else if ( elementType == "Int32" )
            {
                csElementType = "int";
                isKnownType   = true;
            }
            else if ( elementType == "UInt32" )
            {
                csElementType = "uint";
                isKnownType   = true;
            }
            else if ( elementType == "UInt16" )
            {
                csElementType = "ushort";
                isKnownType   = true;
            }
            else if ( elementType == "Int16" )
            {
                csElementType = "short";
                isKnownType   = true;
            }
            else if ( elementType == "Byte" )
            {
                csElementType = "byte";
                isKnownType   = true;
            }
            else if ( elementType == "Bool" )
            {
                csElementType = "bool";
                isKnownType   = true;
            }
            else
            {
                csElementType = GetNumericsType( elementType );
                for ( const auto &[ structName, s ] : structMap )
                {
                    if ( s->ShortName == elementType )
                    {
                        isKnownType = true;
                        break;
                    }
                }
                if ( IsNumericsType( elementType ) )
                {
                    isKnownType = true;
                }

                if ( !isKnownType )
                {
                    const auto &handleTypeNames = m_library->GetHandleTypeNames( );
                    for ( const std::string &handleName : handleTypeNames )
                    {
                        if ( NativeConvention::StripPrefix( handleName ) == elementType )
                        {
                            wrapperType   = elementType;
                            csElementType = "ulong";
                            isHandleType  = true;
                            isKnownType   = true;
                            break;
                        }
                    }
                }
            }

            if ( !isKnownType )
            {
                continue;
            }

            stream.WriteLine( "public partial struct {}", shortName );
            stream.OpenBlock( );

            if ( isHandleType )
            {
                stream.WriteLine( "private static readonly System.Runtime.CompilerServices.ConditionalWeakTable<object, {}?[]> _arrayCache = new();", wrapperType );
                stream.WriteLine( );

                stream.WriteLine( "public readonly {}?[] ToArray()", wrapperType );
                stream.OpenBlock( );
                stream.WriteLine( "if (Elements == IntPtr.Zero || NumElements == 0) return Array.Empty<{}>();", wrapperType );
                stream.WriteLine( );
                stream.WriteLine( "var key = (object)Elements;" );
                stream.WriteLine( "if (_arrayCache.TryGetValue(key, out var cached))" );
                stream.OpenBlock( );
                stream.WriteLine( "if (cached.Length == (int)NumElements) return cached;" );
                stream.CloseBlock( );
                stream.WriteLine( );
                stream.WriteLine( "var result = new {}?[NumElements];", wrapperType );
                stream.WriteLine( "unsafe" );
                stream.OpenBlock( );
                stream.WriteLine( "var ptr = (ulong*)Elements.ToPointer();" );
                stream.WriteLine( "for (int i = 0; i < (int)NumElements; i++)" );
                stream.OpenBlock( );
                stream.WriteLine( "result[i] = new {}(ptr[i], ownsHandle: false);", wrapperType );
                stream.CloseBlock( );
                stream.CloseBlock( );
                stream.WriteLine( "_arrayCache.AddOrUpdate(key, result);" );
                stream.WriteLine( "return result;" );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public readonly Span<ulong> AsHandleSpan()" );
                stream.OpenBlock( );
                stream.WriteLine( "if (Elements == IntPtr.Zero || NumElements == 0) return Span<ulong>.Empty;" );
                stream.WriteLine( "unsafe {{ return new Span<ulong>(Elements.ToPointer(), (int)NumElements); }}" );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public static {} FromHandles(ulong[] handles)", shortName );
                stream.OpenBlock( );
                stream.WriteLine( "if (handles == null || handles.Length == 0) return default;" );
                stream.WriteLine( "var pinned = new Pinned(handles);" );
                stream.WriteLine( "return pinned.Value;" );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public static {} FromPinned(GCHandle handle, int count)", shortName );
                stream.OpenBlock( );
                stream.WriteLine( "return new {} {{ Elements = handle.AddrOfPinnedObject(), NumElements = ({})count }};", shortName, numElementsType );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public static unsafe {} FromPointer(ulong* ptr, int count)", shortName );
                stream.OpenBlock( );
                stream.WriteLine( "return new {} {{ Elements = (IntPtr)ptr, NumElements = ({})count }};", shortName, numElementsType );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public static Pinned Create({}[] array) => new Pinned(array);", wrapperType );
                stream.WriteLine( );
                stream.WriteLine( "public sealed class Pinned : IDisposable" );
                stream.OpenBlock( );
                stream.WriteLine( "private GCHandle _handle;" );
                stream.WriteLine( "public {} Value {{ get; }}", shortName );
                stream.WriteLine( );
                stream.WriteLine( "public Pinned({}[] wrappers)", wrapperType );
                stream.OpenBlock( );
                stream.WriteLine( "if (wrappers == null || wrappers.Length == 0) { Value = default; return; }" );
                stream.WriteLine( "var handles = new ulong[wrappers.Length];" );
                stream.WriteLine( "for (int i = 0; i < wrappers.Length; i++) handles[i] = wrappers[i]?.Handle ?? 0;" );
                stream.WriteLine( "_handle = GCHandle.Alloc(handles, GCHandleType.Pinned);" );
                stream.WriteLine( "Value = new {} {{ Elements = _handle.AddrOfPinnedObject(), NumElements = ({})wrappers.Length }};", shortName, numElementsType );
                stream.CloseBlock( );
                stream.WriteLine( );
                stream.WriteLine( "public Pinned(ulong[] handles)" );
                stream.OpenBlock( );
                stream.WriteLine( "if (handles == null || handles.Length == 0) { Value = default; return; }" );
                stream.WriteLine( "_handle = GCHandle.Alloc(handles, GCHandleType.Pinned);" );
                stream.WriteLine( "Value = new {} {{ Elements = _handle.AddrOfPinnedObject(), NumElements = ({})handles.Length }};", shortName, numElementsType );
                stream.CloseBlock( );
                stream.WriteLine( );
                stream.WriteLine( "public static implicit operator {}(Pinned p) => p.Value;", shortName );
                stream.WriteLine( "~Pinned() => Dispose();" );
                stream.WriteLine( "public void Dispose() {{ if (_handle.IsAllocated) {{ _handle.Free(); GC.SuppressFinalize(this); }} }}" );
                stream.CloseBlock( );
            }
            else
            {
                stream.WriteLine( "public readonly {}[] ToArray()", csElementType );
                stream.OpenBlock( );
                stream.WriteLine( "if (Elements == IntPtr.Zero || NumElements == 0) return Array.Empty<{}>();", csElementType );
                stream.WriteLine( "var result = new {}[NumElements];", csElementType );
                stream.WriteLine( "int elementSize = Marshal.SizeOf<{}>();", csElementType );
                stream.WriteLine( "for (int i = 0; i < (int)NumElements; i++)" );
                stream.OpenBlock( );
                stream.WriteLine( "result[i] = Marshal.PtrToStructure<{}>(Elements + i * elementSize);", csElementType );
                stream.CloseBlock( );
                stream.WriteLine( "return result;" );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public readonly Span<{}> AsSpan()", csElementType );
                stream.OpenBlock( );
                stream.WriteLine( "if (Elements == IntPtr.Zero || NumElements == 0) return Span<{}>.Empty;", csElementType );
                stream.WriteLine( "unsafe {{ return new Span<{}>(Elements.ToPointer(), (int)NumElements); }}", csElementType );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public static {} FromPinned(GCHandle handle, int count)", shortName );
                stream.OpenBlock( );
                stream.WriteLine( "return new {} {{ Elements = handle.AddrOfPinnedObject(), NumElements = ({})count }};", shortName, numElementsType );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public static unsafe {} FromPointer({}* ptr, int count)", shortName, csElementType );
                stream.OpenBlock( );
                stream.WriteLine( "return new {} {{ Elements = (IntPtr)ptr, NumElements = ({})count }};", shortName, numElementsType );
                stream.CloseBlock( );

                stream.WriteLine( );
                stream.WriteLine( "public static Pinned Create({}[] array) => new Pinned(array);", csElementType );
                stream.WriteLine( );
                stream.WriteLine( "public sealed class Pinned : IDisposable" );
                stream.OpenBlock( );
                stream.WriteLine( "private GCHandle _handle;" );
                stream.WriteLine( "public {} Value {{ get; }}", shortName );
                stream.WriteLine( );
                stream.WriteLine( "public Pinned({}[] array)", csElementType );
                stream.OpenBlock( );
                stream.WriteLine( "if (array == null || array.Length == 0) { Value = default; return; }" );
                stream.WriteLine( "_handle = GCHandle.Alloc(array, GCHandleType.Pinned);" );
                stream.WriteLine( "Value = new {} {{ Elements = _handle.AddrOfPinnedObject(), NumElements = ({})array.Length }};", shortName, numElementsType );
                stream.CloseBlock( );
                stream.WriteLine( );
                stream.WriteLine( "public static implicit operator {}(Pinned p) => p.Value;", shortName );
                stream.WriteLine( "~Pinned() => Dispose();" );
                stream.WriteLine( "public void Dispose() {{ if (_handle.IsAllocated) {{ _handle.Free(); GC.SuppressFinalize(this); }} }}" );
                stream.CloseBlock( );
            }

            stream.CloseBlock( );
            stream.WriteLine( );
        }
    }

    stream.CloseBlock( );
}

void CSharpEmitter::EmitStructExtensionsFile( const std::unordered_map<std::string, std::vector<const DzirFunction *>> &structFuncs )
{
    const auto &structMap = m_library->GetStructMap( );

    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> structsWithHandleFields;

    for ( const auto &[ name, structType ] : structMap )
    {
        std::vector<std::pair<std::string, std::string>> handleFields;
        for ( const DzirField *field : structType->Fields )
        {
            if ( const auto *handleRef = std::get_if<DzirHandleRef>( &field->Type ) )
            {
                std::string wrapperName = NativeConvention::StripPrefix( handleRef->Name );
                handleFields.emplace_back( field->Name, wrapperName );
            }
        }
        if ( !handleFields.empty( ) )
        {
            structsWithHandleFields[ structType->ShortName ] = std::move( handleFields );
        }
    }

    if ( structFuncs.empty( ) && structsWithHandleFields.empty( ) )
    {
        return;
    }

    EmitterStreamDesc desc;
    desc.FilePath    = m_outputDir / "StructExtensions.g.cs";
    desc.IndentWidth = 4;
    desc.Formatting  = &CSharpFormatting;

    EmitterStream stream( desc );

    stream.WriteLine( "// Auto-generated file - do not edit" );
    stream.WriteLine( "using System;" );
    stream.WriteLine( "using System.Numerics;" );
    stream.WriteLine( "using System.Runtime.InteropServices;" );
    stream.WriteLine( "using {}.Native;", m_config.Namespace );
    stream.WriteLine( );

    stream.WriteLine( "namespace {}", m_config.Namespace );
    stream.OpenBlock( );

    std::unordered_set<std::string> emittedStructs;

    for ( const auto &[ structName, funcs ] : structFuncs )
    {
        stream.WriteLine( "public partial struct {}", structName );
        stream.OpenBlock( );

        for ( const DzirFunction *func : funcs )
        {
            EmitStaticMethod( stream, func );
        }

        auto handleIt = structsWithHandleFields.find( structName );
        if ( handleIt != structsWithHandleFields.end( ) )
        {
            for ( const auto &[ fieldName, wrapperName ] : handleIt->second )
            {
                stream.WriteLine( "public readonly {}? Get{}() => {} != 0 ? new {}({}, ownsHandle: false) : null;", wrapperName, fieldName, fieldName, wrapperName, fieldName );
                stream.WriteLine( );
            }
        }

        stream.CloseBlock( );
        stream.WriteLine( );
        emittedStructs.insert( structName );
    }

    for ( const auto &[ structName, handleFields ] : structsWithHandleFields )
    {
        if ( emittedStructs.count( structName ) )
        {
            continue;
        }

        stream.WriteLine( "public partial struct {}", structName );
        stream.OpenBlock( );

        for ( const auto &[ fieldName, wrapperName ] : handleFields )
        {
            stream.WriteLine( "public readonly {}? Get{}() => {} != 0 ? new {}({}, ownsHandle: false) : null;", wrapperName, fieldName, fieldName, wrapperName, fieldName );
            stream.WriteLine( );
        }

        stream.CloseBlock( );
        stream.WriteLine( );
    }

    stream.CloseBlock( );
}

void CSharpEmitter::EmitGlobalFunctionsFile( )
{
    const auto &globalFunctions = m_library->GetGlobalFunctions( );
    if ( globalFunctions.empty( ) )
    {
        return;
    }

    const auto &ownerTypes      = m_library->GetOwnerTypes( );
    const auto &enumMap         = m_library->GetEnumMap( );
    const auto &structMap       = m_library->GetStructMap( );
    const auto &handleTypeNames = m_library->GetHandleTypeNames( );

    std::unordered_set<std::string> enumShortNames;
    std::unordered_set<std::string> structShortNames;
    std::unordered_set<std::string> reservedShortNames;

    for ( const auto &[ name, enumType ] : enumMap )
    {
        enumShortNames.insert( enumType->ShortName );
        reservedShortNames.insert( enumType->ShortName );
    }

    for ( const auto &[ name, structType ] : structMap )
    {
        structShortNames.insert( structType->ShortName );
        reservedShortNames.insert( structType->ShortName );
    }

    for ( const auto &[ name, ownerType ] : ownerTypes )
    {
        reservedShortNames.insert( ownerType->ShortName );
    }

    for ( const std::string &handleName : handleTypeNames )
    {
        reservedShortNames.insert( NativeConvention::StripPrefix( handleName ) );
    }

    std::vector<const DzirFunction *>                                  trulyGlobalFuncs;
    std::unordered_map<std::string, std::vector<const DzirFunction *>> typedFuncs;
    std::unordered_map<std::string, std::vector<const DzirFunction *>> structFuncs;

    for ( const DzirFunction *func : globalFunctions )
    {
        if ( func->ParsedName.OwnerType.empty( ) )
        {
            trulyGlobalFuncs.push_back( func );
            continue;
        }

        std::string shortName = NativeConvention::StripPrefix( func->ParsedName.OwnerType );

        if ( reservedShortNames.count( shortName ) != 0 )
        {
            if ( structShortNames.count( shortName ) != 0 )
            {
                structFuncs[ shortName ].push_back( func );
            }
            else if ( enumShortNames.count( shortName ) != 0 )
            {
                trulyGlobalFuncs.push_back( func );
            }
            continue;
        }

        typedFuncs[ func->ParsedName.OwnerType ].push_back( func );
    }

    EmitStructExtensionsFile( structFuncs );

    if ( !trulyGlobalFuncs.empty( ) )
    {
        EmitterStreamDesc desc;
        desc.FilePath    = m_outputDir / "DenOfIzGraphics.g.cs";
        desc.IndentWidth = 4;
        desc.Formatting  = &CSharpFormatting;

        EmitterStream stream( desc );

        stream.WriteLine( "// Auto-generated file - do not edit" );
        stream.WriteLine( "using System;" );
        stream.WriteLine( "using System.Numerics;" );
        stream.WriteLine( "using {}.Native;", m_config.Namespace );
        stream.WriteLine( );

        stream.WriteLine( "namespace {}", m_config.Namespace );
        stream.OpenBlock( );

        stream.WriteLine( "public static partial class DenOfIzGraphics" );
        stream.OpenBlock( );

        for ( const DzirFunction *func : trulyGlobalFuncs )
        {
            EmitStaticMethod( stream, func );
        }

        stream.CloseBlock( );
        stream.CloseBlock( );
    }

    for ( const auto &[ ownerType, funcs ] : typedFuncs )
    {
        std::string shortName = NativeConvention::StripPrefix( ownerType );

        if ( reservedShortNames.count( shortName ) != 0 )
        {
            continue;
        }

        EmitterStreamDesc desc;
        desc.FilePath    = m_outputDir / ( shortName + ".g.cs" );
        desc.IndentWidth = 4;
        desc.Formatting  = &CSharpFormatting;

        EmitterStream stream( desc );

        stream.WriteLine( "// Auto-generated file - do not edit" );
        stream.WriteLine( "using System;" );
        stream.WriteLine( "using System.Numerics;" );
        stream.WriteLine( "using {}.Native;", m_config.Namespace );
        stream.WriteLine( );

        stream.WriteLine( "namespace {}", m_config.Namespace );
        stream.OpenBlock( );

        stream.WriteLine( "public static partial class {}", shortName );
        stream.OpenBlock( );

        for ( const DzirFunction *func : funcs )
        {
            EmitStaticMethod( stream, func );
        }

        stream.CloseBlock( );
        stream.CloseBlock( );
    }
}

void CSharpEmitter::EmitWrapperFile( const DzirOwnerType *ownerType )
{
    const auto &enumMap   = m_library->GetEnumMap( );
    const auto &structMap = m_library->GetStructMap( );

    for ( const auto &[ name, enumType ] : enumMap )
    {
        if ( enumType->ShortName == ownerType->ShortName )
        {
            return;
        }
    }

    for ( const auto &[ name, structType ] : structMap )
    {
        if ( structType->ShortName == ownerType->ShortName )
        {
            return;
        }
    }

    EmitterStreamDesc desc;
    desc.FilePath    = m_outputDir / ( ownerType->ShortName + ".g.cs" );
    desc.IndentWidth = 4;
    desc.Formatting  = &CSharpFormatting;

    EmitterStream stream( desc );

    stream.WriteLine( "// Auto-generated file - do not edit" );
    stream.WriteLine( "using System;" );
    stream.WriteLine( "using System.Numerics;" );
    stream.WriteLine( "using System.Runtime.InteropServices;" );
    stream.WriteLine( "using {}.Native;", m_config.Namespace );
    stream.WriteLine( );

    stream.WriteLine( "namespace {}", m_config.Namespace );
    stream.OpenBlock( );

    EmitWrapperClass( stream, ownerType );

    stream.CloseBlock( );
}

void CSharpEmitter::EmitOrphanHandleWrappers( )
{
    const auto &handleTypeNames = m_library->GetHandleTypeNames( );
    const auto &ownerTypes      = m_library->GetOwnerTypes( );
    const auto &globalFunctions = m_library->GetGlobalFunctions( );
    const auto &enumMap         = m_library->GetEnumMap( );
    const auto &structMap       = m_library->GetStructMap( );

    std::unordered_set<std::string> ownerTypeNames;
    for ( const auto &[ name, ownerType ] : ownerTypes )
    {
        ownerTypeNames.insert( name );
    }

    std::unordered_set<std::string> enumShortNames;
    std::unordered_set<std::string> structShortNames;

    for ( const auto &[ name, enumType ] : enumMap )
    {
        enumShortNames.insert( enumType->ShortName );
    }

    for ( const auto &[ name, structType ] : structMap )
    {
        structShortNames.insert( structType->ShortName );
    }

    for ( const std::string &handleName : handleTypeNames )
    {
        if ( ownerTypeNames.count( handleName ) )
        {
            continue;
        }

        std::string shortName = NativeConvention::StripPrefix( handleName );

        if ( enumShortNames.count( shortName ) != 0 || structShortNames.count( shortName ) != 0 )
        {
            continue;
        }

        EmitterStreamDesc desc;
        desc.FilePath    = m_outputDir / ( shortName + ".g.cs" );
        desc.IndentWidth = 4;
        desc.Formatting  = &CSharpFormatting;

        EmitterStream stream( desc );

        stream.WriteLine( "// Auto-generated file - do not edit" );
        stream.WriteLine( "using System;" );
        stream.WriteLine( "using System.Numerics;" );
        stream.WriteLine( "using {}.Native;", m_config.Namespace );
        stream.WriteLine( );

        stream.WriteLine( "namespace {}", m_config.Namespace );
        stream.OpenBlock( );

        const DzirFunction               *createFunc  = nullptr;
        const DzirFunction               *destroyFunc = nullptr;
        std::vector<const DzirFunction *> otherFuncs;

        for ( const DzirFunction *func : globalFunctions )
        {
            if ( func->ParsedName.OwnerType == shortName )
            {
                if ( func->ParsedName.MethodName == "Create" )
                {
                    createFunc = func;
                }
                else if ( func->ParsedName.MethodName == "Destroy" )
                {
                    destroyFunc = func;
                }
                else
                {
                    otherFuncs.push_back( func );
                }
            }
        }

        bool hasDispose = destroyFunc != nullptr;
        if ( hasDispose )
        {
            stream.WriteLine( "public sealed partial class {} : IDisposable", shortName );
        }
        else
        {
            stream.WriteLine( "public sealed partial class {}", shortName );
        }
        stream.OpenBlock( );

        stream.WriteLine( "internal ulong Handle { get; private set; }" );
        if ( hasDispose )
        {
            stream.WriteLine( "private readonly bool _ownsHandle;" );
        }
        stream.WriteLine( );

        if ( createFunc )
        {
            std::string paramList = GetWrapperParameterList( createFunc, false );
            std::string callArgs  = GetNativeCallArguments( createFunc, false );
            stream.WriteLine( "public {}({})", shortName, paramList );
            stream.OpenBlock( );
            stream.WriteLine( "Handle = Methods.{}({});", createFunc->ParsedName.FullName, callArgs );
            if ( hasDispose )
            {
                stream.WriteLine( "_ownsHandle = true;" );
            }
            stream.CloseBlock( );
            stream.WriteLine( );
        }

        stream.WriteLine( "internal {}(ulong handle, bool ownsHandle = false)", shortName );
        stream.OpenBlock( );
        stream.WriteLine( "Handle = handle;" );
        if ( hasDispose )
        {
            stream.WriteLine( "_ownsHandle = ownsHandle;" );
        }
        stream.CloseBlock( );
        stream.WriteLine( );

        stream.WriteLine( "public static implicit operator ulong({} wrapper) => wrapper?.Handle ?? 0;", shortName );
        stream.WriteLine( );

        for ( const DzirFunction *func : otherFuncs )
        {
            EmitStaticMethod( stream, func );
        }

        if ( hasDispose )
        {
            stream.WriteLine( "public void Dispose()" );
            stream.OpenBlock( );
            stream.WriteLine( "if (Handle == 0)" );
            stream.OpenBlock( );
            stream.WriteLine( "return;" );
            stream.CloseBlock( );
            stream.WriteLine( );
            stream.WriteLine( "if (_ownsHandle)" );
            stream.OpenBlock( );
            stream.WriteLine( "Methods.{}(Handle);", destroyFunc->ParsedName.FullName );
            stream.CloseBlock( );
            stream.WriteLine( );
            stream.WriteLine( "Handle = 0;" );
            stream.CloseBlock( );
            stream.WriteLine( );
            stream.WriteLine( "private void ThrowIfDisposed()" );
            stream.OpenBlock( );
            stream.WriteLine( "if (Handle == 0)" );
            stream.OpenBlock( );
            stream.WriteLine( "throw new ObjectDisposedException(nameof({}));", shortName );
            stream.CloseBlock( );
            stream.CloseBlock( );
        }

        stream.CloseBlock( );
        stream.CloseBlock( );
    }
}

void CSharpEmitter::EmitEnum( EmitterStream &stream, const DzirEnum *enumType )
{
    bool needsUlong = false;
    for ( const DzirEnumValue *value : enumType->Values )
    {
        if ( value->Value > 0xFFFFFFFF )
        {
            needsUlong = true;
            break;
        }
    }

    const std::string &shortName  = enumType->ShortName;
    bool               isFlagBits = shortName.length( ) > 8 && shortName.substr( shortName.length( ) - 8 ) == "FlagBits";

    if ( isFlagBits )
    {
        stream.WriteLine( "[Flags]" );
    }
    stream.WriteLine( "public enum {} : {}", enumType->ShortName, needsUlong ? "ulong" : "uint" );
    stream.OpenBlock( );

    std::string commonPrefix;
    if ( !enumType->Values.empty( ) )
    {
        commonPrefix = enumType->Values[ 0 ]->Name;
        for ( const DzirEnumValue *val : enumType->Values )
        {
            while ( !commonPrefix.empty( ) && val->Name.find( commonPrefix ) != 0 )
            {
                commonPrefix.pop_back( );
            }
        }
        size_t lastUnderscore = commonPrefix.rfind( '_' );
        if ( lastUnderscore != std::string::npos )
        {
            commonPrefix = commonPrefix.substr( 0, lastUnderscore + 1 );
        }
        else
        {
            commonPrefix.clear( );
        }
    }

    for ( size_t i = 0; i < enumType->Values.size( ); ++i )
    {
        const DzirEnumValue *value     = enumType->Values[ i ];
        std::string          valueName = value->Name;

        if ( !commonPrefix.empty( ) && valueName.find( commonPrefix ) == 0 )
        {
            valueName = valueName.substr( commonPrefix.size( ) );
        }

        if ( isFlagBits )
        {
            if ( valueName.length( ) > 4 && valueName.substr( valueName.length( ) - 4 ) == "_BIT" )
            {
                valueName = valueName.substr( 0, valueName.length( ) - 4 );
            }
        }

        std::string pascalName;
        bool        nextUpper = true;
        char        prevChar  = 0;
        for ( char c : valueName )
        {
            if ( c == '_' )
            {
                nextUpper = true;
            }
            else if ( nextUpper )
            {
                pascalName += static_cast<char>( std::toupper( static_cast<unsigned char>( c ) ) );
                nextUpper = false;
            }
            else if ( std::isdigit( static_cast<unsigned char>( prevChar ) ) && std::isupper( static_cast<unsigned char>( c ) ) )
            {
                pascalName += c;
            }
            else
            {
                pascalName += static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
            }
            prevChar = c;
        }

        if ( !pascalName.empty( ) && std::isdigit( static_cast<unsigned char>( pascalName[ 0 ] ) ) )
        {
            pascalName = "_" + pascalName;
        }

        if ( i < enumType->Values.size( ) - 1 )
        {
            stream.WriteLine( "{} = {},", pascalName, value->Value );
        }
        else
        {
            stream.WriteLine( "{} = {}", pascalName, value->Value );
        }
    }

    stream.CloseBlock( );
}

void CSharpEmitter::EmitStruct( EmitterStream &stream, const DzirStruct *structType )
{
    stream.WriteLine( "[StructLayout(LayoutKind.Sequential)]" );
    stream.WriteLine( "public partial struct {}", structType->ShortName );
    stream.OpenBlock( );

    for ( const DzirField *field : structType->Fields )
    {
        std::string marshalAttr = GetMarshalAttribute( field->Type );
        if ( !marshalAttr.empty( ) )
        {
            stream.WriteLine( "{}", marshalAttr );
        }

        std::string csType = FieldTypeToCSharp( field->Type, true );
        stream.WriteLine( "public {} {};", csType, field->Name );
    }

    stream.CloseBlock( );
}

void CSharpEmitter::EmitNativeFunction( EmitterStream &stream, const DzirFunction *function )
{
    stream.WriteLine( "[DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]" );

    const std::string returnMarshalAttr = GetMarshalAttribute( function->ReturnType );
    if ( !returnMarshalAttr.empty( ) )
    {
        stream.WriteLine( "[return: {}", returnMarshalAttr.substr( 1 ) );
    }

    std::string returnType = GetNativeReturnType( function );
    std::string paramList  = GetNativeParameterList( function );

    stream.WriteLine( "public static extern {} {}({});", returnType, function->ParsedName.FullName, paramList );
    stream.WriteLine( );
}

void CSharpEmitter::EmitWrapperClass( EmitterStream &stream, const DzirOwnerType *ownerType )
{
    stream.WriteLine( "public sealed partial class {} : IDisposable", ownerType->ShortName );
    stream.OpenBlock( );

    stream.WriteLine( "internal ulong Handle { get; private set; }" );
    stream.WriteLine( "private readonly bool _ownsHandle;" );
    stream.WriteLine( );
    stream.WriteLine( "public static implicit operator ulong({} wrapper) => wrapper?.Handle ?? 0;", ownerType->ShortName );
    stream.WriteLine( );

    if ( ownerType->Constructor )
    {
        EmitWrapperConstructor( stream, ownerType );
        stream.WriteLine( );
    }

    stream.WriteLine( "internal {}(ulong handle, bool ownsHandle = false)", ownerType->ShortName );
    stream.OpenBlock( );
    stream.WriteLine( "Handle = handle;" );
    stream.WriteLine( "_ownsHandle = ownsHandle;" );
    stream.CloseBlock( );
    stream.WriteLine( );

    for ( const DzirFunction *method : ownerType->Methods )
    {
        EmitWrapperMethod( stream, method );
        stream.WriteLine( );
    }

    for ( const DzirFunction *factory : ownerType->FactoryMethods )
    {
        EmitWrapperFactoryMethod( stream, factory );
        stream.WriteLine( );
    }

    const DzirFunction *createFunc = nullptr;
    for ( const DzirFunction *func : m_library->GetGlobalFunctions( ) )
    {
        if ( func->ParsedName.OwnerType == ownerType->ShortName )
        {
            if ( func->ParsedName.MethodName == "Create" )
            {
                createFunc = func;
            }
            else
            {
                EmitStaticMethod( stream, func );
            }
        }
    }

    if ( createFunc && !ownerType->Constructor )
    {
        std::string paramList = GetWrapperParameterList( createFunc, false );
        std::string callArgs  = GetNativeCallArguments( createFunc, false );

        const DzirParameter *outputParam = nullptr;
        for ( const DzirParameter *param : createFunc->Parameters )
        {
            if ( param->IsOutput )
            {
                outputParam = param;
                break;
            }
        }

        stream.WriteLine( "public {}({})", ownerType->ShortName, paramList );
        stream.OpenBlock( );
        if ( outputParam )
        {
            stream.WriteLine( "ulong {} = 0;", outputParam->Name );
            stream.WriteLine( "Methods.{}({});", createFunc->ParsedName.FullName, callArgs );
            stream.WriteLine( "Handle = {};", outputParam->Name );
        }
        else
        {
            stream.WriteLine( "Handle = Methods.{}({});", createFunc->ParsedName.FullName, callArgs );
        }
        stream.WriteLine( "_ownsHandle = true;" );
        stream.CloseBlock( );
        stream.WriteLine( );
    }

    EmitWrapperDestructor( stream, ownerType );

    stream.CloseBlock( );
}

void CSharpEmitter::EmitWrapperConstructor( EmitterStream &stream, const DzirOwnerType *ownerType )
{
    const DzirFunction *ctor = ownerType->Constructor;

    std::string paramList = GetWrapperParameterList( ctor, false );

    stream.WriteLine( "public {}({})", ownerType->ShortName, paramList );
    stream.OpenBlock( );

    std::string callArgs = GetNativeCallArguments( ctor, false );
    stream.WriteLine( "Handle = Methods.{}({});", ctor->ParsedName.FullName, callArgs );
    stream.WriteLine( "_ownsHandle = true;" );

    stream.CloseBlock( );
}

void CSharpEmitter::EmitWrapperDestructor( EmitterStream &stream, const DzirOwnerType *ownerType )
{
    stream.WriteLine( "public void Dispose()" );
    stream.OpenBlock( );
    stream.WriteLine( "if (Handle == 0)" );
    stream.OpenBlock( );
    stream.WriteLine( "return;" );
    stream.CloseBlock( );
    stream.WriteLine( );

    if ( ownerType->Destructor )
    {
        stream.WriteLine( "if (_ownsHandle)" );
        stream.OpenBlock( );
        stream.WriteLine( "Methods.{}(Handle);", ownerType->Destructor->ParsedName.FullName );
        stream.CloseBlock( );
        stream.WriteLine( );
    }

    stream.WriteLine( "Handle = 0;" );
    stream.CloseBlock( );
    stream.WriteLine( );

    stream.WriteLine( "private void ThrowIfDisposed()" );
    stream.OpenBlock( );
    stream.WriteLine( "if (Handle == 0)" );
    stream.OpenBlock( );
    stream.WriteLine( "throw new ObjectDisposedException(nameof({}));", ownerType->ShortName );
    stream.CloseBlock( );
    stream.CloseBlock( );
}

static const std::unordered_set<std::string> ReservedMethodNames = { "GetType", "Equals", "ToString", "GetHashCode", "Finalize", "MemberwiseClone" };

void CSharpEmitter::EmitWrapperMethod( EmitterStream &stream, const DzirFunction *function )
{
    std::string returnType = GetWrapperReturnType( function );
    std::string methodName = ToPascalCase( function->ParsedName.MethodName );

    if ( ReservedMethodNames.count( methodName ) )
    {
        methodName = "Get" + NativeConvention::StripPrefix( function->ParsedName.OwnerType ) + methodName.substr( 3 );
    }

    const DzirParameter               *outputParam = nullptr;
    std::vector<const DzirParameter *> bufferParams;

    for ( const DzirParameter *param : function->Parameters )
    {
        if ( param->IsOutput )
        {
            outputParam = param;
        }
        if ( param->IsBuffer )
        {
            bufferParams.push_back( param );
        }
    }

    bool hasReturn = !std::holds_alternative<PrimitiveKind>( function->ReturnType ) || std::get<PrimitiveKind>( function->ReturnType ) != PrimitiveKind::Void;

    bool returnsResult = false;
    if ( const auto *enumRef = std::get_if<DzirEnumRef>( &function->ReturnType ) )
    {
        std::string shortName = NativeConvention::StripPrefix( enumRef->Name );
        returnsResult         = ( shortName == "Result" );
    }

    bool returnOutputThrowOnError = returnsResult && outputParam != nullptr;

    std::string paramList = GetWrapperParameterList( function, true, hasReturn && outputParam != nullptr && !returnOutputThrowOnError );

    if ( returnOutputThrowOnError || ( !hasReturn && outputParam ) )
    {
        std::string outType = "ulong";
        if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &outputParam->Type ) )
        {
            if ( *ptr && ( *ptr )->Pointee )
            {
                if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                {
                    if ( *prim == PrimitiveKind::Void )
                    {
                        outType = "IntPtr";
                    }
                    else
                    {
                        outType = FieldTypeToCSharp( *( *ptr )->Pointee, false );
                    }
                }
                else
                {
                    outType = FieldTypeToCSharp( *( *ptr )->Pointee, false );
                }
            }
        }
        returnType = outType;
    }

    stream.WriteLine( "public {} {}({})", returnType, methodName, paramList );
    stream.OpenBlock( );
    stream.WriteLine( "ThrowIfDisposed();" );

    std::string outputVarType;
    if ( outputParam )
    {
        outputVarType = "ulong";
        if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &outputParam->Type ) )
        {
            if ( *ptr && ( *ptr )->Pointee )
            {
                if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                {
                    if ( *prim == PrimitiveKind::Void )
                    {
                        outputVarType = "IntPtr";
                    }
                    else
                    {
                        outputVarType = FieldTypeToCSharp( *( *ptr )->Pointee, true );
                    }
                }
                else
                {
                    outputVarType = FieldTypeToCSharp( *( *ptr )->Pointee, true );
                }
            }
        }
        if ( !hasReturn || returnOutputThrowOnError )
        {
            stream.WriteLine( "{} {} = default;", outputVarType, outputParam->Name );
        }
    }

    if ( !bufferParams.empty( ) )
    {
        for ( const DzirParameter *bufParam : bufferParams )
        {
            stream.WriteLine( "var __pin_{} = GCHandle.Alloc({}, GCHandleType.Pinned);", bufParam->Name, ToCamelCase( bufParam->Name ) );
        }
        stream.WriteLine( "try" );
        stream.OpenBlock( );
    }

    std::string callArgs = GetNativeCallArguments( function, true, !bufferParams.empty( ) );

    if ( returnOutputThrowOnError )
    {
        stream.WriteLine( "var __result = Methods.{}({});", function->ParsedName.FullName, callArgs );
        stream.WriteLine( "if (__result != Result.Success) throw new DenOfIzException(__result);" );
        bool outputIsHandle = IsHandleTypeName( returnType );
        bool outputIsStruct = IsStructTypeName( returnType );
        if ( outputIsHandle )
        {
            stream.WriteLine( "return new {}({}, ownsHandle: false);", returnType, outputParam->Name );
        }
        else if ( outputIsStruct && outputVarType == "IntPtr" )
        {
            stream.WriteLine( "return Marshal.PtrToStructure<{}>({});", returnType, outputParam->Name );
        }
        else
        {
            stream.WriteLine( "return {};", outputParam->Name );
        }
    }
    else if ( hasReturn )
    {
        bool        returnsHandle    = IsHandleTypeName( returnType );
        bool        returnsStruct    = IsStructTypeName( returnType );
        std::string nativeReturnType = GetNativeReturnType( function );
        if ( returnsHandle )
        {
            stream.WriteLine( "return new {}(Methods.{}({}), ownsHandle: false);", returnType, function->ParsedName.FullName, callArgs );
        }
        else if ( returnsStruct && nativeReturnType == "IntPtr" )
        {
            stream.WriteLine( "return Marshal.PtrToStructure<{}>(Methods.{}({}));", returnType, function->ParsedName.FullName, callArgs );
        }
        else
        {
            stream.WriteLine( "return Methods.{}({});", function->ParsedName.FullName, callArgs );
        }
    }
    else if ( outputParam )
    {
        stream.WriteLine( "Methods.{}({});", function->ParsedName.FullName, callArgs );
        bool outputIsHandle = IsHandleTypeName( returnType );
        bool outputIsStruct = IsStructTypeName( returnType );
        if ( outputIsHandle )
        {
            stream.WriteLine( "return new {}({}, ownsHandle: false);", returnType, outputParam->Name );
        }
        else if ( outputIsStruct && outputVarType == "IntPtr" )
        {
            stream.WriteLine( "return Marshal.PtrToStructure<{}>({});", returnType, outputParam->Name );
        }
        else
        {
            stream.WriteLine( "return {};", outputParam->Name );
        }
    }
    else
    {
        stream.WriteLine( "Methods.{}({});", function->ParsedName.FullName, callArgs );
    }

    if ( !bufferParams.empty( ) )
    {
        stream.CloseBlock( );
        stream.WriteLine( "finally" );
        stream.OpenBlock( );
        for ( const DzirParameter *bufParam : bufferParams )
        {
            stream.WriteLine( "__pin_{}.Free();", bufParam->Name );
        }
        stream.CloseBlock( );
    }

    stream.CloseBlock( );
}

void CSharpEmitter::EmitWrapperFactoryMethod( EmitterStream &stream, const DzirFunction *function )
{
    std::string factoryReturnType = NativeConvention::StripPrefix( function->FactoryReturnType );
    std::string methodName        = ToPascalCase( function->ParsedName.MethodName );
    std::string paramList         = GetWrapperParameterList( function, true );

    stream.WriteLine( "public {} {}({})", factoryReturnType, methodName, paramList );
    stream.OpenBlock( );
    stream.WriteLine( "ThrowIfDisposed();" );

    for ( const DzirParameter *param : function->Parameters )
    {
        if ( param->IsOutput )
        {
            stream.WriteLine( "ulong {} = 0;", param->Name );
        }
    }

    std::string callArgs = GetNativeCallArguments( function, true );
    stream.WriteLine( "Methods.{}({});", function->ParsedName.FullName, callArgs );

    for ( const DzirParameter *param : function->Parameters )
    {
        if ( param->IsOutput )
        {
            stream.WriteLine( "return new {}({}, ownsHandle: true);", factoryReturnType, param->Name );
            break;
        }
    }

    stream.CloseBlock( );
}

void CSharpEmitter::EmitStaticMethod( EmitterStream &stream, const DzirFunction *function )
{
    std::string returnType = GetWrapperReturnType( function );
    std::string methodName = ToPascalCase( function->ParsedName.MethodName );
    if ( methodName.empty( ) )
    {
        methodName = ToPascalCase( function->ParsedName.FullName );
        if ( methodName.rfind( "DenOfIz_", 0 ) == 0 )
        {
            methodName = methodName.substr( 8 );
        }
    }

    const DzirParameter *outputParam = nullptr;
    for ( const DzirParameter *param : function->Parameters )
    {
        if ( param->IsOutput )
        {
            outputParam = param;
            break;
        }
    }

    bool hasReturn = !std::holds_alternative<PrimitiveKind>( function->ReturnType ) || std::get<PrimitiveKind>( function->ReturnType ) != PrimitiveKind::Void;

    if ( !hasReturn && outputParam )
    {
        std::string outType = "ulong";
        if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &outputParam->Type ) )
        {
            if ( *ptr && ( *ptr )->Pointee )
            {
                if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                {
                    if ( *prim == PrimitiveKind::Void )
                    {
                        outType = "IntPtr";
                    }
                    else
                    {
                        outType = FieldTypeToCSharp( *( *ptr )->Pointee, false );
                    }
                }
                else
                {
                    outType = FieldTypeToCSharp( *( *ptr )->Pointee, false );
                }
            }
        }
        returnType = outType;
    }

    std::string paramList = GetWrapperParameterList( function, false, hasReturn && outputParam != nullptr );
    std::string callArgs  = GetNativeCallArguments( function, false );

    stream.WriteLine( "public static {} {}({})", returnType, methodName, paramList );
    stream.OpenBlock( );

    std::string outputVarType;
    if ( outputParam )
    {
        outputVarType = "ulong";
        if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &outputParam->Type ) )
        {
            if ( *ptr && ( *ptr )->Pointee )
            {
                if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                {
                    if ( *prim == PrimitiveKind::Void )
                    {
                        outputVarType = "IntPtr";
                    }
                    else
                    {
                        outputVarType = FieldTypeToCSharp( *( *ptr )->Pointee, true );
                    }
                }
                else
                {
                    outputVarType = FieldTypeToCSharp( *( *ptr )->Pointee, true );
                }
            }
        }
        if ( !hasReturn )
        {
            stream.WriteLine( "{} {} = default;", outputVarType, outputParam->Name );
        }
    }

    if ( hasReturn )
    {
        bool        returnsHandle    = IsHandleTypeName( returnType );
        bool        returnsStruct    = IsStructTypeName( returnType );
        std::string nativeReturnType = GetNativeReturnType( function );
        if ( returnsHandle )
        {
            stream.WriteLine( "return new {}(Methods.{}({}), ownsHandle: false);", returnType, function->ParsedName.FullName, callArgs );
        }
        else if ( returnsStruct && nativeReturnType == "IntPtr" )
        {
            stream.WriteLine( "return Marshal.PtrToStructure<{}>(Methods.{}({}));", returnType, function->ParsedName.FullName, callArgs );
        }
        else
        {
            stream.WriteLine( "return Methods.{}({});", function->ParsedName.FullName, callArgs );
        }
    }
    else if ( outputParam )
    {
        stream.WriteLine( "Methods.{}({});", function->ParsedName.FullName, callArgs );
        bool outputIsHandle = IsHandleTypeName( returnType );
        bool outputIsStruct = IsStructTypeName( returnType );
        if ( outputIsHandle )
        {
            stream.WriteLine( "return new {}({}, ownsHandle: false);", returnType, outputParam->Name );
        }
        else if ( outputIsStruct && outputVarType == "IntPtr" )
        {
            stream.WriteLine( "return Marshal.PtrToStructure<{}>({});", returnType, outputParam->Name );
        }
        else
        {
            stream.WriteLine( "return {};", outputParam->Name );
        }
    }
    else
    {
        stream.WriteLine( "Methods.{}({});", function->ParsedName.FullName, callArgs );
    }

    stream.CloseBlock( );
    stream.WriteLine( );
}

std::string CSharpEmitter::GetNativeReturnType( const DzirFunction *function ) const
{
    return FieldTypeToCSharp( function->ReturnType, true );
}

std::string CSharpEmitter::GetNativeParameterList( const DzirFunction *function ) const
{
    std::string result;
    bool        first = true;

    for ( const DzirParameter *param : function->Parameters )
    {
        if ( !first )
        {
            result += ", ";
        }
        first = false;

        std::string marshalAttr = GetMarshalAttribute( param->Type );
        if ( !marshalAttr.empty( ) )
        {
            result += marshalAttr + " ";
        }

        std::string csType = FieldTypeToCSharp( param->Type, true );

        if ( param->IsOutput )
        {
            if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &param->Type ) )
            {
                if ( *ptr && ( *ptr )->Pointee )
                {
                    // Check if pointee is void - void* should become IntPtr
                    if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                    {
                        if ( *prim == PrimitiveKind::Void )
                        {
                            csType = "out IntPtr";
                        }
                        else
                        {
                            csType = "out " + FieldTypeToCSharp( *( *ptr )->Pointee, true );
                        }
                    }
                    else
                    {
                        csType = "out " + FieldTypeToCSharp( *( *ptr )->Pointee, true );
                    }
                }
            }
        }
        else if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &param->Type ) )
        {
            if ( *ptr && ( *ptr )->Pointee )
            {
                if ( const auto *structRef = std::get_if<DzirStructRef>( ( *ptr )->Pointee.get( ) ) )
                {
                    bool isConst = ( ( *ptr )->Attributes & POINTER_ATTRIBUTE_CONST ) != 0;
                    csType       = ( isConst ? "in " : "ref " ) + GetNumericsType( NativeConvention::StripPrefix( structRef->Name ) );
                }
                else if ( const auto *handleRef = std::get_if<DzirHandleRef>( ( *ptr )->Pointee.get( ) ) )
                {
                    bool isConst = ( ( *ptr )->Attributes & POINTER_ATTRIBUTE_CONST ) != 0;
                    csType       = ( isConst ? "in ulong" : "ref ulong" );
                }
                else if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                {
                    if ( *prim != PrimitiveKind::Void )
                    {
                        csType = "ref " + NativeConvention::PrimitiveTypeToString( *prim );
                    }
                }
            }
        }

        result += csType + " " + param->Name;
    }

    return result;
}

std::string CSharpEmitter::GetWrapperReturnType( const DzirFunction *function ) const
{
    return FieldTypeToCSharp( function->ReturnType, false );
}

std::string CSharpEmitter::GetWrapperParameterList( const DzirFunction *function, bool skipSelf, bool includeOutputAsOut ) const
{
    std::string result;
    bool        first        = true;
    bool        skippedFirst = false;

    for ( const DzirParameter *param : function->Parameters )
    {
        if ( skipSelf && !skippedFirst )
        {
            if ( const auto *handleRef = std::get_if<DzirHandleRef>( &param->Type ) )
            {
                skippedFirst = true;
                continue;
            }
        }

        if ( param->IsOutput )
        {
            if ( !includeOutputAsOut )
            {
                continue;
            }

            if ( !first )
            {
                result += ", ";
            }
            first = false;

            std::string outType = "ulong";
            if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &param->Type ) )
            {
                if ( *ptr && ( *ptr )->Pointee )
                {
                    outType = FieldTypeToCSharp( *( *ptr )->Pointee, false );
                }
            }
            result += "out " + outType + " " + ToCamelCase( param->Name );
            continue;
        }

        if ( !first )
        {
            result += ", ";
        }
        first = false;

        std::string csType = FieldTypeToCSharp( param->Type, false );

        if ( const auto *handleRef = std::get_if<DzirHandleRef>( &param->Type ) )
        {
            csType += "?";
        }

        if ( param->IsBuffer )
        {
            csType = "byte[]";
        }
        else if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &param->Type ) )
        {
            if ( *ptr && ( *ptr )->Pointee )
            {
                if ( const auto *structRef = std::get_if<DzirStructRef>( ( *ptr )->Pointee.get( ) ) )
                {
                    bool isConst = ( ( *ptr )->Attributes & POINTER_ATTRIBUTE_CONST ) != 0;
                    csType       = ( isConst ? "in " : "ref " ) + GetNumericsType( NativeConvention::StripPrefix( structRef->Name ) );
                }
                else if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                {
                    if ( *prim != PrimitiveKind::Void )
                    {
                        csType = "ref " + NativeConvention::PrimitiveTypeToString( *prim );
                    }
                }
            }
        }

        result += csType + " " + ToCamelCase( param->Name );
    }

    return result;
}

std::string CSharpEmitter::GetNativeCallArguments( const DzirFunction *function, bool includeSelf, bool usePinnedBuffers ) const
{
    std::string result;
    bool        first     = true;
    bool        addedSelf = false;

    for ( const DzirParameter *param : function->Parameters )
    {
        if ( !first )
        {
            result += ", ";
        }
        first = false;

        if ( includeSelf && !addedSelf )
        {
            if ( const auto *handleRef = std::get_if<DzirHandleRef>( &param->Type ) )
            {
                result += "Handle";
                addedSelf = true;
                continue;
            }
        }

        if ( param->IsOutput )
        {
            result += "out " + param->Name;
        }
        else if ( param->IsBuffer && usePinnedBuffers )
        {
            result += "__pin_" + param->Name + ".AddrOfPinnedObject()";
        }
        else if ( const auto *handleRef = std::get_if<DzirHandleRef>( &param->Type ) )
        {
            result += ToCamelCase( param->Name );
        }
        else if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &param->Type ) )
        {
            if ( *ptr && ( *ptr )->Pointee )
            {
                if ( std::get_if<DzirStructRef>( ( *ptr )->Pointee.get( ) ) )
                {
                    bool isConst = ( ( *ptr )->Attributes & POINTER_ATTRIBUTE_CONST ) != 0;
                    result += ( isConst ? "in " : "ref " ) + ToCamelCase( param->Name );
                    continue;
                }
                else if ( std::get_if<DzirHandleRef>( ( *ptr )->Pointee.get( ) ) )
                {
                    bool isConst = ( ( *ptr )->Attributes & POINTER_ATTRIBUTE_CONST ) != 0;
                    if ( isConst )
                    {
                        result += "in " + ToCamelCase( param->Name ) + ".Handle";
                    }
                    else
                    {
                        result += ToCamelCase( param->Name );
                    }
                    continue;
                }
                else if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                {
                    if ( *prim != PrimitiveKind::Void )
                    {
                        result += "ref " + ToCamelCase( param->Name );
                        continue;
                    }
                }
            }
            result += ToCamelCase( param->Name );
        }
        else
        {
            result += ToCamelCase( param->Name );
        }
    }

    return result;
}

std::string CSharpEmitter::FieldTypeToCSharp( const DzirFieldType &type, bool forNative ) const
{
    std::string result = "void";

    DzirTypeVisitorDesc desc;

    desc.OnPrimitive = [ &result ]( const PrimitiveKind kind ) { result = NativeConvention::PrimitiveTypeToString( kind ); };

    desc.OnEnumRef = [ &result ]( const DzirEnumRef &ref ) { result = NativeConvention::StripPrefix( ref.Name ); };

    desc.OnStructRef = [ &result ]( const DzirStructRef &ref ) { result = GetNumericsType( NativeConvention::StripPrefix( ref.Name ) ); };

    desc.OnHandleRef = [ &result, forNative ]( const DzirHandleRef &ref )
    {
        if ( forNative )
        {
            result = "ulong";
        }
        else
        {
            result = NativeConvention::StripPrefix( ref.Name );
        }
    };

    desc.OnPointer = [ this, &result, forNative ]( const DzirPointer &ptr )
    {
        if ( ptr.Pointee )
        {
            if ( const auto *primitive = std::get_if<PrimitiveKind>( ptr.Pointee.get( ) ) )
            {
                if ( *primitive == PrimitiveKind::Void )
                {
                    result = "IntPtr";
                    return;
                }
            }

            if ( forNative )
            {
                result = "IntPtr";
            }
            else
            {
                result = FieldTypeToCSharp( *ptr.Pointee, false );
            }
        }
        else
        {
            result = "IntPtr";
        }
    };

    desc.OnArray = [ &result ]( const DzirArray &arr ) { result = "IntPtr"; };

    DzirTypeVisitor::Visit( type, desc );
    return result;
}

std::string CSharpEmitter::GetMarshalAttribute( const DzirFieldType &type ) const
{
    std::string result;

    DzirTypeVisitorDesc desc;

    desc.OnPrimitive = [ &result ]( const PrimitiveKind kind )
    {
        if ( kind == PrimitiveKind::Bool )
        {
            result = "[MarshalAs(UnmanagedType.I1)]";
        }
    };

    DzirTypeVisitor::Visit( type, desc );
    return result;
}

bool CSharpEmitter::IsHandleTypeName( const std::string &shortName ) const
{
    const auto &handleTypeNames = m_library->GetHandleTypeNames( );
    for ( const std::string &handleName : handleTypeNames )
    {
        if ( NativeConvention::StripPrefix( handleName ) == shortName )
        {
            return true;
        }
    }
    return false;
}

bool CSharpEmitter::IsStructTypeName( const std::string &shortName ) const
{
    const auto &structMap = m_library->GetStructMap( );
    for ( const auto &[ name, structType ] : structMap )
    {
        if ( structType->ShortName == shortName )
        {
            return true;
        }
    }
    return false;
}

std::string CSharpEmitter::ToPascalCase( const std::string &name )
{
    if ( name.empty( ) )
    {
        return name;
    }

    std::string result = name;
    result[ 0 ]        = static_cast<char>( std::toupper( static_cast<unsigned char>( result[ 0 ] ) ) );
    return result;
}

std::string CSharpEmitter::ToCamelCase( const std::string &name )
{
    if ( name.empty( ) )
    {
        return name;
    }

    std::string result = name;
    result[ 0 ]        = static_cast<char>( std::tolower( static_cast<unsigned char>( result[ 0 ] ) ) );
    return result;
}
