#pragma once

#include <GR/GRTypes.h>

#include <map>
#include <list>
#include <string>

#include <Lang/Service.h>

#include "DatabaseTable.h"



struct IIOStream;

namespace GR
{
  namespace Database
  {

    class Database : public GR::Service::Service
    {

      protected:

        typedef std::map<GR::String,Table*>   tMapTables;
        
        tMapTables            m_Tables;

        GR::u32               m_Language;


      public:

        static Database&      Instance();

        size_t                GetTableCount() const;
        Table*                GetTable( size_t Index );
        GR::String            GetTableName( size_t Index );
        Table*                GetTable( const GR::String& Desc );
        void                  GetTableList( std::list<GR::String>& Tables );
        void                  InsertTable( const GR::String& Desc, Table* pTable );
        void                  RenameTable( Table* pTable, const GR::String& Desc );
        void                  RemoveTable( Table* pTable );

        void                  SetLangID( GR::u32 Lang );
        GR::u32               LangID();

        void                  Clear();
        bool                  Load( IIOStream& Stream );
        void                  Save( IIOStream& Stream );


	      Database();
        virtual ~Database();


    };

  }
}
