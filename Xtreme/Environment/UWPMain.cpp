#include "UWPMain.h"

#include <ppltasks.h>
#include <wrl.h>
#include <wrl/client.h>

#include <MasterFrame/XFrameApp.h>
#include <Lang/Service.h>

#include <Xtreme/Input/XInputUniversal.h>



using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Microsoft::WRL;



struct __declspec( uuid( "45D64A29-A63E-4CB6-B498-5781D298CB4F" ) ) __declspec( novtable )
  ICoreWindowInterop : IUnknown
{
  virtual HRESULT __stdcall get_WindowHandle( HWND * hwnd ) = 0;
  virtual HRESULT __stdcall put_MessageHandled( unsigned char ) = 0;
};



UWPAppMediator::UWPAppMediator() :
	m_windowClosed( false ),
	m_windowVisible( true )
{
}



void UWPAppMediator::Initialize( CoreApplicationView^ applicationView )
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>( this, &UWPAppMediator::OnActivated );

	CoreApplication::Suspending +=
		ref new Windows::Foundation::EventHandler<SuspendingEventArgs^>( this, &UWPAppMediator::OnSuspending );

	CoreApplication::Resuming +=
		ref new Windows::Foundation::EventHandler<Platform::Object^>( this, &UWPAppMediator::OnResuming );

  //CoreApplication::UnhandledErrorDetected += ref new Windows::Foundation::EventHandler<Windows::ApplicationModel::Core::UnhandledErrorDetectedEventArgs^>( this, &UWPAppMediator::UnhandledError );

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
}


/*
void UWPAppMediator::UnhandledError( Platform::Object^ sender, UnhandledErrorDetectedEventArgs^ eventArgs )
{
  try
  {
    eventArgs->UnhandledError->Propagate();
  }
  catch ( Platform::Exception^ e )
  {
    eventArgs->UnhandledError->Handled = false; 
    Windows::System::Env
    Environment
    e->Message
    e->Message
    auto properties = ref new System::Collections::Generic::Dictionary<Platform::String^, Platform::String^>()
    {
      {
        "trace", e.StackTrace
      },
      { "message", e.Message },
    };

    telemetryClient.TrackCrash( e, properties );
  }
}
*/

// Called when the CoreWindow object is created (or re-created).
void UWPAppMediator::SetWindow( CoreWindow^ window )
{
	window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &UWPAppMediator::OnWindowSizeChanged);

	window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &UWPAppMediator::OnVisibilityChanged);

	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &UWPAppMediator::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWPAppMediator::OnDpiChanged);

	currentDisplayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWPAppMediator::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWPAppMediator::OnDisplayContentsInvalidated);

  XFrameApp*   pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );

  pApp->m_Window.CoreWindow = window;

  // HWND is already created, use external handle
  HWND    hwndOut;

  ComPtr<ICoreWindowInterop> interop;

  HRESULT hr = reinterpret_cast<IUnknown *>( window )->QueryInterface( interop.GetAddressOf() );
  if ( FAILED( hr ) )
  {
  }

  hr = interop->get_WindowHandle( &hwndOut );

  pApp->m_RenderFrame.m_hwndMain = hwndOut;
  pApp->m_Window.Hwnd = pApp->m_RenderFrame.m_hwndMain;
  pApp->m_Window.CoreWindow = window;
}



void UWPAppMediator::Load( Platform::String^ entryPoint )
{
	
}



void UWPAppMediator::ApplyRotationLock( Xtreme::EnvironmentConfig& Config )
{
  DisplayOrientations orientations = DisplayOrientations::None;

  if ( Config.SupportedDisplayConfigurations & Xtreme::SupportedDisplayConfiguration::LANDSCAPE )
  {
    orientations = orientations | DisplayOrientations::Landscape;
  }
  if ( Config.SupportedDisplayConfigurations & Xtreme::SupportedDisplayConfiguration::PORTRAIT )
  {
    orientations = orientations | DisplayOrientations::Portrait;
  }
  if ( Config.SupportedDisplayConfigurations & Xtreme::SupportedDisplayConfiguration::LANDSCAPE_FLIPPED )
  {
    orientations = orientations | DisplayOrientations::LandscapeFlipped;
  }
  if ( Config.SupportedDisplayConfigurations & Xtreme::SupportedDisplayConfiguration::PORTRAIT_FLIPPED )
  {
    orientations = orientations | DisplayOrientations::PortraitFlipped;
  }
  DisplayInformation::AutoRotationPreferences = orientations;
}



