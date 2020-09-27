#include "Path.h"

#include "StringUtil.h"



GR::String Path::RemoveFileSpec( const GR::String& Path, const GR::String& Separator )
{
  GR::String    result = Path;
  size_t            pos = result.length();

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
  GR::String    result = Path;
  size_t            pos = result.length();

  while ( pos > 0 )
  {
    --pos;
    if ( IsSeparator( result[pos], Separator ) )
    {
      return result.substr( pos + 1 );
    }
  }
  return result;
}



bool Path::IsRelative( const GR::String& Path, const GR::String& Separator )
{
  if ( Path.length() >= 2 )
  {
    if ( ( ( Separator.find( Path[0] ) != GR::String::npos )
    &&     ( Separator.find( Path[1] ) != GR::String::npos ) )   // Windows Netzwerkpfad
    ||   ( Path[1] == ':' ) )
    {
      return false;
    }
  }
  return true;
}



GR::String Path::RelativeTo( const GR::String& strFrom, const bool bFromDir, const GR::String& strTo, const bool bToDir, const GR::String& Separator )
{
  GR::String     strTempFrom = strFrom;
  GR::String     strTempTo = strTo;

  if ( ( strFrom.empty() )
  ||   ( strTo.empty() ) )
  {
    return strTo;
  }

  if ( !bFromDir )
  {
    strTempFrom = Path::RemoveFileSpec( strTempFrom );
  }
  if ( !bToDir )
  {
    strTempTo = Path::RemoveFileSpec( strTempTo );
  }

  strTempFrom = Path::AddBackslash( strTempFrom );
  strTempTo = Path::AddBackslash( strTempTo );


  if ( toupper( strFrom[0] ) != toupper( strTo[0] ) )
  {
    // unterschiedliches Hauptverzeichnis
    return strTo;
  }

  GR::String     strCommon = CommonPrefix( strTempFrom, strTempTo, Separator );
  GR::String     strResult;

  size_t      iPos =  strCommon.length();

  while ( iPos < strTempFrom.length() )
  {
    if ( Separator.find( strTempFrom[iPos] ) == GR::String::npos )
    {
      size_t    iPos2 = FindNextSeparator( strTempFrom, iPos + 1, Separator );
      if ( iPos2 != GR::String::npos )
      {
        strResult += "..";
        strResult += Separator[0];
      }
      else
      {
        strResult += "..";
        break;
      }
      iPos = iPos2;
    }

    ++iPos;
  }

  if ( strCommon.length() < strTo.length() )
  {
    strResult = Path::Append( strResult, strTo.substr( strCommon.length() ) );
  }

  if ( Separator.find( strResult[0] ) != GR::String::npos )
  {
    strResult = strResult.substr( 1 );
  }
  strResult = Path::RemoveBackslash( strResult, Separator );

  return strResult;
}



GR::String Path::CommonPrefix( const GR::String& Path1, const GR::String& Path2, const GR::String& Separator )
{
  if ( ( Path1.empty() )
  ||   ( Path2.empty() ) )
  {
    return GR::String();
  }
  if ( GR::Strings::CompareUpperCase( Path1, Path2 ) == 0 )
  {
    return Path1;
  }

  GR::String     strResult;

  size_t     iLength = Path1.length();
  if ( Path2.length() > iLength )
  {
    iLength = Path2.length();
  }

  size_t    BackslashPos1 = 0;
  size_t    BackslashPos2 = 0;
  while ( ( BackslashPos1 < Path1.length() )
  &&      ( BackslashPos2 < Path2.length() ) )
  {
    size_t NewBackslashPos1 = FindNextSeparator( Path1, BackslashPos1, Separator );
    size_t NewBackslashPos2 = FindNextSeparator( Path2, BackslashPos2, Separator );

    if ( NewBackslashPos1 == (size_t)-1 )
    {
      NewBackslashPos1 = Path1.length();
    }
    if ( NewBackslashPos2 == (size_t)-1 )
    {
      NewBackslashPos2 = Path2.length();
    }
    if ( NewBackslashPos1 != NewBackslashPos2 )
    {
      // ab hier gibt es Unterschiede
      break;
    }
    bool            DifferenceFound = false;
    GR::String     SubResult;
    for ( size_t i = BackslashPos1; i < NewBackslashPos1; ++i )
    {
      if ( toupper( Path1[i] ) != toupper( Path2[i] ) )
      {
        if ( ( IsSeparator( Path1[i] ) )
        &&   ( IsSeparator( Path2[i] ) ) )
        {
        }
        else
        {
          DifferenceFound = true;
          break;
        }
      }
      SubResult += Path1[i];
    }
    if ( !DifferenceFound )
    {
      strResult += SubResult;
      strResult += Separator[0];
    }
    BackslashPos1 = NewBackslashPos1 + 1;
    BackslashPos2 = NewBackslashPos2 + 1;
  }

  if ( strResult.length() > 3 )
  {
    // bei mehr als nur Root soll kein Backslash dran sein
    strResult = Path::RemoveBackslash( strResult, Separator );
  }
  return strResult;
}



