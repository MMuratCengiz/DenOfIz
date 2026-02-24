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

#include <future>

#include "DenOfIzBindings/Core/ProjectContext.h"

#include <clang-c/Index.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace DenOfIz
{
    struct DZInclude
    {
        bool        IsSystem;
        std::string FullPath;
        std::string RelPath;
    };

    struct DZTranslationUnit
    {
        std::string            FullPath;
        std::string            RelPath;
        std::vector<DZInclude> Includes;
        uint32_t               Order;
        CXTranslationUnit      TranslationUnit;
    };

    class DZIndex
    {
        struct VisitCtx
        {
            DZIndex    *Self = nullptr;
            std::string CurrentHeaderPath;
        };

        ProjectContext                                    *m_projectContext;
        CXIndex                                            m_index;
        std::vector<DZTranslationUnit *>                   m_translationUnitsVec;
        std::unordered_map<std::string, DZTranslationUnit> m_translationUnits;
        std::vector<std::future<void>>                     m_translationUnitFutures;
        uint32_t                                           m_orderCounter = 0;
        std::mutex                                         m_mu;
        std::unordered_set<std::string>                    m_scheduled;

    public:
        explicit DZIndex( ProjectContext *projectContext, const char *entryHeader );
        const std::vector<DZTranslationUnit *> &GetTranslationUnits( ) const;
        const DZTranslationUnit                *GetTranslationUnit( const std::string &relPath ) const;
        ~DZIndex( );

    private:
        void               WaitCompilation( ) const;
        bool               TrySchedule( const std::string &absPath );
        CXChildVisitResult ProcessInclusionDirective( const CXCursor &cursor, const VisitCtx *ctx, DZIndex *self ) const;
        CXChildVisitResult VisitTranslationUnit( CXCursor cursor, CXCursor parent, CXClientData clientData ) const;
        void               CreateTranslationUnit( const char *header );
        bool               IsInternalPath( const std::string &path ) const;
        void               AssignDepthOrdersFrom( const std::string &entryHeaderRel );
        uint32_t           ComputeDepth( const std::string &headerRel, std::unordered_set<std::string> &temp, std::unordered_map<std::string, uint32_t> &memo );
    };
} // namespace DenOfIz
