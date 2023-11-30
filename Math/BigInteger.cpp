#include "BigInteger.h"

#include <iomanip>
#include <stack>

#include <String/Convert.h>



namespace
{

  constexpr GR::u32 max_size_fast_mul = std::numeric_limits<GR::u32>::max() / 255 / 255 / 255 * ( 255 - 1 ) - 1;

  constexpr size_t karatsuba_min_size = 50;

  const BigInteger reductionBarrettPrecomputation( const BigInteger& mod )
  {
    size_t                rbc = mod.DigitCount() * 2 + 1;
    std::vector<GR::u8>   ba( rbc );
    ba.back() = 1;
    return BigInteger( ba, rbc ) / mod;
  }

  constexpr size_t  KarySize = 256;
  constexpr GR::u8  KaryMask = 0xFF;
  constexpr size_t  KaryBits = 8;

}



const BigInteger BigInteger::BigInteger0;
const BigInteger BigInteger::BigInteger1( 1 );



BigInteger::BigInteger() 
  : m_Data( 1 ), 
    m_Negative( false ) 
{
}



BigInteger::BigInteger( size_t NumBytesOfBuffer, int )
  : m_Data( (size_t)NumBytesOfBuffer ),
    m_Negative( false ) 
{
}



BigInteger::BigInteger( const BigInteger& BigInt ) 
  : m_Data( BigInt.m_Data ), 
    m_Negative( BigInt.m_Negative ) 
{
}



BigInteger::BigInteger( BigInteger&& BigInt ) noexcept 
  : m_Data( std::move( BigInt.m_Data ) ), 
    m_Negative( BigInt.m_Negative ) 
{
}



BigInteger::BigInteger( const ByteBuffer& Data ) noexcept 
  : m_Negative( false )
{
  m_Data.resize( Data.Size() );
  for ( size_t i = 0; i < Data.Size(); ++i )
  {
    m_Data[m_Data.size() - i - 1] = Data.ByteAt( i );
  }
  Normalize( );
}



BigInteger::BigInteger( std::vector<GR::u8>&& v ) noexcept 
  : m_Data( std::move( v ) ), 
    m_Negative( false ) 
{ 
  Normalize(); 
}



BigInteger::BigInteger( const std::vector<GR::u8>& v ) 
  : m_Data( v ), 
    m_Negative( false ) 
{ 
  Normalize(); 
}



BigInteger::BigInteger( std::vector<GR::u8>&& v, size_t rbc ) 
  : m_Data( std::move( v ) ), 
    m_Negative( false ) 
{ 
  m_Data.resize( rbc ); 
}



BigInteger::BigInteger( const std::vector<GR::u8>& v, size_t rbc ) 
  : m_Data( v.begin(), v.begin() + rbc ), 
    m_Negative( false ) 
{
}



BigInteger::BigInteger( int64_t x ) 
  : m_Data( sizeof( int64_t ) ), 
    m_Negative( x < 0 )
{
  if ( m_Negative )
  {
    x = -x;
  }
  for ( size_t i = 0; i < m_Data.size(); i++, x >>= 8 )
  {
    m_Data[i] = static_cast<GR::u8>( x );
  }
  Normalize();
}



BigInteger::BigInteger( const GR::String& NumericString, int Base ) 
  : m_Negative( false )
{
  int     offset = 0;
  m_Negative = NumericString.StartsWith( "-" );
  if ( m_Negative )
  {
    ++offset;
  }
  if ( Base <= 10 )
  {
    BigInteger BigInt( 1, 0 );

    for ( size_t i = offset; i < NumericString.length(); ++i )
    {
      char c = NumericString[i];

      if ( ( c < '0' )
      ||   ( c > '0' + Base - 1 ) )
      {
        continue;
      }
      BigInt.PerformMultiply( Base );
      BigInt += BigInteger( (GR::u64)( c - '0' ) );
    }
    m_Data = move( BigInt.m_Data );
    return;
  }

  if ( Base != 16 )
  {
    throw new std::exception( "alles doof" );
  }

  // radix: hex
  size_t length = NumericString.length();
  m_Data.resize( ( length - offset + 2 - 1 ) / 2 );

  size_t index = m_Data.size() - 1;
  size_t shift = 2 - ( m_Data.size() * 2 - length + offset );

  for ( size_t i = offset; i < length; ++i )
  {
    GR::u8 d;
    if ( ( NumericString[i] >= '0' )
    &&   ( NumericString[i] <= '9' ) )
    {
      d = NumericString[i] - '0';
    }
    else if ( ( NumericString[i] >= 'A' )
    &&        ( NumericString[i] <= 'F' ) )
    {
      d = NumericString[i] - 'A' + 10;
    }
    else if ( ( NumericString[i] >= 'a' )
    &&        ( NumericString[i] <= 'f' ) )
    {
      d = NumericString[i] - 'a' + 10;
    }
    else
    {
      throw std::invalid_argument( std::string( "BigInteger constructor: invalid char '" ) + NumericString[i] + "' in HEX string" );
    }
    m_Data[index] = ( m_Data[index] << 4 ) | d;
    --shift;
    if ( shift == 0 )
    {
      shift = 2;
      --index;
    }
  }
  Normalize();
}



void BigInteger::Normalize() noexcept
{
  while ( ( m_Data.size() > 1 )
  &&      ( m_Data.back() == 0 ) )
  {
    m_Data.pop_back();
  }
  if ( ( m_Data.size() == 1 )
  &&   ( m_Data[0] == 0 ) )
  {
    m_Negative = false;
  }
}



void BigInteger::swap( BigInteger& BigInteger ) noexcept
{
  m_Data.swap( BigInteger.m_Data );
}



BigInteger& BigInteger::operator=( const BigInteger& BigInt )
{
  if ( this == &BigInt )
  {
    return *this;
  }

  m_Data      = BigInt.m_Data;
  m_Negative  = BigInt.m_Negative;
  return *this;
}



BigInteger& BigInteger::operator=( BigInteger&& BigInt ) noexcept
{
  if ( this == &BigInt )
  {
    return *this;
  }

  m_Data      = move( BigInt.m_Data );
  m_Negative  = BigInt.m_Negative;
  return *this;
}



const BigInteger BigInteger::operator+( const BigInteger& BigInt ) const
{
  return ( std::move( BigInteger( *this ) ) += BigInt );
}



