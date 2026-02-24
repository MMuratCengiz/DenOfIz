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

#include "DenOfIzBindings/Core/DZIndex.h"

#include <algorithm>
#include <clang-c/Index.h>
#include <filesystem>
#include <future>
#include <ranges>
#include <vector>

#include <spdlog/spdlog.h>

using namespace DenOfIz;

DZIndex::DZIndex( ProjectContext *projectContext, const char *entryHeader ) : m_projectContext( projectContext )
{
    m_index = clang_createIndex( 0, 0 );

    const std::string entryAbs = m_projectContext->ResolvePath( entryHeader );
    {
        std::lock_guard lg( m_mu );
        m_scheduled.insert( entryAbs );
    }
    CreateTranslationUnit( entryAbs.c_str( ) );
    WaitCompilation( );

    const std::string entryRel = m_projectContext->Relativize( entryAbs );
    AssignDepthOrdersFrom( entryRel );

    m_translationUnitsVec.reserve( m_translationUnits.size( ) );
    for ( auto &tu : m_translationUnits | std::views::values )
    {
        m_translationUnitsVec.push_back( &tu );
    }
    std::sort( m_translationUnitsVec.begin( ), m_translationUnitsVec.end( ), []( const DZTranslationUnit *a, const DZTranslationUnit *b ) { return a->Order < b->Order; } );
}

const std::vector<DZTranslationUnit *> &DZIndex::GetTranslationUnits( ) const
{
    return m_translationUnitsVec;
}

const DZTranslationUnit *DZIndex::GetTranslationUnit( const std::string &relPath ) const
{
    return &m_translationUnits.find( relPath )->second;
}

void DZIndex::WaitCompilation( ) const
{
    for ( auto &future : m_translationUnitFutures )
    {
        future.wait( );
    }
}

DZIndex::~DZIndex( )
{
    for ( const auto &val : m_translationUnits | std::views::values )
    {
        if ( val.TranslationUnit )
        {
            clang_disposeTranslationUnit( val.TranslationUnit );
        }
    }
    clang_disposeIndex( m_index );
}

bool DZIndex::TrySchedule( const std::string &absPath )
{
    std::lock_guard lg( m_mu );
    if ( m_scheduled.contains( absPath ) )
    {
        return false;
    }
    m_scheduled.insert( absPath );
    return true;
}

CXChildVisitResult DZIndex::ProcessInclusionDirective( const CXCursor &cursor, const VisitCtx *ctx, DZIndex *self ) const
{
    const CXString incSpelling = clang_getCursorSpelling( cursor );
    std::string    incText     = clang_getCString( incSpelling ) ? clang_getCString( incSpelling ) : "";
    clang_disposeString( incSpelling );

    DZInclude inc{ };
    if ( const CXFile file = clang_getIncludedFile( cursor ) )
    {
        const CXString filePath = clang_getFileName( file );
        inc.FullPath            = clang_getCString( filePath ) ? clang_getCString( filePath ) : incText;
        clang_disposeString( filePath );
        inc.RelPath = m_projectContext->Relativize( inc.FullPath );

        const bool internal = self->IsInternalPath( inc.RelPath );
        inc.IsSystem        = !internal;

        const auto it = self->m_translationUnits.find( ctx->CurrentHeaderPath );
        if ( it != self->m_translationUnits.end( ) )
        {
            it->second.Includes.push_back( inc );
        }

        if ( internal )
        {
            const std::string abs = self->m_projectContext->ResolvePath( inc.RelPath.c_str( ) );
            if ( self->TrySchedule( abs ) )
            {
                self->m_translationUnitFutures.emplace_back( std::async( std::launch::async, [ self, abs ]( ) { self->CreateTranslationUnit( abs.c_str( ) ); } ) );
            }
        }
    }
    else
    {
        inc.IsSystem = true;
        inc.RelPath  = incText;
        inc.FullPath = "";

        const auto it = self->m_translationUnits.find( ctx->CurrentHeaderPath );
        if ( it != self->m_translationUnits.end( ) )
        {
            it->second.Includes.push_back( inc );
        }
    }
    return CXChildVisit_Continue;
}

