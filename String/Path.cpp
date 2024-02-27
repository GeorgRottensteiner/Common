#include "Path.h"

#include "StringUtil.h"



GR::String Path::RemoveFileSpec( const GR::String& Path, const GR::String& Separator )
{
  GR::String    result = Path;
  size_t        pos = result.length();

  while ( pos > 0 )
  {
    --pos;
    if ( IsSeparator( result[pos], Separator ) )
    {
      return result.substr( 0, pos );
    }
  }
  return GR::String();
}



GR::String Path::Append( const GR::String& Path, const GR::String& SecondPath, const GR::String& Separator )
{
  GR::String       result = Path;

  if ( ( !result.empty() )
  &&   ( !IsSeparator( result[result.length() - 1], Separator ) ) )
  {
    result += Separator[0];
  }
  if ( ( !SecondPath.empty() )
  &&   ( IsSeparator( SecondPath[0], Separator ) ) )
  {
    result += SecondPath.substr( 1 );
  }
  else
  {
    result += SecondPath;
  }
  return Normalize( result, false );
}



GR::String Path::StripPath( const GR::String& Path, const GR::String& Separator )
{
  size_t        pos = Path.length();

  while ( pos > 0 )
  {
    --pos;
    if ( IsSeparator( Path[pos], Separator ) )
    {
      return Path.substr( pos + 1 );
    }
  }
  return Path;
}



bool Path::IsRelative( const GR::String& Path, const GR::String& Separator )
{
#if OPERATING_SYSTEM == OS_WINDOWS
  if ( Path.length() >= 2 )
  {
    if ( ( Separator.find( Path[0] ) != GR::String::npos )
    &&   ( Separator.find( Path[1] ) != GR::String::npos ) )
    {
      return false;
    }
    if ( ( Path[1] == ':' )
    &&   ( toupper( Path[0] ) >= 'A' )
    &&   ( toupper( Path[0] ) <= 'Z' ) )
    {
      // drive letter
      return false;
    }
  }
#endif
#if OPERATING_SYSTEM != OS_WINDOWS
  if ( ( Path.length() > 0 )
  &&   ( IsSeparator( Path[0], Separator ) ) )
  {
    return false;
  }
#endif
  return true;
}



bool Path::IsRootPath( const GR::String& Path, const GR::String& Separator )
{
  if ( IsRelative( Path, Separator ) )
  {
    return false;
  }

#if OPERATING_SYSTEM == OS_WINDOWS
  if ( Path.length() >= 2 )
  {
    // Windows UNC path
    if ( ( Separator.find( Path[0] ) != GR::String::npos )
    &&   ( Separator.find( Path[1] ) != GR::String::npos ) )
    {
      size_t  nextSeparatorPos = FindNextSeparator( Path, 2, Separator );
      if ( ( nextSeparatorPos == GR::String::npos )
      ||   ( nextSeparatorPos + 1 == Path.length() ) )
      {
        return true;
      }
    }
    else if ( ( Path.length() == 2 )
    &&        ( Path[1] == ':' )
    &&        ( toupper( Path[0] ) >= 'A' )
    &&        ( toupper( Path[0] ) <= 'Z' ) )
    {
      // only the drive letter
      return true;
    }
    else if ( ( Path.length() == 3 )
    &&        ( Path[1] == ':' )
    &&        ( IsSeparator( Path[2], Separator ) )
    &&        ( toupper( Path[0] ) >= 'A' )
    &&        ( toupper( Path[0] ) <= 'Z' ) )
    {
      // only the drive letter plus separator
      return true;
    }
  }
#else
  if ( ( Path.length() == 1  )
  &&   ( IsSeparator( Path[0], Separator ) ) )
  {
    return true;
  }
#endif
  return false;
}