void BigInteger::PerformPlus1()
{
  for ( auto& i : m_Data )
  {
    ++i;
    if ( i != 0 )
    {
      return;
    }
  }
  m_Data.emplace_back( 1 );
}



void BigInteger::PerformMinus1()
{
  for ( auto& i : m_Data )
  {
    if ( i )
    {
      --i;
      return;
    }
    --i;
  }
}



BigInteger& BigInteger::operator++()
{
  if ( m_Negative )
  {
    PerformMinus1();
    if ( IsZero() )
    {
      m_Negative = false;
    }
  }
  else
  {
    PerformPlus1();
  }
  return *this;
}



void BigInteger::PerformPlus( const BigInteger& BigInt )
{
  m_Data.resize( std::max( m_Data.size(), BigInt.m_Data.size() ) );

  GR::u16 sum = 0;
  for ( size_t pos = 0; pos < BigInt.m_Data.size(); pos++ )
  {
    sum = ( sum >> 8 ) + m_Data[pos] + BigInt.m_Data[pos];
    m_Data[pos] = (GR::u8)sum;
  }

  for ( size_t pos = BigInt.m_Data.size(); pos < m_Data.size(); pos++ )
  {
    sum = ( sum >> 8 ) + m_Data[pos];
    m_Data[pos] = (GR::u8)sum;
  }

  sum >>= 8;
  if ( sum )
  {
    m_Data.emplace_back( (GR::u8)sum );
  }
}



BigInteger& BigInteger::operator/=( const BigInteger& BigInt )
{
  *this = *this / BigInt;

  return *this;
}



BigInteger& BigInteger::operator+=( const BigInteger& BigInt )
{
  if ( m_Negative != BigInt.m_Negative )
  {
    /*
    if ( BigInt.m_Negative )
    {

      *this -= BigInt;
      return *this;
    }*/
    BigInteger temp( BigInt );

    temp.PerformSubtract( *this );
    *this = temp;
    return *this;
  }

  PerformPlus( BigInt );
  return *this;
}



const BigInteger BigInteger::operator&( const BigInteger& BigInt ) const
{
  size_t  dataSize = std::min( m_Data.size(), BigInt.m_Data.size() );

  BigInteger  result( *this );
  result.m_Data.resize( dataSize );

  for ( size_t i = 0; i < dataSize; ++i )
  {
    result.m_Data[i] = m_Data[i] & BigInt.m_Data[i];
  }
  return result;
}



const BigInteger BigInteger::operator|( const BigInteger& BigInt ) const
{
  size_t  dataSize = std::max( m_Data.size(), BigInt.m_Data.size() );

  BigInteger  result( *this );
  result.m_Data.resize( dataSize );

  for ( size_t i = 0; i < dataSize; ++i )
  {
    if ( i >= m_Data.size() )
    {
      result.m_Data[i] = BigInt.m_Data[i];
    }
    else if ( i >= BigInt.m_Data.size() )
    {
      result.m_Data[i] = m_Data[i];
    }
    else
    {
      result.m_Data[i] = m_Data[i] | BigInt.m_Data[i];
    }
  }
  return result;
}



const BigInteger BigInteger::operator-( const BigInteger& BigInt ) const
{
  return std::move( BigInteger( *this ) -= BigInt );
}



BigInteger& BigInteger::operator--() noexcept
{
  if ( IsZero() )
  {
    PerformMinus1();
    m_Negative = true;
  }
  else if ( m_Negative )
  {
    PerformPlus1();
  }
  else
  {
    PerformMinus1();
  }

  Normalize();
  return *this;
}



void BigInteger::PerformSubtract( const BigInteger& BigInt )
{
  if ( m_Data.size() < BigInt.m_Data.size() )
  {
    m_Data.resize( BigInt.m_Data.size() );
  }

  bool flag = 0;
  size_t pos = 0;
  for ( ; pos < BigInt.m_Data.size() && pos < m_Data.size(); ++pos )
  {
    GR::i16 res = (GR::i16)( m_Data[pos] ) - BigInt.m_Data[pos] - flag;
    m_Data[pos] = (GR::u8)( res );
    flag = ( res < 0 );
  }

  for ( ; flag && pos < m_Data.size(); ++pos )
  {
    if ( m_Data[pos] )
    {
      flag = false;
    }
    --m_Data[pos];
  }
}



BigInteger& BigInteger::operator-=( const BigInteger& BigInt )
{
  if ( m_Negative != BigInt.m_Negative )
  {
    if ( BigInt.AbsoluteCompare( *this ) > 0 )
    {
      // amount of BigInt is greater than I am
      BigInteger   higher;
      higher.m_Data = BigInt.m_Data;

      higher.PerformPlus( *this );
      higher.m_Negative = m_Negative;

      *this = higher;
      Normalize();
      return *this;
    }
    // amount of me is greather tha BigInt
    BigInteger   higher;
    higher.m_Data = m_Data;

    higher.PerformPlus( BigInt );

    higher.m_Negative = m_Negative;

    *this = higher;
    Normalize();
    return *this;
  }
  if ( *this >= BigInt )
  {
    PerformSubtract( BigInt );
  }
  else
  {
    BigInteger  temp( BigInt );

    temp.PerformSubtract( *this );
    *this = temp;
    m_Negative = true;
  }
  Normalize();
  return *this;
}



BigInteger BigInteger::MultiplyBy8( size_t t ) const
{
  if ( t == 0 )
  {
    return *this;
  }

  BigInteger res( m_Data.size() + t, 0 );
  for ( size_t i = 0; i < m_Data.size(); ++i )
  {
    res.m_Data[i + t] = m_Data[i];
  }
  res.Normalize();
  return res;
}



BigInteger BigInteger::DivideBy8( size_t t ) const
{
  if ( t >= m_Data.size() )
  {
    return BigInteger::BigInteger0;
  }
  return std::move( std::vector<GR::u8>( m_Data.begin() + t, m_Data.end() ) );
}



BigInteger BigInteger::ModBy8( size_t t ) const
{
  if ( t >= m_Data.size() )
  {
    return *this;
  }
  return std::move( std::vector<GR::u8>( m_Data.begin(), m_Data.begin() + t ) );
}



