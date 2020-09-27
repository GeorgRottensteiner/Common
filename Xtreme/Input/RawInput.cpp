#include "RawInput.h"

#include <debug/debugclient.h>

#include <String/StringUtil.h>

#include <Memory/ByteBuffer.h>

#include <WinSys/RegistryHelper.h>

#include <Xtreme/Environment/XWindow.h>



extern "C"
{
  //#include <Hidpi.h>
  #include <hidsdi.h>
}

#pragma comment( lib, "hid.lib" )
#pragma comment( lib, "hidparse.lib" )



RawInput::RawInput() :
  m_RawMouseX( 0 ),
  m_RawMouseY( 0 )
{
}



RawInput::~RawInput()
{
}



bool RawInput::Initialize( GR::IEnvironment& Environment )
{
  if ( m_Initialized )
  {
    return true;
  }

  if ( !XBasicInput::Initialize( Environment ) )
  {
    return false;
  }

  if ( !EnumerateDevices() )
  {
    XBasicInput::Release();
    return false;
  }
  // register all possible input devices
  Xtreme::IAppWindow* pWindowService = ( Xtreme::IAppWindow* )Environment.Service( "Window" );
  HWND      hWnd = NULL;
  if ( pWindowService != NULL )
  {
    hWnd = (HWND)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
  }


  m_RegisteredDevices.resize( 5 );

  // gamepad
  m_RegisteredDevices[0].usUsagePage  = 0x01;
  m_RegisteredDevices[0].usUsage      = 0x05;
  m_RegisteredDevices[0].dwFlags      = 0;
  m_RegisteredDevices[0].hwndTarget   = hWnd;

  // joystick
  m_RegisteredDevices[1].usUsagePage  = 0x01;
  m_RegisteredDevices[1].usUsage      = 0x04;
  m_RegisteredDevices[1].dwFlags      = 0;
  m_RegisteredDevices[1].hwndTarget   = hWnd;

  // mouse
  m_RegisteredDevices[2].usUsagePage  = 0x01;
  m_RegisteredDevices[2].usUsage      = 0x02;
  m_RegisteredDevices[2].dwFlags      = 0;
  m_RegisteredDevices[2].hwndTarget   = hWnd;

  // keyboard
  m_RegisteredDevices[3].usUsagePage  = 0x01;
  m_RegisteredDevices[3].usUsage      = 0x06;
  m_RegisteredDevices[3].dwFlags      = 0;
  m_RegisteredDevices[3].hwndTarget   = hWnd;

  // card reader?
  m_RegisteredDevices[4].usUsagePage  = 0xff00;
  m_RegisteredDevices[4].usUsage      = 0x01;
  m_RegisteredDevices[4].dwFlags      = 0;
  m_RegisteredDevices[4].hwndTarget   = hWnd;

  /*
  Rid[2].usUsagePage = 0x0B;
  Rid[2].usUsage = 0x00;
  Rid[2].dwFlags = RIDEV_PAGEONLY;    //adds devices from telephony page
  Rid[2].hwndTarget = 0;

  Rid[3].usUsagePage = 0x0B;
  Rid[3].usUsage = 0x02;
  Rid[3].dwFlags = RIDEV_EXCLUDE;     //excludes answering machines
  Rid[3].hwndTarget = 0;
  */

  if ( RegisterRawInputDevices( &m_RegisteredDevices[0], m_RegisteredDevices.size(), sizeof( m_RegisteredDevices[0] ) ) == FALSE )
  {
    //registration failed. Call GetLastError for the cause of the error.
    DWORD     lastError = GetLastError();
    return false;
  }

  CreateVirtualKeys();
  m_Initialized = true;
  return true;
}



bool RawInput::Release()
{
  if ( !m_Initialized )
  {
    return true;
  }

  for ( size_t i = 0; i < m_Devices.size(); ++i )
  {
    DeviceInfo*     pInfo = (DeviceInfo*)m_Devices[i].m_pDevicePointer;
    if ( pInfo )
    {
      if ( pInfo->ToInputReportProtocol != NULL )
      {
        HidD_FreePreparsedData( pInfo->ToInputReportProtocol );
      }
      delete pInfo;
    }
  }

  for ( size_t i = 0; i < m_RegisteredDevices.size(); ++i )
  {
    m_RegisteredDevices[i].dwFlags = RIDEV_REMOVE;
    m_RegisteredDevices[i].hwndTarget = NULL;
  }
  RegisterRawInputDevices( &m_RegisteredDevices[0], m_RegisteredDevices.size(), sizeof( m_RegisteredDevices[0] ) );

  return XBasicInput::Release();
}



struct DirectInputCalibration
{
  GR::i32 LogicalMin;
  GR::i32 LogicalCenter;
  GR::i32 LogicalMax;
};


struct DIOBJECTATTRIBUTES
{
  DWORD dwFlags;
  WORD  wUsagePage;
  WORD  wUsage;
};



