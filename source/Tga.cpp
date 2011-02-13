/*
								+---------------------------------+
								|                                 |
								|        ***   Targa   ***        |
								|                                 |
								|  Copyright  © -tHE SWINe- 2006  |
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

#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Tga.h"

/*
 *								=== CTgaCodec ===
 */

namespace __tga__ {

/*
 *	template <class T>
 *	struct TColorConversion
 *		- color nonversion template (conversion from some color T to RGBA8)
 */
template <class T>
struct TColorConversion {
	typedef T TDataType;
	inline uint32_t operator()(const TDataType &r_t_color) const { return 0; }
};

/*
 *	template <class TPaletteIndex>
 *	struct TPaletteColor
 *		- conversion from palette index TPaletteIndex to RGBA8
 */
template <class TPaletteIndex>
struct TPaletteColor : public TColorConversion<TPaletteIndex> {
protected:
	const uint32_t *m_p_palette;
	int m_n_palette_entries;
	int m_n_offset;

public:
	TPaletteColor(const uint32_t *p_palette, int n_palette_entries, int n_offset)
		:m_p_palette(p_palette), m_n_palette_entries(n_palette_entries), m_n_offset(n_offset)
	{}

	inline uint32_t operator()(const TPaletteIndex &r_t_color) const
	{
		return (r_t_color + m_n_offset >= 0 &&
			r_t_color + m_n_offset < m_n_palette_entries)? m_p_palette[r_t_color + m_n_offset] : 0;
	}
};

/*
 *	struct T_RGB8_Color
 *		- conversion from RGB8 to RGBA8
 */
struct T_RGB8_Color : public TColorConversion<uint8_t[3]> {
	inline uint32_t operator()(const TDataType &r_t_color) const
	{
		return ((uint32_t)r_t_color[0] << 16) | ((uint32_t)r_t_color[1] << 8) |
			(uint32_t)r_t_color[2] | 0xff000000;
	}
};

/*
 *	struct T_BGRA8_Color
 *		- conversion from BGRA8 to RGBA8
 */
struct T_BGRA8_Color : public TColorConversion<uint32_t> {    
	inline uint32_t operator()(const TDataType &r_t_color) const
	{
		return (r_t_color & 0xff00ff00) | ((r_t_color & 0xff0000) >> 16) |
			((r_t_color & 0xff) << 16);
	}
};

/*
 *	struct T_RGB5_Color
 *		- conversion from RGB5 to RGBA8
 */
struct T_RGB5_Color : public TColorConversion<uint16_t> {    
	inline uint32_t operator()(const TDataType &r_t_color) const
	{
		return n_5_to_8((r_t_color >> 10) & 0x1f) |
			  (n_5_to_8((r_t_color >> 5) & 0x1f) << 8) |
			  (n_5_to_8(r_t_color & 0x1f) << 16) | 0xff000000;
	}

protected:
	static inline int n_5_to_8(int n_x)
	{
		return (n_x * 255 / 31) & 0xff;
	}
};

#pragma pack(1)
struct TTgaHeader {
	uint8_t n_id_length;
	uint8_t n_palette_type;
	uint8_t n_image_type;
	uint16_t n_first_color;
	uint16_t n_palette_colors;
	uint8_t n_palette_entry_size;
	uint16_t n_left;
	uint16_t n_top;
	uint16_t n_image_width;
	uint16_t n_image_height;
	uint8_t n_bpp;
	uint8_t n_descriptor_bits;
};
#pragma pack()

enum {
	tga_Compressed_Mask = 8,
	tga_ImageType_Mask = 7,
	tga_ColorMapped = 1,
	tga_RGB = 2,
	tga_Grayscale = 3
};

/*
 *	template <class TColorStruct>
 *	static bool _ReadRLEScanline(TColorStruct t_color_converter,
 *		uint32_t *p_output, int n_width, FILE *p_fr)
 *		- function for reading run-length compressed scanlines
 *		- t_color_converter is object, used for color conversion
 */
template <class TColorStruct>
static bool _ReadRLEScanline(TColorStruct t_color_converter,
	uint32_t *p_output, int n_width, FILE *p_fr)
{    
	for(int i = 0; i < n_width;) {
		uint8_t n_count;
		if(fread(&n_count, sizeof(char), 1, p_fr) != 1)
			return false;
		if(n_count & 0x80) {
			if(i + (n_count = (n_count & 0x7f) + 1) > n_width)
				return false;
			typename TColorStruct::TDataType t_value;
			if(fread(&t_value, sizeof(typename TColorStruct::TDataType), 1, p_fr) != 1)
				return false;
			for(uint32_t *p_end = p_output + n_count, n_color =
			   t_color_converter(t_value); p_output < p_end;)
				*p_output ++ = n_color;
			// long block with constant color
		} else {
			if(i + ++ n_count > n_width)
				return false;
			typename TColorStruct::TDataType p_value[256], *p_cur_value = &p_value[0];
			if(fread(p_value, sizeof(typename TColorStruct::TDataType), n_count, p_fr) != n_count)
				return false;
			for(uint32_t *p_end = p_output + n_count; p_output < p_end;)
				*p_output ++ = t_color_converter(*p_cur_value ++);
			// block with varying pixels
		}
		i += n_count;
	}

	return true;
}

/*
 *	template <class TColorStruct>
 *	static bool _ReadScanline(TColorStruct t_color_converter,
 *		uint32_t *p_output, int n_width, FILE *p_fr)
 *		- function for reading uncompressed scanlines
 *		- t_color_converter is object, used for color conversion
 */
template <class TColorStruct>
static bool _ReadScanline(TColorStruct t_color_converter,
	uint32_t *p_output, int n_width, FILE *p_fr)
{
	for(int i = 0, n_step = (n_width > 256)? 256 : n_width; i < n_width;
	   i += n_step, n_step = (n_width - i > 256)? 256 : n_width - i) {
		typename TColorStruct::TDataType p_value[256], *p_cur_value = &p_value[0];
		if(fread(p_value, sizeof(typename TColorStruct::TDataType),
		   n_step, p_fr) != (unsigned)n_step)
			return false;
		for(uint32_t *p_end = p_output + n_step; p_output < p_end;)
			*p_output ++ = t_color_converter(*p_cur_value ++);
		// block with varying pixels
	}

	return true;
}

}; // ~__tga__
using namespace __tga__;

