#include <vector>
#include <iterator>

#include <Interface/IIOStream.h>

#include <Misc/Format.h>


#include "KeyTable.h"



namespace GR
{
  namespace Database
  {

    void KeyTable::SetEntry( size_t iIndex, const GR::String& strText, GR::u32 LangID )
    {
      GR::String      Key = Misc::Format() << iIndex;

      SetEntry( Key, strText, LangID );
    }



    void KeyTable::SetEntry( const GR::String& Key, const GR::String& Text, GR::u32 LangID )
    {
      if ( LangID == 0 )
      {
        LangID = m_LanguageID;
      }
      tIndex::iterator    it( m_Index.find( Key ) );
      if ( it == m_Index.end() )
      {
        // add entry
        GR::Collections::MultiColumnTable<GR::String>::MultiColumnRow   row;

        row.SubItems.push_back( Text );
        size_t    indexPos = m_Entries[LangID].AddRow( row );

        m_Index[Key] = indexPos;
        return;
      }
      m_Entries[LangID].Row( it->second ).SubItems[0] = Text;
    }



    void KeyTable::InsertEntry( size_t iIndex, const GR::String& strText )
    {
      SetEntry( iIndex, strText );
    }



    GR::String KeyTable::Entry( size_t iIndex, GR::u32 LangID ) const
    {
      return Entry( Misc::Format() << iIndex, LangID );
    }



    GR::String KeyTable::Entry( const GR::String& Key, GR::u32 LangID ) const
    {
      tIndex::const_iterator    it( m_Index.find( Key ) );
      if ( it == m_Index.end() )
      {
        return GR::String();
      }
      if ( LangID == 0 )
      {
        LangID = m_LanguageID;
      }

      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::const_iterator   itLang( m_Entries.find( LangID ) );
      if ( itLang == m_Entries.end() )
      {
        return GR::String();
      }
      const GR::Collections::MultiColumnTable<GR::String>&    table( itLang->second );

      return table.Row( it->second ).SubItems[0];
    }



    GR::String KeyTable::Key( const GR::u32 KeyIndex ) const
    {
      if ( KeyIndex >= (GR::u32)m_Index.size() )
      {
        return GR::String();
      }
      tIndex::const_iterator    it( m_Index.begin() );
      std::advance( it, KeyIndex );
      return it->first;
    }



    bool KeyTable::IsEntryValid( size_t iIndex ) const
    {
      return IsEntryValid( Misc::Format() << iIndex );
    }



    bool KeyTable::IsEntryValid( const GR::String& Key ) const
    {
      return ( m_Index.find( Key ) != m_Index.end() );
    }



    void KeyTable::DeleteEntry( size_t iIndex )
    {
      DeleteEntry( Misc::Format() << iIndex );
    }



    void KeyTable::DeleteEntry( const GR::String& Key )
    {
      tIndex::iterator    it( m_Index.find( Key ) );
      if ( it == m_Index.end() )
      {
        return;
      }

      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::iterator   itLang( m_Entries.begin() );
      while ( itLang != m_Entries.end() )
      {
        itLang->second.DeleteRow( it->second );

        ++itLang;
      }
      m_Index.erase( it );
    }



    void KeyTable::Save( IIOStream& Stream )
    {
      // Version
      Stream.WriteU32( 0x0100 );

      // Anzahl Sprachen
      Stream.WriteU32( (unsigned long)m_Entries.size() );

      // Anzahl Einträge
      Stream.WriteU32( (GR::u32)m_Index.size() );

      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::const_iterator    it( m_Entries.begin() );
      while ( it != m_Entries.end() )
      {
        const GR::Collections::MultiColumnTable<GR::String>&   table( it->second );

        // Sprache
        Stream.WriteU32( it->first );

        // Einträge
        tIndex::const_iterator    itIndex( m_Index.begin() );
        while ( itIndex != m_Index.end() )
        {
          Stream.WriteString( itIndex->first );
          Stream.WriteString( table.Row( itIndex->second ).SubItems[0] );

          ++itIndex;
        }

        ++it;
      }
    }



