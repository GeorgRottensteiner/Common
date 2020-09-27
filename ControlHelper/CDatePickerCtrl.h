#ifndef INCLUDE_CDATEPICKERCTRL_H
#define INCLUDE_CDATEPICKERCTRL_H



#include <ControlHelper\CDialog.h>
#include <ControlHelper\CButton.h>

#include <DateTime\DateTime.h>



namespace WindowsWrapper
{

  class CDatePickerCtrl : public CDialog
  {
    // Constructors
    public:


    CDatePickerCtrl();
    virtual ~CDatePickerCtrl();


    virtual BOOL          Create( HWND hwndParent,
                                  DWORD dwStyle = WS_POPUP | WS_VISIBLE,
                                  DWORD dwID = 0 );


    SYSTEMTIME            PickedDate() const;


    protected:

    CButton               m_buttonOK,
      m_buttonMonthUp,
      m_buttonMonthDown;


    HWND                  m_hwndParent,
      m_hwndButtonOK,
      m_hwndButtonMonthUp,
      m_hwndButtonMonthDown;

    GR::String            m_strDayOfWeek[7];

    GR::String            m_strMonth[12];

    int                   m_iCurrentMonth,
                          m_iCurrentYear,

                          m_iActDay,
                          m_iActMonth,
                          m_iActYear,

                          m_iSelectedDay,
                          m_iSelectedMonth,
                          m_iSelectedYear;

    GR::DateTime::DateTime  m_ShowDate;

    virtual BOOL          PreTranslateMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );

    virtual LRESULT       WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    virtual void          OnButtonDown( POINT& ptPos );

    void                  _RegisterClass();

    void                  OnPaint( HDC hdc );

  };


}
 

#endif // INCLUDE_CDATEPICKERCTRL_H



