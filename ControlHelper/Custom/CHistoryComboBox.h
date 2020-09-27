#ifndef __INCLUDE_CHISTORYCOMBOBOX_H__
#define __INCLUDE_CHISTORYCOMBOBOX_H__
/*--------------------+-------------------------------------------------------+
 | Programmname       : ListControl-Helper                                    |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 17.01.2002                                            |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <ControlHelper\CComboBox.h>



/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CHistoryComboBox : public CComboBox
{

  protected:

    bool                  m_bBackSpace,
                          m_bDelete;

  public:


	  CHistoryComboBox();
    virtual ~CHistoryComboBox();


    void                  AddToHistory( const GR::String& strText );

    virtual LRESULT       WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    virtual BOOL          PreTranslateMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );

};



#endif //__INCLUDE_CHISTORYCOMBOBOX_H__