bool RawInput::EnumerateDevices()
{
  UINT        numDevices = 0;

  if ( GetRawInputDeviceList( NULL, &numDevices, sizeof( RAWINPUTDEVICELIST ) ) != 0 )
  {
    // Error?
    return false;
  }

  RAWINPUTDEVICELIST*   pDeviceData = new RAWINPUTDEVICELIST[numDevices];
  if ( GetRawInputDeviceList( pDeviceData, &numDevices, sizeof( RAWINPUTDEVICELIST ) ) == -1 )
  {
    // Error
    delete[] pDeviceData;
    return false;
  }

  int numTotalControls    = 0;
  int numTotalVirtualKeys = 0;
  int currentHIDJoystickOrGamepad = 0;

  for ( UINT i = 0; i < numDevices; ++i )
  {
    wchar_t             pathBuffer[260 + 4];

    UINT                bufferSize = UINT( sizeof( pathBuffer ) / sizeof( pathBuffer[0] ) );

    GR::WString         name;



    bufferSize = GetRawInputDeviceInfoW( pDeviceData[i].hDevice, RIDI_DEVICENAME, pathBuffer, &bufferSize );
    if ( sizeof pathBuffer / sizeof pathBuffer[0] < bufferSize )
    {
      // Negative UINTs werden sehr groß positiv
      dh::Log( "invalid HID (%d, %x): could not retrieve its path", i, pDeviceData[i].hDevice );
      continue;
    }
    GR::WString    originalName = pathBuffer;
    pathBuffer[1] = L'\\';

    HANDLE              ntHandle = CreateFileW( pathBuffer,
                                                0u,
                                                FILE_SHARE_READ | FILE_SHARE_WRITE, // wir können anderen Prozessen nicht verbieten, das HID zu benutzen
                                                NULL,
                                                OPEN_EXISTING,
                                                0u,
                                                NULL );
    if ( INVALID_HANDLE_VALUE == ntHandle )
    {
      dh::Log( "%d: invalid HID: could not open its handle", i );
      continue;
    }

    if ( GR::Strings::ToUpper( pathBuffer ).find( L"ROOT" ) != GR::String::npos )
    {
      // skip combined devices, we only want real devices
      //dh::Log( "skip combined device %d (%x)", i, pDeviceData[i].hDevice );
      continue;
    }

    {
      wchar_t     nameBuffer[255];
      if ( FALSE == HidD_GetManufacturerString( ntHandle, nameBuffer, 127 ) )
      {
        //wcscpy_s( nameBuffer, 255, L"(unknown)" );
        GR::WString   itemDesc = originalName;
        itemDesc = itemDesc.substr( 4 );

        std::list<GR::String>    params;

        GR::Strings::Split( GR::Convert::ToUTF8( itemDesc ), '#', params );

        if ( params.size() > 3 )
        {
          std::list<GR::String>::iterator    it( params.begin() );

          GR::String    id1 = *it;
          ++it;
          GR::String    id2 = *it;
          ++it;
          GR::String    id3 = *it;

          GR::String    regKey = "System\\CurrentControlSet\\Enum\\" + id1 + "\\" + id2 + "\\" + id3;

          GR::String    value;

          Registry::GetKey( HKEY_LOCAL_MACHINE, regKey, "DeviceDesc", value );
          /*

          // TODO - enumerate all buttons/axis of device
          DeviceInfo*     pInfo = new DeviceInfo();

          pInfo->RawInputHandle = pDeviceData[i].hDevice;

          tInputDevice    device;

          device.m_strDevice      = GR::Convert::ToStringA( value );
          device.m_pDevicePointer = (GR::up*)pInfo;
          AddDevice( device );
          //dh::Log( "Is %s", GR::Convert::ToStringA( value ).c_str() );
          */
          name = GR::Convert::ToUTF16( value );
        }
      }
      else
      {
        size_t manufacturerLength = wcslen( nameBuffer );
        nameBuffer[manufacturerLength++] = ' ';
        HidD_GetProductString( ntHandle, nameBuffer + manufacturerLength, ULONG( 255 - manufacturerLength ) );

        name = nameBuffer;
      }
      // Hier solltet ihr jetzt den Namen loggen oder so
      //dh::Log( "%d (%x): Is %s", i, pDeviceData[i].hDevice, GR::Convert::ToStringA( name ).c_str() );
    }


    DeviceInfo*     pInfo = new DeviceInfo();
    pInfo->RawInputHandle = pDeviceData[i].hDevice;

    {
      tInputDevice    device;

      device.m_Device         = GR::Convert::ToStringA( name );
      device.m_pDevicePointer = (GR::up*)pInfo;

      if ( pDeviceData[i].dwType == RIM_TYPEHID )
      {
        if ( FALSE == HidD_GetPreparsedData( ntHandle, &pInfo->ToInputReportProtocol ) )
        {
          dh::Log( "%d: invalid HID: no input report protocol", i );
          CloseHandle( ntHandle );
          continue;
        }
      }
      pInfo->DeviceIndex = AddDevice( device );
    }

    // fetch dinput axis mapping
    struct MappingAndCalibration
    {
      WORD                    usagePage; // Null falls unbenutzt
      WORD                    usage;
      bool                    isCalibrated;
      DirectInputCalibration  calibration;
      wchar_t                 name[32];
    }
    dInputAxisMapping[7] = { }; // Nullinitialisierung

    HIDD_ATTRIBUTES vendorAndProductID;
    if ( FALSE != HidD_GetAttributes( ntHandle, &vendorAndProductID ) )
    {
      wchar_t path[128] = L"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_????&PID_????\\Axes\\?";
      path[84] = wchar_t("0123456789ABCDEF"[(vendorAndProductID.VendorID >> 12) & 0xF]);
      path[85] = wchar_t("0123456789ABCDEF"[(vendorAndProductID.VendorID >>  8) & 0xF]);
      path[86] = wchar_t("0123456789ABCDEF"[(vendorAndProductID.VendorID >>  4) & 0xF]);
      path[87] = wchar_t("0123456789ABCDEF"[(vendorAndProductID.VendorID >>  0) & 0xF]);
      path[93] = wchar_t("0123456789ABCDEF"[(vendorAndProductID.ProductID >> 12) & 0xF]);
      path[94] = wchar_t("0123456789ABCDEF"[(vendorAndProductID.ProductID >>  8) & 0xF]);
      path[95] = wchar_t("0123456789ABCDEF"[(vendorAndProductID.ProductID >>  4) & 0xF]);
      path[96] = wchar_t("0123456789ABCDEF"[(vendorAndProductID.ProductID >>  0) & 0xF]);

      for ( size_t i = 0; i < sizeof dInputAxisMapping / sizeof dInputAxisMapping[0]; ++i )
      {
        path[103] = wchar_t( '0' + i );

        HKEY key = NULL;
        if ( 0 != RegOpenKeyExW( HKEY_CURRENT_USER, path, 0, KEY_READ, &key ) )
        {
          if ( 0 != RegOpenKeyExW( HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &key ) )
          {
            continue; // Die Achse wurde nicht überschrieben; nächste prüfen!
          }
        }

        // Viele Treiber weisen den HID-Achsen einen menschenlesbaren Namen zu; z.B. combined pedals oder throttle.
        // Den nehmen wir direkt mit, wenn wir schonmal hier sind. Beachtet, dass Strings aus der Registry nicht zwingend nullterminiert sind!

        {
          DWORD valueType = REG_NONE;
          DWORD valueSize = 0;
          RegQueryValueExW( key, L"", NULL, &valueType, NULL, &valueSize );
          if ( ( REG_SZ == valueType )
          &&   ( sizeof( dInputAxisMapping[i].name ) > valueSize ) )
          {
            RegQueryValueExW( key, L"", NULL, &valueType, LPBYTE( dInputAxisMapping[i].name ), &valueSize );
            // Der Name wurde bereits bei der Erzeugung genullt; kein Grund, die Null manuell anzufügen.
          }
        }
        DIOBJECTATTRIBUTES mapping;
        DWORD              valueType = REG_NONE;
        DWORD              valueSize = 0;
        RegQueryValueExW( key, L"Attributes", NULL, &valueType, NULL, &valueSize );
        if ( ( REG_BINARY == valueType )
        &&   ( sizeof( mapping ) == valueSize ) )
        {
          RegQueryValueExW( key, L"Attributes", NULL, &valueType, LPBYTE( &mapping ), &valueSize );
          if ( 0x15 > mapping.wUsagePage )
          {
            // Gültige Usage Page?
            dInputAxisMapping[i].usagePage = mapping.wUsagePage;
            dInputAxisMapping[i].usage     = mapping.wUsage;
          }
          // Hier solltet ihr Debug-Informationen ausgeben um das Ergebnis zu kontrollieren …
        }
        RegCloseKey( key );
      }
    }

    CloseHandle( ntHandle );

    tInputDevice&  device( m_Devices.back() );

    if ( pDeviceData[i].dwType == RIM_TYPEHID )
    {
      //dh::Log( "Is HID: check caps" );
      // not all objects have caps available?
      if ( SUCCEEDED( GetRawInputDeviceInfo( pDeviceData[i].hDevice, RIDI_PREPARSEDDATA, NULL, &bufferSize ) ) )
      {
        if ( bufferSize > 0 )
        {
          PHIDP_PREPARSED_DATA preparsedData = (PHIDP_PREPARSED_DATA)malloc( bufferSize );
          if ( preparsedData != NULL )
          {
            if ( ( bufferSize = GetRawInputDeviceInfo( pDeviceData[i].hDevice, RIDI_PREPARSEDDATA, preparsedData, &bufferSize ) ) >= 0 )
            {
              HRESULT   result = HidP_GetCaps( preparsedData, &pInfo->Caps );
              if ( result == HIDP_STATUS_SUCCESS )
              {
                /*
                dh::Log( "Has %d buttons", pInfo->Caps.NumberInputButtonCaps );
                dh::Log( "Has %d values", pInfo->Caps.NumberInputValueCaps );
                dh::Log( "Has %d input data indices", pInfo->Caps.NumberInputDataIndices );
                dh::Log( "Usage %02x, UsagePage %02x", pInfo->Caps.Usage, pInfo->Caps.UsagePage );
                */

                bool      isJoystickOrGamePad = false;
                GR::u32   deviceType = 0;

                if ( ( pInfo->Caps.UsagePage == 0x01 )
                &&   ( ( pInfo->Caps.Usage == 0x04 )
                ||     ( pInfo->Caps.Usage == 0x05 )
                ||     ( pInfo->Caps.Usage == 0x08 ) ) )
                {
                  isJoystickOrGamePad = true;
                  if ( currentHIDJoystickOrGamepad == 0 )
                  {
                    deviceType = Xtreme::VKT_JOYSTICK_1;
                  }
                  else if ( currentHIDJoystickOrGamepad == 1 )
                  {
                    deviceType = Xtreme::VKT_JOYSTICK_2;
                  }
                }

                pInfo->SizeOfInputReport = sizeof( RAWINPUTHEADER ) + sizeof( RAWHID ) + pInfo->Caps.InputReportByteLength;

                pInfo->ButtonClasses.resize( pInfo->Caps.NumberInputButtonCaps );
                pInfo->ButtonGlobalOffset.resize( pInfo->Caps.NumberInputButtonCaps );
                pInfo->AxisClasses.resize( pInfo->Caps.NumberInputValueCaps );
                pInfo->AxisGlobalOffset.resize( pInfo->Caps.NumberInputValueCaps );

                //Nun holen wir uns alle Klassen von Achsen und Knöpfen:
                int     numButtons = 0;
                if ( pInfo->Caps.NumberInputButtonCaps > 0 )
                {
                  struct Mapping
                  {
                    WORD    usagePage; // Null falls unbenutzt
                    WORD    usage;
                    wchar_t name[32];
                  }
                  dInputButtonMapping[128] = { }; // Nullinitialisierung



                  if ( HIDP_STATUS_SUCCESS != HidP_GetButtonCaps( HidP_Input, &pInfo->ButtonClasses[0], &pInfo->Caps.NumberInputButtonCaps, preparsedData ) )
                  {
                    dh::Log( "invalid HID: could not retrieve its button classes" );
                  }
                  for ( size_t buttonIndex = 0; buttonIndex < pInfo->ButtonClasses.size(); ++buttonIndex )
                  {
                    HIDP_BUTTON_CAPS& buttonClass( pInfo->ButtonClasses[buttonIndex] );

                    if ( buttonIndex > 0 )
                    {
                      pInfo->ButtonGlobalOffset[buttonIndex] = numButtons;
                    }

                    if ( buttonClass.IsRange )
                    {
                      numButtons += buttonClass.Range.UsageMax - buttonClass.Range.UsageMin + 1u;
                    }
                    else
                    {
                      buttonClass.Range.UsageMin = buttonClass.Range.UsageMax = buttonClass.NotRange.Usage;
                      buttonClass.Range.DataIndexMin = buttonClass.Range.DataIndexMax = buttonClass.NotRange.DataIndex;
                      buttonClass.IsRange = 1;
                      ++numButtons;
                    }
                  }

                  for ( size_t buttonIndex = 0; buttonIndex < pInfo->ButtonClasses.size(); ++buttonIndex )
                  {
                    HIDP_BUTTON_CAPS& currentClass( pInfo->ButtonClasses[buttonIndex] );

                    const USAGE firstUsage  = currentClass.Range.UsageMin;
                    const USAGE lastUsage   = currentClass.Range.UsageMax;

                    for ( WORD currentUsage = firstUsage, currentIndex = currentClass.Range.DataIndexMin; currentUsage <= lastUsage; ++currentUsage, ++currentIndex )
                    {
                      // Wurde der Name von DirectInput oder dem Treiber überschrieben?
                      GR::WString  toName;

                      toName = GR::Convert::ToUTF16( Misc::Format( "Button %1%" ) << pInfo->ButtonGlobalOffset[buttonIndex] + currentUsage - firstUsage );

                      for ( int k = 0; k < 7; ++k )
                      {
                        if ( ( currentClass.UsagePage == dInputButtonMapping[k].usagePage )
                        &&   ( currentUsage == dInputButtonMapping[k].usage ) )
                        {
                          toName = dInputButtonMapping[k].name;

                          // Optimierung: bei zukünfigen Durchläufen überspringen
                          dInputButtonMapping[k].usage = 0;
                          break;
                        }
                      }

                      Button button;
                      button.UsagePage = currentClass.UsagePage;
                      button.Usage     = currentUsage;
                      button.Index     = currentIndex;
                      button.Name      = toName;

                      XBasicInput::tInputCtrl   ctrl( pInfo->DeviceIndex, GR::Convert::ToStringA( button.Name ), Xtreme::CT_BUTTON, 0 );

                      ctrl.m_DeviceType = deviceType;
                      button.DeviceControlIndex = AddDeviceControl( pInfo->DeviceIndex, ctrl );
                      button.Index = (USHORT)pInfo->Buttons.size();

                      ++numTotalControls;
                      ++numTotalVirtualKeys;

                      pInfo->Buttons.push_back( button );
                    }
                  }
                }
                //dh::Log( "-has %d buttons", numButtons );

                int numAxis = 0;
                if ( pInfo->Caps.NumberInputValueCaps > 0 )
                {
                  if ( HIDP_STATUS_SUCCESS != HidP_GetValueCaps( HidP_Input, &pInfo->AxisClasses[0], &pInfo->Caps.NumberInputValueCaps, preparsedData ) )
                  {
                    dh::Log( "invalid HID: could not retrieve its axis classes" );
                  }
                  for ( size_t axisClassIndex = 0; axisClassIndex < pInfo->AxisClasses.size(); ++axisClassIndex )
                  {
                    HIDP_VALUE_CAPS& axisClass( pInfo->AxisClasses[axisClassIndex] );

                    if ( axisClassIndex > 0 )
                    {
                      pInfo->AxisGlobalOffset[axisClassIndex] = numAxis;
                    }

                    if ( axisClass.IsRange )
                    {
                      numAxis += axisClass.Range.UsageMax - axisClass.Range.UsageMin + 1u;
                    }
                    else
                    {
                      axisClass.Range.UsageMin = axisClass.Range.UsageMax = axisClass.NotRange.Usage;
                      axisClass.Range.DataIndexMin = axisClass.Range.DataIndexMax = axisClass.NotRange.DataIndex;
                      axisClass.IsRange = 1;
                      ++numAxis;
                    }
                  }

                  for ( size_t axisClassIndex = 0; axisClassIndex < pInfo->AxisClasses.size(); ++axisClassIndex )
                  {
                    HIDP_VALUE_CAPS& axisClass( pInfo->AxisClasses[axisClassIndex] );

                    if ( axisClass.UsagePage != HID_USAGE_PAGE_GENERIC )
                    {
                      continue;
                    }

                    const USAGE firstUsage = axisClass.Range.UsageMin;
                    const USAGE lastUsage = axisClass.Range.UsageMax;
                    for ( USHORT currentUsage = firstUsage; currentUsage <= lastUsage; ++currentUsage )
                    {
                      MappingAndCalibration* toHit = NULL;
                      switch ( currentUsage )
                      {
                        case HID_USAGE_GENERIC_X:
                          toHit = &dInputAxisMapping[0];
                          break;
                        case HID_USAGE_GENERIC_Y:
                          toHit = &dInputAxisMapping[1];
                          break;
                        case HID_USAGE_GENERIC_Z:
                          toHit = &dInputAxisMapping[2];
                          break;
                        case HID_USAGE_GENERIC_RX:
                          toHit = &dInputAxisMapping[3];
                          break;
                        case HID_USAGE_GENERIC_RY:
                          toHit = &dInputAxisMapping[4];
                          break;
                        case HID_USAGE_GENERIC_RZ:
                          toHit = &dInputAxisMapping[5];
                          break;
                        case HID_USAGE_GENERIC_SLIDER:
                          toHit = &dInputAxisMapping[6];
                          break;
                      }
                      if ( toHit != NULL )
                      {
                        toHit->usagePage  = HID_USAGE_PAGE_GENERIC;
                        toHit->usage      = currentUsage;
                      }
                    }
                  }

                  for ( size_t axisClassIndex = 0; axisClassIndex < pInfo->AxisClasses.size(); ++axisClassIndex )
                  {
                    HIDP_VALUE_CAPS&    currentClass( pInfo->AxisClasses[axisClassIndex] );

                    const USAGE firstUsage = currentClass.Range.UsageMin;
                    const USAGE lastUsage = currentClass.Range.UsageMax;
                    for ( WORD currentUsage = firstUsage, currentIndex = currentClass.Range.DataIndexMin; currentUsage <= lastUsage; ++currentUsage, ++currentIndex )
                    {
                      bool            isCalibrated      = false;
                      GR::i32         calibratedMinimum = 0;
                      GR::i32         calibratedMaximum = 0;
                      GR::i32         calibratedCenter  = 0;
                      GR::WString toName = GR::Convert::ToUTF16( Misc::Format( "Unknown UP/P %01x:4%/%02x:2%" )
                                                      << currentClass.UsagePage
                                                      << currentUsage );

                      if ( currentClass.UsagePage == 0x01 )
                      {
                        switch ( currentUsage )
                        {
                          case 0x30:
                            toName = L"X Axis";
                            break;
                          case 0x31:
                            toName = L"Y Axis";
                            break;
                          case 0x32:
                            toName = L"Z Axis";
                            break;
                          case 0x33:
                            toName = L"Rx Axis";
                            break;
                          case 0x34:
                            toName = L"Ry Axis";
                            break;
                          case 0x35:
                            toName = L"Rz Axis";
                            break;
                          case 0x36:
                            toName = L"Slider";
                            break;
                          case 0x37:
                            toName = L"Dial";
                            break;
                          case 0x38:
                            toName = L"Wheel";
                            break;
                          case 0x39:
                            toName = L"Hat Switch";
                            break;
                        }
                      }

                      //dh::Log( "Axis - currentUsage %x, page %x", currentUsage, currentClass.UsagePage );

                      // Wurden Kalibrierungsdaten oder Name überschrieben?
                      for ( int k = 0; k < 7; ++k )
                      {
                        MappingAndCalibration&  mapping( dInputAxisMapping[k] );
                        if ( ( currentClass.UsagePage == mapping.usagePage )
                        &&   ( currentUsage == mapping.usage ) )
                        {
                          if ( wcslen( mapping.name ) )
                          {
                            toName = mapping.name;
                          }
                          isCalibrated      = mapping.isCalibrated;
                          if ( mapping.isCalibrated )
                          {
                            calibratedMinimum = mapping.calibration.LogicalMin;
                            calibratedCenter  = mapping.calibration.LogicalCenter;
                            calibratedMaximum = mapping.calibration.LogicalMax;
                          }
                          mapping.usage = 0; // Optimierung: bei zukünfigen Durchläufen überspringen
                          break;
                        }
                      }

                      Axis axis;
                      axis.UsagePage      = currentClass.UsagePage;
                      axis.Usage          = currentUsage;
                      axis.Index          = currentIndex;
                      axis.LogicalMinimum = currentClass.LogicalMin;
                      axis.LogicalMaximum = currentClass.LogicalMax;
                      if ( currentClass.LogicalMax == -1 )
                      {
                        axis.LogicalMaximum = 65535;
                      }
                      axis.IsCalibrated   = isCalibrated;
                      if ( isCalibrated )
                      {
                        axis.LogicalCalibratedMinimum = calibratedMinimum;
                        axis.LogicalCalibratedMaximum = calibratedMaximum;
                        axis.LogicalCalibratedCenter  = calibratedCenter;
                      }
                      axis.PhysicalMinimum = float(currentClass.PhysicalMin);
                      axis.PhysicalMaximum = float(currentClass.PhysicalMax);
                      axis.Name            = toName;
                      axis.Index           = (USHORT)pInfo->Axis.size();

                      if ( ( currentClass.UsagePage == 0x01 )
                      &&   ( currentUsage == 0x39 ) )
                      {
                        // Hat Switch
                        axis.Type = Xtreme::CT_HAT_SWITCH;
                        XBasicInput::tInputCtrl   ctrl( pInfo->DeviceIndex, GR::Convert::ToStringA( axis.Name ), Xtreme::CT_HAT_SWITCH, 0 );

                        ctrl.m_DeviceType       = deviceType;
                        ctrl.m_Min              = axis.LogicalMinimum;
                        ctrl.m_Max              = axis.LogicalMaximum;
                        axis.DeviceControlIndex = AddDeviceControl( pInfo->DeviceIndex, ctrl );
                        ++numTotalControls;
                        numTotalVirtualKeys += 4;
                      }
                      else
                      {
                        // two controls per Axis (for digital)
                        axis.Type = Xtreme::CT_AXIS;

                        XBasicInput::tInputCtrl   ctrl( pInfo->DeviceIndex, GR::Convert::ToStringA( axis.Name ), Xtreme::CT_AXIS, 0 );

                        ctrl.m_DeviceType     = deviceType;
                        ctrl.m_Min            = axis.LogicalMinimum;
                        ctrl.m_Max            = axis.LogicalMaximum;
                        axis.DeviceControlIndex = AddDeviceControl( pInfo->DeviceIndex, ctrl );
                        numTotalVirtualKeys += 2;
                        ++numTotalControls;
                      }

                      pInfo->Axis.push_back( axis );
                    }
                  }
                }
                //dh::Log( "-has %d axis", numAxis );

                if ( isJoystickOrGamePad )
                {
                  ++currentHIDJoystickOrGamepad;
                }
              }
              else
              {
                dh::Log( "HidP_GetCaps failed (%x)", result );
              }
            }
            else
            {
              dh::Log( "inner GetRawInputDeviceInfo failed" );
            }
            free( preparsedData );
          }
        }
      }
      else
      {
        dh::Log( "GetRawInputDeviceInfo failed" );
      }
    }
    else if ( pDeviceData[i].dwType == RIM_TYPEKEYBOARD )
    {
      RID_DEVICE_INFO     deviceInfo;

      bufferSize        = sizeof( deviceInfo );
      deviceInfo.cbSize = sizeof( deviceInfo );
      if ( GetRawInputDeviceInfoA( pDeviceData[i].hDevice, RIDI_DEVICEINFO, &deviceInfo, &bufferSize ) > 0 )
      {
        /*
        dh::Log( "Has %d keys, %d function keys, %d indicators",
                deviceInfo.keyboard.dwNumberOfKeysTotal,
                deviceInfo.keyboard.dwNumberOfFunctionKeys,
                deviceInfo.keyboard.dwNumberOfIndicators );
                */

        GR::u32     maxKeys = deviceInfo.keyboard.dwNumberOfKeysTotal;
        if ( maxKeys > Xtreme::LAST_KEYBOARD_KEY + 1 )
        {
          maxKeys = Xtreme::LAST_KEYBOARD_KEY - 1;
        }

        for ( GR::u32 j = 0; j < maxKeys; ++j )
        {
          Button button;

          wchar_t     keyName[2048];

          memset( keyName, 0, 2048 );
          GetKeyNameTextW( j << 16, keyName, 2048 );
          if ( keyName[0] == 0 )
          {
            // extended key?
            GetKeyNameTextW( ( j << 16 ) | 0x1000, keyName, 2048 );
          }

          button.Index  = (USHORT)j;
          button.Name = keyName;

          XBasicInput::tInputCtrl   ctrl( pInfo->DeviceIndex, GR::Convert::ToStringA( button.Name ), Xtreme::CT_BUTTON, 0 );
          ctrl.m_VirtualKeyCode = MapVirtualKey( j, 1 );
          ctrl.m_DeviceType     = Xtreme::VKT_KEYBOARD;

          button.DeviceControlIndex = AddDeviceControl( pInfo->DeviceIndex, ctrl );

          ++numTotalControls;
          ++numTotalVirtualKeys;

          pInfo->Buttons.push_back( button );
        }
      }
    }
    else if ( pDeviceData[i].dwType == RIM_TYPEMOUSE )
    {
      RID_DEVICE_INFO     deviceInfo;

      bufferSize        = sizeof( deviceInfo );
      deviceInfo.cbSize = sizeof( deviceInfo );
      if ( GetRawInputDeviceInfoA( pDeviceData[i].hDevice, RIDI_DEVICEINFO, &deviceInfo, &bufferSize ) > 0 )
      {
        /*
        dh::Log( "Has %d buttons, Horz wheel %d",
                deviceInfo.mouse.dwNumberOfButtons,
                deviceInfo.mouse.fHasHorizontalWheel );*/

        XBasicInput::tInputCtrl   ctrl( pInfo->DeviceIndex, GR::Convert::ToStringA( L"Mouse X" ), Xtreme::CT_RELATIVE_AXIS, 0 );

        ctrl.m_DeviceType       = Xtreme::VKT_DEFAULT_MOUSE | Xtreme::VKT_X_AXIS;
        int deviceControlIndex  = AddDeviceControl( pInfo->DeviceIndex, ctrl );

        XBasicInput::tInputCtrl   ctrlY( pInfo->DeviceIndex, GR::Convert::ToStringA( L"Mouse Y" ), Xtreme::CT_RELATIVE_AXIS, 0 );

        ctrlY.m_DeviceType       = Xtreme::VKT_DEFAULT_MOUSE | Xtreme::VKT_Y_AXIS;
        deviceControlIndex  = AddDeviceControl( pInfo->DeviceIndex, ctrlY );

        //numTotalVirtualKeys += 2;
        ++numTotalControls;

        for ( GR::u32 k = 0; k < deviceInfo.mouse.dwNumberOfButtons; ++k )
        {
          XBasicInput::tInputCtrl   ctrl( pInfo->DeviceIndex, Misc::Format( "Mouse Button %1%" ) << k + 1, Xtreme::CT_BUTTON, 0 );

          ctrl.m_DeviceType         = Xtreme::VKT_DEFAULT_MOUSE;;
          int deviceControlIndex = AddDeviceControl( pInfo->DeviceIndex, ctrl );
        }

        // x/y axis
        numTotalVirtualKeys += 4;
        // buttons
        numTotalVirtualKeys += deviceInfo.mouse.dwNumberOfButtons;
      }
    }

    /*
    if ( GetRawInputDeviceInfoW( pDeviceData[i].hDevice, RIDI_DEVICENAME, tempName, &bufferSize ) > 0 )
    {
      //dh::Log( "Found %s", tempName );
      GR::WString     deviceName = tempName;

      if ( GR::Strings::ToUpper( deviceName ).find( L"ROOT" ) != GR::String::npos )
      {
        // skip combined devices, we only want real devices
        continue;
      }

      //\\??\\ACPI#PNP0303#3&13c0b0c5&0#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}

      // This string mirrors the device's entry in the Registry; parsing it therefore allows us to find the relevant Registry key, which contains further information on the device. So the first step is to break down the relevant part of the string:
      // remove the \??\
      //item = item.Substring( 4 );
      GR::WString   itemDesc = tempName;
      itemDesc = itemDesc.substr( 4 );

      std::list<GR::WString>    params;

      GR::Strings::SplitW( itemDesc, L'#', params );

      if ( params.size() > 3 )
      {
        std::list<GR::WString>::iterator    it( params.begin() );

        GR::WString   id1 = *it;
        ++it;
        GR::WString   id2 = *it;
        ++it;
        GR::WString   id3 = *it;

        GR::WString   regKey = L"System\\CurrentControlSet\\Enum\\" + id1 + L"\\" + id2 + L"\\" + id3;

        GR::WString   value;

        Registry::GetKeyW( HKEY_LOCAL_MACHINE, regKey, L"DeviceDesc", value );


        // TODO - enumerate all buttons/axis of device
        DeviceInfo*     pInfo = new DeviceInfo();

        pInfo->RawInputHandle = pDeviceData[i].hDevice;

        tInputDevice    device;

        device.m_strDevice      = GR::Convert::ToStringA( value );
        device.m_pDevicePointer = (GR::up*)pInfo;
        AddDevice( device );
        //dh::Log( "Is %s", GR::Convert::ToStringA( value ).c_str() );
      }
    }
    bufferSize = sizeof( deviceInfo );
    deviceInfo.cbSize = sizeof( deviceInfo );
    if ( GetRawInputDeviceInfoA( pDeviceData[i].hDevice, RIDI_DEVICEINFO, &deviceInfo, &bufferSize ) > 0 )
    {
      //dh::Log( "Found %s", deviceInfo.. );
    }*/

    /*
    dh::Log( "Device %s", GR::Convert::ToStringA( pInfo->Name ).c_str() );
    for ( size_t i = 0; i < pInfo->Buttons.size(); ++i )
    {
      dh::Log( "-Button %s", GR::Convert::ToStringA( pInfo->Buttons[i].name ).c_str() );
    }
    for ( size_t i = 0; i < pInfo->Axis.size(); ++i )
    {
      dh::Log( "-Axis %s", GR::Convert::ToStringA( pInfo->Axis[i].name ).c_str() );
    }*/
  }

  m_VirtualKeys = numTotalVirtualKeys + 1;

  delete[] pDeviceData;
  return true;
}



