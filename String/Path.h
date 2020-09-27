#pragma once

#include <string>

#include <GR/GRTypes.h>


namespace Path
{

#if OPERATING_SYSTEM == OS_WINDOWS
  #define OS_PATH_SEPARATORS  "\\/"
#elif OPERATING_SYSTEM == OS_TANDEM
  #ifdef TANDEM_GUARDIAN
    #define OS_PATH_SEPARATORS  "."
  #else
    #define OS_PATH_SEPARATORS  "/"
  #endif
#else
  #define OS_PATH_SEPARATORS  "/"
#endif

  GR::String      StripPath( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      RemoveFileSpec( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      Append( const GR::String& Path, const GR::String& SecondPath, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      RelativeTo( const GR::String& strFrom, const bool bFromDir,
                                  const GR::String& strTo, const bool bToDir, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      CommonPrefix( const GR::String& path1, const GR::String& path2, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      AddBackslash( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      RemoveBackslash( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      TraverseUp( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      ParentDirectory( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      RemoveExtension( const GR::String& Path );

  GR::String      RenameExtension( const GR::String& Path, const GR::String& strExtension );

  GR::String      Extension( const GR::String& Path );

  GR::String      FileName( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      FileNameWithoutExtension( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      Normalize( const GR::String& Path, bool IsDir, const GR::String& Separator = OS_PATH_SEPARATORS );

  GR::String      RenameFile( const GR::String& OldPath, const GR::String& strNewFileName, const GR::String& Separator = OS_PATH_SEPARATORS );

  bool                IsRelative( const GR::String& Path, const GR::String& Separator = OS_PATH_SEPARATORS );

  bool                IsSubPath( const GR::String& Path, const GR::String& strBasePath );

  bool                IsFileNameValid( const GR::String& Path );

  bool                IsSeparator( const GR::Char Char, const GR::String& Separator = OS_PATH_SEPARATORS );
    
  size_t              FindNextSeparator( const GR::String& Path, size_t Offset = 0, const GR::String& Separator = OS_PATH_SEPARATORS );

  bool                IsPathEqual( const GR::String& Path1, const GR::String& Path2, const GR::String& Separator = OS_PATH_SEPARATORS );


};