const BigInteger BigInteger::Multiply( const GR::u8& multiplier ) const
{
  return std::move( BigInteger( *this ).PerformMultiply( multiplier ) );
}



BigInteger& BigInteger::PerformMultiply( const GR::u8& multiplier )
{
  if ( multiplier == 0 )
  {
    m_Data.resize( 1 );
    m_Data.front() = 0;
    return *this;
  }

  GR::u16 curr = 0;
  for ( auto& i : m_Data )
  {
    i = (GR::u8)( curr += (GR::u16)( i * multiplier ) );
    curr >>= 8;
  }
  if ( curr )
  {
    m_Data.emplace_back( (GR::u8)curr );
  }
  return *this;
}



const BigInteger BigInteger::Divide( const GR::u8& Divisor ) const
{
  return std::move( BigInteger( *this ).Divide( Divisor ) );
}



BigInteger& BigInteger::Divide( const GR::u8& Divisor )
{
  if ( Divisor == 0 )
  {
    throw new std::logic_error( "Divide by zero" );
  }

  GR::u16 curr = 0;
  for ( size_t i = m_Data.size(); i; --i )
  {
    curr = ( curr % Divisor << 8 ) + m_Data[i - 1];
    m_Data[i - 1] = curr / Divisor;
  }

  Normalize();
  return *this;
}



const BigInteger BigInteger::ModuloBase( const GR::u8& Divisor )const
{
  if ( Divisor == 0 )
  {
    throw new std::logic_error( "Divide by zero" );
  }

  GR::u16 curr = 0;
  for ( int i = (int)m_Data.size() - 1; i >= 0; --i )
  {
    curr <<= 8;
    curr += m_Data[i];
    curr %= Divisor;
  }

  BigInteger result;
  result.m_Data[0] = (GR::u8)curr;
  return result;
}



BigInteger& BigInteger::modbaseappr( const GR::u8& Divisor )
{
  if ( Divisor == 0 )
  {
    throw new std::logic_error( "Divide by zero" );
  }

  GR::u16 curr = 0;
  for ( int i = (int)m_Data.size() - 1; i >= 0; --i )
  {
    curr <<= 8;
    curr += m_Data[i];
    m_Data[i] = curr / Divisor;
    curr %= Divisor;
  }

  m_Data.resize( 1 );
  m_Data[0] = (GR::u8)curr;
  return *this;
}



const BigInteger BigInteger::operator*( const BigInteger& BigInteger ) const
{
  return KaratsubaMultiply( BigInteger );
}



const BigInteger BigInteger::NaiveMultiply( const BigInteger& BigInt ) const
{
  // classical O(n*n) multiplication.
  // b * a is slightly faster than a * b
  const BigInteger& b = m_Data.size() > BigInt.m_Data.size() ? *this : BigInt;
  const BigInteger& a = m_Data.size() > BigInt.m_Data.size() ? BigInt : *this;

  if ( a.m_Data.size() == 1 )
  {
    return b.Multiply( a.m_Data.front() );
  }

  BigInteger result( a.m_Data.size() + b.m_Data.size(), 0 );
  for ( size_t i = 0; i < b.m_Data.size(); ++i )
  {
    GR::u16 curr = 0;
    GR::u16 x = b.m_Data[i];
    for ( size_t j = 0; j < a.m_Data.size(); ++j )
    {
      curr = ( curr >> 8 ) + result.m_Data[i + j] + x * a.m_Data[j];
      result.m_Data[i + j] = (GR::u8)curr;
    }
    result.m_Data[i + a.m_Data.size()] = curr >> 8;
  }
  result.Normalize();
  return result;
}



const BigInteger BigInteger::FastMultiply( const BigInteger& BigInt ) const
{
  size_t n = m_Data.size();
  size_t m = BigInt.m_Data.size();

  BigInteger result( n + m, 0 );

  GR::u32 t = 0;
  for ( size_t s = 0; s < m + n - 1; s++ )
  {
    size_t end_index = std::min( n - 1, s );
    size_t start_index = s >= m ? s - m + 1 : 0;
    for ( size_t i = start_index, j = s - start_index; i <= end_index; i++, --j )
    {
      t += (GR::u16)( m_Data[i] ) * BigInt.m_Data[j];
    }

    result.m_Data[s] = t;
    t = t >> 8;
  }

  result.m_Data.back() = t;
  result.Normalize();

  result.m_Negative = ( m_Negative != BigInt.m_Negative );
  return result;
}



inline std::vector<GR::u8> karatsubaSum( const std::vector<GR::u8>& u, size_t start, size_t n, size_t m )
{
  std::vector<GR::u8> result;
  result.reserve( m + 1 );

  GR::u16 sum = 0;
  for ( size_t pos = 0; pos < n; ++pos )
  {
    sum += u[start + pos] + u[start + pos + n];
    result.emplace_back( (GR::u8)sum );
    sum >>= 8;
  }
  if ( n != m )
  {
    sum += u[start + n + n];
    result.emplace_back( (GR::u8)sum );
    sum >>= 8;
  }
  result.emplace_back( (GR::u8)sum );
  return result;
}



std::vector<GR::u8> karatsubaRecursive( const std::vector<GR::u8>& U, const std::vector<GR::u8>& V, size_t start, size_t count )
{
  const size_t len = count, n = len / 2, m = len - n;

  if ( n < karatsuba_min_size )
  {
    std::vector<GR::u8> result;
    result.reserve( len + len );

    GR::u32 t = 0;
    for ( size_t s = 0; s < len + len - 1; s++ )
    {

      size_t end_index = std::min( len - 1, s ) + start;
      size_t start_index = s >= len ? s - len + 1 : 0;
      for ( size_t i = start_index + start, j = s - start_index + start; i <= end_index; i++, --j )
      {
        t += (GR::u16)( U[i] ) * V[j];
      }

      result.emplace_back( t );
      t = t >> 8;
    }
    result.emplace_back( t );
    return result;
  }

  const std::vector<GR::u8>& u01 = karatsubaSum( U, start, n, m );
  const std::vector<GR::u8>& v01 = karatsubaSum( V, start, n, m );

  std::vector<GR::u8> A = std::move( karatsubaRecursive( U, V, start + n, m ) );
  std::vector<GR::u8> B = std::move( karatsubaRecursive( U, V, start, n ) );
  const std::vector<GR::u8> C = std::move( karatsubaRecursive( u01, v01, 0, m + 1 ) );

  std::vector<GR::u8> result = B;
  result.resize( len + len );

  for ( size_t i = 0; i < A.size(); ++i )
  {
    result[i + n + n] = A[i];
  }

  const size_t abcSize = m + m + 2;
  A.resize( abcSize );
  B.resize( abcSize );

  GR::i16 sum = 0;
  for ( size_t i = 0; i < abcSize; ++i )
  {
    sum += result[i + n];
    sum += C[i];
    sum -= A[i];
    sum -= B[i];
    result[i + n] = (GR::u8)sum;
    sum >>= 8;
  }
  for ( size_t i = n + abcSize; i < result.size(); ++i )
  {
    sum += result[i];
    result[i] = (GR::u8)sum;
    sum >>= 8;
  }
  return result;
}



