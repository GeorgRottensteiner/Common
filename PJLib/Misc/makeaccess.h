#define MAKEACCESS(type,prefix,variable) \
 public: \
 inline type get_##variable (){return m_##prefix##variable ; } \
 inline void set_##variable (type _##prefix##variable){ m_##prefix##variable = _##prefix##variable ;}\
 protected: //Or private, as you wish.

#define MAKEACC(type,prefix,variable) \
 public: \
 inline type ##variable () const {return m_##prefix##variable ; } \
 inline void ##variable (const type& _##prefix##variable){ m_##prefix##variable = _##prefix##variable ;}\
 protected: //Or private, as you wish.



// class CmyClass
// {
      // public:
            // CmyClass();
      // protected:
            // int   m_iVar;
            // MAKEACCESS(int,i,Var);
// }







#define MAKEMEMBER(type,prefix,variable) \
  public: \
  inline type get_##variable (){return m_##prefix##variable ; } \
  inline void set_##variable (type _##prefix##variable){ m_##prefix##variable = _##prefix##variable ;}\
  protected: \
  type m_##prefix##variable ;