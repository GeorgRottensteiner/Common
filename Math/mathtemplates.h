#ifndef GR_MATH_TEMPLATES_H
#define GR_MATH_TEMPLATES_H

namespace math
{

  template <typename T> const T clamp( const T& value, const T& minValue, const T& maxValue )
  {
    if ( value < minValue )
    {
      return minValue;
    }
    if ( value > maxValue )
    {
      return maxValue;
    }
    return value;
  }

  template <typename T> const T maxValue( const T& Value1, const T& Value2 )
  {
    return Value1 >= Value2 ? Value1 : Value2;
  }

  template <typename T> const T minValue( const T& Value1, const T& Value2 )
  {
    return Value1 <= Value2 ? Value1 : Value2;
  }

  template <typename T> const T absolute( const T& Value )
  {
    if ( Value < 0 )
    {
      return -Value;
    }
    return Value;
  }

  // Nähere Wert an Grenzwert, gibt true zurück, wenn der Grenzwert erreicht ist
  template <typename T> bool AdjustValueToLimit( T& Value, const T Limit, const T Delta )
  {
    if ( Delta > 0 )
    {
      Value += Delta;
      if ( Value >= Limit )
      {
        Value = Limit;
        return true;
      }
      return false;
    }
    else if ( Delta < 0 )
    {
      Value += Delta;
      if ( Value <= Limit )
      {
        Value = Limit;
        return true;
      }
    }
    return( Value == Limit );
  }



  // change value towards target, do not overreach
  template <typename T> bool ApproachValue( T& Value, const T Limit, const T Delta )
  {
    if ( Value < Limit )
    {
      Value += Delta;
      if ( Value >= Limit )
      {
        Value = Limit;
        return true;
      }
      return false;
    }
    else if ( Value > Limit )
    {
      Value -= Delta;
      if ( Value <= Limit )
      {
        Value = Limit;
        return true;
      }
    }
    return ( Value == Limit );
  }



  // ist ein Wert zwischen zwei anderen, wobei diese nicht sortiert sein müssen
  template <typename T> bool ValueBetween( const T value1, const T myValue, const T value2 )
  {
    if ( ( myValue >= value1 )
    &&   ( value2 >= myValue ) )
    {
      return true;
    }
    if ( ( myValue >= value2 )
    &&   ( value1 >= myValue ) )
    {
      return true;
    }
    return false;
  }

}  // namespace math


#endif // GR_MATH_TEMPLATES_H