const BigInteger BigInteger::KaratsubaMultiply( const BigInteger& BigInteger ) const
{
  size_t x = m_Data.size(), y = BigInteger.m_Data.size(), len = std::max( x, y );

  if ( std::min( x, y ) < karatsuba_min_size )
  {
    return FastMultiply( BigInteger );
  }

  std::vector<GR::u8> U = m_Data;
  std::vector<GR::u8> V = BigInteger.m_Data;
  U.resize( len ); V.resize( len );

  return karatsubaRecursive( U, V, 0, len );
}



void BigInteger::DivideModulo( const BigInteger& BigInt, BigInteger& div, BigInteger& mod ) const
{
  if ( BigInt.IsZero() )
  {
    throw new std::logic_error( "Divide by zero" );
  }

  if ( BigInt.m_Data.size() == 1 )
  {
    div = Divide( BigInt.m_Data.front() );
    mod = ModuloBase( BigInt.m_Data.front() );

    div.m_Negative = ( BigInt.m_Negative != m_Negative );
    mod.m_Negative = m_Negative;
    return;
  }

  if ( *this < BigInt )
  {
    div = BigInteger::BigInteger0;
    mod = *this;
    return;
  }

  BigInteger dividend( *this ), divider( BigInt );

  GR::u8 d = 256 / ( BigInt.m_Data.back() + 1 );
  if ( d != 1 )
  {
    dividend.PerformMultiply( d );
    divider.PerformMultiply( d );
  }

  size_t n = divider.m_Data.size();
  size_t m = dividend.m_Data.size() - n + 1;

  dividend.m_Data.resize( dividend.m_Data.size() + 2 );
  divider.m_Data.resize( divider.m_Data.size() + 1 );

  div.m_Data.resize( m + 1 );

  std::vector<GR::u8> temp( n + 1 );
  for ( int j = (int)m; j >= 0; --j )
  {
    GR::u16 q = ( dividend.m_Data[j + n] * 256 + dividend.m_Data[j + n - 1] ) / divider.m_Data[n - 1];
    GR::u16 r = ( dividend.m_Data[j + n] * 256 + dividend.m_Data[j + n - 1] ) % divider.m_Data[n - 1];

    if ( ( q == 256 )
    ||   ( q * divider.m_Data[n - 2] > 256 * r + dividend.m_Data[j + n - 2] ) )
    {
      --q;
      r += divider.m_Data[n - 1];
      if ( ( r < 256 )
      &&   ( q * divider.m_Data[n - 2] > 256 * r + dividend.m_Data[j + n - 2] ) )
      {
        --q;
      }
    }

    if ( !q )
    {
      div.m_Data[j] = 0;
      continue;
    }

    GR::i32 x = 0;
    for ( size_t i = 0; i < n; ++i )
    {
      x += dividend.m_Data[j + i];
      x -= q * divider.m_Data[i];
      dividend.m_Data[j + i] = x;
      x >>= 8;
    }
    x += dividend.m_Data[j + n];
    dividend.m_Data[j + n] = x;

    // If `x' is negative, than `q' is too large.
    // Decrement `q' and update `dividend'.
    if ( x < 0 )
    {
      --q;
      x = 0;
      for ( size_t i = 0; i < n; ++i )
      {
        x += dividend.m_Data[j + i];
        x += divider.m_Data[i];
        dividend.m_Data[j + i] = x;
        x >>= 8;
      }
      x += dividend.m_Data[j + n];
      dividend.m_Data[j + n] = x;
    }

    div.m_Data[j] = (GR::u8)q;
  }

  div.Normalize();
  dividend.Normalize();

  if ( d != 1 )
  {
    dividend.Divide( d );
  }

  div.m_Negative = ( BigInt.m_Negative != m_Negative );
  dividend.m_Negative = ( BigInt.m_Negative != m_Negative );
  mod = std::move( dividend );
}



const BigInteger BigInteger::operator/( const BigInteger& BigInt ) const
{
  BigInteger div, mod;
  DivideModulo( BigInt, div, mod );
  return div;
}



const BigInteger BigInteger::operator%( const BigInteger& BigInt ) const
{
  BigInteger div, mod;
  DivideModulo( BigInt, div, mod );
  return mod;
}



const BigInteger BigInteger::operator^( const BigInteger& BigInt ) const
{
  return Power( BigInt );
}



bool BigInteger::operator<( const BigInteger& BigInt ) const noexcept
{
  if ( m_Negative != BigInt.m_Negative )
  {
    return m_Negative;
  }
  if ( m_Data.size() > BigInt.m_Data.size() )
  {
    return false;
  }
  if ( m_Data.size() < BigInt.m_Data.size() )
  {
    return true;
  }
  for ( int i = (int)m_Data.size() - 1; i >= 0; i-- )
  {
    if ( m_Data[i] > BigInt.m_Data[i] )
    {
      return false;
    }
    if ( m_Data[i] < BigInt.m_Data[i] )
    {
      return true;
    }
  }

  return false;
}



bool BigInteger::operator>( const BigInteger& BigInt ) const noexcept
{
  return BigInt < *this;
}



bool BigInteger::operator<=( const BigInteger& BigInt ) const noexcept
{
  return !( *this > BigInt );
}



bool BigInteger::operator>=( const BigInteger& BigInt ) const noexcept
{
  return !( *this < BigInt );
}



bool BigInteger::operator==( const BigInteger& BigInt ) const noexcept
{
  return ( m_Negative == BigInt.m_Negative )
      && ( m_Data == BigInt.m_Data );
}



