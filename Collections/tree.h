#ifndef TREE_H_
#define TREE_H_

#include <iterator>



namespace GR
{

//- match (nicht-full-match) in assoc umbenennen, interface an find anpassen (iterator zurückgeben!)
//- überlegen, ob siblings grundsätzlich alphabetisch geordnet sein sollen oder frei gruppierbar
//- (wie liste - macht das Sinn?)
//- const_iteration usw. einbauen!!!!
//- --iterator einbauen

//- auch einzel-keys suchen: iterator find( iterator first, iterator last, key ) wie std::find

template <typename V> class tree
{

  public:


    typedef V value_type;

    class iterator;

  private:

    class node
    {

      public:

        bool            m_bCollapsed;
        value_type      entry_;
        node*           parent_;
        node*           prev_;
        node*           next_;
        node*           child_;


        explicit node( const value_type& v ) 
          : entry_( v ), 
            parent_( NULL ), 
            child_( NULL ), 
            prev_( NULL ), 
            next_( NULL ), 
            m_bCollapsed( true )
        {
        }

        explicit node( const node& rhs ) 
          : entry_( rhs.entry_ ), 
            parent_( NULL ), 
            child_( NULL ), 
            prev_( NULL ), 
            next_( NULL ), 
            m_bCollapsed( true )
        { 
          copy_from( rhs ); 
        }

        bool is_visible() const
        {
          const node*     nTemp = this;
          do
          {
            if ( nTemp->parent_ == NULL )
            {
              return true;
            }
            if ( nTemp->parent_->m_bCollapsed )
            {
              return false;
            }
            nTemp = nTemp->parent_;
          }
          while ( nTemp );

          return true;
        }

        node& operator=( const node& rhs )        
        { 
          if ( this != &rhs )
          {
            entry_ = rhs.entry_;
            copy_from( rhs ); 
          }
          return *this;
        }

        ~node() 
        { 
          clear(); 
        }

        bool empty() const 
        { 
          return child_ == 0; 
        }
  
        void clear()
        {
          node* temp = 0;
          for ( node* c = child_; c != 0; c = temp )
          {
            temp = c->next_;
            c->clear();
            delete c;
          }
          child_ = 0;
        }

        node& swap( node& rhs ) //- sollte nur für den head_ der assoc_tree augerufen werden
        {
          //- bei meinen kinder rhs als parent setzen
          node* c = 0;
          for ( c = child_;     c != 0; c = c->next_ ) c->parent_ = &rhs; //- bei rhs' kinder this als parent setzen
          for ( c = rhs.child_; c != 0; c = c->next_ ) c->parent_ = this; //- rhs' und meinen child_->pointer tauschen
          std::swap( child_, rhs.child_ );
          return *this;
        }

        node* find( const value_type& v )
        {
          for ( node* c = child_; c != 0; c = c->next_ ) 
          {
            if ( c->entry_ == v ) 
            {
              return c;
            }
          }
          return 0;
        }

        const node* const_find( const value_type& v ) const
        {
          for ( const node* c = child_; c != 0; c = c->next_ )
          {
            if ( c->entry_ == v ) 
            {
              return c;
            }
          }
          return 0;
        }

        template <typename IT> IT insert_3( IT first, IT last, const value_type& value )   
        {
          if ( first == last ) 
          {
            entry_ = value;
            return iterator( this );
          }
          node* n = find( *first );
          if ( !n )
          {
            n = new node( *first );
            if ( child_ ) 
            {
              child_->prev_ = n; //- erstmal vorne drantun
            }
            n->next_        = child_;
            child_          = n;
            n->parent_      = this;
          }
          return n->insert_( ++first, last, value );  
        }

        node* insert_child_( node* nodeParent, const value_type& value )
        {
          if ( nodeParent )
          {
            return nodeParent->insert_( value );
          }
          return NULL;
        }
  
        node* insert_2( node* pPrevNode, const value_type& value )
        {

          if ( pPrevNode == NULL )
          {
            // am Ende einsetzen
            pPrevNode = this;
            while ( pPrevNode->next_ )
            {
              pPrevNode = pPrevNode->next_;
            }
          }

          // hier soll er rein
          node* oldnext = pPrevNode->next_;

          pPrevNode->next_          = new node();
          pPrevNode->next_->prev_   = pPrevNode;
          pPrevNode->next_->entry_  = value;
          pPrevNode->next_->parent_ = pPrevNode->parent_;
          pPrevNode->next_->next_   = oldnext;
          if ( oldnext )
          {
            oldnext->prev_ = pPrevNode->next_;
          }
          return pPrevNode->next_;
        }
  
        node* insert_( const value_type& value )   
        {
          if ( !child_ )
          {
            node* pNode = new node();
            child_ = pNode;
            child_->entry_ = value;
            child_->parent_ = this;
            return pNode;
          }

          node* newNode = child_;
          while ( newNode->next_ )
          {
            newNode = newNode->next_;
          }

          newNode->next_ = new node();
          newNode->next_->prev_ = newNode;
          newNode->next_->entry_ = value;
          newNode->next_->parent_ = this;

          return newNode->next_;
        }

        template <typename IT> IT insert_it_( const IT& it, IT first, IT last, const value_type& value )   
        {
          if ( it.n_ )
          {
            return it.n_->insert_( first, last, value );  
          }
          else 
          {
            return iterator();
          }
        }

        //- nur für head_ aufrufen!
        template <typename IT> IT erase_it_( const IT& it )   
        {
          if ( it.n_ && it.n_ != this )
          {
            node* n = it.n_;
            
            node* n_return = n->prev_;
            if ( !n_return ) 
            {
              n_return = n->parent_;
            }

            if ( n->prev_ )
            {
              n->prev_->next_ = n->next_;
            }
            if ( n->next_ )
            {
              n->next_->prev_ = n->prev_;
            }
            if ( ( n->parent_ ) 
            &&   ( n->parent_->child_ == n ) )
            {
              n->parent_->child_ = n->next_;
            }
            delete n;

            return ++iterator( n_return );
          }
          else 
          {
            return iterator();
          }
        }



        int size() const
        {
          int count = 1;

          node*   n = next_;
          while ( n )
          {
            ++count;
            if ( n->child_ )
            {
              count += n->child_->size();
            }
            n = n->next_;
          }

          if ( child_ )
          {
            count += child_->size();
          }
          return count;
        }



        int childcount() const
        {
          int     count = 0;
          node*   n = child_;
          while ( n )
          {
            ++count;
            n = n->next_;
          }
          return count;
        }


      private:

        node() 
          : entry_( value_type() ), 
            parent_( NULL ), 
            child_( NULL ), 
            prev_( NULL ), 
            next_( NULL ) 
        {
          // n.i.
        }

        void copy_from( const node& rhs )
        {
          if ( this == &rhs ) 
          {
            return;
          }
          clear();
          entry_        = rhs.entry_;
          m_bCollapsed  = rhs.m_bCollapsed;

          if ( rhs.child_ )
          {
            child_ = new node( *rhs.child_ );
            child_->parent_ = this;
            node* c           = 0;
            node* attach_here = child_;
            for ( c = rhs.child_->next_; c != 0; c = c->next_ ) 
            {
              node* n = new node( *c );
              n->parent_         = this;
              n->prev_           = attach_here;
              attach_here->next_ = n;
              attach_here        = n;
            }
          }
        }

        friend class iterator;

    };

  public:

    class iterator : public std::iterator<std::bidirectional_iterator_tag, value_type>
    {

      public:

        bool operator==( const int* cValue ) const 
        { 
          if ( cValue == NULL )
          {
            return n_ == NULL;
          }
          return false;
        }

        bool operator!=( const int* cValue ) const 
        { 
          if ( cValue != NULL )
          {
            return n_ != NULL;
          }
          return true;
        }

        bool operator==( const iterator& rhs ) const 
        { 
          return n_ == rhs.n_; 
        }

        bool operator!=( const iterator& rhs ) const 
        { 
          return n_ != rhs.n_; 
        }

        explicit iterator( node* n = 0 ) 
          : n_( n ) 
        {
        }

        iterator( const iterator& rhs ) 
          : n_( rhs.n_ ) 
        {
        }

        ~iterator()  
        {
        }

        iterator& operator=( const iterator& rhs )
        {
          if ( this != &rhs ) n_ = rhs.n_;
          return *this;
        }
        iterator& operator ++() //- pre-inc
        {
          inc();
          return *this;
        }
        iterator operator ++(int) //- post-inc
        {
          iterator tmp( *this );
          inc();
          return tmp;
        }
        iterator& operator --() //- pre-dec
        {
          dec();
          return *this;
        }
        iterator operator --(int) //- post-dec
        {
          iterator tmp( *this );
          dec();
          return tmp;
        }

        value_type* operator->() const { return (&(this->n_->entry_)); }
        value_type& operator*()  const { return n_->entry_; }

        void toggle()
        {
          n_->m_bCollapsed = !n_->m_bCollapsed;
        }

        bool is_collapsed() const
        {
          return n_->m_bCollapsed;
        }

        bool is_visible() const
        {
          return n_->is_visible();
        }

        int level() const
        {
          int res = 0;
          for ( node* p = n_->parent_; p; p = p->parent_ )
          {
            ++res;
          }
          return res;
        }

        iterator next_sibling() const
        {
          if ( n_ )
          {
            return iterator( n_->next_ );
          }
          return iterator();
        }

        iterator prev_sibling() const
        {
          if ( n_ )
          {
            return iterator( n_->prev_ );
          }
          return iterator();
        }

        iterator child() const
        {
          if ( n_ )
          {
            return iterator( n_->child_ );
          }
          return iterator();
        }

        iterator parent() const
        {
          if ( n_ )
          {
            return iterator( n_->parent_ );
          }
          return iterator();
        }


      protected:
        node* n_;
        void inc()
        {
          //- aua - knifflig
          if ( !n_ ) return;
          if      ( n_->child_  )  n_ = n_->child_;
          else if ( n_->next_   )  n_ = n_->next_;
          else if ( n_->parent_ )  
          {
            while ( n_->parent_ && !n_->parent_->next_ )
              n_ = n_->parent_; 
            
            if ( n_->parent_ ) n_ = n_->parent_->next_; 
            else               n_ = 0;
          }
          else                     n_ = 0;
        }

        void dec() 
        {
          if ( !n_ ) return;
          if ( n_->prev_ )
          {
            if ( !n_->prev_->child_ )
            {
              n_ = n_->prev_;
              return;
            }
            n_ = n_->prev_->child_;
            while ( ( n_->next_ )
            ||      ( n_->child_ ) )
            {
              if ( n_->next_ )
              {
                n_ = n_->next_;
              }
              else if ( n_->child_ )
              {
                n_ = n_->child_;
              }
            }
          }
          else if ( n_->parent_ )   n_ = n_->parent_;
        }

      public:

        void inc_visible()
        {
          //- aua - knifflig
          if ( !n_ ) return;

          if      ( ( n_->child_ )
          &&        ( !is_collapsed() ) )
          {
            n_ = n_->child_;
            return;
          }
          if ( n_->next_ )
          {
            n_ = n_->next_;
            return;
          }
          if ( n_->parent_ )  
          {
            while ( ( n_->parent_ )
            &&      ( !n_->parent_->next_ ) )
            {
              n_ = n_->parent_; 
            }
            
            if ( n_->parent_ )
            {
              n_ = n_->parent_->next_;
            }
            else
            {
              n_ = 0;
            }
          }
          else
          {
            n_ = 0;
          }
        }

        friend class node;
        friend class tree;
    };

    iterator begin() const 
    { 
      if ( m_pHeadNode )
      {
        return iterator( m_pHeadNode );
      }
      return iterator();
    }

    iterator end() const 
    { 
      return iterator(); 
    }

    bool empty() const 
    { 
      return ( m_pHeadNode == NULL );
    }
  

    iterator erase( iterator it )
    {
      if ( it.n_ == m_pHeadNode )
      {
        m_pHeadNode->clear();
        if ( m_pHeadNode->next_ )
        {
          m_pHeadNode = m_pHeadNode->next_;

          delete m_pHeadNode->prev_;
          m_pHeadNode->prev_ = NULL;

          return iterator( m_pHeadNode );
        }
        delete m_pHeadNode;
        m_pHeadNode = NULL;

        return iterator();
      }

      return m_pHeadNode->erase_it_( it );
    }

    iterator insert( const value_type& value ) 
    {
      if ( m_pHeadNode == NULL )
      {
        m_pHeadNode = new node( value );

        return iterator( m_pHeadNode );
      }
      node*   pNode = m_pHeadNode;

      while ( pNode->next_ )
      {
        pNode = pNode->next_;
      }
      pNode->next_ = new node( value );

      pNode->next_->prev_ = pNode;

      return iterator( pNode->next_ );
    }

    iterator insert( iterator itPrev, const value_type& value ) 
    {
      if ( m_pHeadNode == NULL )
      {
        m_pHeadNode = new node( value );

        return iterator( m_pHeadNode );
      }
      if ( itPrev.n_ == NULL )
      {
        return insert( value );
      }

      node*   pPrevNode = itPrev.n_;
      node*   pNextNode = pPrevNode->next_;

      node*   pNewNode = new node( value );

      pPrevNode->next_ = pNewNode;
      pNewNode->prev_  = pPrevNode;
      pNewNode->next_  = pNextNode;

      if ( pNextNode )
      {
        pNextNode->prev_ = pNewNode;
      }
      return iterator( pNewNode );
    }

    iterator insert_child( iterator itParent, const value_type& value ) 
    {
      if ( m_pHeadNode == NULL )
      {
        // ungültiges Parent?
        return iterator();
      }
      return iterator( m_pHeadNode->insert_child_( itParent.n_, value ) );
    }


    /////////////////////////////////////////////////////////////////////
    //-    finden
    /////////////////////////////////////////////////////////////////////
    //- nichtrekursives volles matching
    //- volles matching bedeutet, das die komplette eingabesequenz auflösbar sein muss 
    //- dieses matching eignet sich vor allem für Eigenschaftsbäume, bei denen die Suchsequenz
    //- vollständig im Baum vorhanden sein muss, um das matching zu erfüllen. Es kann dann bereits
    //- das *vorhandenseinsein* eines keys eine eigenschaft bedeuten, unabhängig von einem eventuell
    //- gesetzten value (z.B. der key "essbar", oder "Ist_aus_watte")

    iterator find( const value_type& value )
    {
      iterator it    = begin();
      iterator itend = end();
      for( ; it != itend; ++it )
      {
        if ( *it == value )
        {
          return it;
        }
      }
      return iterator();
    }

    tree() 
      : m_pHeadNode( NULL )
    {
    }
    tree( const tree& rhs )
      : m_pHeadNode( NULL )
    {
      if ( rhs.m_pHeadNode )
      {
        m_pHeadNode = new node( *rhs.m_pHeadNode );
      }
    }

    tree& operator=( const tree& rhs ) 
    { 
      if ( this != &rhs )
      {
        clear();

        if ( rhs.m_pHeadNode )
        {
          m_pHeadNode = new node( *rhs.m_pHeadNode );
        }
      }
      return *this; 
    }

    ~tree() 
    {
      clear();
    }

    bool      IsItemVisible( const iterator& it ) const
    {
      return it.n_->is_visible();
    }

    size_t size() const
    {
      if ( m_pHeadNode == NULL )
      {
        return 0;
      }
      //return m_pHeadNode->size() - 1;
      return m_pHeadNode->size();
    }

    void clear()  
    { 
      while ( m_pHeadNode != NULL )
      {
        erase( begin() );
      }
    }

    int child_count( const iterator it ) const
    {
      if ( it.n_ == NULL )
      {
        return 0;
      }
      return it.n_->childcount();
    }

    iterator next( const iterator it ) const 
    { 
      return it.next_sibling();
    }

    iterator next_visible( const iterator it ) const 
    { 
      return it.next_visible();
    }

    iterator prev( const iterator it ) const 
    { 
      return it.prev_sibling();
    }

    iterator child( const iterator it ) const 
    { 
      return it.child();
    }

    iterator parent( const iterator it ) const 
    { 
      return it.parent();
    }

  private:

    node*           m_pHeadNode;
  
};


}; // namespace

#endif // TREE_H