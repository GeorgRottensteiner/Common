#include "Database.h"
#include "DatabaseTable.h"
#include "KeyTable.h"
#include "SimpleTable.h"

#include <IO/FileStream.h>

#include <debug/debugclient.h>



namespace GR
{

  namespace Database
  {

    Database::Database() :
      m_Language( 0 )
    {
    }



    Database::~Database()
    {
      Clear();
    }



    Database& Database::Instance()
    {
      static Database    g_Database;

      return g_Database;
    }



    void Database::Clear()
    {
      tMapTables::iterator    it( m_Tables.begin() );
      while ( it != m_Tables.end() )
      {
        delete it->second;

        ++it;
      }
      m_Tables.clear();
    }



    void Database::InsertTable( const GR::String& Desc, Table* pTable )
    {
      tMapTables::iterator    it( m_Tables.find( Desc ) );
      if ( it != m_Tables.end() )
      {
        delete it->second;
      }
      m_Tables[Desc] = pTable;
    }



    void Database::RemoveTable( Table* pTable )
    {
      if ( pTable == NULL )
      {
        return;
      }
      tMapTables::iterator    it( m_Tables.begin() );
      while ( it != m_Tables.end() )
      {
        if ( it->second == pTable )
        {
          delete pTable;
          m_Tables.erase( it );
          return;
        }
        ++it;
      }
    }



    void Database::RenameTable( Table* pTable, const GR::String& Desc )
    {
      if ( pTable == NULL )
      {
        return;
      }
      tMapTables::iterator    it( m_Tables.begin() );
      while ( it != m_Tables.end() )
      {
        Table*   pTable2 = it->second;

        if ( pTable2 == pTable )
        {
          if ( m_Tables.find( Desc ) != m_Tables.end() )
          {
            dh::Log( "Database::RenameTable denied, target name already used" );
            return;
          }
          m_Tables.erase( it );
          m_Tables[Desc] = pTable;
          return;
        }

        ++it;
      }
    }



    size_t Database::GetTableCount() const
    {
      return m_Tables.size();
    }



    Table* Database::GetTable( const GR::String& Desc )
    {
      tMapTables::iterator    it( m_Tables.find( Desc ) );
      if ( it == m_Tables.end() )
      {
        return NULL;
      }
      return it->second;
    }



    Table* Database::GetTable( size_t Index )
    {
      if ( Index >= m_Tables.size() )
      {
        return NULL;
      }
      tMapTables::iterator    it( m_Tables.begin() );
      std::advance( it, Index );

      return it->second;
    }



    GR::String Database::GetTableName( size_t Index )
    {
      if ( Index >= m_Tables.size() )
      {
        return GR::String();
      }
      tMapTables::iterator    it( m_Tables.begin() );
      std::advance( it, Index );

      return it->first;
    }



    bool Database::Load( IIOStream& Stream )
    {
      Clear();

      GR::u32       magicNumber = Stream.ReadU32();

      if ( magicNumber != 0x1234 )
      {
        Stream.Close();
        dh::Log( "Database::Load Missing Magic Number (Read %x)", magicNumber );
        return false;
      }

      GR::u32   version = Stream.ReadU32();
      if ( version >= 0x0002 )
      {
        // Sprache
        m_Language = Stream.ReadU32();
      }
      if ( version >= 0x0004 )
      {
        // Tables
        int   tableCount = Stream.ReadU32();

        for ( int i = 0; i < tableCount; ++i )
        {
          GR::u32       tableType = Stream.ReadU32();

          if ( tableType == 0 )
          {
            // Ein Simple-Table
            GR::String   key;

            Stream.ReadString( key );

            SimpleTable*    pSimpleTable = new SimpleTable();
            m_Tables[key] = pSimpleTable;

            if ( version >= 0x0003 )
            {
              pSimpleTable->Load( Stream );
            }
            else
            {
              pSimpleTable->LoadOld( Stream );
            }
          }
          else if ( tableType == 1 )
          {
            // ein Key-Table
            GR::String   key;

            Stream.ReadString( key );

            m_Tables[key] = new KeyTable();
            m_Tables[key]->Load( Stream );
          }
        }
      }
      else if ( version >= 0x0001 )
      {
        // Tables
        int   tableCount = Stream.ReadU32();

        for ( int i = 0; i < tableCount; ++i )
        {
          GR::String   key;

          Stream.ReadString( key );

          SimpleTable*    pSimpleTable = new SimpleTable();
          m_Tables[key] = pSimpleTable;

          if ( version >= 0x0003 )
          {
            pSimpleTable->Load( Stream );
          }
          else
          {
            pSimpleTable->LoadOld( Stream );
          }
        }
      }

      Stream.Close();

      return true;
    }



    void Database::Save( IIOStream& Stream )
    {
      Stream.WriteU32( 0x1234 );

      // Version
      Stream.WriteU32( 0x0004 );

      Stream.WriteU32( m_Language );

      Stream.WriteU32( (GR::u32)m_Tables.size() );

      tMapTables::iterator    it( m_Tables.begin() );
      while ( it != m_Tables.end() )
      {
        Table* pTable = it->second;

        Stream.WriteU32( pTable->TableType() );
        Stream.WriteString( it->first );
        pTable->Save( Stream );

        ++it;
      }
    }



    void Database::GetTableList( std::list<GR::String>& Tables )
    {
      tMapTables::iterator    it( m_Tables.begin() );
      while ( it != m_Tables.end() )
      {
        Tables.push_back( it->first );

        ++it;
      }
    }



    void Database::SetLangID( GR::u32 Language )
    {
      m_Language = Language;

      tMapTables::iterator    it( m_Tables.begin() );
      while ( it != m_Tables.end() )
      {
        Table* pTable = it->second;

        pTable->SetLangID( m_Language );

        ++it;
      }
    }



    GR::u32 Database::LangID()
    {
      return m_Language;
    }



  }
}