void RawInput::CreateVirtualKeys()
{
  // plus Fake-Mauswheel
  m_VirtualKeys += 2;

  tInputCtrl    CtrlMWUp;

  CtrlMWUp.m_DeviceType      = Xtreme::VKT_DEFAULT_MOUSE;
  CtrlMWUp.m_APIDataOffset  = 0;
  CtrlMWUp.m_Name           = "Mousewheel up";
  CtrlMWUp.m_Type           = Xtreme::CT_UNKNOWN;

  m_Controls.push_back( CtrlMWUp );

  tInputCtrl    CtrlMWDown;

  CtrlMWDown.m_DeviceType   = Xtreme::VKT_DEFAULT_MOUSE;
  CtrlMWDown.m_APIDataOffset = 1;
  CtrlMWDown.m_Name         = "Mousewheel down";
  CtrlMWDown.m_Type         = Xtreme::CT_UNKNOWN;

  m_Controls.push_back( CtrlMWDown );



  m_pVirtualKey = new tVirtualKey[m_VirtualKeys];

  m_pVirtualKey[0].m_Name         = "No Key";
  m_pVirtualKey[0].m_GlobalIndex  = 0;
  m_pVirtualKey[0].m_EnumIndex    = Xtreme::KEY_INVALID;

  int   curVKey = 1;
  int   currentMButton = 0;

  for ( size_t i = 0; i < m_Controls.size(); ++i )
  {
    tInputCtrl&   Ctrl = m_Controls[i];

    if ( Ctrl.m_Device != -1 )
    {
      tInputCtrl&   deviceCtrl = m_Devices[Ctrl.m_Device].m_Controls[Ctrl.m_DeviceControlIndex];
      deviceCtrl.m_VirtualIndex  = curVKey;
    }

    m_pVirtualKey[curVKey].m_GlobalIndex        = (GR::u32)i;
    m_pVirtualKey[curVKey].m_Name               = Ctrl.m_Name;
    m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
    Ctrl.m_VirtualIndex        = curVKey;

    ++curVKey;

    if ( Ctrl.m_Type == Xtreme::CT_BUTTON )
    {
      if ( Ctrl.m_DeviceType & Xtreme::VKT_KEYBOARD )
      {
        m_pVirtualKey[curVKey - 1].m_EnumIndex = (Xtreme::eInputDefaultButtons)Ctrl.m_DeviceControlIndex;

        if ( Ctrl.m_VirtualKeyCode != 0 )
        {
          m_VirtualKeyToXtremeKey.insert( std::make_pair( (Xtreme::eInputDefaultButtons)m_VirtualCode[Ctrl.m_VirtualKeyCode], Ctrl.m_VirtualIndex ) );
        }
        if ( Ctrl.m_DeviceControlIndex < Xtreme::LAST_KEYBOARD_KEY )
        {
          m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)Ctrl.m_DeviceControlIndex, Ctrl.m_VirtualIndex ) );
        }
      }
      else if ( Ctrl.m_DeviceType & Xtreme::VKT_DEFAULT_MOUSE )
      {
        if ( currentMButton < 3 )
        {
          m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)( Xtreme::MOUSE_BUTTON_1 + currentMButton ), Ctrl.m_VirtualIndex ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = (Xtreme::eInputDefaultButtons)( Xtreme::MOUSE_BUTTON_1 + currentMButton );
          m_pVirtualKey[curVKey - 1].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
        }
        ++currentMButton;
      }
    }
    else if ( Ctrl.m_Type == Xtreme::CT_AXIS )
    {
      m_pVirtualKey[curVKey - 1].m_Name = Ctrl.m_Name + " down";

      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " up";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;
    }
    else if ( Ctrl.m_Type == Xtreme::CT_RELATIVE_AXIS )
    {
      m_pVirtualKey[curVKey - 1].m_Name = Ctrl.m_Name + " down";

      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " up";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;

      if ( Ctrl.m_DeviceType & Xtreme::VKT_DEFAULT_MOUSE )
      {
        if ( Ctrl.m_DeviceType & Xtreme::VKT_X_AXIS )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_LEFT, curVKey - 2 ) );
          m_pVirtualKey[curVKey - 2].m_EnumIndex = Xtreme::MOUSE_LEFT;
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_RIGHT, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::MOUSE_RIGHT;
        }
        if ( Ctrl.m_DeviceType & Xtreme::VKT_Y_AXIS )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_UP, curVKey - 2 ) );
          m_pVirtualKey[curVKey - 2].m_EnumIndex = Xtreme::MOUSE_UP;
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_DOWN, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::MOUSE_DOWN;
        }
      }
    }
    else if ( Ctrl.m_Type == Xtreme::CT_HAT_SWITCH )
    {
      m_pVirtualKey[curVKey - 1].m_Name = Ctrl.m_Name + " left";

      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " right";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;
      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " up";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;
      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " down";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;
    }
    else if ( Ctrl.m_Type == Xtreme::CT_UNKNOWN )
    {
      // manuell gefakte Keys (Mousewheel)
      // Mousewheel
      if ( Ctrl.m_DeviceType == Xtreme::VKT_DEFAULT_MOUSE )
      {
        if ( Ctrl.m_APIDataOffset == 0 )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_WHEEL_UP, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::MOUSE_WHEEL_UP;
        }
        if ( Ctrl.m_APIDataOffset == 1 )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_WHEEL_DOWN, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::MOUSE_WHEEL_DOWN;
        }
      }
    }
    else
    {
      dh::Log( "unset type" );
    }
    //dh::Log( "Ctrl %d = VK %d", i, Ctrl.m_VirtualIndex );
  }
  if ( curVKey != m_VirtualKeys )
  {
    dh::Error( "Mismatching number of virtual keys!" );
  }
  /*
  for ( GR::u32 i = 0; i < m_VirtualKeys; ++i )
  {
    dh::Log( "VK %d = %s", i, m_pVirtualKey[i].m_Name.c_str() );
  }
  */
}