GR::String Path::RelativeTo( const GR::String& From, const bool FromDir, const GR::String& To, const bool ToDir, const GR::String& Separator )
{
  GR::String     tempFrom = From;
  GR::String     tempTo = To;

  if ( ( From.empty() )
  ||   ( To.empty() ) )
  {
    return To;
  }

  if ( !FromDir )
  {
    tempFrom = Path::RemoveFileSpec( tempFrom );
  }
  if ( !ToDir )
  {
    tempTo = Path::RemoveFileSpec( tempTo );
  }

  tempFrom  = Path::AddSeparator( tempFrom );
  tempTo    = Path::AddSeparator( tempTo );

#if OPERATING_SYSTEM == OS_WINDOWS
  if ( ( !IsRelative( From ) )
  &&   ( !IsRelative( To ) )
  &&   ( toupper( From[0] ) != toupper( To[0] ) ) )
  {
    // unterschiedliches Hauptverzeichnis
    return To;
  }
#endif

  GR::String     common = CommonPrefix( tempFrom, tempTo, Separator );
  GR::String     result;

  size_t      pos =  common.length();

  while ( pos < tempFrom.length() )
  {
    if ( Separator.find( tempFrom[pos] ) == GR::String::npos )
    {
      size_t    pos2 = FindNextSeparator( tempFrom, pos + 1, Separator );
      if ( pos2 != GR::String::npos )
      {
        result += "..";
        result += Separator[0];
      }
      else
      {
        result += "..";
        break;
      }
      pos = pos2;
    }

    ++pos;
  }

  if ( common.length() < To.length() )
  {
    result = Path::Append( result, To.substr( common.length() ) );
  }

  if ( Separator.find( result[0] ) != GR::String::npos )
  {
    result = result.substr( 1 );
  }
  result = Path::RemoveSeparator( result, Separator );

  return result;
}



GR::String Path::CommonPrefix( const GR::String& Path1, const GR::String& Path2, const GR::String& Separator )
{
  if ( ( Path1.empty() )
  ||   ( Path2.empty() ) )
  {
    return GR::String();
  }
#if OPERATING_SYSTEM == OS_WINDOWS
  if ( GR::Strings::CompareUpperCase( Path1, Path2 ) == 0 )
  {
    return Path1;
  }
#endif

  GR::String     result;

  size_t     length = Path1.length();
  if ( Path2.length() > length )
  {
    length = Path2.length();
  }

  size_t    separatorPos1 = 0;
  size_t    separatorPos2 = 0;
  while ( ( separatorPos1 < Path1.length() )
  &&      ( separatorPos2 < Path2.length() ) )
  {
    size_t newSeparatorPos1 = FindNextSeparator( Path1, separatorPos1, Separator );
    size_t newSeparatorPos2 = FindNextSeparator( Path2, separatorPos2, Separator );

    if ( newSeparatorPos1 == (size_t)-1 )
    {
      newSeparatorPos1 = Path1.length();
    }
    if ( newSeparatorPos2 == (size_t)-1 )
    {
      newSeparatorPos2 = Path2.length();
    }
    if ( newSeparatorPos1 != newSeparatorPos2 )
    {
      // ab hier gibt es Unterschiede
      break;
    }
    bool           differenceFound = false;
    GR::String     subResult;
    for ( size_t i = separatorPos1; i < newSeparatorPos1; ++i )
    {
      if ( toupper( Path1[i] ) != toupper( Path2[i] ) )
      {
        if ( ( IsSeparator( Path1[i] ) )
        &&   ( IsSeparator( Path2[i] ) ) )
        {
        }
        else
        {
          differenceFound = true;
          break;
        }
      }
      subResult += Path1[i];
    }
    if ( !differenceFound )
    {
      result += subResult;
      result += Separator[0];
    }
    separatorPos1 = newSeparatorPos1 + 1;
    separatorPos2 = newSeparatorPos2 + 1;
  }

  if ( result.length() > 3 )
  {
    // bei mehr als nur Root soll kein Backslash dran sein
    result = Path::RemoveSeparator( result, Separator );
  }
  return result;
}



GR::String Path::RemoveSeparator( const GR::String& Path, const GR::String& Separator )
{
  if ( Path.empty() )
  {
    return Path;
  }
  if ( IsSeparator( Path[Path.length() - 1], Separator ) )
  {
    return Path.substr( 0, Path.length() - 1 );
  }
  return Path;
}



