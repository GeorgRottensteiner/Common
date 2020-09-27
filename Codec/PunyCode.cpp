#include <locale>

#include ".\punycode.h"

#include <String/Convert.h>
#include <String/StringUtil.h>


// re-enable warning in std::vector
#if OPERATING_SYSTEM == OS_TANDEM
#pragma nowarn( 1506 )
#endif


// encode_basic(bcp,flag) forces a basic code point to lowercase
// if flag is zero, uppercase if flag is nonzero, and returns
// the resulting code point.  The code point is unchanged if it
// is caseless.  The behavior is undefined if bcp is not a basic
// code point.

char PunyCode::PunyEncodeBasic( unsigned short bcp, bool Uppercase )
{

  bcp -= ( (unsigned int)bcp - 97 < 26 ) << 5;
  
  // GR: was
  //     return bcp + ( ( !Uppercase && ( (unsigned int)bcp - 65 < 26 ) ) << 5 );
  if ( ( !Uppercase )
  &&   ( (unsigned int)bcp - 65 < 26 ) )
  {
    bcp += 32;
  }
  
  return (char)bcp;

}



// encode_digit(d,flag) returns the basic code point whose value
// (when used for representing integers) is d, which needs to be in
// the range 0 to base-1.  The lowercase form is used unless flag is
// nonzero, in which case the uppercase form is used.  The behavior
// is undefined if flag is nonzero and digit d has no uppercase form.

char PunyCode::EncodeDigit( unsigned short d, bool Uppercase )
{
  //  0..25 map to ASCII a..z or A..Z
  // 26..35 map to ASCII 0..9
  if ( Uppercase )
  {
    return (char)( d + 22 + 75 * ( d < 26 ) - 32 );
  }
  return (char)( d + 22 + 75 * ( d < 26 ) );
}



// decode_digit(cp) returns the numeric value of a basic code
// point (for use in representing integers) in the range 0 to
// base-1, or base if cp is does not represent a value.

unsigned short PunyCode::DecodeDigit( unsigned short cp )
{

  return  cp - 48 < 10 ? cp - 22 :  cp - 65 < 26 ? cp - 65 : cp - 97 < 26 ? cp - 97 : base;

}



// Bias adaptation function

unsigned short PunyCode::Adapt( unsigned short delta, unsigned short numpoints, int firsttime )
{
  
  unsigned short k;

  delta = (unsigned short)( firsttime ? delta / damp : delta / 2 );
  delta += delta / numpoints;

  for ( k = 0;  delta > ( ( base - tmin ) * tmax ) / 2;  k += base ) 
  {
    delta /= base - tmin;
  }

  return k + ( base - tmin + 1 ) * delta / ( delta + skew );
  
}



void PunyCode::Split( const GR::WString& strSource, const wchar_t cSeparator, std::list<GR::WString>& listParts )
{
  size_t      iPos = 0;

  size_t      iStartPos = 0;

  while ( ( iPos = strSource.find( cSeparator, iStartPos ) ) != GR::String::npos )
  {
    listParts.push_back( strSource.substr( iStartPos, iPos - iStartPos ) );
    iStartPos = iPos + 1;
  }
  if ( ( strSource.length() >= iStartPos )
  &&   ( !strSource.empty() ) )
  {
    listParts.push_back( strSource.substr( iStartPos, strSource.length() - iStartPos ) );
  }
}



bool PunyCode::EncodeDomain( const GR::WString& FullURL, GR::String& Output )
{
  size_t      ProtocolEnd = FullURL.find( L"//" );
  if ( ProtocolEnd == GR::String::npos )
  {
    // End of Protocol (//) not found in URL
    return false;
  }
  
  size_t      DomainEnd  = FullURL.find( '/', ProtocolEnd + 2 );
  if ( DomainEnd == GR::String::npos )
  {
    DomainEnd = FullURL.size();
  }
  
  std::list<GR::WString>   DomainParts;
  
  Split( FullURL.substr( ProtocolEnd + 2, DomainEnd - ProtocolEnd - 2 ), '.', DomainParts );
  
  GR::String     FinalResult = GR::Convert::ToStringA( FullURL.substr( 0, ProtocolEnd + 2 ) );
  
  std::list<GR::WString>::iterator   itParts( DomainParts.begin() );
  while ( itParts != DomainParts.end() )
  {
    GR::String     PartResult;
    
    if ( !PunyCode::Encode( *itParts, PartResult ) )
    {
      // PunyCode::Encode failed for domain part
      return false;
    }
    if ( PartResult.length() != itParts->length() )
    {
      // part was touched
      FinalResult += "xn--";
    }
    FinalResult += PartResult;
    ++itParts;
    if ( itParts != DomainParts.end() )
    {
      FinalResult += ".";
    }
  }
  
  FinalResult += GR::Convert::ToStringA( FullURL.substr( DomainEnd ) );
  
  Output = FinalResult;
  return true;
}



