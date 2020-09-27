#include <vector>
#include <iterator>

#include <Interface/IIOStream.h>

#include <Misc/Format.h>


#include "DatabaseTable.h"



namespace GR
{
  namespace Database
  {

    Table::Table() :
      m_LanguageID( 0 )
    {

    }



    Table::~Table()
    {

    }



    void Table::SetLangID( GR::u32 Language )
    {
      m_LanguageID = Language;
    }



    GR::u32 Table::LangID()
    {

      return m_LanguageID;

    }

  }
}