/*
 *	static TBmp *CTgaCodec::p_Load_TGA(const char *p_s_filename)
 *		- loads image from p_s_filename
 *		- returns pointer to bitmap object or 0 if loading failed
 */
TBmp *CTgaCodec::p_Load_TGA(const char *p_s_filename)
{
	FILE *p_fr;
#if defined(_MSC_VER) && !defined(__MWERKS__) && _MSC_VER >= 1400
	if(fopen_s(&p_fr, p_s_filename, "rb"))
#else //_MSC_VER && !__MWERKS__ && _MSC_VER >= 1400
	if(!(p_fr = fopen(p_s_filename, "rb")))
#endif //_MSC_VER && !__MWERKS__ && _MSC_VER >= 1400
		return 0;
	// open file

	TTgaHeader t_header;
	if(fread(&t_header, sizeof(TTgaHeader), 1, p_fr) != 1 ||
	   fseek(p_fr, t_header.n_id_length, SEEK_CUR) != 0) {
		fclose(p_fr);
		return 0;
	}
	// read header

	if((t_header.n_image_type & tga_ImageType_Mask) == tga_Grayscale)
		t_header.n_palette_colors = 256; // fixme - is it already there?

	uint32_t *p_palette = 0;
	if(t_header.n_palette_colors &&
	   !(p_palette = new(std::nothrow) uint32_t[t_header.n_palette_colors])) {
		fclose(p_fr);
		return 0;
	}
	// alloc memory for palette

	if(t_header.n_palette_colors &&
	   (t_header.n_image_type & tga_ImageType_Mask) == tga_ColorMapped) {
		bool b_result;
		if(t_header.n_palette_entry_size == 16) {
			b_result = _ReadScanline(T_RGB5_Color(), p_palette,
				t_header.n_palette_colors, p_fr);
		} else if(t_header.n_palette_entry_size == 24) {
			b_result = _ReadScanline(T_RGB8_Color(), p_palette,
				t_header.n_palette_colors, p_fr);
		} else if(t_header.n_palette_entry_size == 32) {
			b_result = _ReadScanline(T_BGRA8_Color(), p_palette,
				t_header.n_palette_colors, p_fr);
		} else
			b_result = false;
		if(!b_result) {
			delete[] p_palette;
			fclose(p_fr);
			return 0;
		}
	}
	// read palette

	if((t_header.n_image_type & tga_ImageType_Mask) == tga_Grayscale) {
		for(uint32_t *p_color = p_palette, *p_end = p_palette + 256, n_color = 0xff000000;
		   p_color < p_end; n_color += 0x00010101)
			*p_color ++ = n_color;
	}
	// grayscale pal

	TBmp *p_bitmap;
	if((t_header.n_image_type & tga_ImageType_Mask) < tga_ColorMapped ||
	   (t_header.n_image_type & tga_ImageType_Mask) > tga_Grayscale || !(p_bitmap = new(std::nothrow) TBmp)) {
		if(p_palette)
			delete[] p_palette;
		fclose(p_fr);
		return 0;
	}
	if(!(p_bitmap->p_buffer = new(std::nothrow) uint32_t[t_header.n_image_width *
	   t_header.n_image_height])) {
		if(p_palette)
			delete[] p_palette;
		delete p_bitmap;
		fclose(p_fr);
		return 0;
	}
	p_bitmap->n_width = t_header.n_image_width;
	p_bitmap->n_height = t_header.n_image_height;
	p_bitmap->b_grayscale = (t_header.n_image_type & tga_ImageType_Mask) == tga_Grayscale;
	p_bitmap->n_former_bpp = (p_bitmap->b_grayscale)? 8 :
		(((t_header.n_image_type & tga_ImageType_Mask) == tga_RGB)?
		t_header.n_bpp : t_header.n_palette_entry_size);
	p_bitmap->b_alpha = p_bitmap->n_former_bpp == 32;
	// alloc bitmap

	const int n_step = (t_header.n_descriptor_bits & 0x20)? t_header.n_image_width :
		-t_header.n_image_width;
	for(uint32_t *p_scanline = p_bitmap->p_buffer + ((t_header.n_descriptor_bits & 0x20)?
	   0 : t_header.n_image_width * (t_header.n_image_height - 1)), *p_end = p_bitmap->p_buffer +
	   ((t_header.n_descriptor_bits & 0x20)? t_header.n_image_width * t_header.n_image_height :
	   -t_header.n_image_width); p_scanline != p_end; p_scanline += n_step) {
		bool b_result;
		if((t_header.n_image_type & tga_ImageType_Mask) == tga_RGB) {
			if(t_header.n_image_type & tga_Compressed_Mask) {
				if(t_header.n_bpp == 16) {
					b_result = _ReadRLEScanline(T_RGB5_Color(), p_scanline,
						p_bitmap->n_width, p_fr);
				} else if(t_header.n_bpp == 24) {
					b_result = _ReadRLEScanline(T_RGB8_Color(), p_scanline,
						p_bitmap->n_width, p_fr);
				} else if(t_header.n_bpp == 32) {
					b_result = _ReadRLEScanline(T_BGRA8_Color(), p_scanline,
						p_bitmap->n_width, p_fr);
				} else
					b_result = false;
			} else {
				if(t_header.n_bpp == 16) {
					b_result = _ReadScanline(T_RGB5_Color(), p_scanline,
						p_bitmap->n_width, p_fr);
				} else if(t_header.n_bpp == 24) {
					b_result = _ReadScanline(T_RGB8_Color(), p_scanline,
						p_bitmap->n_width, p_fr);
				} else if(t_header.n_bpp == 32) {
					b_result = _ReadScanline(T_BGRA8_Color(), p_scanline,
						p_bitmap->n_width, p_fr);
				} else
					b_result = false;
			}
		} else {
			if(t_header.n_image_type & tga_Compressed_Mask) {
				b_result = _ReadRLEScanline(TPaletteColor<uint8_t>(p_palette,
					t_header.n_palette_colors, -t_header.n_first_color),
					p_scanline, p_bitmap->n_width, p_fr);
			} else {
				b_result = _ReadScanline(TPaletteColor<uint8_t>(p_palette,
					t_header.n_palette_colors, -t_header.n_first_color),
					p_scanline, p_bitmap->n_width, p_fr);
			}
		}
		if(!b_result) {
			if(p_palette)
				delete[] p_palette;
			fclose(p_fr);
			delete[] p_bitmap->p_buffer;
			delete p_bitmap;
			return 0;
		}
	}

	if(p_palette)
		delete[] p_palette;
	fclose(p_fr);
	// cleanup

	return p_bitmap;
}

