#include "Base64.h"

#include <string>



ByteBuffer Base64::Encode( const ByteBuffer& memSource )
{

  char    szTable[] = { 'A','B','C','D','E','F','G','H','I','J',
                        'K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y',
                        'Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n',
                        'o','p','q','r','s','t','u','v','w','x','y','z','0','1','2',
                        '3','4','5','6','7','8','9','+','/'};

  GR::String   strBinaer = "",
                strResult = "";

  unsigned char*   pSource = (unsigned char*)memSource.Data();

  for ( size_t i = 0; i < memSource.Size(); ++i )
  {
    unsigned char    bChar = pSource[i];

    for ( int j = 7; j >= 0; j-- )
    {
      if ( bChar & ( 1 << j ) )
      {
        strBinaer += '1';
      }
      else
      {
        strBinaer += '0';
      }
    }
  }

  size_t k = memSource.Size();

  int iPadNum = 0;

  while ( ( k % 3 ) !=0 )
  {
    iPadNum++;
    k++;
  }

  if ( iPadNum == 1 )
  {
    strBinaer += "00";
  }
  else if ( iPadNum == 2 )
  {
    strBinaer += "0000";
  }

  // jetzt die Binär-Kodes durch den Table jagen

  size_t   iPos = 0;

  while ( iPos < strBinaer.length() )
  {
    GR::String   strBinaerValue = strBinaer.substr( iPos, 6 );

    unsigned char    bValue = 0;

    for ( int j = 0; j < 6; j++ )
    {
      if ( strBinaerValue[j] == '1' )
      {
        bValue |= ( 1 << ( 5 - j ) );
      }
    }
    strResult += szTable[bValue];

    iPos += 6;
  }

  if ( iPadNum == 1 )
  {
    strResult += "=";
  }
  else if ( iPadNum == 2 )
  {
    strResult += "==";
  }

  return ByteBuffer( strResult.c_str(), strResult.length() );

}



ByteBuffer Base64::Decode( const ByteBuffer& memSource )
{

  char    szTable[] = { 'A','B','C','D','E','F','G','H','I','J',
                        'K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y',
                        'Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n',
                        'o','p','q','r','s','t','u','v','w','x','y','z','0','1','2',
                        '3','4','5','6','7','8','9','+','/'};

  GR::String stringcode = "";

  ByteBuffer    decodedData;

  size_t l = memSource.Size();

  int padnum = 0;

  //reject zero-length strings
  if ( l == 0 )
  {
    return ByteBuffer();
  }

  //if string length not multiple of 4 then error
  if ( l % 4 !=0 )
  {
    //printf( "Illegal string length. Base64 decoding not possible." );
    //printf( "Length %d\n", l );
    //return decodedstring;
  }

  //otherwise remove padding
  while ( memSource.ByteAt( l - 1 ) == '=' )
  {
    padnum++;
    l--;
  }

  //maximum 2 padding characters can be there in a valid Base64
  //encoded string
  if ( padnum > 2 )
  {
    //printf( "Excessive padding. Base64 decoding not possible." );
    //return decodedstring;
  }

  //for all the characters in the string find value, convert
  //into 6bit binary and form concatenated string
  for ( size_t i = 0; i < l; i++ )
  {
    bool done = false;

    int charval = 0;

    while ( !done )
    {

      char check = (char)memSource.ByteAt( i );

      //find the value corresponding to each character

      if ( szTable[charval] == check )
      {
        done = true;
        //convert the value into 6bit string
        //and form concatenated string
        unsigned char    bValue = (unsigned char)charval;

        for ( int j = 0; j < 6; j++ )
        {
          if ( bValue & ( 1 << ( 5 - j ) ) )
          {
            stringcode += '1';
          }
          else
          {
            stringcode += '0';
          }
        }
      }
      else
      {
        //try next entry in b64table
        charval++;
      }
      //if character not in b64table then error

      if ( charval > 63 )
      {
        //printf( "Illegal character in string : \n" );
      }
    }
  }


  //proceed if no error
  l = stringcode.length();

  //if padded then the last 2 or 4 bits must be zero.
  //if not then error
  //otherwise remove the zeroes
  if ( padnum == 1 )
  {
    if ( ( stringcode.substr( l - 2, 2 ) ) == "00" )
    {
      if ( ( ( l - 2 ) % 8 ) ==0 )
      {
        l = l - 2;
      }
      else
      {
        //printf( "a Improper coding. Decoding not possible.\n" );
      }
    }
    else
    {
      //printf( "b Improper coding. Decoding not possible.\n" );
    }
  }
  else if ( padnum == 2 )
  {
    if ( ( stringcode.substr( l - 4, 4 ) ) == "0000" )
    {
      if ( ( ( l - 4 ) % 8 ) == 0 )
      {
        l = l - 4;
      }
      else
      {
        //printf( "c Improper coding. Decoding not possible.\n" );
      }
    }
    else
    {
      //printf( "d Improper coding. Decoding not possible.\n" );
    }
  }


  //proceed if no error

  GR::String bitstring = "";

  size_t cursor = 0;

  while ( cursor < l )
  {
    // find decimal value of each 8bit group
    int   value = 0;

    if ( cursor + 8 > stringcode.length() )
    {
      return ByteBuffer();
    }

    for ( int j = 0; j < 8; j++ )
    {
      if ( stringcode[cursor + j] == '1' )
      {
        value |= ( 1 << ( 7 - j ) );
      }
    }

    //the decimal value can't be greater than 127 as the

    //original string must have had only ascii characters

    if ( value > 127 )
    {
      //printf( "e Improper coding. Decoding not possible.\n" );
    }

    cursor = cursor + 8;


    // update decodedstring
    decodedData.AppendU8( (GR::u8)value );
    //decodedstring += (char)value;

    value = 0;
  }

  return decodedData;//ByteBuffer( decodedstring.c_str(), decodedstring.length() );

}



