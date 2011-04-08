/*
								+---------------------------------+
								|                                 |
								|  ***   OpenGL 3.0 driver   ***  |
								|                                 |
								|  Copyright  © -tHE SWINe- 2009  |
								|                                 |
								|         OpenGL30Drv.cpp         |
								|                                 |
								+---------------------------------+
*/

/**
 *	@file OpenGL30Drv.cpp
 *	@author -tHE SWINe-
 *	@brief (Windows) OpenGL 3.0 driver
 */

#include "OpenGL30Drv.h"
#include <GL/wglew.h>

/*
 *								=== CGL30Driver ===
 */

CGL30Driver::CGL30Driver()
	:m_b_status(false)
{
}

CGL30Driver::~CGL30Driver()
{
    if(m_b_status)
        Shutdown();
}

bool CGL30Driver::Init(HWND h_wnd, bool b_forward_compatible,
	int n_opengl_major, int n_opengl_minor, int n_width, int n_height, int n_bpp,
	int n_depth_bpp, int n_stencil_bpp, bool b_fullscreen)
{
    DEVMODE t_device_mode;

	if(m_b_status)
		Shutdown();

    if(b_fullscreen) {
        memset(&t_device_mode, 0, sizeof(t_device_mode));
        t_device_mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        t_device_mode.dmSize = sizeof(DEVMODE);
        t_device_mode.dmPelsWidth = n_width;
        t_device_mode.dmPelsHeight = n_height;
        t_device_mode.dmBitsPerPel = n_bpp;
        // sets device mode for fullscreen mode

        if(ChangeDisplaySettings(&t_device_mode, 4) != DISP_CHANGE_SUCCESSFUL) {
            if(MessageBoxA(NULL, "Unable to set fullscreen mode.\n"
               "Use windowed mode instead?", "Fullscreen", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                m_b_fullscreen = 0;
            else
                return false;
        }
        // tries to change the mode, allows fallback to windowed

        ShowCursor(false);
    }

    memset(&m_t_pixel_format, 0, sizeof(PIXELFORMATDESCRIPTOR));
    m_t_pixel_format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    m_t_pixel_format.nVersion = 1;
    m_t_pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE;
    m_t_pixel_format.dwLayerMask = PFD_MAIN_PLANE;
    m_t_pixel_format.iPixelType = PFD_TYPE_RGBA;
    m_t_pixel_format.cColorBits = n_bpp;
    m_t_pixel_format.cDepthBits = n_depth_bpp;
    m_t_pixel_format.cAccumBits = 0;
    m_t_pixel_format.cStencilBits = n_stencil_bpp;
    // sets pixel format descriptor

    if(!(m_h_dc = GetDC(m_h_wnd = h_wnd))) {
        MessageBoxA(NULL, "Unable to create a GL device context.", "OpenGL driver", MB_OK | MB_ICONERROR);
        return false;
    }
    if(!(m_n_pixel_format_id = ChoosePixelFormat(m_h_dc, &m_t_pixel_format))) {
        MessageBoxA(NULL, "Unable to find a suitable pixelformat.", "OpenGL driver", MB_OK | MB_ICONERROR);
        return false;
    }
    if(!SetPixelFormat(m_h_dc, m_n_pixel_format_id, &m_t_pixel_format)) {
        MessageBoxA(NULL, "Unable to set the pixelformat.", "OpenGL driver", MB_OK | MB_ICONERROR);
        return false;
    }

	HGLRC h_gl_rc;
	if(!(h_gl_rc = wglCreateContext(m_h_dc))) {
        MessageBoxA(NULL, "Unable to create a GL rendering context.", "OpenGL driver", MB_OK | MB_ICONERROR);
        return false;
	}
    if(!wglMakeCurrent(m_h_dc, h_gl_rc)) {
        MessageBoxA(NULL, "Unable to activate the GL rendering context.", "OpenGL driver", MB_OK | MB_ICONERROR);
        return false;
    }
	// create and activate dummy context

	if(glewInit() != GLEW_OK) {
		MessageBoxA(NULL, "Failed to initialize GLEW.", "OpenGL driver", MB_OK | MB_ICONERROR);
		return false;
	}
	if(b_forward_compatible) {
		if(!WGLEW_ARB_create_context) {
			MessageBoxA(NULL, "OpenGL 3.0 not supported by window system\n"
				"(WGL_ARB_create_context not present).", "OpenGL driver", MB_OK | MB_ICONERROR);
			return false;
		}
		// needs wglCreateContextAttribsARB function (WGLEW_ARB_create_context)

		const int p_params[] = {
			WGL_CONTEXT_LAYER_PLANE_ARB, 0, // main plane
			WGL_CONTEXT_MAJOR_VERSION_ARB, n_opengl_major,
			WGL_CONTEXT_MINOR_VERSION_ARB, n_opengl_minor,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, // want forward compatible context
			//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // want core profile; this is set by default and we do not set it here as nvidia drivers have some problems with that
			0
		};
		if(!(m_h_glrc = wglCreateContextAttribsARB(m_h_dc, 0, p_params))) {
			MessageBoxA(NULL, "Unable to create OpenGL 3.0 rendering context.", "OpenGL driver", MB_OK | MB_ICONERROR);
			return false;
		}
		if(!wglMakeCurrent(m_h_dc, m_h_glrc)) {
			MessageBoxA(NULL, "Unable to activate the GL rendering context.", "OpenGL driver", MB_OK | MB_ICONERROR);
			return false;
		}
		// creates forward compatible OpenGL context

		wglDeleteContext(h_gl_rc);
		// deletes the dummy context
	} else
		m_h_glrc = h_gl_rc; // otherwise just use the dummy context

    glViewport(0, 0, n_width, n_height);
    // sets viewport

    m_n_width = n_width;
    m_n_height = n_height;
    m_b_status = true;
    m_b_fullscreen = b_fullscreen;

    return true;
}

bool CGL30Driver::Shutdown()
{
	if(!m_b_status)
		return true;

    bool b_result = true;

    m_b_status = false;

    if(m_b_fullscreen) {
        ChangeDisplaySettings(NULL, 0);
        ShowCursor(true);
    }
	// returns from fullscreen mode

    if(m_h_glrc) {
        if(!wglMakeCurrent(NULL, NULL)) {
            MessageBoxA(NULL, "Release of DC and RC failed.", "OpenGL driver", MB_OK | MB_ICONERROR);
            b_result = false;
        }
        if(!wglDeleteContext(m_h_glrc)) {
            MessageBoxA(NULL, "Release rendering context failed.", "OpenGL driver", MB_OK | MB_ICONERROR);
            m_h_glrc = NULL;
            b_result = false;
        }
    }

    if(m_h_dc && !ReleaseDC(m_h_wnd, m_h_dc)) {
		MessageBoxA(NULL, "Release device context failed.", "OpenGL driver", MB_OK | MB_ICONERROR);
        m_h_dc = NULL;
        b_result = false;
    }

    return b_result;
}

bool CGL30Driver::b_Status() const
{
	return m_b_status;
}

bool CGL30Driver::MakeCurrent()
{
    if(!wglMakeCurrent(m_h_dc, m_h_glrc))
        return false;
	return true;
}

void CGL30Driver::ResetViewport() const
{
    glViewport(0, 0, m_n_width, m_n_height);
}

void CGL30Driver::Blit() const
{
    SwapBuffers(m_h_dc);
}

HGLRC CGL30Driver::GetContext() 
{
	return m_h_glrc;
}

/*
 *								=== ~CGL30Driver ===
 */