bool BigInteger::operator!=( const BigInteger& BigInt ) const noexcept
{
  return !( *this == BigInt );
}



int BigInteger::AbsoluteCompare( const BigInteger& BigInt ) const noexcept
{
  if ( m_Data.size() < BigInt.m_Data.size() )
  {
    return -1;
  }
  else if ( m_Data.size() > BigInt.m_Data.size() )
  {
    return 1;
  }
  // need to compare data
  for ( size_t i = 0; i < m_Data.size(); ++i )
  {
    if ( m_Data[i] > BigInt.m_Data[i] )
    {
      return 1;
    }
    else if ( m_Data[i] < BigInt.m_Data[i] )
    {
      return -1;
    }
  }
  return 0;
}



GR::u8 BigInteger::operator []( size_t index ) const noexcept
{
  return m_Data[index];
}



size_t BigInteger::DigitCount() const noexcept
{
  return m_Data.size();
}



size_t BigInteger::BitCount() const noexcept
{
  size_t x = 0;
  GR::u8 value = m_Data.back();
  while ( value )
  {
    ++x;
    value >>= 1;
  }
  return ( m_Data.size() - 1 ) * 8 + x;
}



const BigInteger BigInteger::operator>>( size_t Shift ) const
{
  return std::move( BigInteger( *this ) >>= Shift );
}



const BigInteger BigInteger::operator<<( size_t Shift ) const
{
  return std::move( BigInteger( *this ) <<= Shift );
}



BigInteger& BigInteger::operator>>=( size_t Shift )
{
  if ( Shift == 0 )
  {
    return *this;
  }
  size_t baseshift = Shift / 8;
  size_t realshift = Shift - baseshift * 8;

  if ( realshift == 0 )
  {
    return *this = DivideBy8( baseshift );
  }

  if ( baseshift >= m_Data.size() )
  {
    m_Data.resize( 0 );
    m_Data[0] = 0;
    return *this;
  }

  for ( size_t i = 0; i < m_Data.size() - baseshift - 1; ++i )
  {
    m_Data[i] = ( m_Data[i + baseshift] >> realshift ) 
              | ( m_Data[i + baseshift + 1] << ( 8 - realshift ) );
  }
  m_Data[m_Data.size() - baseshift - 1] = m_Data.back() >> realshift;
  m_Data.resize( m_Data.size() - baseshift );

  Normalize();
  return *this;
}



BigInteger& BigInteger::operator<<=( size_t Shift )
{
  if ( Shift == 0 )
  {
    return *this;
  }

  size_t baseshift = Shift / 8;
  size_t realshift = Shift - baseshift * 8;

  if ( realshift == 0 )
  {
    return *this = MultiplyBy8( baseshift );
  }

  m_Data.resize( m_Data.size() + baseshift + 1, 0 );
  m_Data.back() = m_Data.back() >> ( 8 - realshift );
  for ( size_t i = m_Data.size() - 1; i; --i )
  {
    m_Data[i + baseshift] = ( m_Data[i - 1] >> ( 8 - realshift ) ) 
                          | ( m_Data[i] << realshift );
  }
  m_Data[baseshift] = m_Data[0] << realshift;
  Normalize();
  return *this;
}



BigInteger BigInteger::reductionBarrett( const BigInteger& mod, const BigInteger& mu ) const
{
  size_t k = mod.m_Data.size();
  if ( k * 2 < m_Data.size() )
  {
    return ( *this ) % mod;
  }

  BigInteger  q1 = DivideBy8( k - 1 ), 
              q2 = q1 * mu, 
              q3 = q2.DivideBy8( k + 1 );

  BigInteger  r1 = ModBy8( k + 1 ), 
              r2 = ( q3 * mod ).ModBy8( k + 1 );

  r1 -= r2;
  while ( r1 >= mod )
  {
    r1 -= mod;
  }
  return r1;
}



BigInteger BigInteger::Power( GR::u64 power ) const
{
  if ( power == 0 )
  {
    return BigInteger::BigInteger1;
  }

  BigInteger res( BigInteger::BigInteger1 );
  BigInteger t = *this;

  do
  {
    if ( power & 1 )
    {
      res = res * t;
    }
    power >>= 1;
    if ( power )
    {
      t = t.qrt();
    }
  }
  while ( power );

  return res;
}



BigInteger BigInteger::Power( const BigInteger& power ) const
{
  if ( power.IsZero() )
  {
    return BigInteger::BigInteger1;
  }

  BigInteger  res( BigInteger::BigInteger1 );
  BigInteger  t = *this;
  BigInteger  tempPower( power );

  do
  {
    if ( !tempPower.IsEven() )
    {
      res = res * t;
    }
    tempPower >>= 1;
    if ( !tempPower.IsZero() )
    {
      t = t.qrt();
    }
  }
  while ( !tempPower.IsZero() );

  return res;
}



BigInteger BigInteger::ModuloExponentiation( GR::u64 power, const BigInteger& mod ) const
{
  if ( power == 0 )
  {
    return BigInteger::BigInteger1;
  }

  BigInteger res( BigInteger::BigInteger1 );
  BigInteger t = *this % mod;

  do
  {
    if ( power & 1 )
    {
      res = res * t % mod;
    }
    power >>= 1;
    if ( power )
    {
      t = t.qrt() % mod;
    }
  }
  while ( power );

  return res;
}



BigInteger BigInteger::ModuloExponentiation( const BigInteger& power, const BigInteger& mod ) const
{
  return expRightToLeft( power, mod );
}



BigInteger BigInteger::powmodBarrett( const BigInteger& power, const BigInteger& mod ) const
{
  if ( power.IsZero() )
  {
    return BigInteger::BigInteger1;
  }

  BigInteger mu = reductionBarrettPrecomputation( mod );
  BigInteger res( BigInteger::BigInteger1 );
  BigInteger t = ( *this ) % mod;

  int len = (int)power.BitCount();
  GR::u8 mask = 1;
  const GR::u8* curr = &*power.m_Data.begin();
  for ( int i = 0; i < len; i++ )
  {
    if ( !mask )
    {
      mask = 1;
      ++curr;
    }
    if ( ( *curr ) & mask )
    {
      res = ( res * t ).reductionBarrett( mod, mu );
    }

    if ( i + 1 != len )
    {
      t = t.qrt().reductionBarrett( mod, mu );
    }
    mask <<= 1;
  }
  return res;
}



