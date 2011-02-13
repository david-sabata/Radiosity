/*
								+---------------------------------+
								|                                 |
								|  ***   OpenGL 3.0 driver   ***  |
								|                                 |
								|  Copyright  © -tHE SWINe- 2009  |
								|                                 |
								|          OpenGL30Drv.h          |
								|                                 |
								+---------------------------------+
*/

/**
 *	@file OpenGL30Drv.h
 *	@author -tHE SWINe-
 *	@brief (Windows) OpenGL 3.0 driver
 *
 *	@date 2010-09-22
 *
 *	modified to be useful with GLEW, instead of ÜberLame GLEH
 */

#ifndef __GL30_DRIVER_INCLUDED
#define __GL30_DRIVER_INCLUDED

#include <windows.h>
#include <GL/glew.h>

/**
 *	@brief OpenGL driver
 *
 *	Initialization of OpenGL 3 context with ability to create
 *		forward-compatible contexts (uses WGL_ARB_create_context).
 *
 *	@note This implementation is limited to Windows.
 */
class CGL30Driver {
private:
    HDC m_h_dc;
    HGLRC m_h_glrc;
    HWND m_h_wnd;
    bool m_b_fullscreen;

    int m_n_width;
    int m_n_height;

    bool m_b_status;

	PIXELFORMATDESCRIPTOR m_t_pixel_format;
    GLuint m_n_pixel_format_id;

public:
	/**
	 *	@brief default constructor
	 *
	 *	Default constructor; has no effect.
	 */
    CGL30Driver();

	/**
	 *	@brief destructor
	 *
	 *	Takes care of OpenGL shutdown in case Shutdown() wasn't called.
	 */
    ~CGL30Driver();

	/**
	 *	@brief initializes OpenGL
	 *
	 *	Initializes OpenGL, enables creating forward-compatible OpenGL contexts.
	 *		Regular OpenGL context is created with b_forward_compatible set to false
	 *		and both n_opengl_major and n_opengl_minor set to -1. That results in
	 *		backward-compatible context, running newest implemented version of OpenGL.
	 *		Forward-compatible context is created with b_forward_compatible set to true
	 *		and n_opengl_major, n_opengl_minor set to required OpenGL version
	 *		(3.0, 3.1 or 3.2 are available to date)
	 *
	 *	@param[in] h_wnd is handle of window to init OpenGL in
	 *	@param[in] b_forward_compatible requests forward compatible OpenGL mode
	 *		(in which case, n_opengl_major and n_opengl_minor contains required version).
	 *	@param[in] n_opengl_major is required OpenGL major version (only valid if
	 *		b_forward_compatible is set)
	 *	@param[in] n_opengl_minor is required OpenGL minor version (only valid if
	 *		b_forward_compatible is set)
	 *	@param[in] n_width is viewport width in pixels
	 *	@param[in] n_height is viewport height in pixels
	 *	@param[in] n_bpp is number of bits per pixel
	 *	@param[in] n_depth_bpp is number of depth bits per pixel
	 *	@param[in] n_stencil_bpp is number of stencil bits per pixel
	 *	@param[in] b_fullscreen enables switching to fullscreen mode if set, otherwise leaves screen mode intact
	 *
	 *	@return Returns true on success, false on failure.
	 *
	 *	@note In case OpenGL is already initialized, it is first shut down,
	 *		causing all OpenGL objects being effectively deleted.
	 */
    bool Init(HWND h_wnd, bool b_forward_compatible, int n_opengl_major, int n_opengl_minor,
		int n_width, int n_height, int n_bpp, int n_depth_bpp, int n_stencil_bpp, bool b_fullscreen);

	/**
	 *	@brief shuts OpenGL down
	 *
	 *	@return Returns true on success, false on failure.
	 *
	 *	@note This always succeeds in case OpenGL was not initialized.
	 */
    bool Shutdown();

	/**
	 *	@brief returns OpenGL status
	 *
	 *	@return Returns true if OpenGL was successfuly initialized, otherwise returns false.
	 */
	bool b_Status() const;

	/**
	 *	@brief makes context, associated with this OpenGL driver, current
	 *
	 *	@return Returns true on success, false on failure.
	 *
	 *	@note This doesn't explicitly handle case where OpenGL was not initialized.
	 */
	bool MakeCurrent();

	/**
	 *	@brief gets viewport width
	 *
	 *	@return Returns width, in pixels, as passed to Init().
	 *
	 *	@note This value may have changed, in case OpenGL window was resized.
	 */
	inline int n_Width() const { return m_n_width; }

	/**
	 *	@brief gets viewport height
	 *
	 *	@return Returns height, in pixels,
	 *
	 *	@note This value may have changed, in case OpenGL window was resized.
	 */
	inline int n_Height() const { return m_n_height; }

	/**
	 *	@brief gets PIXELFORMATDESCRIPTOR
	 *
	 *	@return Returns current pixel format descriptor. Return value is undefined
	 *		in case Init() wasn't called / did not succeed.
	 */
	inline const PIXELFORMATDESCRIPTOR &t_PixelFormat() const { return m_t_pixel_format; }

	/**
	 *	@brief gets pixel format index
	 *
	 *	@return Returns current pixel format index. Return value is undefined
	 *		in case Init() wasn't called / did not succeed.
	 */
	inline GLuint n_PixelFormat() const { return m_n_pixel_format_id; }

	/**
	 *	@brief resets viewport
	 *
	 *	Calls glViewport with width / height values, as passed to Init().
	 *
	 *	@note This doesn't explicitly handle case where OpenGL was not initialized.
	 */
	void ResetViewport() const;

	/**
	 *	@brief exchanges front and back buffer
	 *
	 *	Exchanges front and back buffer, implicates back buffer contains undefined data
	 *		after the swap (most likely contains one of previous frames), reading it back
	 *		may give unexpected results. Justification for this limitation is no need
	 *		to physically copy contents of the buffers, resulting in greater speed.
	 *
	 *	@note This doesn't explicitly handle case where OpenGL was not initialized.
	 */
    void Blit() const;
};

#endif //__GL30_DRIVER_INCLUDED