CXChildVisitResult DZIndex::VisitTranslationUnit( CXCursor cursor, CXCursor parent, CXClientData clientData ) const
{
    const auto *ctx  = static_cast<VisitCtx *>( clientData );
    auto       *self = ctx->Self;

    const CXCursorKind kind = clang_getCursorKind( cursor );
    if ( kind == CXCursor_InclusionDirective )
    {
        return ProcessInclusionDirective( cursor, ctx, self );
    }
    return CXChildVisit_Continue;
}

void DZIndex::CreateTranslationUnit( const char *header )
{
    std::string       headerPath = m_projectContext->ResolvePath( header );
    const std::string relPath    = m_projectContext->Relativize( headerPath );
    if ( m_translationUnits.contains( relPath ) )
    {
        return;
    }

    DZTranslationUnit tu{ };
    tu.FullPath = headerPath;
    tu.RelPath  = relPath;

    const std::string        includeRoot = m_projectContext->GetIncludeRoot( );
    std::vector<std::string> defineArgs;
    defineArgs.emplace_back( "-DDZ_API=__attribute__((annotate(\"DZ_API\")))" );
    std::vector<const char *> arguments = { "-x", "c++", "-std=c++20" };
    arguments.reserve( arguments.size( ) + defineArgs.size( ) + 2 );
    for ( const std::string &define : defineArgs )
    {
        arguments.push_back( define.c_str( ) );
    }
    arguments.push_back( "-I" );
    arguments.push_back( includeRoot.c_str( ) );

    const CXTranslationUnit unit = clang_parseTranslationUnit( m_index, headerPath.c_str( ), arguments.data( ), static_cast<int>( arguments.size( ) ), nullptr, 0,
                                                               CXTranslationUnit_DetailedPreprocessingRecord );

    if ( unit == nullptr )
    {
        spdlog::critical( "Failed to create translation unit for header: {}", headerPath );
        m_translationUnits.emplace( relPath, std::move( tu ) );
        return;
    }

    tu.TranslationUnit = unit;
    m_translationUnits.emplace( relPath, std::move( tu ) );

    const CXCursor cursor = clang_getTranslationUnitCursor( unit );
    VisitCtx       ctx{ this, relPath };
    clang_visitChildren(
        cursor,
        []( CXCursor cursor, CXCursor parent, CXClientData clientData )
        {
            const auto *visitCtx = static_cast<VisitCtx *>( clientData );
            return visitCtx->Self->VisitTranslationUnit( cursor, parent, clientData );
        },
        &ctx );
}

bool DZIndex::IsInternalPath( const std::string &path ) const
{
    return path.starts_with( "DenOfIzGraphics" );
}

void DZIndex::AssignDepthOrdersFrom( const std::string &entryHeaderRel )
{
    std::unordered_set<std::string>           temp;
    std::unordered_map<std::string, uint32_t> memo;

    (void)ComputeDepth( entryHeaderRel, temp, memo );

    for ( const auto &key : m_translationUnits | std::views::keys )
    {
        const std::string &rel = key;
        if ( !memo.contains( rel ) )
        {
            temp.clear( );
            (void)ComputeDepth( rel, temp, memo );
        }
    }
    for ( auto &kv : m_translationUnits )
    {
        kv.second.Order = memo[ kv.first ];
    }
}

uint32_t DZIndex::ComputeDepth( const std::string &headerRel, std::unordered_set<std::string> &temp, std::unordered_map<std::string, uint32_t> &memo )
{
    if ( const auto it = memo.find( headerRel ); it != memo.end( ) )
    {
        return it->second;
    }
    if ( temp.contains( headerRel ) )
    {
        spdlog::warn( "Include cycle detected at (relative): {}", headerRel );
        return 0;
    }
    temp.insert( headerRel );

    uint32_t best = 0;
    if ( const auto tuIt = m_translationUnits.find( headerRel ); tuIt != m_translationUnits.end( ) )
    {
        for ( const auto &inc : tuIt->second.Includes )
        {
            if ( inc.IsSystem )
            {
                continue;
            }

            const std::string &childRel   = inc.RelPath;
            uint32_t           childDepth = 0;
            if ( m_translationUnits.contains( childRel ) )
            {
                childDepth = ComputeDepth( childRel, temp, memo );
            }
            best = std::max<uint32_t>( best, childDepth + 1 );
        }
    }
    temp.erase( headerRel );
    memo[ headerRel ] = best;
    return best;
}
