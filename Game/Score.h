#ifndef SCORE_H
#define SCORE_H



#include <vector>

#include <GR/GRTypes.h>



class HighScore
{

  protected:

    struct tScoreEntry
    {
      GR::String        m_Name;
      int               m_Score;
      size_t            m_ItemLength;
      GR::u8*           m_pExtraData;

      tScoreEntry() :
        m_Score( 0 ),
        m_ItemLength( 0 ),
        m_pExtraData( NULL )
        {
      }

      tScoreEntry( const tScoreEntry& rhs )
      {
        m_Name        = rhs.m_Name;
        m_Score       = rhs.m_Score;
        m_ItemLength  = rhs.m_ItemLength;
        if ( m_ItemLength )
        {
          m_pExtraData  = new GR::u8[m_ItemLength];
          memcpy( m_pExtraData, rhs.m_pExtraData, m_ItemLength );
        }
        else
        {
          m_pExtraData = NULL;
        }
      }

      tScoreEntry& operator=( const tScoreEntry& rhs )
      {
        if ( &rhs == this )
        {
          return *this;
        }
        m_Name        = rhs.m_Name;
        m_Score       = rhs.m_Score;
        m_ItemLength  = rhs.m_ItemLength;
        if ( m_ItemLength )
        {
          m_pExtraData  = new GR::u8[m_ItemLength];
          memcpy( m_pExtraData, rhs.m_pExtraData, m_ItemLength );
        }
        else
        {
          m_pExtraData = NULL;
        }
        return *this;
      }

      ~tScoreEntry()
      {
        delete[] m_pExtraData;
      }
    };

    typedef std::vector<tScoreEntry>    tEntries;

    tEntries            m_Entries;

    size_t              m_MaxEntries;

    size_t              m_ItemDataLength;

    bool                m_HigherUp;


  public:


    HighScore();
    HighScore( GR::u32 Entries, size_t ItemDataLength = 0, bool HigherScoreUp = true );

    ~HighScore();


    bool                SetEntry( GR::u32 Nr, int Score, const GR::String& Name, void* pData = NULL );
    int                 Score( GR::u32 Nr );
    GR::String          Name( GR::u32 Nr );
    bool                ItemData( GR::u32 Nr, void* pBuffer );
    bool                SetItemData( GR::u32 Nr, void* pBuffer );
    GR::u32             GetPosition( int Score );
    GR::u32             Entries() const;
    bool                Insert( int Score, const GR::String& Name, void* pItemData = NULL );
    bool                Remove( GR::u32 Pos );

    void                Clear();
    bool                Initialise( GR::u32 Entries, size_t ItemDataLength = 0, bool HigherScoreUp = true );

    bool                Load( const GR::String& FileName );
    void                Save( const GR::String& FileName );

    // returns true if the score would add a new entry
    bool                CouldEnter( int Score );

};



#endif // SCORE_H