BigInteger BigInteger::SquareRoot() const
{
  if ( IsZero() )
  {
    return BigInteger::BigInteger1;
  }

  size_t rbc2 = ( m_Data.size() + 1 ) / 2 + 1;
  BigInteger x( rbc2, 0 );

  x.m_Data.back() = 1;
  BigInteger x0;

  do
  {
    x0 = x;
    x += *this / x;
    x >>= 1;
  }
  while ( x0 > x );

  return x0;
}



BigInteger BigInteger::qrt() const
{
  if ( m_Data.size() < max_size_fast_mul )
  {
    return fast_qrt();
  }

  BigInteger res( 2 * m_Data.size() + 1, 0 );
  for ( size_t i = 0; i < m_Data.size(); ++i )
  {
    GR::u32 cuv = res.m_Data[2 * i] + (GR::u16)( m_Data[i] ) * m_Data[i];
    res.m_Data[2 * i] = cuv;
    for ( size_t j = i + 1; j < m_Data.size(); ++j )
    {
      cuv = (GR::u32)( res.m_Data[i + j] ) +
        ( ( (GR::u32)( m_Data[i] ) * m_Data[j] ) << 1 ) +
        ( cuv >> 8 );
      res.m_Data[i + j] = cuv;
    }
    cuv = res.m_Data[i + m_Data.size()] + ( cuv >> 8 );
    res.m_Data[i + m_Data.size()] = cuv;
    res.m_Data[i + m_Data.size() + 1] += ( cuv >> 8 );
  }

  res.Normalize();
  return res;
}



BigInteger BigInteger::fast_qrt() const
{
  size_t n = m_Data.size();

  BigInteger result( n + n, 0 );

  GR::u32 t = 0;
  for ( size_t s = 0; s < n + n - 1; s++ )
  {
    size_t start_index = s >= n ? s - n + 1 : 0;
    size_t end_index = std::min( n - 1, s );
    while ( start_index < end_index )
    {
      GR::u16 m = (GR::u16)( m_Data[start_index] ) * m_Data[end_index];
      t += m;
      t += m;
      ++start_index;
      --end_index;
    }
    if ( start_index == end_index )
    {
      t += (GR::u16)( m_Data[start_index] ) * m_Data[end_index];
    }

    result.m_Data[s] = t;
    t = t >> 8;
  }

  result.m_Data[n + n - 1] = t;
  result.Normalize();
  return std::move( result );
}



BigInteger BigInteger::expRightToLeft( const BigInteger& exponent, const BigInteger& mod ) const
{
  if ( exponent.IsZero() )
  {
    return BigInteger::BigInteger1;
  }

  BigInteger result( BigInteger::BigInteger1 );
  BigInteger S = *this % mod;

  size_t len = exponent.BitCount();
  GR::u8 mask = 1;

  const GR::u8* curr = &*exponent.m_Data.begin();
  for ( size_t i = 0; i < len; i++ )
  {
    if ( !mask )
    {
      mask = 1;
      ++curr;
    }
    if ( *curr & mask )
    {
      result = std::move( result * S % mod );
    }

    if ( i + 1 != len )
    {
      S = std::move( S.qrt() % mod );
    }
    mask <<= 1;
  }
  return result;
}



std::vector<BigInteger> BigInteger::expLeftToRightKaryPrecomputation( const BigInteger& mod ) const
{
  BigInteger g = *this % mod;
  std::vector <BigInteger> garr( KarySize );
  garr[0] = BigInteger::BigInteger1;
  for ( size_t i = 1; i < KarySize; i++ )
  {
    garr[i] = garr[i - 1] * g % mod;
  }
  return garr;
}



BigInteger BigInteger::expLeftToRightKary( const BigInteger& exponent, const BigInteger& mod, const std::vector<BigInteger>& g ) const
{
  if ( exponent.IsZero() )
  {
    return BigInteger::BigInteger1;
  }

  BigInteger A( BigInteger::BigInteger1 );
  for ( int i = (int)exponent.m_Data.size() - 1; i >= 0; i-- )
  {
    GR::u8 value = exponent.m_Data[i];
    for ( size_t b = 0; b < 1; --b )
    {
      for ( size_t k = 0; k < KaryBits; k++ )
      {
        A = A.qrt() % mod;
      }
      A = A * g[( value >> KaryBits * b ) & KaryMask] % mod;
    }
  }
  return A;
}



std::vector<BigInteger> BigInteger::expLeftToRightKaryVarPrecomputation( const BigInteger& mod, size_t K ) const
{
  int Kmax = ( 1 << K );
  BigInteger g = *this % mod;

  std::vector <BigInteger> garr( Kmax );
  garr[0] = BigInteger::BigInteger1;

  for ( int i = 1; i < Kmax; i++ )
  {
    garr[i] = garr[i - 1] * g % mod;
  }

  return std::move( garr );
}



BigInteger BigInteger::expLeftToRightKaryVar( const BigInteger& exponent, const BigInteger& mod, const std::vector<BigInteger>& g, size_t K ) const
{
  if ( exponent.IsZero() )
  {
    return BigInteger::BigInteger1;
  }

  BigInteger A( BigInteger::BigInteger1 );

  int x = (int)K; 
  for ( size_t i = exponent.m_Data.size() * 8 - 1; i >= K; i -= K )
  {
    x = (int)i;
    for ( size_t k = 0; k < K; k++ )
    {
      A = A.qrt() % mod;
    }
    int curr = 0;
    for ( size_t k = 0; k < K; k++ )
    {
      curr <<= 1;
      curr |= exponent.GetBit( i - k ) ? 1 : 0;
    }
    A = A * g[curr] % mod;
  }

  uint32_t curr = 0;
  for ( int i = x - (int)K; i >= 0; i-- )
  {
    A = A.qrt() % mod;
    curr <<= 1;
    curr |= exponent.GetBit( i ) ? 1 : 0;
  }
  return A * g[curr] % mod;
}



