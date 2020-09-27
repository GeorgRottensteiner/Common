#ifndef _MAP_H_
#define _MAP_H_

namespace GR
{

template <typename K, typename T> class map
{

  public:

    typedef K   key_t;
    typedef T   entry_t;


//  protected:

    struct tNode
    {
      tNode*      pPrev;
      tNode*      pNext;

      key_t       Key;
      entry_t     Content;

      tNode() :
        pPrev( NULL ),
        pNext( NULL ),
        Key(),
        Content()
      {
      }
    };


    tNode*        m_pHead;
    tNode*        m_pTail;

  public:


    map() :
      m_pHead( NULL ),
      m_pTail( NULL )
    {
    }

    ~map()
    {
      while ( m_pHead )
      {
        erase( m_pHead );
      }
    }


    void insert( const key_t& Key, const entry_t& tNew )
    {
      tNode*    pFind = find( Key );

      if ( pFind )
      {
        pFind->Content = tNew;
        return;
      }

      tNode*    pNewNode = (tNode*)HeapAlloc( GetProcessHeap(), 0, sizeof( tNode ) );

      pNewNode->pPrev = NULL;
      pNewNode->pNext = NULL;

      pNewNode->Key     = Key;
      pNewNode->Content = tNew;

      if ( m_pHead == NULL )
      {
        m_pHead = pNewNode;
        m_pTail = pNewNode;
        return;
      }
      m_pTail->pNext = pNewNode;
      pNewNode->pPrev = m_pTail;
      m_pTail = pNewNode;
    }

    void erase( tNode* pNode )
    {
      tNode*    pTempNode = m_pHead;

      while ( pTempNode )
      {
        if ( pTempNode == pNode )
        {
          if ( pNode->pPrev )
          {
            pNode->pPrev->pNext = pNode->pNext;
          }
          if ( pNode->pNext )
          {
            pNode->pNext->pPrev = pNode->pPrev;
          }
          if ( pNode == m_pTail )
          {
            m_pTail = pNode->pPrev;
          }
          if ( pNode == m_pHead )
          {
            m_pHead = pNode->pNext;
          }

          HeapFree( GetProcessHeap(), 0, pNode );
          return;
        }
        pTempNode = pTempNode->pNext;
      }
    }

    size_t size()
    {
      size_t    iSize = 0;

      tNode*  pNode = m_pHead;

      while ( pNode )
      {
        ++iSize;
        pNode = pNode->pNext;
      }

      return iSize;
    }

    tNode* find( const key_t& Key )
    {
      tNode*    pTempNode = m_pHead;

      while ( pTempNode )
      {
        if ( pTempNode->Key == Key )
        {
          return pTempNode;
        }
        pTempNode = pTempNode->pNext;
      }
      return NULL;
    }

};

};

#endif //_MAP_H_