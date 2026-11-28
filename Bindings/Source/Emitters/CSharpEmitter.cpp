/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzBindings/Emitters/CSharpEmitter.h"
#include "DenOfIzBindings/Core/DzirTypeVisitor.h"
#include "DenOfIzBindings/Core/NativeConvention.h"

#include <fstream>
#include <unordered_map>

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
    EmitNativeFile( );
    EmitEnumsFile( );
    EmitStructsFile( );
    EmitHelpersFile( );
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

    stream.WriteLine( "private const string LibraryName = \"{}\";", m_config.LibraryName );
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
    stream.WriteLine( );

    stream.WriteLine( "private const int ScratchSlotCount = 32;" );
    stream.WriteLine( "[ThreadStatic] private static byte[][] _scratchBuffers;" );
    stream.WriteLine( "[ThreadStatic] private static GCHandle[] _scratchHandles;" );
    stream.WriteLine( "[ThreadStatic] private static int _scratchIndex;" );
    stream.WriteLine( );
    stream.WriteLine( "// Returns a StringView backed by a reusable pinned buffer. Valid until this thread makes ScratchSlotCount further Scratch calls; use only for strings consumed immediately by the native call." );
    stream.WriteLine( "public static StringView Scratch(string s)" );
    stream.OpenBlock( );
    stream.WriteLine( "if (string.IsNullOrEmpty(s)) return default;" );
    stream.WriteLine( "if (_scratchBuffers == null) { _scratchBuffers = new byte[ScratchSlotCount][]; _scratchHandles = new GCHandle[ScratchSlotCount]; }" );
    stream.WriteLine( "int slot = _scratchIndex;" );
    stream.WriteLine( "_scratchIndex = (slot + 1) % ScratchSlotCount;" );
    stream.WriteLine( "int maxBytes = Encoding.UTF8.GetMaxByteCount(s.Length);" );
    stream.WriteLine( "var buffer = _scratchBuffers[slot];" );
    stream.WriteLine( "if (buffer == null || buffer.Length < maxBytes)" );
    stream.OpenBlock( );
    stream.WriteLine( "if (_scratchHandles[slot].IsAllocated) _scratchHandles[slot].Free();" );
    stream.WriteLine( "buffer = new byte[Math.Max(maxBytes, 256)];" );
    stream.WriteLine( "_scratchBuffers[slot] = buffer;" );
    stream.WriteLine( "_scratchHandles[slot] = GCHandle.Alloc(buffer, GCHandleType.Pinned);" );
    stream.CloseBlock( );
    stream.WriteLine( "int numBytes = Encoding.UTF8.GetBytes(s, 0, s.Length, buffer, 0);" );
    stream.WriteLine( "return new StringView { Chars = _scratchHandles[slot].AddrOfPinnedObject(), NumChars = (ulong)numBytes };" );
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
            bool        isPointerArray  = false;
            std::string numElementsType = "ulong";
            for ( const DzirField *field : structType->Fields )
            {
                if ( field->Name == "Elements" )
                {
                    hasElements = true;
                    if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &field->Type ) )
                    {
                        isPointerArray = *ptr && ( *ptr )->Pointee && std::holds_alternative<std::unique_ptr<DzirPointer>>( *( *ptr )->Pointee );
                    }
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
            else if ( isPointerArray )
            {
                stream.WriteLine( "public readonly {}[] ToArray()", csElementType );
                stream.OpenBlock( );
                stream.WriteLine( "if (Elements == IntPtr.Zero || NumElements == 0) return Array.Empty<{}>();", csElementType );
                stream.WriteLine( "var result = new {}[NumElements];", csElementType );
                stream.WriteLine( "unsafe" );
                stream.OpenBlock( );
                stream.WriteLine( "var ptr = (IntPtr*)Elements.ToPointer();" );
                stream.WriteLine( "for (int i = 0; i < (int)NumElements; i++)" );
                stream.OpenBlock( );
                stream.WriteLine( "result[i] = Marshal.PtrToStructure<{}>(ptr[i]);", csElementType );
                stream.CloseBlock( );
                stream.CloseBlock( );
                stream.WriteLine( "return result;" );
                stream.CloseBlock( );
                stream.WriteLine( );

                stream.WriteLine( "public readonly unsafe ref {} this[int index] => ref *(({}**)Elements.ToPointer())[index];", csElementType, csElementType );
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
