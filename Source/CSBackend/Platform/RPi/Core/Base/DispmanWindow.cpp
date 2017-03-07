//
//  The MIT License (MIT)
//
//  Copyright (c) 2017 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#ifdef CS_TARGETPLATFORM_RPI

#include <CSBackend/Platform/RPi/Core/Base/DispmanWindow.h>
#include <CSBackend/Platform/RPi/Core/Base/SystemInfoFactory.h>

#include <ChilliSource/Core/Base/StandardMacros.h>
#include <ChilliSource/Core/Base/AppConfig.h>
#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Base/LifecycleManager.h>
#include <ChilliSource/Core/Base/SystemInfo.h>
#include <ChilliSource/Core/Container/VectorUtils.h>

namespace CSBackend
{
	namespace RPi
	{
		//----------------------------------------------------------------------------------
		void DispmanWindow::Run() noexcept
		{
			//The display setup we use mimics Minecraft on the Raspberry Pi. Essentially we use a dispman display
			//to take advantage of hardware acceleration but we layer that on top of an X window which we use to listen
			//for events and to allow the user to control the size of the dispman display.

			//Start interfacing with Raspberry Pi.
			if(!m_bcmInitialised)
			{
				bcm_host_init();
				m_bcmInitialised = true;
			}

			//TODO: Get from app config
			u32 displayWidth, displayHeight;
			graphics_get_display_size(0, &displayWidth, &displayHeight);
			m_windowSize.x = (s32)displayWidth;
			m_windowSize.y = (s32)displayHeight;

			InitXWindow(m_windowSize);
			InitEGLDispmanWindow(m_windowSize);

			//NOTE: We are creating ChilliSource here so no CS calls can be made prior to this including logging.
			ChilliSource::ApplicationUPtr app = ChilliSource::ApplicationUPtr(CreateApplication(SystemInfoFactory::CreateSystemInfo()));
			m_lifecycleManager = ChilliSource::LifecycleManagerUPtr(new ChilliSource::LifecycleManager(app.get()));
			m_lifecycleManager->Resume();

			RunLoop();
		}

		//-----------------------------------------------------------------------------------
		void DispmanWindow::InitXWindow(const ChilliSource::Integer2& windowSize) noexcept
		{
			m_xdisplay = XOpenDisplay(NULL);
			if(m_xdisplay == nullptr)
			{
				printf("[ChilliSource] Failed to create X11 display. Exiting");
				return;
			}

			m_xwindow = XCreateSimpleWindow(m_xdisplay, XDefaultRootWindow(m_xdisplay), 0, 0, windowSize.x, windowSize.y, 0, 0, 0);
			XMapWindow(m_xdisplay, m_xwindow);

			//TODO: Set the window name.
			//All the events we need to listen for
			XSelectInput(m_xdisplay, m_xwindow, PointerMotionMask | ButtonMotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | FocusChangeMask | StructureNotifyMask);

			//Sends all the commands to the X server.
			XFlush(m_xdisplay);
		}

		//-----------------------------------------------------------------------------------
		void DispmanWindow::InitEGLDispmanWindow(const ChilliSource::Integer2& windowSize) noexcept
		{
			// TODO: Get attributes from Config
			static const EGLint attributeList[] =
			{
				EGL_RED_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_BLUE_SIZE, 8,
				EGL_ALPHA_SIZE, 8,
				EGL_DEPTH_SIZE, 16,
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_NONE
			};

			// Set up OpenGL context version
			static const EGLint contextAttributeList[] =
			{
				EGL_CONTEXT_CLIENT_VERSION, 2,
				EGL_NONE
			};

			// Get EGL display & initialize it
			m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
			eglInitialize(m_eglDisplay, NULL, NULL);

			// Set up config
			eglChooseConfig(m_eglDisplay, attributeList, &m_eglConfig, 1, &m_eglConfigNum);

			// Bind to OpenGL ES 2.0
			eglBindAPI(EGL_OPENGL_ES_API);

			// Create context
			m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig, EGL_NO_CONTEXT, contextAttributeList);

			// Set up blit rects.
			m_dstRect.x = 0;
			m_dstRect.y = 0;
			m_dstRect.width = windowSize.x;
			m_dstRect.height = windowSize.y;

			m_srcRect.x = 0;
			m_srcRect.y = 0;
			m_srcRect.width = windowSize.x << 16;
			m_srcRect.height = windowSize.y << 16;

			// Set up dispmanx
			m_displayManagerDisplay = vc_dispmanx_display_open(0);
			m_displayManagerUpdate = vc_dispmanx_update_start(0);
			m_displayManagerElement = vc_dispmanx_element_add(m_displayManagerUpdate, m_displayManagerDisplay, 0, &m_dstRect, 0, &m_srcRect, DISPMANX_PROTECTION_NONE, 0, 0, (DISPMANX_TRANSFORM_T)0);

			// Set up native window. TODO: Attach to X? Size to X Window?
			m_nativeWindow.element = m_displayManagerElement;
			m_nativeWindow.width = windowSize.x;
			m_nativeWindow.height = windowSize.y;

			// Instruct VC chip to use this display manager to sync
			vc_dispmanx_update_submit_sync(m_displayManagerUpdate);