GR::String Path::RemoveBackslash( const GR::String& Path, const GR::String& Separator )
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



GR::String Path::AddBackslash( const GR::String& Path, const GR::String& Separator )
{
  if ( Path.empty() )
  {
    GR::String   strResult;

    strResult += Separator[0];
    return strResult;
  }

  if ( !IsSeparator( Path[Path.length() - 1], Separator ) )
  {
    return Path + Separator[0];
  }
  return Path;
}



GR::String Path::TraverseUp( const GR::String& Path, const GR::String& Separator )
{
  GR::String     strResult = Path;

  if ( IsSeparator( strResult[strResult.length() - 1], Separator ) )
  {
    strResult = strResult.substr( 0, strResult.length() - 1 );
  }

  size_t    iPos = strResult.length();
  bool      foundSeparator = false;
  while ( iPos > 0 )
  {
    --iPos;
    if ( IsSeparator( strResult[iPos], Separator ) )
    {
      foundSeparator = true;
      break;
    }
  }
  if ( !foundSeparator )
  {
    if ( strResult.length() <= 2 )
    {
      // ein Root-Verzeichnis
      return Path;
    }
    return strResult;
  }

  return strResult.substr( 0, iPos );
}



GR::String Path::ParentDirectory( const GR::String& Path, const GR::String& Separator )
{
  GR::String     strResult = Path;

  if ( IsSeparator( strResult[strResult.length() - 1], Separator ) )
  {
    strResult = strResult.substr( 0, strResult.length() - 1 );
  }

  size_t    iPos = strResult.length();
  bool      foundSeparator = false;
  while ( iPos > 0 )
  {
    --iPos;
    if ( IsSeparator( strResult[iPos], Separator ) )
    {
      foundSeparator = true;
      break;
    }
  }
  if ( !foundSeparator )
  {
    if ( strResult.length() <= 2 )
    {
      // ein Root-Verzeichnis
      return Path;
    }
    return strResult;
  }

  return strResult.substr( iPos + 1 );
}



GR::String Path::FileNameWithoutExtension( const GR::String& Path, const GR::String& Separator )
{
  return Path::StripPath( Path::RemoveExtension( Path ), Separator );
}



GR::String Path::RemoveExtension( const GR::String& Path )
{
  size_t    iPos = Path.find_last_of( '.' );
  if ( iPos == GR::String::npos )
  {
    return Path;
  }
  return Path.substr( 0, iPos );
}



GR::String Path::RenameExtension( const GR::String& Path, const GR::String& strExtension )
{
  GR::String     strResult = RemoveExtension( Path );

  strResult += '.';
  strResult += strExtension;

  return strResult;
}



GR::String Path::Extension( const GR::String& Path )
{
  size_t    iPos = Path.find_last_of( '.' );
  if ( iPos == GR::String::npos )
  {
    return GR::String();
  }
  if ( iPos >= Path.length() - 1 )
  {
    return GR::String();
  }
  return Path.substr( iPos + 1 );
}



GR::String Path::FileName( const GR::String& Path, const GR::String& Separator )
{
  size_t    iPos = Path.length();
  bool      foundSeparator = false;
  while ( iPos > 0 )
  {
    --iPos;
    if ( IsSeparator( Path[iPos], Separator ) )
    {
      foundSeparator = true;
      break;
    }
  }
  if ( !foundSeparator )
  {
    return Path;
  }
  if ( iPos >= Path.length() - 1 )
  {
    return GR::String();
  }
  return Path.substr( iPos + 1 );
}



GR::String Path::Normalize( const GR::String& Path, bool IsDir, const GR::String& Separator )
{
  GR::String  result;

  size_t          pos = Path.length() - 1,
                  lastSeparatorPos = (size_t)-1;

  int             skipNextDirCount = 0;

  if ( Path.empty() )
  {
    return Path;
  }

  if ( !IsDir )
  {
    while ( true )
    {
      GR::Char    cLetter = Path[pos];
      if ( IsSeparator( cLetter, Separator ) )
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
    GR::Char    cLetter = Path[pos];

    if ( IsSeparator( cLetter, Separator ) )
    {
      // auf .. prüfen
      GR::String   strSubPath;
      if ( lastSeparatorPos == (size_t)-1 )
      {
        // bis zum Ende des Strings
        strSubPath = Path.substr( pos + 1 );
      }
      else
      {
        strSubPath = Path.substr( pos + 1, lastSeparatorPos - pos - 1 );
      }
      if ( ( strSubPath == "." )
      ||   ( strSubPath.empty() ) )
      {
        // komplett ignorieren
      }
      else if ( strSubPath == ".." )
      {
        ++skipNextDirCount;
      }
      else
      {
        if ( skipNextDirCount == 0 )
        {
          if ( result.empty() )
          {
            result = strSubPath;
          }
          else
          {
            result = strSubPath + Separator[0] + result;
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



bool Path::IsSubPath( const GR::String& Path, const GR::String& strBasePath )
{
  return ( Path.find( strBasePath ) == 0 );
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
  return ( CommonPrefix( Path1, Path2, Separators ) == RemoveBackslash( Path1 ) );
}