bool PunyCode::Encode( const GR::WString& Input, GR::String& Result )
{

  Result.erase();
  if ( Input.empty() )
  {
    return true;
  }

  unsigned short n, delta, h, b, out, max_out, bias, m, q, k, t;

  size_t        j;

  // Initialize the state
  n       = initial_n;
  delta   = out = 0;
  max_out = 2048;
  bias    = initial_bias;

  // Handle the basic code points

  for ( j = 0;  j < Input.length(); ++j ) 
  {
    if ( Input[j] < 0x80 )
    {
      if ( max_out - out < 2 )
      {
        //return punycode_big_output;
        return false;
      }
      Result += PunyEncodeBasic( Input[j], std::isupper( Input[j], std::locale::classic() ) );
      ++out;
    }
    // else if (input[j] < n) return punycode_bad_input;
    // (not needed for Punycode with unsigned code points)
  }
  h = b = out;

  // h is the number of code points that have been handled, b is the
  // number of basic code points, and out is the number of characters
  // that have been output.

  if ( out < Input.length() )
  {
    Result += delimiter;
  }

  // Main encoding loop

  while ( h < Input.length() ) 
  {
    // All non-basic code points < n have been handled already.  Find the next larger one

    for ( m = (unsigned short)-1, j = 0;  j < Input.length();  ++j ) 
    {
      // if (basic(input[j])) continue;  // (not needed for Punycode)
      if (Input[j] >= n && Input[j] < m) m = Input[j];
    }

    // Increase delta enough to advance the decoder's <n,i> state to <m,0>, but guard against overflow

    if ( m - n > ( ( (unsigned short)-1 ) - delta ) / ( h + 1 ) )
    {
      //return punycode_overflow;
      return false;
    }
    delta += ( m - n ) * ( h + 1 );
    n = m;

    for ( j = 0;  j < Input.length();  ++j ) 
    {
      // Punycode does not need to check whether input[j] is basic
      if ( Input[j] < n ) 
      {
        if ( ++delta == 0 )
        {
          //return punycode_overflow;
          return false;
        }
      }

      if ( Input[j] == n ) 
      {
        // Represent delta as a generalized variable-length integer
        for ( q = delta, k = base; ; k += base ) 
        {
          if ( out >= max_out )
          {
            //return punycode_big_output;
            return false;
          }

          t = ( k <= bias ) /* + tmin */ ? tmin :     // +tmin not needed
          k >= bias + tmax ? tmax : k - bias;
          if ( q < t )
          {
            break;
          }

          Result += EncodeDigit( (unsigned short)( t + ( q - t ) % ( base - t ) ), 0 );

          ++out;
          q = ( q - t ) / ( base - t );
        }

        Result += EncodeDigit( q, std::isupper( (wchar_t)q, std::locale::classic() ) );
        ++out;
        bias = Adapt( delta, h + 1, h == b );
        delta = 0;
        ++h;
      }
    }

    ++delta, ++n;
  }

  return true;

}





bool PunyCode::Decode( const GR::String& Input, GR::WString& Result )
{

  Result.erase();
  if ( Input.empty() )
  {
    return true;
  }
  unsigned short n, out, i, max_out, bias, b, j, in, oldi, w, k, digit, t;

  // Initialize the state

  n         = initial_n;
  out       = 0;
  i         = 0;
  max_out   = 2048;
  bias      = initial_bias;


  // Handle the basic code points:  Let b be the number of input code
  // points before the last delimiter, or 0 if there is none, then
  // copy the first b code points to the output.

  for ( b = j = 0; j < Input.length(); ++j )
  {
    if ( Input[j] == delimiter )
    {
      b = j;
    }
  }
  if ( b > max_out )
  {
    //return punycode_big_output;
    return false;
  }

  for ( j = 0;  j < b;  ++j ) 
  {
    if ( Input[j] >= 0x80 )
    {
      // must not contain non-ASCII character
      //return punycode_bad_input;
      return false;
    }
    Result += Input[j];
    ++out;
  }

  // Main decoding loop:  Start just after the last delimiter 
  //                      if any basic code points were copied; start at the beginning otherwise

  for ( in = b > 0 ? b + 1 : 0; in < Input.length(); ++out ) 
  {
    // in is the index of the next character to be consumed, and
    // out is the number of code points in the output array.

    // Decode a generalized variable-length integer into delta,
    // which gets added to i.  The overflow checking is easier
    // if we increase i as we go, then subtract off its starting
    // value at the end to obtain delta.

    for ( oldi = i, w = 1, k = base; ; k += base ) 
    {
      if ( in >= Input.length() )
      {
        //return punycode_bad_input;
        return false;
      }

      digit = DecodeDigit( Input[in++] );
      if ( digit >= base )
      {
        //return punycode_bad_input;
        return false;
      }

      if ( digit > ( ( (unsigned short)-1 ) - i ) / w )
      {
        //return punycode_overflow;
        return false;
      }
      i += digit * w;
      t = ( k <= bias ) /* + tmin */ ? tmin :     // +tmin not needed
      k >= bias + tmax ? tmax : k - bias;
      if ( digit < t )
      {
        break;
      }
      if ( w > (unsigned short)-1 / ( base - t ) )
      {
        //return punycode_overflow;
        return false;
      }
      w *= ( base - t );
    }

    bias = Adapt( i - oldi, out + 1, oldi == 0 );

    // i was supposed to wrap around from out+1 to 0, incrementing n each time, so we'll fix that now

    if ( i / ( out + 1 ) > ( (unsigned short)-1 ) - n ) 
    {
      //return punycode_overflow;
      return false;
    }

    n += i / ( out + 1 );
    i %= ( out + 1 );

    // Insert n at position i of the output
    Result.insert( Result.begin() + i, n );
  }

  return true;

}