			// Set up EGL surface and connect the context to it
			m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig, &m_nativeWindow, NULL);
			eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
		}

		//-----------------------------------------------------------------------------------
		void DispmanWindow::RunLoop() noexcept
		{
			m_isRunning = true;

			while(m_isRunning == true)
			{
				while(XPending(m_xdisplay))
				{
					XEvent event;
					XNextEvent(m_xdisplay, &event);

					switch (event.type)
					{
						case FocusIn:
						{
							m_isFocused = true;
							m_lifecycleManager->Foreground();
							break;
						}
						case FocusOut:
						{
							m_isFocused = false;
							m_lifecycleManager->Background();
							break;
						}
						case MotionNotify:
						{
							std::unique_lock<std::mutex> lock(m_mouseMutex);
							if (m_mouseMovedDelegate)
							{
								m_mouseMovedDelegate(event.xbutton.x, event.xbutton.y);
							}
							break;
						}
						case ButtonPress:
						{
							std::unique_lock<std::mutex> lock(m_mouseMutex);
							if (m_mouseButtonDelegate)
							{
								m_mouseButtonDelegate(event.xbutton.button, MouseButtonEvent::k_pressed);
							}
							break;
						}
						case ButtonRelease:
						{
							std::unique_lock<std::mutex> lock(m_mouseMutex);
							if (m_mouseButtonDelegate)
							{
								m_mouseButtonDelegate(event.xbutton.button, MouseButtonEvent::k_released);
							}
							break;
						}
						case DestroyNotify:
						{
							Quit();
							return;
						}
					}
				}

				//Update, render and then flip display buffer
				m_lifecycleManager->SystemUpdate();
				m_lifecycleManager->Render();
				eglSwapBuffers(m_eglDisplay, m_eglSurface);

				if(m_quitScheduled)
				{
					Quit();
				}
			}
		}

		//-----------------------------------------------------------------------------------
		void DispmanWindow::SetMouseDelegates(MouseButtonDelegate mouseButtonDelegate, MouseMovedDelegate mouseMovedDelegate, MouseWheelDelegate mouseWheelDelegate) noexcept
		{
			std::unique_lock<std::mutex> lock(m_mouseMutex);

			CS_ASSERT(mouseButtonDelegate, "Mouse button event delegate invalid.");
			CS_ASSERT(mouseMovedDelegate, "Mouse moved delegate invalid.");
			CS_ASSERT(mouseWheelDelegate, "Mouse wheel scroll delegate invalid.");
			CS_ASSERT(!m_mouseButtonDelegate, "Mouse button event delegate already set.");
			CS_ASSERT(!m_mouseMovedDelegate, "Mouse moved delegate already set.");
			CS_ASSERT(!m_mouseWheelDelegate, "Mouse wheel scroll delegate already set.");

			m_mouseButtonDelegate = std::move(mouseButtonDelegate);
			m_mouseMovedDelegate = std::move(mouseMovedDelegate);
			m_mouseWheelDelegate = std::move(mouseWheelDelegate);
		}

		//-----------------------------------------------------------------------------------
		void DispmanWindow::RemoveMouseDelegates() noexcept
		{
			std::unique_lock<std::mutex> lock(m_mouseMutex);
			m_mouseButtonDelegate = nullptr;
			m_mouseMovedDelegate = nullptr;
			m_mouseWheelDelegate = nullptr;
		}

		//-----------------------------------------------------------------------------------
		ChilliSource::Integer2 DispmanWindow::GetMousePosition() const noexcept
		{
			Window root, child;
			s32 rootX, rootY, winX, winY;
			u32 buttonMask;
			XQueryPointer(m_xdisplay, m_xwindow, &root, &child, &rootX, &rootY, &winX, &winY, &buttonMask);

			return ChilliSource::Integer2(winX, winY);
		}

		//-----------------------------------------------------------------------------------
		std::vector<ChilliSource::Integer2> DispmanWindow::GetSupportedResolutions() const noexcept
		{
			//TODO: Find out if we can query the BCM or dispman for supported resolutions
			return std::vector<ChilliSource::Integer2> { m_windowSize };
		}

		//-----------------------------------------------------------------------------------
		void DispmanWindow::Quit() noexcept
		{
			if (m_isFocused == true)
			{
				m_lifecycleManager->Background();
				m_isFocused = false;
			}

			m_lifecycleManager->Suspend();
			m_lifecycleManager.reset();

			m_isRunning = false;
		}

		//-----------------------------------------------------------------------------------
		DispmanWindow::~DispmanWindow()
		{
			CS_ASSERT(!m_mouseButtonDelegate, "Mouse button event delegate not removed.");
			CS_ASSERT(!m_mouseMovedDelegate, "Mouse moved delegate not removed.");
			CS_ASSERT(!m_mouseWheelDelegate, "Mouse wheel scroll delegate not removed.");

			if(m_bcmInitialised)
			{
				bcm_host_deinit();
			}

			if(m_xdisplay)
			{
				XDestroyWindow(m_xdisplay, m_xwindow);
				XCloseDisplay(m_xdisplay);
			}
		}
	}
}


#endif
