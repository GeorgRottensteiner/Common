#ifndef GR_Database_TABLE_H
#define GR_Database_TABLE_H


#include <vector>
#include <string>
#include <map>

#include <GR/GRTypes.h>



struct IIOStream;

namespace GR
{
  namespace Database
  {

    class Table  
    {

      protected:

        GR::u32               m_LanguageID;


      public:

        enum eTableType
        {
          SIMPLE_TABLE = 0,
          KEY_TABLE = 1,
        };



	      Table();
	      virtual ~Table();

        virtual size_t        GetEntryCount() const = 0;
        virtual GR::String    Entry( size_t Index, GR::u32 LangID = 0 ) const = 0;
        virtual bool          IsEntryValid( size_t Index ) const = 0;
        virtual void          SetEntry( size_t Index, const GR::String& Text, GR::u32 LangID = 0 ) = 0;
        virtual void          InsertEntry( size_t Index, const GR::String& Text ) = 0;
        virtual void          DeleteEntry( size_t Index ) = 0;

        virtual bool          Load( IIOStream& Stream ) = 0;
        virtual void          Save( IIOStream& Stream ) = 0;

        void                  SetLangID( GR::u32 Lang );
        GR::u32               LangID();

        // Anzahl Sprachen
        virtual GR::u32       GetLangCount() const = 0;
        // Sprache Nr. X
        virtual GR::u32       GetLangID( GR::u32 Index ) const = 0;

        virtual bool          ChangeLangID( GR::u32 OldLangID, GR::u32 NewLangID ) = 0;
        virtual bool          AddLangID( GR::u32 NewLangID, GR::u32 CopyFromLangID = 0 ) = 0;
        virtual void          RemoveLangID( GR::u32 NewLangID ) = 0;

        virtual bool          HasLanguage( GR::u32 LanguageID ) const = 0;

        virtual eTableType    TableType() const = 0;

    };

  }

}
#endif // GR_Database_TABLE_H
