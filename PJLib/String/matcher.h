#ifndef _pj_matcher_h
#define _pj_matcher_h


#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <map>


namespace match
{
  //- dieser verwendet maps - eventuell wäre eine brute force lösung effizienter
  //- aber auch eine hashmap wäre gut
  template <typename K, typename V>
  class matcher
  {
  public:
    typedef K key_type;
    typedef V value_type;

  private:
    class node
    {
    public:
      typedef std::map< key_type, node* > child_map_t;

      value_type  value_;
      child_map_t childs_;

      node() : value_( value_type() ), childs_() {}

      void clear()
      {
        child_map_t::iterator it( childs_.begin() ), itend( childs_.end() );
        for ( ; it != itend; ++it )
        {
          it->second->clear();
          delete it->second;
        }
        childs_.clear();
      }

      ~node()
      {
        clear();
      }

      template <typename IT>
      void reg_pattern_rec( IT first, IT last, const value_type& value )
      {
        if ( first == last )
        {
          value_ = value;
          return;
        }

        node* n = 0;
        child_map_t::iterator itchild = childs_.find( *first );
        if ( itchild == childs_.end() )
        {
          n = new node();
          childs_[ *first ] = n;
        }
        else
        {
          // dh() << "#";
          n = itchild->second;
        }

        // dh() << *first;
        n->reg_pattern_rec( ++first, last, value );
      }

      // template <typename IT>
      // void match_rec( IT first, IT last, IT& new_first, value_type& value ) const
      // {
        // if ( first == last )
        // {
          // new_first = last;
          // value = value_;
          // return;
        // }

        // child_map_t::const_iterator itchild = childs_.find( *first );
        // if ( itchild == childs_.end() )
        // {
          // new_first = first;
          // value = value_;
          // return;
        // }
        // else
        // {
          // itchild->second->match_rec( ++first, last, new_first, value );
          // return;
        // }
      // }

    private:
      node( const node& rhs );
      node& operator=( const node& rhs ); //- n.i.
    };

  public:

    template <typename IT>
    void reg_pattern( IT first, IT last, const value_type& value )
    {
      // dh() << "reg_pattern(\"" << GR::String( first, last ) << "\") '";
      head_.reg_pattern_rec( first, last, value );
      // dh() << "'\n";
    }

    // //- rekursives matching (vermutlich langsamer als das nichtrekursive unten)
    // template <typename IT>
    // void match_rec( IT first, IT last, IT& new_first, value_type& value ) const
    // {
      // head_.match_rec( first, last, new_first, value );
      // // head_.match_nonrec( first, last, new_first, value );
    // }

    //- nichtrekursives matching (vermutlich schneller als das rekursive oben)
    template <typename IT>
    void match( IT first, IT last, IT& new_first, value_type& value ) const
    {
      IT                                it_cur   = first;
      const node*                       node_cur = &head_;
      node::child_map_t::const_iterator itchild;

      for(;;) //- forever
      {
        if ( it_cur == last )
        {
          new_first = last;
          value     = node_cur->value_;
          return;
        }

        itchild = node_cur->childs_.find( *it_cur );
        if ( itchild == node_cur->childs_.end() )
        {
          new_first = it_cur;
          value     = node_cur->value_;
          return;
        }
        else
        {
          ++it_cur;
          node_cur = itchild->second;
        }
      }
    }

    matcher() : head_() {}

    ~matcher()
    {
      clear(); //- eigentlich überflüssig, da head_ sowieso zerstört wird und seine Kinder mitnimmt
    }

    void clear()
    {
      head_.clear();
    }

    void dump( std::ostream& os ) const
    {
      dump_rec( os, &head_, 0 );
    }

  private:
    node head_;
    matcher( const matcher& rhs );
    matcher& operator=( const matcher& rhs ); //- n.i.

    void dump_rec( std::ostream& os, const node* n, int level ) const
    {
      using namespace std;
      if ( !n ) return;
      if ( n->value_ ) os << "    " << n->value_;
      os << endl;
      node::child_map_t::const_iterator it( n->childs_.begin() ), itend( n->childs_.end() );
      for ( ; it != itend; ++it )
      {
        for (int i = 1; i <= level + 1 ; ++i ) os << " ";
        os << it->first << " ";
        dump_rec( os, it->second, level + 1 );
      }
    }
  };

