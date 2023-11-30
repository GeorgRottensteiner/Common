#pragma once

#include <string>
#include <vector>

#include <Memory/ByteBuffer.h>



class BigInteger
{

  private:

    std::vector<GR::u8> m_Data;
    bool                m_Negative;

    void                Normalize() noexcept;

    void                PerformPlus1();
    void                PerformMinus1();
    void                PerformPlus( const BigInteger& BigInt );
    void                PerformSubtract( const BigInteger& BigInt );


  public:


    static const BigInteger BigInteger0;
    static const BigInteger BigInteger1;

    static BigInteger   random( size_t byte );


    BigInteger();
    BigInteger( size_t NumBytesOfBuffer, int type );
    BigInteger( const BigInteger& );
    BigInteger( BigInteger&& big_int ) noexcept;
    BigInteger( const ByteBuffer& Data ) noexcept;
    BigInteger( std::vector<GR::u8>&& ) noexcept;
    BigInteger( const std::vector<GR::u8>& );
    BigInteger( std::vector<GR::u8>&&, size_t rbc );
    BigInteger( const std::vector<GR::u8>&, size_t rbc );
    BigInteger( const GR::String& NumericString, int Base = 10 );

    explicit BigInteger( int64_t x );


    void                swap( BigInteger& big_int ) noexcept;

    BigInteger          MultiplyBy8( size_t mul ) const;
    BigInteger          DivideBy8( size_t div ) const;
    BigInteger          ModBy8( size_t mod ) const;

    const BigInteger    Multiply( const GR::u8& ) const;
    BigInteger&         PerformMultiply( const GR::u8& );

    const BigInteger    Divide( const GR::u8& ) const;
    BigInteger&         Divide( const GR::u8& );

    bool                ModInverse( const BigInteger& Divisor, BigInteger& Result ) const;
    const BigInteger    ModuloBase( const GR::u8& ) const;
    BigInteger&         modbaseappr( const GR::u8& );

    BigInteger& operator=( const BigInteger& );
    BigInteger& operator=( BigInteger&& ) noexcept;

    const BigInteger operator+( const BigInteger& ) const;
    BigInteger& operator+=( const BigInteger& );
    BigInteger& operator++();

    const BigInteger operator-( const BigInteger& ) const;
    BigInteger& operator-=( const BigInteger& );
    BigInteger& operator--() noexcept;

    const BigInteger operator*( const BigInteger& ) const;

    // naive multiplication O(n ^ 2)
    const BigInteger    NaiveMultiply( const BigInteger& ) const;

    // quick multiplication O(n ^ 2)
    const BigInteger    FastMultiply( const BigInteger& ) const;

    // Karatsuba Algorithm O(n ^ lg3))
    const BigInteger    KaratsubaMultiply( const BigInteger& ) const;

    void                DivideModulo( const BigInteger&, BigInteger& div, BigInteger& mod ) const;
    const BigInteger operator/( const BigInteger& ) const;
    BigInteger& operator/=( const BigInteger& );
    const BigInteger operator%( const BigInteger& ) const;
    const BigInteger operator^( const BigInteger& ) const;

    const BigInteger operator&( const BigInteger& ) const;
    const BigInteger operator|( const BigInteger& ) const;

    const BigInteger operator >> ( size_t shift ) const;
    BigInteger& operator >>= ( size_t shift );

    const BigInteger operator << ( size_t shift ) const;
    BigInteger& operator <<= ( size_t shift );

    bool operator <  ( const BigInteger& ) const noexcept;
    bool operator <= ( const BigInteger& ) const noexcept;
    bool operator >  ( const BigInteger& ) const noexcept;
    bool operator >= ( const BigInteger& ) const noexcept;
    bool operator == ( const BigInteger& ) const noexcept;
    bool operator != ( const BigInteger& ) const noexcept;
    GR::u8 operator [] ( size_t index_base ) const noexcept;

    // compares values without sign
    int                 AbsoluteCompare( const BigInteger& ) const noexcept;

    BigInteger          Absolute() const;

    size_t              DigitCount() const noexcept;
    size_t              BitCount()   const noexcept;

    BigInteger          reductionBarrett( const BigInteger& mod, const BigInteger& mu ) const;
    BigInteger          Power( GR::u64 ) const;
    BigInteger          Power( const BigInteger& power ) const;
    BigInteger          ModuloExponentiation( GR::u64 power, const BigInteger& mod ) const;
    BigInteger          ModuloExponentiation( const BigInteger& power, const BigInteger& mod ) const;
    BigInteger          powmodBarrett( const BigInteger& power, const BigInteger& mod ) const;

    BigInteger          SquareRoot() const;
    BigInteger          qrt()        const;
    BigInteger          fast_qrt()   const;

    bool                IsZero() const noexcept;
    bool                IsEven() const noexcept;

    BigInteger          expRightToLeft( const BigInteger& power, const BigInteger& mod ) const;

    std::vector<BigInteger> expLeftToRightKaryPrecomputation( const BigInteger& mod ) const;
    BigInteger          expLeftToRightKary( const BigInteger& exponent, const BigInteger& mod, const std::vector<BigInteger>& g ) const;

    std::vector<BigInteger> expLeftToRightKaryVarPrecomputation( const BigInteger& mod, size_t k ) const;
    BigInteger          expLeftToRightKaryVar( const BigInteger&, const BigInteger&, const std::vector<BigInteger>&, size_t k ) const;

    std::vector<BigInteger> expLeftToRightKaryModifPrecomputation( const BigInteger& ) const;
    BigInteger          expLeftToRightKaryMod( const BigInteger&, const BigInteger&, const std::vector<BigInteger>& ) const;

    std::vector<BigInteger> expSlidingWindowPrecomputation( const BigInteger&, size_t k ) const;
    BigInteger          expSlidingWindow( const BigInteger&, const BigInteger&, const std::vector<BigInteger>&, size_t k ) const;

    std::vector<BigInteger> expBest_SlidePrecomp( const BigInteger& mod ) const;
    BigInteger          expBest_Slide( const BigInteger& exponent, const BigInteger& mod, const std::vector<BigInteger>& g ) const;

    size_t              CountZeroDigitsFromRight() const noexcept;
    bool                GetBit( size_t i ) const noexcept;
    GR::u64             ToU64() const noexcept;
    const std::vector<GR::u8> Raw() const noexcept;

    ByteBuffer          ToBuffer() const;
    GR::String          ToString() const;
    GR::String          ToHexString() const;

    bool                IsPrime() const;

    static BigInteger   GreatestCommonDivisor( const BigInteger& Number1, const BigInteger& Number2 );
    static BigInteger   LowestCommonMultiple( const BigInteger& Number1, const BigInteger& Number2 );

};



