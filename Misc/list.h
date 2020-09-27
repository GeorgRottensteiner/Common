#ifndef _LIST_H_
#define _LIST_H_

namespace GR
{

template <typename T> class list
{

  public:

    typedef T   entry_t;


  protected:

    struct tNode
    {
      tNode*      pPrev;
      tNode*      pNext;

      entry_t     Content;

      tNode() :
        pPrev( NULL ),
        pNext( NULL ),
        Content()
      {
      }
    };


    tNode*        m_pHead;
    tNode*        m_pTail;


  public:


    list() :
      m_pHead( NULL ),
      m_pTail( NULL )
    {
    }

    ~list()
    {
      clear();
    }

    void clear()
    {
      while ( m_pHead )
      {
        erase( m_pHead );
      }
    }

    void insert( const entry_t& tNew )
    {
      tNode*    pNewNode = new tNode();

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
            pNode->pNext = pNode->pNext;
          }
          if ( pNode->pNext )
          {
            pNode->pNext = pNode->pPrev;
          }
          if ( pNode == m_pTail )
          {
            m_pTail = pNode->pPrev;
          }
          if ( pNode == m_pHead )
          {
            m_pHead = pNode->pNext;
          }
          delete pNode;
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

    tNode* find( const entry_t& Var )
    {
      tNode*    pTempNode = m_pHead;

      while ( pTempNode )
      {
        if ( pTempNode->Content == Var )
        {
          return pTempNode;
        }
        pTempNode = pTempNode->pNext;
      }
      return NULL;
    }

};

};

#endif //_LIST_H_