    bool KeyTable::Load( IIOStream& Stream )
    {
      m_Entries.clear();
      m_Index.clear();

      // Version
      GR::u32     version = Stream.ReadU32();

      if ( version == 0x0100 )
      {
        // Anzahl Sprachen
        GR::u32     languages = Stream.ReadU32();

        // Anzahl Einträge
        GR::u32     entries = Stream.ReadU32();

        for ( GR::u32 lang = 0; lang < languages; ++lang )
        {
          GR::u32     language = Stream.ReadU32();

          GR::Collections::MultiColumnTable<GR::String>&   table( m_Entries[language] );

          if ( table.NumColumns() == 0 )
          {
            table.AddColumn( "" );
          }

          GR::String                 key;
          GR::String                 value;

          for ( GR::u32 entry = 0; entry < entries; ++entry )
          {
            Stream.ReadString( key );
            Stream.ReadString( value );

            GR::Collections::MultiColumnTable<GR::String>::MultiColumnRow      newRow;

            newRow.SubItems.push_back( value );

            size_t    rowIndex = table.AddRow( newRow );

            // TODO - indices could be off (but should not be)
            m_Index[key] = rowIndex;
          }
        }
        if ( languages )
        {
          SetLangID( m_Entries.begin()->first );
        }
      }

      return true;

    }



    size_t KeyTable::GetEntryCount() const
    {
      return m_Index.size();
    }



    bool KeyTable::ChangeLangID( GR::u32 OldLangID, GR::u32 NewLangID )
    {
      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::iterator   it( m_Entries.find( OldLangID ) );
      if ( it == m_Entries.end() )
      {
        // die Sprache gibt es nicht!
        return false;
      }

      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::iterator   itNew( m_Entries.find( NewLangID ) );
      if ( itNew != m_Entries.end() )
      {
        // die Sprache gibt es schon!
        return false;
      }

      GR::Collections::MultiColumnTable<GR::String>    tempTable = it->second;

      m_Entries.erase( it );
      m_Entries[NewLangID] = tempTable;

      return true;
    }



    bool KeyTable::AddLangID( GR::u32 NewLangID, GR::u32 CopyFromLangID )
    {
      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::iterator   it( m_Entries.find( NewLangID ) );
      if ( it != m_Entries.end() )
      {
        // die Sprache gibt es schon!
        return false;
      }
      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::iterator   itCopy( m_Entries.find( CopyFromLangID ) );
      if ( itCopy != m_Entries.end() )
      {
        m_Entries[NewLangID] = m_Entries[CopyFromLangID];
        return true;
      }

      if ( m_Entries.empty() )
      {
        m_Entries[NewLangID].AddColumn( "" );
        return true;
      }
      m_Entries[NewLangID] = m_Entries.begin()->second;
      return true;

    }



    void KeyTable::RemoveLangID( GR::u32 LangID )
    {
      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::iterator   it( m_Entries.find( LangID ) );
      if ( it == m_Entries.end() )
      {
        return;
      }
      m_Entries.erase( it );
    }



    bool KeyTable::HasLanguage( GR::u32 LanguageID ) const
    {
      return m_Entries.find( LanguageID ) != m_Entries.end();
    }



    GR::u32 KeyTable::GetLangCount() const
    {
      return (GR::u32)m_Entries.size();
    }



    GR::u32 KeyTable::GetLangID( GR::u32 Index ) const
    {
      if ( Index >= (GR::u32)m_Entries.size() )
      {
        return 0;
      }
      std::map<GR::u32,GR::Collections::MultiColumnTable<GR::String> >::const_iterator    it( m_Entries.begin() );
      std::advance( it, Index );
      return it->first;
    }



    GR::Database::Table::eTableType KeyTable::TableType() const
    {
      return KEY_TABLE;
    }



    bool KeyTable::ModifyKey( const GR::String& OldKey, const GR::String& NewKey )
    {
      if ( m_Index.find( NewKey ) != m_Index.end() )
      {
        // Zielkey existiert schon
        return false;
      }
      tIndex::iterator    itOld( m_Index.find( OldKey ) );
      if ( itOld == m_Index.end() )
      {
        // Sourcekey existiert nicht
        return false;
      }
      GR::up    entryIndex = itOld->second;
      m_Index.erase( itOld );
      m_Index[NewKey] = entryIndex;
      return true;
    }


  };
};