GR::String Path::AddSeparator( const GR::String& Path, const GR::String& Separator )
{
  if ( Path.empty() )
  {
    GR::String   result;

    result += Separator[0];
    return result;
  }

  if ( !IsSeparator( Path[Path.length() - 1], Separator ) )
  {
    return Path + Separator[0];
  }
  return Path;
}



GR::String Path::TraverseUp( const GR::String& Path, const GR::String& Separator )
{
  if ( IsRootPath( Path, Separator ) )
  {
    return Path;
  }

  GR::String     result = Path;

  if ( IsSeparator( result[result.length() - 1], Separator ) )
  {
    result = result.substr( 0, result.length() - 1 );
  }

  size_t    pos = result.length();
  bool      foundSeparator = false;
  while ( pos > 0 )
  {
    --pos;
    if ( IsSeparator( result[pos], Separator ) )
    {
      foundSeparator = true;
      break;
    }
  }
  if ( !foundSeparator )
  {
    return result;
  }

  return result.substr( 0, pos );
}



GR::String Path::ParentDirectory( const GR::String& Path, const GR::String& Separator )
{
  if ( IsRootPath( Path, Separator ) )
  {
    // ein Root-Verzeichnis
    return Path;
  }

  GR::String     result = Path;

  if ( IsSeparator( result[result.length() - 1], Separator ) )
  {
    result = result.substr( 0, result.length() - 1 );
  }

  size_t    pos = result.length();
  bool      foundSeparator = false;
  while ( pos > 0 )
  {
    --pos;
    if ( IsSeparator( result[pos], Separator ) )
    {
      foundSeparator = true;
      break;
    }
  }
  if ( !foundSeparator )
  {
    return result;
  }

  return result.substr( pos + 1 );
}



GR::String Path::FileNameWithoutExtension( const GR::String& Path, const GR::String& Separator )
{
  return Path::StripPath( Path::RemoveExtension( Path ), Separator );
}



GR::String Path::RemoveExtension( const GR::String& Path, const GR::String& Separator )
{
  size_t        pos = Path.length();

  while ( pos > 0 )
  {
    --pos;
    if ( IsSeparator( Path[pos], Separator ) )
    {
      size_t    dotPos = Path.find_last_of( '.' );
      if ( ( dotPos != GR::String::npos )
      &&   ( dotPos >= pos ) )
      {
        return Path.substr( 0, dotPos );
      }
      return Path;
    }
  }
  size_t    dotPos = Path.find_last_of( '.' );
  if ( ( dotPos != GR::String::npos )
  &&   ( dotPos >= pos ) )
  {
    return Path.substr( 0, dotPos );
  }
  return Path;
}



GR::String Path::RenameExtension( const GR::String& Path, const GR::String& Extension, const GR::String& Separator )
{
  GR::String     result = RemoveExtension( Path, Separator );

  if ( !Extension.StartsWith( "." ) )
  {
    result += '.';
  }
  result += Extension;

  return result;
}



GR::String Path::Extension( const GR::String& Path, const GR::String& Separator )
{
  size_t        pos = Path.length();

  while ( pos > 0 )
  {
    --pos;
    if ( IsSeparator( Path[pos], Separator ) )
    {
      size_t    dotPos = Path.find_last_of( '.' );
      if ( ( dotPos != GR::String::npos )
      &&   ( dotPos >= pos ) )
      {
        return Path.substr( dotPos + 1 );
      }
      return GR::String();
    }
  }
  size_t    dotPos = Path.find_last_of( '.' );
  if ( ( dotPos != GR::String::npos )
  &&   ( dotPos >= pos ) )
  {
    return Path.substr( dotPos + 1 );
  }
  return GR::String();
}



