/*
								+---------------------------------+
								|                                 |
								|        ***   Targa   ***        |
								|                                 |
								|  Copyright  © -tHE SWINe- 2002  |
								|                                 |
								|             Tga.cpp             |
								|                                 |
								+---------------------------------+
*/

/*
 *	passed code revision at 2006-06-08
 *
 *	rewrote to C++, used templates and template functions to reduce code length
 *
 *	2006-07-31
 *
 *	added slight template hack to be compilable under g++/linux
 *
 *	2007-03-26
 *
 *	removed template hack, fixed the code so there's same codepath for both g++/linux and msvc/win
 *
 *	2007-12-24
 *
 *	improved linux compatibility by using posix integer types
 *
 *	2008-03-04
 *
 *	now using Integer.h header, created CTgaCodec class, exposing color conversion routines,
 *	added CTgaCodec::Save_TGA() for writing TGA images
 *
 *	2008-05-08
 *
 *	fixed minor issues in CTgaCodec::Save_TGA (RGB / BGR, alpha channel)
 *
 *	2008-11-09
 *
 *	fixed bug in CTgaCodec::Save_TGA in greyscale RLE code, runs shorter than 3 pixels came
 *	undetected, there was also typo in RGB / grey decission (= instead of ==)
 *
 *	2009-05-04
 *
 *	fixed mixed windows / linux line endings
 *
 *	2009-10-20
 *
 *	fixed some warnings when compiling under VC 2005, implemented "Security
 *	Enhancements in the CRT " for VC 2008. compare against MyProjects_2009-10-19_
 *
 */

#ifndef __BMP_INCLUDED
#define __BMP_INCLUDED

#if defined(_MSC_VER) && !defined(__MWERKS__)
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
#endif //__MSC_VER__ && !__MWERKS__
// std integer types (quick fix for MSVC)

/**
 *	@brief simple bitmap structure
 */
struct TBmp {
	char n_former_bpp; /**< former bpp, before conversion to RGBA8 */
	bool b_grayscale; /**< does the bitmap contain colors? */
	bool b_alpha; /**< does the bitmap contain alpha channel? */
	int n_width; /**< image width, in pixels */
	int n_height; /**< image height, in pixels */
	uint32_t *p_buffer; /**< framebuffer, contains n_height horizontal scanlines, each n_width pixels long; first pixel in the buffer is top left */

	/**
	 *	@brief deletes data buffer
	 */
	void Free()
	{
		delete[] p_buffer;
	}

	/**
	 *	@brief deletes data buffer and this
	 */
	void Delete()
	{
		delete[] p_buffer;
		delete this;
	}
};

#endif //__BMP_INCLUDED

#ifndef __TARGA_INCLUDED
#define __TARGA_INCLUDED

class CTgaCodec {
public:
	/*
	 *	static TBmp *CTgaCodec::p_Load_TGA(const char *p_s_filename)
	 *		- loads image from p_s_filename
	 *		- returns pointer to bitmap object or 0 if loading failed
	 */
	static TBmp *p_Load_TGA(const char *p_s_filename);

	/*
	 *	static bool CTgaCodec::Save_TGA(const char *p_s_filename, const TBmp &r_t_bmp)
	 *		- write RGB targa image to file p_s_file
	 *		- if r_t_bmp.b_alpha is set, image is written as RGBA,
	 *		  otherwise as RGB
	 *		- if r_t_bmp.b_grayscale is set (while r_t_bmp.b_alpha is not),
	 *		  image is written as RLE - compressed greyscale
	 *		- note the n_former_bpp field is ignored, images are always saved as 8bpp
	 *		- returns true on success, false on failure
	 */
	static bool Save_TGA(const char *p_s_filename, const TBmp &r_t_bmp);

protected:
	static inline uint8_t n_Red(uint32_t n_rgba);
};

#endif //__TARGA_INCLUDED