std::vector <BigInteger> BigInteger::expLeftToRightKaryModifPrecomputation( const BigInteger& mod ) const
{
  BigInteger g = *this % mod;
  std::vector<BigInteger> garr( KarySize );

  garr[0] = BigInteger::BigInteger1;
  garr[1] = g; garr[2] = g.qrt() % mod;

  for ( size_t i = 1; i < KarySize / 2; i++ )
  {
    garr[2 * i + 1] = garr[2 * i - 1] * garr[2] % mod;
  }

  return std::move( garr );
}



BigInteger BigInteger::expLeftToRightKaryMod( const BigInteger& exponent, const BigInteger& mod, const std::vector<BigInteger>& g ) const
{
  if ( exponent.IsZero() )
  {
    return BigInteger::BigInteger1;
  }

  BigInteger A( BigInteger::BigInteger1 );
  for ( int i = (int)exponent.m_Data.size() - 1; i >= 0; i-- )
  {
    for ( size_t b = 0; b < 1; --b )
    {
      GR::u8 ei = ( exponent.m_Data[i] >> KaryBits * b ) & KaryMask;

      size_t hi = 0;
      if ( ei != 0 )
      {
        while ( !( ei & 1 ) )
        {
          ei >>= 1;
          hi++;
        }
      }

      for ( size_t k = 0; k + hi < KaryBits; k++ )
      {
        A = A.qrt() % mod;
      }
      A = A * g[ei] % mod;
      for ( size_t k = 0; k < hi; k++ )
      {
        A = A.qrt() % mod;
      }
    }
  }

  return A;
}



std::vector<BigInteger> BigInteger::expSlidingWindowPrecomputation( const BigInteger& mod, size_t k ) const
{
  size_t k_pow = 2 << ( k - 1 );
  std::vector <BigInteger> garr( k_pow );
  BigInteger g = *this % mod;

  garr[0] = BigInteger::BigInteger1;
  garr[1] = g;
  garr[2] = g.qrt() % mod;

  for ( size_t i = 1; i < k_pow / 2; i++ )
  {
    garr[2 * i + 1] = garr[2 * i - 1] * garr[2] % mod;
  }

  return std::move( garr );
}



BigInteger BigInteger::expSlidingWindow( const BigInteger& exponent, const BigInteger& mod, const std::vector<BigInteger>& g, size_t K ) const
{
  BigInteger A( BigInteger::BigInteger1 );
  int i = (int)exponent.BitCount() - 1;

  while ( i >= 0 )
  {
    if ( exponent.GetBit( i ) == 0 )
    {
      A = A.qrt() % mod;
      i--;
      continue;
    }
    int l = std::max( i - static_cast<int>( K ) + 1, 0 );
    while ( exponent.GetBit( l ) == 0 )
    {
      l++;
    }

    int gx = 0;
    for ( int j = i; j >= l; j-- )
    {
      gx = ( gx << 1 ) | ( exponent.GetBit( j ) ? 1 : 0 );
    }
    for ( int j = 0; j < i - l + 1; j++ )
    {
      A = A.qrt() % mod;
    }
    A = A * g[gx] % mod;
    i = l - 1;
  }

  return A;
}



std::vector<BigInteger> BigInteger::expBest_SlidePrecomp( const BigInteger& mod ) const
{
  std::vector <BigInteger> garr( KarySize );
  BigInteger mu = reductionBarrettPrecomputation( mod );
  BigInteger g = this->reductionBarrett( mod, mu );

  garr[0] = BigInteger::BigInteger1;
  garr[1] = g;
  garr[2] = g.qrt().reductionBarrett( mod, mu );

  for ( GR::u16 i = 1; i < KarySize / 2; i++ )
  {
    garr[2 * i + 1] = ( garr[2 * i - 1] * garr[2] ).reductionBarrett( mod, mu );
  }

  return garr;
}



BigInteger BigInteger::expBest_Slide( const BigInteger& exponent, const BigInteger& mod, const std::vector<BigInteger>& g ) const
{
  BigInteger A( BigInteger::BigInteger1 );
  BigInteger mu = reductionBarrettPrecomputation( mod );
  int i = (int)exponent.BitCount() - 1;
  int k = KaryBits;

  while ( i >= 0 )
  {
    if ( exponent.GetBit( i ) == 0 )
    {
      A = A.qrt().reductionBarrett( mod, mu );
      i--;
      continue;
    }

    int l = std::max( i - k + 1, 0 );
    while ( exponent.GetBit( l ) == 0 )
    {
      l++;
    }

    int gx = 0;
    for ( int j = i; j >= l; j-- )
    {
      gx = ( gx << 1 ) | ( exponent.GetBit( j ) ? 1 : 0 );
    }
    for ( int j = 0; j < i - l + 1; j++ )
    {
      A = A.qrt().reductionBarrett( mod, mu );
    }

    A = ( A * ( g[gx] ) ).reductionBarrett( mod, mu );
    i = l - 1;
  }

  return A;
}



size_t BigInteger::CountZeroDigitsFromRight() const noexcept
{
  if ( m_Data[0] & 1 )
  {
    return 0;
  }
  if ( IsZero() )
  {
    return 0;
  }

  size_t countBytes = 0;
  while ( !m_Data[countBytes] )
  {
    ++countBytes;
  }

  GR::u8 last = m_Data[countBytes];
  size_t result = countBytes * 8;
  while ( !( last & 1 ) )
  {
    ++result;
    last >>= 1;
  }
  return result;
}



bool BigInteger::GetBit( size_t index ) const noexcept
{
  if ( index >= 8 * m_Data.size() )
  {
    return false;
  }

  GR::u8 mask = 1;
  mask <<= ( index % 8 );

  if ( m_Data[index / 8] & mask )
  {
    return true;
  }
  return false;
}



GR::u64 BigInteger::ToU64() const noexcept
{
  GR::u64 result = 0;
  for ( size_t i = std::min( m_Data.size(), sizeof( GR::u64 ) ) - 1; i < sizeof( GR::u64 ); i-- )
  {
    result = ( result << 8 ) | m_Data[i];
  }
  return result;
}



const std::vector<GR::u8> BigInteger::Raw() const noexcept
{
  return m_Data;
}



bool BigInteger::IsZero() const noexcept
{
  if ( ( m_Data.size() > 1 )
  ||   ( m_Data[0] ) )
  {
    return false;
  }
  return true;
}