GR::String Path::FileName( const GR::String& Path, const GR::String& Separator )
{
  size_t    pos = Path.length();
  bool      foundSeparator = false;
  while ( pos > 0 )
  {
    --pos;
    if ( IsSeparator( Path[pos], Separator ) )
    {
      foundSeparator = true;
      break;
    }
  }
  if ( !foundSeparator )
  {
    return Path;
  }
  if ( pos >= Path.length() - 1 )
  {
    return GR::String();
  }
  return Path.substr( pos + 1 );
}



GR::String Path::Normalize( const GR::String& Path, bool IsDir, const GR::String& Separator )
{
  GR::String  result;

  size_t      pos = Path.length() - 1,
              lastSeparatorPos = (size_t)-1;

  int         skipNextDirCount = 0;


  if ( Path.empty() )
  {
    return Path;
  }

  if ( !IsDir )
  {
    while ( true )
    {
      GR::Char    pathLetter = Path[pos];
      if ( IsSeparator( pathLetter, Separator ) )
      {
        lastSeparatorPos = pos + 1;
        result = Path.substr( lastSeparatorPos );
        break;
      }
      if ( pos == 0 )
      {
        return Path;
      }
      --pos;
    }
    if ( lastSeparatorPos == (size_t)-1 )
    {
      return Path;
    }
  }

  while ( true )
  {
    GR::Char    pathLetter = Path[pos];

    if ( IsSeparator( pathLetter, Separator ) )
    {
      // auf .. prüfen
      GR::String   subPath;
      if ( lastSeparatorPos == (size_t)-1 )
      {
        // bis zum Ende des Strings
        subPath = Path.substr( pos + 1 );
      }
      else
      {
        subPath = Path.substr( pos + 1, lastSeparatorPos - pos - 1 );
      }
      if ( ( subPath == "." )
      ||   ( subPath.empty() ) )
      {
        // komplett ignorieren
      }
      else if ( subPath == ".." )
      {
        ++skipNextDirCount;
      }
      else
      {
        if ( skipNextDirCount == 0 )
        {
          if ( result.empty() )
          {
            result = subPath;
          }
          else
          {
            result = subPath + Separator[0] + result;
          }
        }
        else
        {
          --skipNextDirCount;
        }
      }
      lastSeparatorPos = pos;
    }
    if ( pos == 0 )
    {
      break;
    }
    --pos;
  }
  if ( lastSeparatorPos == (size_t)-1 )
  {
    result = Path;
  }
  else if ( skipNextDirCount == 0 )
  {
    result = Path.substr( 0, lastSeparatorPos ) + Separator[0] + result;
  }
  else
  {
    for ( int i = 0; i < skipNextDirCount; ++i )
    {
      result = GR::String( ".." ) + Separator[0] + result;
    }
    if ( lastSeparatorPos > 0 )
    {
      result = Path.substr( 0, lastSeparatorPos + 1 ) + result;
    }
  }
  return result;
}



GR::String Path::RenameFile( const GR::String& OldPath, const GR::String& NewFileName, const GR::String& Separator )
{
  GR::String   Result = Path::RemoveFileSpec( OldPath, Separator );
  Result = Path::Append( Result, NewFileName, Separator );

  return Result;
}



bool Path::IsSubPath( const GR::String& Path, const GR::String& BasePath )
{
  return ( Path.find( BasePath ) == 0 );
}



bool Path::IsFileNameValid( const GR::String& Path )
{
  if ( Path.empty() )
  {
    return false;
  }
  return Path.find_first_of( "/\":*?" ) == GR::String::npos;
}



bool Path::IsSeparator( const GR::Char Char, const GR::String& Separator )
{
  return ( Separator.find( Char ) != GR::String::npos );
}



size_t Path::FindNextSeparator( const GR::String& Path, size_t Offset, const GR::String& Separator )
{
  while ( Offset < Path.length() )
  {
    if ( IsSeparator( Path[Offset], Separator ) )
    {
      return Offset;
    }
    ++Offset;
  }
  return GR::String::npos;
}



bool Path::IsPathEqual( const GR::String& Path1, const GR::String& Path2, const GR::String& Separators )
{
  return ( CommonPrefix( Path1, Path2, Separators ) == RemoveSeparator( Path1 ) );
}




