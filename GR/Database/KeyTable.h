#ifndef GR_DATABASE_KEYTABLE_H
#define GR_DATABASE_KEYTABLE_H

#include <string>
#include <map>

#include <GR/GRTypes.h>
#include <Collections/MultiColumnTable.h>

#include "DatabaseTable.h"



struct IIOStream;

namespace GR
{
  namespace Database
  {

    class KeyTable : public GR::Database::Table
    {

      protected:

        typedef std::map<GR::String, GR::up>     tIndex;

        std::map<GR::u32, GR::Collections::MultiColumnTable<GR::String> >    m_Entries;

        tIndex                m_Index;


      public:


        virtual size_t          GetEntryCount() const;
        virtual GR::String  Entry( size_t Index, GR::u32 LangID = 0 ) const;
        virtual bool            IsEntryValid( size_t Index ) const;
        virtual void            SetEntry( size_t Index, const GR::String& Text, GR::u32 LangID = 0 );
        void                    InsertEntry( size_t Index, const GR::String& Text );
        virtual void            DeleteEntry( size_t Index );

        bool                    Load( IIOStream& Stream );
        void                    Save( IIOStream& Stream );

        void                    SetEntry( const GR::String& Key, const GR::String& Text, GR::u32 LangID = 0 );
        GR::String          Entry( const GR::String& Key, GR::u32 LangID = 0 ) const;
        GR::String          Key( const GR::u32 Index ) const;
        bool                    IsEntryValid( const GR::String& Key ) const;
        void                    DeleteEntry( const GR::String& Key );

        bool                    ModifyKey( const GR::String& OldKey, const GR::String& NewKey );

        virtual GR::u32         GetLangCount() const;
        virtual GR::u32         GetLangID( GR::u32 Index ) const;

        virtual bool            ChangeLangID( GR::u32 OldLangID, GR::u32 NewLangID );
        virtual bool            AddLangID( GR::u32 NewLangID, GR::u32 CopyFromLangID );
        virtual void            RemoveLangID( GR::u32 NewLangID );
        virtual bool            HasLanguage( GR::u32 LanguageID ) const;

        virtual eTableType      TableType() const;

    };

  };

};
#endif // GR_DATABASE_KEYTABLE_H