bool RawInput::HasDeviceForceFeedback( GR::u32 Device ) const
{
  return false;
}



GR::u32 RawInput::DeviceForceFeedbackAxisCount( GR::u32 Device ) const
{
  return 0;
}



bool RawInput::SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const
{
  return false;
}



void RawInput::Poll()
{
}



void RawInput::Init()
{
}



void RawInput::Exit()
{
}



void RawInput::Update( const float fElapsedTime )
{
}



void RawInput::InterpretHIDData( RAWINPUT* raw, DeviceInfo* pDevice )
{
  if ( pDevice == NULL )
  {
    return;
  }

  UINT    bufferSize = 0;

  /*
  dh::Log( "Has %d button classes", pDevice->Caps.NumberInputButtonCaps );
  dh::Log( "Has %d value classes", pDevice->Caps.NumberInputValueCaps );
  dh::Log( "Has %d input data indices", pDevice->Caps.NumberInputDataIndices );
  */

  if ( pDevice->Caps.NumberInputButtonCaps > 0 )
  {
    #define MAX_BUTTONS		128

    USAGE         usage[MAX_BUTTONS];
    ULONG         usageLength = pDevice->Buttons.size();

    for ( size_t buttonIndex = 0; buttonIndex < pDevice->Buttons.size(); ++buttonIndex )
    {
      pDevice->Buttons[buttonIndex].Pushed = false;
    }

    for ( size_t buttonClassIndex = 0; buttonClassIndex < pDevice->ButtonClasses.size(); ++buttonClassIndex )
    {
      if ( HidP_GetUsages( HidP_Input,
                           pDevice->ButtonClasses[buttonClassIndex].UsagePage,
                           0,
                           usage,
                           &usageLength,
                           pDevice->ToInputReportProtocol,
                           (PCHAR)raw->data.hid.bRawData,
                           raw->data.hid.dwSizeHid ) == HIDP_STATUS_SUCCESS )
      {
        for ( ULONG i = 0; i < usageLength; i++ )
        {
          int     globalButtonIndex = pDevice->ButtonGlobalOffset[buttonClassIndex] + usage[i] - pDevice->ButtonClasses[buttonClassIndex].Range.UsageMin;
          pDevice->Buttons[globalButtonIndex].Pushed = true;
          /*
          dh::Log( "Button %s (%d) is pressed",
            GR::Convert::ToStringA( pDevice->Buttons[globalButtonIndex].name ).c_str(),
            usage[i] - pDevice->buttonClasses[buttonClassIndex].Range.UsageMin );
          */
        }
      }
    }
    for ( size_t buttonIndex = 0; buttonIndex < pDevice->Buttons.size(); ++buttonIndex )
    {
      XBasicInput::tInputCtrl&    Ctrl( m_Devices[pDevice->DeviceIndex].m_Controls[pDevice->Buttons[buttonIndex].DeviceControlIndex] );
      SetButtonPressed( Ctrl.m_VirtualIndex, pDevice->Buttons[buttonIndex].Pushed );
    }

    /*
    PHIDP_BUTTON_CAPS     pButtonCaps = (PHIDP_BUTTON_CAPS)malloc( sizeof( HIDP_BUTTON_CAPS ) * Caps.NumberInputButtonCaps );
    USHORT capsLength = Caps.NumberInputButtonCaps;
    if ( HidP_GetButtonCaps( HidP_Input, pButtonCaps, &capsLength, preparsedData ) == HIDP_STATUS_SUCCESS )
    {
      int numberOfButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

    }
    free( pButtonCaps );
    */
  }
  if ( pDevice->Caps.NumberInputValueCaps > 0 )
  {
    //int numberOfValues = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;
    for ( int i = 0; i < pDevice->Caps.NumberInputValueCaps; i++ )
    {
      ULONG   value = 0;

      if ( pDevice->AxisClasses[i].ReportCount == 1 )
      {
        if ( HidP_GetUsageValue( HidP_Input,
                                 pDevice->AxisClasses[i].UsagePage,
                                 0,
                                 pDevice->AxisClasses[i].Range.UsageMin,
                                 &value,
                                 pDevice->ToInputReportProtocol,
                                 (PCHAR)raw->data.hid.bRawData,
                                 raw->data.hid.dwSizeHid ) == HIDP_STATUS_SUCCESS )
        {
          int     globalAxis = pDevice->AxisGlobalOffset[i];

          pDevice->Axis[globalAxis].Value = value;
          /*
          dh::Log( "Value Index %02d: UsagePage %02x, UsageMin:%02x, Value:%d (Min %d, Max %d)",
              i,
              pDevice->axisClasses[i].UsagePage,
              pDevice->axisClasses[i].Range.UsageMin,
              value,
              pDevice->axisClasses[i].LogicalMin,  pDevice->axisClasses[i].LogicalMax );*/

          XBasicInput::tInputCtrl&    Ctrl( m_Devices[pDevice->DeviceIndex].m_Controls[pDevice->Axis[globalAxis].DeviceControlIndex] );


          if ( pDevice->Axis[globalAxis].Type == Xtreme::CT_HAT_SWITCH )
          {
            // Hat Switch
            //dh::Log( "Hat Switch set to %d", (int)value );
            if ( (int)value != 0 )
            {
              int     degrees = (int)( ( (int)value - Ctrl.m_Min ) * 360.0f / ( Ctrl.m_Max - Ctrl.m_Min + 1 ) );
              //dh::Log( "degrees %d", degrees );

              SetButtonPressed( Ctrl.m_VirtualIndex, ( ( degrees >= 225 ) && ( degrees <= 315 ) ) );
              SetButtonPressed( Ctrl.m_VirtualIndex + 1, ( ( degrees >= 45 ) && ( degrees <= 135 ) ) );
              SetButtonPressed( Ctrl.m_VirtualIndex + 2, ( ( degrees <= 45 ) || ( degrees >= 315 ) ) );
              SetButtonPressed( Ctrl.m_VirtualIndex + 3, ( ( degrees >= 135 ) && ( degrees <= 225 ) ) );
            }
            else
            {
              // Mittelstellung
              SetButtonPressed( Ctrl.m_VirtualIndex + 0, false );
              SetButtonPressed( Ctrl.m_VirtualIndex + 1, false );
              SetButtonPressed( Ctrl.m_VirtualIndex + 2, false );
              SetButtonPressed( Ctrl.m_VirtualIndex + 3, false );
            }
          }
          else
          {
            // Axis

            // minus
            SetButtonPressed( Ctrl.m_VirtualIndex, ( (int)value < ( Ctrl.m_Max - Ctrl.m_Min ) / 2 - m_InputJoyTreshold ) );
            // plus
            SetButtonPressed( Ctrl.m_VirtualIndex + 1, ( (int)value > ( Ctrl.m_Max - Ctrl.m_Min ) / 2 + m_InputJoyTreshold ) );
          }
        }
        else
        {
          dh::Log( "HidP_GetUsageValue failed" );
        }
      }
      else
      {
        // magnet stripe reader etc.
        USHORT      byteLength = (USHORT)( ( pDevice->AxisClasses[i].BitSize * pDevice->AxisClasses[i].ReportCount + 7 ) / 8 );
        GR::u8*     pValueData = (GR::u8*)malloc( byteLength );

        NTSTATUS    status = HidP_GetUsageValueArray( HidP_Input,
                                      pDevice->AxisClasses[i].UsagePage,
                                      0,
                                      pDevice->AxisClasses[i].Range.UsageMin,
                                      (PCHAR)pValueData,
                                      byteLength,
                                      pDevice->ToInputReportProtocol,
                                      (PCHAR)raw->data.hid.bRawData,
                                      raw->data.hid.dwSizeHid );
        if ( status == HIDP_STATUS_SUCCESS )
        {
          ByteBuffer    data( pValueData, byteLength );
          dh::Log( "Value Array Index %02d: UsageMin:%02x, Value:%s", i, pDevice->AxisClasses[i].Range.UsageMin, data.ToHexString().c_str() );
        }
        else
        {
          // HIDP_STATUS_NOT_VALUE_ARRAY = 0x0c011000b;
          dh::Log( "HidP_GetUsageValueArray failed (%x) for Usage %02x", status, pDevice->AxisClasses[i].Range.UsageMin );
        }
        free( pValueData );
      }
    }
  }
}



