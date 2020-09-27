#ifndef _IPADDR_H							// Only include this file once
#define	_IPADDR_H

#include <ControlHelper/CEdit.h>



struct IPA_ADDR 
{
	unsigned char	nAddr1;	// The four bytes of the IP address
	unsigned char	nAddr2;
	unsigned char	nAddr3;
	unsigned char	nAddr4;
	UINT	nInError;		// Address byte with erroroneous/missing data (1..4)

  IPA_ADDR() :
    nAddr1( 0 ),
    nAddr2( 0 ),
    nAddr3( 0 ),
    nAddr4( 0 ),
    nInError( 0 )
  {
  }
};


// IP Address control style bits

#define	IPAS_NOVALIDATE	0x0001	// Don't validate fields as they are input


// Messages to/from the IP Address control
enum IPAM 
{
	IPAM_GETADDRESS = WM_USER,
	IPAM_SETADDRESS,
	IPAM_SETREADONLY
};

// Resource IDs for our edit controls
enum RID 
{
	IDC_ADDR1 = 1,
	IDC_ADDR2,
	IDC_ADDR3,
	IDC_ADDR4
};

/////////////////////////////////////////////////////////////////////////////
// CIPAddrEdit window

class CIPEdit;

class CIPSingleEdit : public CEdit
{

  // Construction
  public:
	  CIPSingleEdit();

	  friend class CIPEdit;

	  virtual ~CIPSingleEdit();

	  // Generated message map functions
  protected:
	  CIPEdit* m_pParent;
	  void                      SetParent(CIPEdit* pParent);
	  void                      OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	  void                      OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    virtual LRESULT           WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

};



class CIPEdit : public CWnd
{

  // Construction
  public:


	  CIPEdit();

    virtual ~CIPEdit();


	  static BOOL               Register();

    BOOL                      Create(DWORD dwStle, const RECT& rect, HWND hwndParent, UINT nID, DWORD dwExStyle=0);

	  BOOL                      GetAddress( BOOL bPrintErrors, IPA_ADDR* lpIPAddr );
	  BOOL                      SetAddress( IPA_ADDR* lpIPAddr );
	  BOOL                      SetAddress( BYTE ucAddr1, BYTE ucAddr2, BYTE ucAddr3, BYTE ucAddr4 );
    BOOL                      GetAddress( BYTE& ucAddr1, BYTE& ucAddr2, BYTE& ucAddr3, BYTE& ucAddr4 );
	  BOOL                      SetReadOnly( BOOL bReadOnly = TRUE );
    CIPSingleEdit*            GetEditControl( int nIndex );

    virtual LRESULT           WindowProc( UINT message, WPARAM wParam, LPARAM lParam );


	  friend class CIPSingleEdit;


  protected:

	  void                      OnChildChar( UINT nChar, UINT nRepCnt, UINT nFlags, CIPSingleEdit& child );
	  int                       OnCreate( LPCREATESTRUCT lpCreateStruct );
	  void                      OnSize( UINT nType, int cx, int cy );
	  void                      OnSetFocus( HWND hwndOld );
	  void                      OnPaint();
	  void                      OnEnable( BOOL bEnable = TRUE );
	  LONG                      OnSetFont( WPARAM wParam, LPARAM lParam );
	  BOOL                      OnEraseBkgnd( HDC hdc );
	  LONG                      OnGetAddress( WPARAM wParam, LPARAM lParam );
	  LONG                      OnSetAddress( WPARAM wParam, LPARAM lParam );
	  LONG                      OnSetReadOnly( WPARAM wParam, LPARAM lParam );


  private:

	  CIPSingleEdit             m_Addr[4];
	  RECT                      m_rcAddr[4];
	  RECT                      m_rcDot[3];

	  BOOL                      m_bEnabled;
	  BOOL                      m_bReadOnly;
	  BOOL                      m_bNoValidate;

	  static BOOL               m_bRegistered;


	  BOOL                      ParseAddressPart( CEdit& edit, int& n );

};

// WNDPROC

/////////////////////////////////////////////////////////////////////////////

#endif	// _IPADDR_H
