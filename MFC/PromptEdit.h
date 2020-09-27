#pragma once


// CPromptEdit

class CPromptEdit : public CEdit
{

	  DECLARE_DYNAMIC(CPromptEdit)

  public:

	  CPromptEdit();
	  virtual ~CPromptEdit();

    void                  SetPromptText( const char* szPrompt );
    bool                  IsActive();


  protected:

    CString               m_strPromptText,
                          m_strEditText;

    HFONT                 m_hFontNormal,
                          m_hFontItalic;

    bool                  m_bActive,
                          m_bShowingPromptText;

    void                  ToItalic();
    void                  ToNormal();

    void                  Initialize();


	  DECLARE_MESSAGE_MAP()


  public:


    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnDestroy();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);

    void                  SetWindowText( const CString& rString );
    void                  SetFont( CFont* pFont, BOOL bRedraw = TRUE );
    void                  GetWindowText( CString& rString ) const;
    int                   GetWindowTextLength() const;
};