  template <typename K, typename V>
  std::ostream& operator<<( std::ostream& os, const matcher<K,V>& m )
  {
    m.dump( os );
    return os;
  }



}

//- das folgende ist die matcherversion, mit der der fred funktionierte (9.8.2002)
//- sie ist jedoch nicht sehr flexibel, deshalb sollte die obere version bevorzugt werden.
//- sie muss jedoch noch im fred getestet werden und vor allem auch in der xbox-version
//- (die andere stl-implementierung des sdk hält einige fallstricke bereit)
//- ausserdem sind die node keine geschachtelte klass, was ich doof finde, da sie ausschliesslich
//- *innerhalb* des matchers verwendet werden

namespace no_template_zum_testen_und_nur_fuer_fred_und_nicht_flexibel_und_doof
{
  class matcher_node;
  typedef std::map< char, matcher_node* > child_map_t;

  class matcher_node
  {
  public:

    void*  value_;
    child_map_t childs_;

    matcher_node() : value_( 0 ), childs_() {}

    void clear()
    {
      child_map_t::iterator it( childs_.begin() ), itend( childs_.end() );
      for ( ; it != itend; ++it )
      {
        it->second->clear();
        delete it->second;
      }
      childs_.clear();
    }

    ~matcher_node()
    {
      clear();
    }

    void reg_pattern_rec( const char* first, const char* last, void* value )
    {
      if ( first == last )
      {
        value_ = value;
        return;
      }

      matcher_node* n = 0;

      child_map_t::iterator itchild = childs_.find( *first );
      if ( itchild == childs_.end() )
      {
        n = new matcher_node();
        childs_[ *first ] = n;
      }
      else
      {
        n = itchild->second;
        // dh() << "#";
      }

      // dh() << *first;
      n->reg_pattern_rec( ++first, last, value );
    }

  private:
    matcher_node( const matcher_node& rhs );
    matcher_node& operator=( const matcher_node& rhs ); //- n.i.
  };



  //- dieser verwendet maps - eventuell wäre eine brute force lösung effizienter
  //- aber auch eine hashmap wäre gut
  class matcher
  {
  public:
  private:

  public:

    void reg_pattern( const char* first, const char* last, void* value )
    {
      // dh() << "reg_pattern(\"" << GR::String( first, last ) << "\") '";
      head_.reg_pattern_rec( first, last, value );
      // dh() << "'\n";
    }

    //- nichtrekursives matching (vermutlich schneller als das rekursive oben)
    void match( const char* first, const char* last, const char** new_first, void** value ) const
    {
      const char*                               it_cur   = first;
      const matcher_node*                       node_cur = &head_;
      child_map_t::const_iterator itchild;

      for(;true;) //- forever
      {
        if ( it_cur == last )
        {
          *new_first = last;
          *value     = node_cur->value_;
          break;
        }


        itchild = node_cur->childs_.find( *it_cur );
        if ( itchild == node_cur->childs_.end() )
        {
          *new_first = it_cur;
          *value     = node_cur->value_;
          break;
        }
        else
        {
          // dh() << *it_cur;
          ++it_cur;
          node_cur = itchild->second;
        }
      }

      // dh() << "\n";

    }

    matcher() : head_() {}

    ~matcher()
    {
      head_.clear();
      // clear(); //- eigentlich überflüssig, da head_ sowieso zerstört wird und seine Kinder mitnimmt
    }

    // void clear()
    // {
      // head_.clear();
    // }

    void dump( std::ostream& os ) const
    {
      dump_rec( os, &head_, 0 );
    }

  private:
    matcher_node head_;
    matcher( const matcher& rhs );
    matcher& operator=( const matcher& rhs ); //- n.i.

    void dump_rec( std::ostream& os, const matcher_node* n, int level ) const
    {
      using namespace std;
      if ( !n ) return;
      if ( n->value_ ) os << "    " << n->value_;
      os << endl;
      child_map_t::const_iterator it( n->childs_.begin() ), itend( n->childs_.end() );
      for ( ; it != itend; ++it )
      {
        for (int i = 1; i <= level ; ++i ) os << " ";
        os << it->first << " ";
        dump_rec( os, it->second, level + 1 );
      }
    }
  };

  inline std::ostream& operator<<( std::ostream& os, const matcher& m )
  {
    m.dump( os );
    return os;
  }
}
#endif//_pj_matcher_h