/*
 *	static inline uint8_t CTgaCodec::n_Red(uint32_t n_rgba)
 *		- returns value of red channel
 */
inline uint8_t CTgaCodec::n_Red(uint32_t n_rgba)
{
	return uint8_t(n_rgba >> 16);
}

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
bool CTgaCodec::Save_TGA(const char *p_s_filename, const TBmp &r_t_bmp)
{
	FILE *p_fw;
#if defined(_MSC_VER) && !defined(__MWERKS__) && _MSC_VER >= 1400
	if(fopen_s(&p_fw, p_s_filename, "wb"))
#else //_MSC_VER && !__MWERKS__ && _MSC_VER >= 1400
	if(!(p_fw = fopen(p_s_filename, "wb")))
#endif //_MSC_VER && !__MWERKS__ && _MSC_VER >= 1400
		return false;

	TTgaHeader t_header;
    memset(&t_header, 0, sizeof(TTgaHeader));
	if(r_t_bmp.b_grayscale && !r_t_bmp.b_alpha) {
		t_header.n_bpp = 8;
		t_header.n_image_type = tga_Grayscale | tga_Compressed_Mask; // grey
	} else {
		t_header.n_bpp = (r_t_bmp.b_alpha)? 32 : 24;
		t_header.n_image_type = tga_RGB; // rgb
	}
	t_header.n_image_width = r_t_bmp.n_width;
	t_header.n_image_height = r_t_bmp.n_height;
	t_header.n_descriptor_bits = 0x20; // upside-down
	// create header

	if(fwrite(&t_header, sizeof(t_header), 1, p_fw) != 1) {
		fclose(p_fw);
		return false;
	}
	// write header

	const int n_width = r_t_bmp.n_width;
	const int n_height = r_t_bmp.n_height;
	// used frequently in the loops below

	if(t_header.n_image_type == tga_RGB) {
		if(t_header.n_bpp == 24) {
			for(const uint32_t *p_scanline = r_t_bmp.p_buffer, *p_end = r_t_bmp.p_buffer +
			   (n_width * n_height); p_scanline != p_end; p_scanline += n_width) {
				for(const uint32_t *p_ptr = p_scanline, *p_end2 = p_scanline + n_width;
				   p_ptr != p_end2; ++ p_ptr) {
					uint32_t n_rgb = *p_ptr;
					uint8_t p_data[3] = {uint8_t(n_rgb), uint8_t(n_rgb >> 8), uint8_t(n_rgb >> 16)};
					if(fwrite(p_data, sizeof(uint8_t), 3, p_fw) != 3) {
						fclose(p_fw);
						return false;
					}
				}
			}
			// RGB
		} else {
			_ASSERTE(t_header.n_bpp == 32);
			for(const uint32_t *p_scanline = r_t_bmp.p_buffer, *p_end = r_t_bmp.p_buffer +
			   (n_width * n_height); p_scanline != p_end; p_scanline += n_width) {
				for(const uint32_t *p_ptr = p_scanline, *p_end2 = p_scanline + n_width;
				   p_ptr != p_end2; ++ p_ptr) {
					uint32_t n_rgba = *p_ptr;
					uint8_t p_data[4] = {uint8_t(n_rgba), uint8_t(n_rgba >> 8),
						uint8_t(n_rgba >> 16), uint8_t(n_rgba >> 24)};
					if(fwrite(p_data, sizeof(uint8_t), 4, p_fw) != 4) {
						fclose(p_fw);
						return false;
					}
				}
			}
			// RGBA
		}
	} else {
		_ASSERTE(t_header.n_image_type == (tga_Grayscale | tga_Compressed_Mask));
		for(const uint32_t *p_scanline = r_t_bmp.p_buffer, *p_end = r_t_bmp.p_buffer +
		   (n_width * n_height); p_scanline != p_end; p_scanline += n_width) {
			for(const uint32_t *p_ptr = p_scanline, *p_end2 = p_scanline + n_width;
			   p_ptr != p_end2; ++ p_ptr) {
				size_t n_remains = p_end2 - p_ptr;
				if(n_remains > 128)
					n_remains = 128;
				bool b_compress = n_remains > 2 && n_Red(*p_ptr) == n_Red(*(p_ptr + 1)) &&
					n_Red(*p_ptr) == n_Red(*(p_ptr + 2));
				size_t n_run_length = n_remains;
				for(size_t i = 1; i < n_remains; ++ i) {
					if(b_compress && n_Red(*p_ptr) != n_Red(*(p_ptr + i))) {
						n_run_length = i;
						break;
					} else if(!b_compress && i + 1 < n_remains &&
					   n_Red(*(p_ptr + i - 1)) == n_Red(*(p_ptr + i)) &&
					   n_Red(*(p_ptr + i - 1)) == n_Red(*(p_ptr + i + 1))) {
						n_run_length = i - 1;
						break;
					}
				}
				// determine run length and wheter to compress data

				uint8_t n_code = uint8_t(n_run_length - 1) | ((b_compress)? 0x80 : 0x00);
				if(fwrite(&n_code, sizeof(uint8_t), 1, p_fw) != 1) {
					fclose(p_fw);
					return false;
				}

				if(b_compress) {
					uint8_t n_grey = n_Red(*p_ptr);
					p_ptr += n_run_length - 1;
					if(fwrite(&n_grey, sizeof(uint8_t), 1, p_fw) != 1) {
						fclose(p_fw);
						return false;
					}
				} else {
					for(const uint32_t *p_end3 = p_ptr + n_run_length; p_ptr != p_end3; ++ p_ptr) {
						uint8_t n_grey = n_Red(*p_ptr);
						if(fwrite(&n_grey, sizeof(uint8_t), 1, p_fw) != 1) {
							fclose(p_fw);
							return false;
						}
					}
					-- p_ptr; // ++ in the for command
				}
			}
		}
		// RLE grey
	}
	// write data

	fclose(p_fw);

	return true;
}

/*
 *								=== ~CTgaCodec ===
 */
