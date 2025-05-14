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

#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/Import/AssetScanner.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <filesystem>

using namespace DenOfIz;

void AssetScanner::AddImporter( IAssetImporter *importer, ImportDesc *desc )
{
    if ( importer == nullptr || desc == nullptr )
    {
        return;
    }

    m_importers.AddElement( importer );
    m_importDescs.AddElement( desc );
}

void AssetScanner::RegisterModifyAssetCallback( ModifyAssetCallback *callback )
{
    if ( callback == nullptr )
    {
        return;
    }

    m_modifyAssetCallbacks.AddElement( callback );
}

void AssetScanner::RegisterFilterAssetCallback( FilterAssetCallback *callback )
{
    if ( callback == nullptr )
    {
        return;
    }

    m_filterAssetCallbacks.AddElement( callback );
}

void AssetScanner::Scan( const InteropString &directoryToScan, const InteropString &targetDirectory )
{
    if ( !FileIO::FileExists( directoryToScan ) )
    {
        LOG( ERROR ) << "Asset scanner root path does not exist: " << directoryToScan.Get( );
        return;
    }
    if ( !FileIO::FileExists( targetDirectory ) )
    {
        LOG( ERROR ) << "Asset scanner target directory does not exist: " << targetDirectory.Get( );
        return;
    }

    const auto fsPath = std::filesystem::path( directoryToScan.Get( ) );
    if ( !std::filesystem::is_directory( fsPath ) )
    {
        LOG( ERROR ) << "Asset scanner root path is not a directory: " << directoryToScan.Get( );
        return;
    }

    LOG( INFO ) << "Scanning for assets in: " << directoryToScan.Get( );
    for ( const auto &entry : std::filesystem::recursive_directory_iterator( fsPath ) )
    {
        if ( !entry.is_regular_file( ) )
        {
            continue;
        }

        const std::string filePath = entry.path( ).string( );
        InteropString     interopPath( filePath.c_str( ) );

        bool processFile = true;
        for ( size_t i = 0; i < m_filterAssetCallbacks.NumElements( ); ++i )
        {
            if ( !m_filterAssetCallbacks.GetElement( i )->ShouldProcessAsset( interopPath ) )
            {
                processFile = false;
                break;
            }
        }

        if ( !processFile )
        {
            continue;
        }

        InteropString modifiedPath = interopPath;
        for ( size_t i = 0; i < m_modifyAssetCallbacks.NumElements( ); ++i )
        {
            modifiedPath = m_modifyAssetCallbacks.GetElement( i )->ModifyPath( modifiedPath );
        }

        const InteropString fileExtension = InteropString( entry.path( ).extension( ).string( ).c_str( ) ).ToLower( );

        for ( size_t i = 0; i < m_importers.NumElements( ); ++i )
        {
            IAssetImporter *importer = m_importers.GetElement( i );
            if ( importer->CanProcessFileExtension( fileExtension ) )
            {
                LOG( INFO ) << "Found asset to process: " << modifiedPath.Get( ) << " with importer: " << importer->GetImporterInfo( ).Name.Get( );

                ImportJobDesc jobDesc;
                jobDesc.SourceFilePath  = modifiedPath;
                jobDesc.TargetDirectory = targetDirectory;
                jobDesc.AssetNamePrefix = InteropString( "" );
                jobDesc.Desc            = m_importDescs.GetElement( i );

                ImporterResult result = importer->Import( jobDesc );

                if ( result.ResultCode != ImporterResultCode::Success )
                {
                    LOG( ERROR ) << "Failed to import asset: " << modifiedPath.Get( ) << " Error: " << result.ErrorMessage.Get( );
                }
                else
                {
                    LOG( INFO ) << "Successfully imported asset: " << modifiedPath.Get( ) << " Created " << result.CreatedAssets.NumElements( ) << " assets";
                }
                break;
            }
        }
    }
}