bool BigInteger::IsEven() const noexcept
{
  if ( m_Data[0] & 1 )
  {
    return false;
  }
  return true;
}



BigInteger BigInteger::random( size_t byteCount )
{
  std::vector<GR::u8> result( byteCount );
  if ( result.empty() )
  {
    result.emplace_back( rand() & 255 );
  }
  else
  {
    for ( auto& i : result )
    {
      i = rand() & 255;
    }
  }

  return std::move( BigInteger( std::move( result ) ) );
}



ByteBuffer BigInteger::ToBuffer() const
{
  return ByteBuffer( ToHexString() );
}



GR::String BigInteger::ToString() const
{
  std::stack<char> chars;

  BigInteger    temp( *this );
  do
  {
    chars.push( (char)( temp.ModuloBase( 10 ).ToU64() + '0' ) );
    temp.Divide( 10 );
  }
  while ( !temp.IsZero() );

  if ( m_Negative )
  {
    chars.push( '-' );
  }

  GR::String s;
  s.reserve( chars.size() + 1 );
  while ( !chars.empty() )
  {
    s = s + chars.top();
    chars.pop();
  }

  return s;
}



GR::string BigInteger::ToHexString() const
{
  GR::String result;

  result.reserve( m_Data.size() );
  const auto& rawx = Raw();
  for ( auto i = rawx.rbegin(); i != rawx.rend(); ++i )
  {
    result += GR::Convert::ToHex( *i, 2 );
  }

  return result;
}



bool BigInteger::IsPrime() const
{
  // Corner cases
  if ( ( *this <= BigInteger1 )
  ||   ( *this == BigInteger( 4 ) ) )
  {
    return false;
  }
  if ( *this <= BigInteger( 3 ) )
  {
    return true;
  }

  // Find r such that n = 2^d * r + 1 for some r >= 1
  BigInteger d = *this - BigInteger1;
  BigInteger two( 2 );
  while ( d % two == BigInteger0 )
  {
    d /= two;
  }

  // Iterate given number of 'k' times
  /*
  for ( int i = 0; i < k; i++ )
    if ( !miillerTest( d, n ) )
      return false;
      */
  return true;
}



/*
bool BigInteger::MillerRabin( ulong n )
{
  ulong[] ar;
  if ( n < 4759123141 ) ar = new ulong[]{ 2, 7, 61 };
  else if ( n < 341550071728321 ) ar = new ulong[]{ 2, 3, 5, 7, 11, 13, 17 };
  else ar = new ulong[]{ 2, 3, 5, 7, 11, 13, 17, 19, 23 };
  ulong d = n - 1;
  int s = 0;
  while ( ( d & 1 ) == 0 ) { d >>= 1; s++; }
  int i, j;
  for ( i = 0; i < ar.Length; i++ )
  {
    ulong a = Math.Min( n - 2, ar[i] );
    ulong now = pow( a, d, n );
    if ( now == 1 ) continue;
    if ( now == n - 1 ) continue;
    for ( j = 1; j < s; j++ )
    {
      now = mul( now, now, n );
      if ( now == n - 1 ) break;
    }
    if ( j == s ) return false;
  }
  return true;
}*/



BigInteger BigInteger::Absolute() const
{
  if ( !m_Negative )
  {
    return *this;
  }
  auto result = *this;
  result.m_Negative = false;

  return result;
}



BigInteger BigInteger::GreatestCommonDivisor( const BigInteger& Number1, const BigInteger& Number2 )
{
  BigInteger  temp1( Number1.Absolute() );
  BigInteger  temp2( Number2.Absolute() );

  while ( !temp2.IsZero() )
  {
    auto temp = temp1 % temp2;
    temp1 = temp2;
    temp2 = temp;
  }
  return temp1;
}



BigInteger BigInteger::LowestCommonMultiple( const BigInteger& Number1, const BigInteger& Number2 )
{
  BigInteger mul = Number1 * Number2;

  BigInteger gcd = BigInteger::GreatestCommonDivisor( Number1, Number2 );

  // calculate lcm using formula: lcm * gcd = x * y
  return mul.Absolute() / gcd;
}



bool BigInteger::ModInverse( const BigInteger& Divisor, BigInteger& Result ) const
{
  { 
    BigInteger c = BigInteger1;
    BigInteger d = BigInteger0;

    BigInteger n = Divisor;
    BigInteger b = Divisor;
    BigInteger a = *this;

    while ( !a.IsZero() )
    {
      BigInteger t = d - b / a * c;
      d = c;
      c = t;
      t = b % a;
      b = a;

      a = t;
    }
    if ( !( b - BigInteger1 ).IsZero() )
    {
      Result = BigInteger0;
      return false;
    }
    Result = ( d + n ) % n;
    return true;
  }

  /*
  BigInteger    r0 = *this;
  BigInteger    r1 = Divisor;

  BigInteger s0 = BigInteger1;     
  BigInteger s1 = BigInteger0;
  BigInteger t0 = BigInteger0;
  BigInteger t1 = BigInteger1;

  while ( !r1.IsZero() )
  {
    BigInteger q = r0 / r1;
    BigInteger r2 = r0 - q * r1;
    BigInteger s2 = s0 - q * s1;
    BigInteger t2 = t0 - q * t1;

    r0 = r1;
    r1 = r2;
    s0 = s1;
    s1 = s2;
    t0 = t1;
    t1 = t2;
  }

  //t.Normalize();
  if ( ( t1.m_Negative )
  &&   ( !Divisor.m_Negative )
  &&   ( t1.Absolute() < Divisor ) )
  {
    t1 += Divisor;
  }

  return t1;*/
  /*
  if ( Divisor.IsZero() )
  {
    return BigInteger1;
  }

  BigInteger a = *this;
  BigInteger b = Divisor;
  BigInteger x2 = BigInteger1, x1 = BigInteger0;
  BigInteger y2 = BigInteger0, y1 = BigInteger1;
  BigInteger x, y, q, r;

  while ( b > BigInteger0 )
  {
    q = a / b;
    r = a - ( b * q );
    x = x2 - ( x1 * q );
    y = y2 - ( y1 * q );
    a = b; 
    b = r; 
    x2 = x1;  
    x1 = x; 
    y2 = y1; 
    y1 = y;
  }
  if ( a != BigInteger1 )
  {
    // huh?
  }
  return x2;*/
}