void RawInput::InterpretKeyboardData( RAWINPUT* raw, DeviceInfo* pDevice )
{
  UINT virtualKey = raw->data.keyboard.VKey;
  UINT scanCode   = raw->data.keyboard.MakeCode;
  UINT flags      = raw->data.keyboard.Flags;

  if ( virtualKey == 255 )
  {
    // discard "fake keys" which are part of an escaped sequence
    return;
  }
  else if ( virtualKey == VK_SHIFT )
  {
    // correct left-hand / right-hand SHIFT
    virtualKey = MapVirtualKey( scanCode, MAPVK_VSC_TO_VK_EX );
  }
  else if ( virtualKey == VK_NUMLOCK )
  {
    // correct PAUSE/BREAK and NUM LOCK silliness, and set the extended bit
    scanCode = ( MapVirtualKey( virtualKey, MAPVK_VK_TO_VSC ) | 0x100 );
  }

  // e0 and e1 are escape sequences used for certain special keys, such as PRINT and PAUSE/BREAK.
  // see http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
  const bool isE0 = ( ( flags & RI_KEY_E0 ) != 0 );
  const bool isE1 = ( ( flags & RI_KEY_E1 ) != 0 );
  if ( isE1 )
  {
    // for escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
    // however, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
    if ( virtualKey == VK_PAUSE )
    {
      scanCode = 0x45;
    }
    else
    {
      scanCode = MapVirtualKey( virtualKey, MAPVK_VK_TO_VSC );
    }
  }


  switch ( virtualKey )
  {
    // right-hand CONTROL and ALT have their e0 bit set
    case VK_CONTROL:
      if ( isE0 )
      {
        virtualKey = VK_RCONTROL;
      }
      else
      {
        virtualKey = VK_LCONTROL;
      }
      break;
    case VK_MENU:
      if ( isE0 )
      {
        virtualKey = VK_RMENU;
      }
      else
      {
        virtualKey = VK_LMENU;
      }
      break;
      // NUMPAD ENTER has its e0 bit set
    case VK_RETURN:
      if ( isE0 )
      {
        virtualKey = VK_SEPARATOR; // NUMPAD_ENTER;
      }
      break;
      // the standard INSERT, DELETE, HOME, END, PRIOR and NEXT keys will always have their e0 bit set, but the
      // corresponding keys on the NUMPAD will not.
    case VK_INSERT:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD0;
      }
      break;
    case VK_DELETE:
      if ( !isE0 )
      {
        virtualKey = VK_DECIMAL;
      }
      break;
    case VK_HOME:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD7;
      }
      break;
    case VK_END:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD1;
      }
      break;
    case VK_PRIOR:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD9;
      }
      break;
    case VK_NEXT:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD3;
      }
      break;
      // the standard arrow keys will always have their e0 bit set, but the
      // corresponding keys on the NUMPAD will not.
    case VK_LEFT:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD4;
      }
      break;
    case VK_RIGHT:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD6;
      }
      break;
    case VK_UP:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD8;
      }
      break;
    case VK_DOWN:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD2;
      }
      break;
      // NUMPAD 5 doesn't have its e0 bit set
    case VK_CLEAR:
      if ( !isE0 )
      {
        virtualKey = VK_NUMPAD5;
      }
      break;
  }

  if ( raw->data.keyboard.Flags & RI_KEY_BREAK )
  {
    // released key
    SetButtonPressed( m_Devices[pDevice->DeviceIndex].m_Controls[pDevice->Buttons[raw->data.keyboard.MakeCode].DeviceControlIndex].m_VirtualIndex, false );
  }
  else
  {
    // set key
    SetButtonPressed( m_Devices[pDevice->DeviceIndex].m_Controls[pDevice->Buttons[raw->data.keyboard.MakeCode].DeviceControlIndex].m_VirtualIndex, true );
  }
  /*
  char    szTempOutput[5000];
  sprintf_s(szTempOutput, 5000, TEXT(" Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n"),
          raw->data.keyboard.MakeCode,
          raw->data.keyboard.Flags,
          raw->data.keyboard.Reserved,
          raw->data.keyboard.ExtraInformation,
          raw->data.keyboard.Message,
          raw->data.keyboard.VKey);
  OutputDebugString( szTempOutput );
  */
}



