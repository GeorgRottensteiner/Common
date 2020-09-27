#ifndef GR_DATABASE_SIMPLETABLE_H
#define GR_DATABASE_SIMPLETABLE_H


#include <vector>
#include <string>
#include <map>

#include <GR/GRTypes.h>

#include "DatabaseTable.h"



struct IIOStream;

namespace GR
{
  namespace Database
  {

    class SimpleTable : public GR::Database::Table
    {

      protected:

        std::map<GR::u32,std::vector<GR::String> >               m_Entries;

        

      public:


	      SimpleTable( size_t Size = 0 );
	      virtual ~SimpleTable();

        size_t                GetEntryCount() const;
        GR::String            Entry( size_t Index, GR::u32 LangID = 0 ) const;
        bool                  IsEntryValid( size_t Index ) const;
        void                  SetEntry( size_t Index, const GR::String& Text, GR::u32 LangID = 0 );
        void                  InsertEntry( size_t Index, const GR::String& Text );
        void                  DeleteEntry( size_t Index );

        bool                  Load( IIOStream& Stream );
        bool                  LoadOld( IIOStream& Stream );
        void                  Save( IIOStream& Stream );

        void                  SetLangID( GR::u32 Lang );
        GR::u32               LangID();

        GR::u32               GetLangCount() const;
        GR::u32               GetLangID( GR::u32 Index ) const;

        bool                  ChangeLangID( GR::u32 OldLangID, GR::u32 NewLangID );
        bool                  AddLangID( GR::u32 NewLangID, GR::u32 CopyFromLangID = 0 );
        void                  RemoveLangID( GR::u32 NewLangID );
        bool                  HasLanguage( GR::u32 LanguageID ) const;

        virtual eTableType    TableType() const;

    };

  };
};

#endif // GR_DATABASE_SIMPLETABLE_H