void UWPAppMediator::Run()
{
  XFrameApp*   pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );

  //pApp->Configure( pApp->m_EnvironmentConfig );

  ApplyRotationLock( pApp->m_EnvironmentConfig );

  pApp->Run();
  pApp->ExitInstance();
}



// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void UWPAppMediator::Uninitialize()
{
}



void UWPAppMediator::OnActivated( CoreApplicationView^ applicationView, IActivatedEventArgs^ args )
{
  // Run() won't start until the CoreWindow is activated.
  XFrameApp*   pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );

  pApp->Configure( pApp->m_EnvironmentConfig );
  pApp->SetActive();
  auto pInput = (XInputUniversal*)pApp->InputClass();
  if ( pInput )
  {
    pInput->SetActive( true );
  }

#if ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
  Windows::Foundation::Size   desiredSize;
#endif

  if ( pApp->m_EnvironmentConfig.FixedSize )
  {
    float DPI = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->LogicalDpi;

#if ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
    Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode = Windows::UI::ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize;

    desiredSize = Windows::Foundation::Size( ( (float)pApp->m_EnvironmentConfig.StartUpWidth * 96.0f / DPI ), ( (float)pApp->m_EnvironmentConfig.StartUpHeight * 96.0f / DPI ) );

    Windows::UI::ViewManagement::ApplicationView::PreferredLaunchViewSize = desiredSize;
#endif
  }

  CoreWindow::GetForCurrentThread()->Activate();

  if ( pApp->m_EnvironmentConfig.FixedSize )
  {
#if ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
    bool result = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->TryResizeView( desiredSize );
#endif
  }

  /*
  auto displayInformation = DisplayInformation::GetForCurrentView();
  dh::Log( "X %d", displayInformation->RawDpiX );
  dh::Log( "Y %d", displayInformation->RawDpiY );
  dh::Log( "RawPixelsPerViewPixel %d", displayInformation->RawPixelsPerViewPixel );
  dh::Log( "ResolutionScale %d", displayInformation->ResolutionScale );*/
}



void UWPAppMediator::OnSuspending( Platform::Object^ sender, SuspendingEventArgs^ args )
{
  XFrameApp*   pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );

  pApp->SetActive( false );
  auto pInput = (XInputUniversal*)pApp->InputClass();
  if ( pInput )
  {
    pInput->SetActive( false );
  }

	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// Insert your code here.
		deferral->Complete();
	});
}



void UWPAppMediator::OnResuming( Platform::Object^ sender, Platform::Object^ args )
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

	// Insert your code here.
  XFrameApp*   pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );

  pApp->SetActive();
  auto pInput = (XInputUniversal*)pApp->InputClass();
  if ( pInput )
  {
    pInput->SetActive( true );
  }
}



void UWPAppMediator::OnWindowSizeChanged( CoreWindow^ sender, WindowSizeChangedEventArgs^ args )
{
}



void UWPAppMediator::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}



void UWPAppMediator::OnWindowClosed( CoreWindow^ sender, CoreWindowEventArgs^ args )
{
  XFrameApp*   pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );

  pApp->ShutDown();
	m_windowClosed = true;
}



void UWPAppMediator::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
}



void UWPAppMediator::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
}



void UWPAppMediator::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
}



IFrameworkView^ UWPApplicationSource::CreateView()
{
  return ref new UWPAppMediator();
}



[Platform::MTAThread]
int main( Platform::Array<Platform::String^>^ )
{
  auto uwpAppSource = ref new UWPApplicationSource();

  CoreApplication::Run( uwpAppSource );
  return 0;
}