void RawInput::HandleMouseAxis( int Delta, GR::u32 NegativeKey, GR::u32 PositiveKey )
{
  if ( abs( Delta ) > m_InputMouseTreshold )
  {
    if ( Delta < 0 )
    {
      SetButtonPressed( NegativeKey, true );
      SetButtonPressed( PositiveKey, false );
    }
    else
    {
      SetButtonPressed( NegativeKey, false );
      SetButtonPressed( PositiveKey, true );
    }
  }
  else
  {
    SetButtonPressed( NegativeKey, false );
    SetButtonPressed( PositiveKey, false );
  }
}



BOOL RawInput::WindowProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam )
{
  switch ( wMessage )
  {
    case WM_INPUT:
      if ( GET_RAWINPUT_CODE_WPARAM( wParam ) == RIM_INPUT )
      {
        UINT    bufferSize = 0;

        GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof( RAWINPUTHEADER ) );
        LPBYTE lpb = new BYTE[bufferSize];
        if ( lpb == NULL )
        {
            return 0;
        }

        if ( GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, lpb, &bufferSize, sizeof( RAWINPUTHEADER ) ) != bufferSize )
        {
          OutputDebugString( TEXT("GetRawInputData does not return correct size !\n" ) );
        }

        RAWINPUT* raw = (RAWINPUT*)lpb;

        tInputDevice*   pDevice = NULL;

        tVectDevices::iterator    itD( m_Devices.begin() );
        while ( itD != m_Devices.end() )
        {
          DeviceInfo*    pDeviceInfo = (DeviceInfo*)itD->m_pDevicePointer;

          if ( pDeviceInfo->RawInputHandle == raw->header.hDevice )
          {
            pDevice = &( *itD );
            //dh::Log( "Input for %s", itD->m_strDevice.c_str() );
            break;
          }

          ++itD;
        }
        if ( pDevice == NULL )
        {
          dh::Log( "Input for unknown device (%x)", raw->header.hDevice );
        }

        if ( raw->header.dwType == RIM_TYPEKEYBOARD )
        {
          InterpretKeyboardData( raw, (DeviceInfo*)pDevice->m_pDevicePointer );
        }
        else if (raw->header.dwType == RIM_TYPEMOUSE)
        {
          int     origX = m_RawMouseX;
          int     origY = m_RawMouseY;
          if ( ( raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE )
          ||   ( raw->data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP ) )
          {
            m_RawMouseX = raw->data.mouse.lLastX;
            m_RawMouseY = raw->data.mouse.lLastY;
          }
          else
          {
            // must be relative
            m_RawMouseX += raw->data.mouse.lLastX;
            m_RawMouseY += raw->data.mouse.lLastY;
          }

          //dh::Log( "Mouse Delta %d,%d", m_RawMouseX - origX, m_RawMouseY - origY );

          HandleMouseAxis( m_RawMouseX - origX, MapKeyToVKey( Xtreme::MOUSE_LEFT ), MapKeyToVKey( Xtreme::MOUSE_RIGHT ) );
          HandleMouseAxis( m_RawMouseY - origY, MapKeyToVKey( Xtreme::MOUSE_UP ), MapKeyToVKey( Xtreme::MOUSE_DOWN ) );
          /*

          char    szTempOutput[5000];
	        sprintf_s(szTempOutput, 5000, TEXT("Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n"),
                  raw->data.mouse.usFlags,
                  raw->data.mouse.ulButtons,
                  raw->data.mouse.usButtonFlags,
                  raw->data.mouse.usButtonData,
                  raw->data.mouse.ulRawButtons,
                  raw->data.mouse.lLastX,
                  raw->data.mouse.lLastY,
                  raw->data.mouse.ulExtraInformation);
          OutputDebugString(szTempOutput);
          */
        }
        else if (raw->header.dwType == RIM_TYPEHID )
        {
          InterpretHIDData( raw, (DeviceInfo*)pDevice->m_pDevicePointer );
        }
        delete[] lpb;
      }
      break;
  }
  return XBasicInput::WindowProc( hWnd, wMessage, wParam, lParam );
}