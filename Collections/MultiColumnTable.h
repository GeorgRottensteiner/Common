#ifndef GR_COLLECTIONS_MULTICOLUMNTABLE_H
#define GR_COLLECTIONS_MULTICOLUMNTABLE_H


#include <vector>
#include <string>

#include <GR/GRTypes.h>

#include <Interface/IIOStream.h>



namespace GR
{
  namespace Collections
  {
    template<class T> class MultiColumnTable  
    {

      public:

        struct MultiColumnRow
        {
          std::vector<T>        SubItems;
        };



      protected:

        std::vector<GR::String>          m_Columns;
        std::vector<MultiColumnRow>       m_Items;


      public:



	      MultiColumnTable()
        {
        }



        bool AddColumn( const GR::String& ColumnName, const T& DefaultContent = T() )
        {
          if ( !ColumnName.empty() )
          {
            for ( size_t i = 0; i < m_Columns.size(); ++i )
            {
              if ( m_Columns[i] == ColumnName )
              {
                return false;
              }
            }
          }
          m_Columns.push_back( ColumnName );

          for ( size_t i = 0; i < m_Items.size(); ++i )
          {
            m_Items[i].SubItems.push_back( DefaultContent );
          }
          return true;
        }



        size_t NumColumns() const
        {
          return m_Columns.size();
        }



        size_t NumRows() const
        {
          return m_Items.size();
        }
        


        MultiColumnRow& Row( size_t Index )
        {
          if ( Index >= m_Items.size() )
          {
            static MultiColumnRow   emptyRow;

            while ( emptyRow.SubItems.size() < m_Columns.size() )
            {
              emptyRow.SubItems.push_back( T() );
            }
            return emptyRow;
          }
          return m_Items[Index];
        }



        const MultiColumnRow& Row( size_t Index ) const
        {
          if ( Index >= m_Items.size() )
          {
            static MultiColumnRow   emptyRow;

            return emptyRow;
          }
          return m_Items[Index];
        }



        size_t AddRow( const MultiColumnRow& Row )
        {
          m_Items.push_back( Row );

          // adjust sub item count to column count
          while ( m_Items.back().SubItems.size() < m_Columns.size() )
          {
            m_Items.back().SubItems.push_back( T() );
          }

          if ( m_Items.back().SubItems.size() > m_Columns.size() )
          {
            m_Items.back().SubItems.resize( m_Columns.size() );
          }
          return m_Items.size() - 1;
        }



        void DeleteRow( size_t RowIndex )
        {
          if ( RowIndex >= m_Items.size() )
          {
            return;
          }
          typename std::vector<MultiColumnRow>::iterator   itR( m_Items.begin() );
          std::advance( itR, RowIndex );
          m_Items.erase( itR );
        }



        void Clear()
        {
          ClearColumns();
          ClearItems();
        }



        void ClearColumns()
        {
          m_Columns.clear();


          typename std::vector<MultiColumnRow>::iterator   itR( m_Items.begin() );
          while ( itR != m_Items.end() )
          {
            itR->SubItems.clear();

            ++itR;
          }
        }



        void ClearItems()
        {
          m_Items.clear();
        }


    };

  };

};
#endif // GR_COLLECTIONS_MULTICOLUMNTABLE_H
