#pragma once

#include <map>
#include <string>

#include <Interface/IIOStream.h>

#include <Misc/Format.h>

#include <String/Convert.h>


namespace GR
{
  namespace Strings
  {
    class ParameterList
    {

      public:

        typedef std::map<GR::String, GR::String>     tMapParams;


      protected:

        tMapParams              m_mapParams;  


      public:



        tMapParams&             GetMap()
        {
          return m_mapParams;
        }

        const tMapParams&             GetMap() const
        {
          return m_mapParams;
        }

        bool                    HasParam( const GR::String& strParamName ) const
        {
          tMapParams::const_iterator    it( m_mapParams.find( strParamName ) );

          return ( it != m_mapParams.end() );
        }

        void                    SetParam( const GR::String& strParamName, const GR::String& Value )
        {
          m_mapParams[strParamName] = Value;
        }

        void                    SetParam( const GR::String& strParamName, const char* Text )
        {
          m_mapParams[strParamName] = GR::Convert::ToString( Text );
        }

        void                    SetParam( const GR::String& strParamName, const GR::WChar* Text )
        {
          m_mapParams[strParamName] = GR::Convert::ToUTF8( Text );
        }

        void                    SetParam( const GR::String& strParamName, const bool Value )
        {
          m_mapParams[strParamName] = Value ? "1" : "0";
        }

        void                    SetParam( const GR::String& strParamName, const GR::i32 Value )
        {
          m_mapParams[strParamName] = ( Misc::Format() << Value ).Result();
        }

        void                    SetParam( const GR::String& strParamName, const GR::f32 Value )
        {
          m_mapParams[strParamName] = ( Misc::Format( "%1::6%" ) << Value ).Result();
        }

        void                    SetParam( const GR::String& strParamName, const GR::i64 Value )
        {
          m_mapParams[strParamName] = ( Misc::Format() << Value ).Result();
        }

        void                    SetParamIP( const GR::String& strParamName, const GR::ip Value )
        {
          m_mapParams[strParamName] = ( Misc::Format() << Value ).Result();
        }

        void                    RemoveParam( const GR::String& strParamName )
        {
          tMapParams::iterator    it( m_mapParams.find( strParamName ) );
          if ( it != m_mapParams.end() )
          {
            m_mapParams.erase( it );
          }
        }



        GR::String GetParam( const GR::String& strParamName ) const
        {
          tMapParams::const_iterator    it( m_mapParams.find( strParamName ) );
          if ( it == m_mapParams.end() )
          {
            return "";
          }
          return it->second;
        }



        void Merge( const ParameterList& OtherList )
        {
          m_mapParams.insert( OtherList.m_mapParams.begin(), OtherList.m_mapParams.end() );
        }



        bool GetParamBool( const GR::String& strParamName ) const
        {
          tMapParams::const_iterator    it( m_mapParams.find( strParamName ) );
          if ( it == m_mapParams.end() )
          {
            return false;
          }
          return ( it->second == "1" );
        }



        GR::i32 GetParamI32( const GR::String& strParamName ) const
        {
          tMapParams::const_iterator    it( m_mapParams.find( strParamName ) );
          if ( it == m_mapParams.end() )
          {
            return 0;
          }
          return GR::Convert::ToI32( it->second );
        }



        GR::ip GetParamIP( const GR::String& strParamName ) const
        {
          tMapParams::const_iterator    it( m_mapParams.find( strParamName ) );
          if ( it == m_mapParams.end() )
          {
            return 0;
          }
          return GR::Convert::ToIP( it->second );
        }



        GR::f32 GetParamF32( const GR::String& strParamName ) const
        {
          tMapParams::const_iterator    it( m_mapParams.find( strParamName ) );
          if ( it == m_mapParams.end() )
          {
            return 0;
          }
          return GR::Convert::ToF32( it->second );
        }



        const GR::String operator[]( const GR::String& strParamName ) const
        {
          return GetParam( strParamName );
        }

        GR::String& operator[]( const GR::String& ParamName )
        {
          return m_mapParams[ParamName];
        }



        bool Save( IIOStream& SaveStream ) const
        {
          if ( !SaveStream.WriteSize( m_mapParams.size() ) )
          {
            return false;
          }
          tMapParams::const_iterator    it( m_mapParams.begin() );
          while ( it != m_mapParams.end() )
          {
            if ( !SaveStream.WriteString( it->first  ) )
            {
              return false;
            }
            if ( !SaveStream.WriteString( it->second ) )
            {
              return false;
            }

            ++it;
          }
          return true;
        }

        bool Load( IIOStream& LoadStream )
        {
          m_mapParams.clear();

          size_t     dwAnzahl = LoadStream.ReadSize();

          for ( size_t i = 0; i < dwAnzahl; ++i )
          {
            GR::String     strKey;
            GR::String     strValue;

            LoadStream.ReadString( strKey );
            LoadStream.ReadString( strValue );

            m_mapParams[strKey] = strValue;
          }
          return true;
        }

        void Clear()
        {
          m_mapParams.clear();
        }

    };

  };  
};