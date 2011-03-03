/*
								+--------------------------------+
								|                                |
								|     ***   OpenGL 3.0   ***     |
								|                                |
								|  Copyright © -tHE SWINe- 2009  |
								|                                |
								|         FrameBuffer.h          |
								|                                |
								+--------------------------------+
*/

/**
 *	@file FrameBuffer.h
 *	@author -tHE SWINe-
 *	@brief OpenGL 3.0 frame-buffer object
 */

#ifndef __RENDER_BUFFER2_INCLUDED
#define __RENDER_BUFFER2_INCLUDED

#include "OpenGL30Drv.h"
#include <crtdbg.h>

/*
 *								=== CGLFrameBufferObject ===
 */

/**
 *	@brief wrapper class for GL_ARB_frame_buffer_object
 *
 *	@todo Implement layered attachments, test depth / stencil / packed variants.
 */
class CGLFrameBufferObject {
public:
	enum {
		max_DrawBuffer_Num = 16
	};

private:
	int m_n_width, m_n_height;

	struct TImageInfo {
		bool b_texture_target;
		GLenum n_internal_format;
		int n_multisample_sample_num; // 0 = no multisampling, 1 to n_Max_Sample_Num() or n_Max_IntegerSample_Num(), depending on n_internal_format

		GLuint Create_RenderBuffer(int n_width, int n_height, GLenum n_attachment) const
		{
			_ASSERTE(!b_texture_target);

			GLuint n_renderbuffer;
			glGenRenderbuffers(1, &n_renderbuffer);
			if(!n_renderbuffer)
				return 0;

			glBindRenderbuffer(GL_RENDERBUFFER, n_renderbuffer);
			if(n_multisample_sample_num == 0) {
				glRenderbufferStorage(GL_RENDERBUFFER, n_internal_format, n_width, n_height);
			} else {
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, n_multisample_sample_num,
					n_internal_format, n_width, n_height);
			}
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, n_attachment, GL_RENDERBUFFER, n_renderbuffer);
			// create and attach renderbuffer

			return n_renderbuffer;
		}
	};

	int m_n_color_buffer_num;
	TImageInfo m_p_color_buffer[max_DrawBuffer_Num];

	bool m_b_depth_stencil_buffer_packed; // in such case, only m_n_depth_rb is created, with packed_depth_stencil format

	bool m_b_depth_buffer;
	TImageInfo m_t_depth_buffer;

	bool m_b_stencil_buffer;
	TImageInfo m_t_stencil_buffer;

	struct TTextureBinding {
		GLenum n_target; // GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_FACE_POSITIVE_X, GL_TEXTURE_3D, ...
		GLuint n_texture_id;
	};

	int m_n_bound_color_texture_num; // number of textures, being currently bound
	TTextureBinding m_p_bound_color_texture[max_DrawBuffer_Num];
	TTextureBinding m_t_bound_depth_texture;
	TTextureBinding m_t_bound_stencil_texture; // current bound texture or 0

	bool m_b_status, m_b_active;

	GLuint m_n_framebuffer;
	GLuint m_p_color_rb[max_DrawBuffer_Num], m_n_depth_rb, m_n_stencil_rb; // FBO objects

	int m_n_my_max_draw_buffer_num; // number of this framebuffer's draw buffers

public:
	/**
	 *	@brief default constructor
	 *
	 *	Creates FBO with n_width per n_height pixels (note dimensions of individual images in a single FBO may
	 *		be different, but it is impossible to set here; it is however posible to render to textures of
	 *		different resolutions). FBO may contain several color images, some of them may be textures, the
	 *		other will be stored in render-buffers. Each render buffer may have different format and multi-sampling.
	 *		FBO may have depth buffer and/or stencil buffer or packed depth/stencil buffer. Each of those
	 *		may be stored in texture / render buffer, each render buffer may have different format and multi-sampling
	 *		again. When using packed depth-stencil, depth-stencil parameters are set trough the corresponding
	 *		depth parameters, while stencil parameters are ignored.
	 *
	 *	@param[in] n_width is framebuffer width, in pixels
	 *	@param[in] n_height is framebuffer height, in pixels
	 *
	 *	@param[in] n_color_buffer_num is number of color buffers (may be 0)
	 *	@param[in] p_color_texture_target contains n_color_buffer_num values,
	 *		true = color buffer will be rendered to texture, false = color buffer will be rendered to render-buffer
	 *	@param[in] p_color_internal_format contains n_color_buffer_num internal formats for
	 *		the corresponding render-buffers. value is ignored if corresponding p_color_texture_target is true.
	 *	@param[in] p_color_multisample_num contains n_color_buffer_num numbers of samples per pixel for
	 *		the corresponding render-buffers; 0 = no multisampling, or 1 to n_Max_Sample_Num() to enable
	 *		multisampling; value is ignored if corresponding p_color_texture_target is true.
	 *
	 *	@param[in] b_depth_buffer is depth buffer requirement flag. if not set, following depth-related
	 *		parameters are ignored
	 *	@param[in] b_depth_texture_target is depth texture target flag (if set, depth values
	 *		will be rendered to texture, otherwise to render-buffer); only valid if b_depth_buffer is set
	 *	@param[in] n_depth_internal_format is internal format for depth buffer (one of GL_DEPTH_COMPONENT16,
	 *		GL_DEPTH_COMPONENT24 or GL_DEPTH_COMPONENT32 if not using packed depth/stencil, otherwise
	 *		one of GL_DEPTH24_STENCIL8); only valid if b_depth_buffer is set and b_depth_texture_target is not set
	 *	@param[in] n_depth_multisample_num is numbers of depth samples per depth buffer pixel;
	 *		0 = no multisampling, or 1 to n_Max_Sample_Num() to enable multisampling; only valid if
	 *		b_depth_buffer is set and b_depth_texture_target is not set
	 *
	 *	@param[in] b_depth_stencil_packed chooses between separate or packed depth stencil.
	 *		if set, the following stencil related parameters are ignored, b_depth_buffer must be set,
	 *		and n_depth_internal_format must be one of DEPTH_STENCIL formats (or b_depth_texture_target
	 *		must be set, and bound texture must have DEPTH_STENCIL format).
	 *
	 *	@param[in] b_stencil_buffer is stencil buffer requirement flag. if not set, following stencil-related
	 *		parameters are ignored; only valid if b_depth_stencil_packed is not set
	 *	@param[in] b_stencil_texture_target is stencil texture target flag (if set, stencil values
	 *		will be rendered to texture, otherwise to render-buffer); only valid if b_stencil_buffer is set
	 *		and b_depth_stencil_packed is not set
	 *	@param[in] n_stencil_internal_format is internal format for stencil buffer (one of GL_STENCIL_INDEX1_EXT,
	 *		GL_STENCIL_INDEX4_EXT, GL_STENCIL_INDEX8_EXT or GL_STENCIL_INDEX16_EXT); only valid if
	 *		b_stencil_buffer is set and b_stencil_texture_target is not set and b_depth_stencil_packed is not set
	 *	@param[in] n_stencil_multisample_num is numbers of stencil samples per stencil buffer pixel;
	 *		0 = no multisampling, or 1 to n_Max_Sample_Num() to enable multisampling; only valid if
	 *		b_stencil_buffer is set and b_stencil_texture_target is not set and b_depth_stencil_packed is not set
	 *
	 *	@note Constructor may or may not succeed, it is recommended to call b_Status()
	 *		immediately after constructor returns.
	 *	@note This is the most versatile constructor, and may be a little confusing. There are other constructors
	 *		with less parameters.
	 */
	inline CGLFrameBufferObject(int n_width, int n_height,
		int n_color_buffer_num, const bool *p_color_texture_target,
		const GLenum *p_color_internal_format, const int *p_color_multisample_num,
		bool b_depth_buffer, bool b_depth_texture_target,
		GLenum n_depth_internal_format, int n_depth_multisample_num,
		bool b_depth_stencil_packed, bool b_stencil_buffer, bool b_stencil_texture_target,
		GLenum n_stencil_internal_format, int n_stencil_multisample_num)
	{
		Construct(n_width, n_height, n_color_buffer_num, p_color_texture_target,
			p_color_internal_format, p_color_multisample_num, b_depth_buffer, b_depth_texture_target,
			n_depth_internal_format, n_depth_multisample_num, b_depth_stencil_packed, b_stencil_buffer,
			b_stencil_texture_target, n_stencil_internal_format, n_stencil_multisample_num);
	}

	/**
	 *	@brief packed depth-stencil constructor
	 *
	 *	Creates FBO with n_width per n_height pixels (note dimensions of individual images in a single FBO may
	 *		be different, but it is impossible to set here; it is however posible to render to textures of
	 *		different resolutions). FBO may contain several color images, some of them may be textures, the
	 *		other will be stored in render-buffers. Each render buffer may have different format and multi-sampling.
	 *		FBO may have packed depth/stencil buffer, stored in texture / render-buffer, it is possible to select
	 *		render-buffer format and multi-sampling again.
	 *
	 *	@param[in] n_width is framebuffer width, in pixels
	 *	@param[in] n_height is framebuffer height, in pixels
	 *
	 *	@param[in] n_color_buffer_num is number of color buffers (may be 0)
	 *	@param[in] p_color_texture_target contains n_color_buffer_num values,
	 *		true = color buffer will be rendered to texture, false = color buffer will be rendered to render-buffer
	 *	@param[in] p_color_internal_format contains n_color_buffer_num internal formats for
	 *		the corresponding render-buffers. value is ignored if corresponding p_color_texture_target is true.
	 *	@param[in] p_color_multisample_num contains n_color_buffer_num numbers of samples per pixel for
	 *		the corresponding render-buffers; 0 = no multisampling, or 1 to n_Max_Sample_Num() to enable
	 *		multisampling; value is ignored if corresponding p_color_texture_target is true.
	 *
	 *	@param[in] b_depth_stencil_buffer is depth/stencil buffer requirement flag. if not set,
	 *		all the following parameters are ignored
	 *	@param[in] b_depth_stencil_texture_target is depth/stencil texture target flag (if set, depth/stencil values
	 *		will be rendered to texture, otherwise to render-buffer); only valid if b_depth_stencil_buffer is set
	 *	@param[in] n_depth_stencil_internal_format is internal format for depth/stencil buffer (GL_DEPTH24_STENCIL8);
	 *		only valid if b_depth_stencil_buffer is set and b_depth_stencil_texture_target is not set
	 *	@param[in] n_depth_stencil_multisample_num is numbers of depth/stencil samples per pixel;
	 *		0 = no multisampling, or 1 to n_Max_Sample_Num() to enable multisampling; only valid if
	 *		b_depth_stencil_buffer is set and b_depth_stencil_texture_target is not set
	 *
	 *	@note Constructor may or may not succeed, it is recommended to call b_Status()
	 *		immediately after constructor returns.
	 */
	inline CGLFrameBufferObject(int n_width, int n_height,
		int n_color_buffer_num, const bool *p_color_texture_target,
		const GLenum *p_color_internal_format, const int *p_color_multisample_num,
		bool b_depth_stencil_buffer, bool b_depth_stencil_texture_target,
		GLenum n_depth_stencil_internal_format, int n_depth_stencil_multisample_num)
	{
		Construct(n_width, n_height, n_color_buffer_num, p_color_texture_target,
			p_color_internal_format, p_color_multisample_num, b_depth_stencil_buffer,
			b_depth_stencil_texture_target, n_depth_stencil_internal_format,
			n_depth_stencil_multisample_num, true, false, false, 0, 0);
	}

	/**
	 *	@brief separate depth stencil constructor
	 *
	 *	Creates FBO with n_width per n_height pixels (note dimensions of individual images in a single FBO may
	 *		be different, but it is impossible to set here; it is however posible to render to textures of
	 *		different resolutions). FBO may contain several color images, some of them may be textures, the
	 *		other will be stored in render-buffers. Each render buffer may have different format and multi-sampling.
	 *		FBO may have depth buffer and/or stencil buffer (note currently only way to have both is to
	 *		use packed depth/stencil). Each of those may be stored in texture / render buffer, each render buffer
	 *		may have different format and multi-sampling again.
	 *
	 *	@param[in] n_width is framebuffer width, in pixels
	 *	@param[in] n_height is framebuffer height, in pixels
	 *
	 *	@param[in] n_color_buffer_num is number of color buffers (may be 0)
	 *	@param[in] p_color_texture_target contains n_color_buffer_num values,
	 *		true = color buffer will be rendered to texture, false = color buffer will be rendered to render-buffer
	 *	@param[in] p_color_internal_format contains n_color_buffer_num internal formats for
	 *		the corresponding render-buffers. value is ignored if corresponding p_color_texture_target is true.
	 *	@param[in] p_color_multisample_num contains n_color_buffer_num numbers of samples per pixel for
	 *		the corresponding render-buffers; 0 = no multisampling, or 1 to n_Max_Sample_Num() to enable
	 *		multisampling; value is ignored if corresponding p_color_texture_target is true.
	 *
	 *	@param[in] b_depth_buffer is depth buffer requirement flag. if not set, following depth-related
	 *		parameters are ignored
	 *	@param[in] b_depth_texture_target is depth texture target flag (if set, depth values
	 *		will be rendered to texture, otherwise to render-buffer); only valid if b_depth_buffer is set
	 *	@param[in] n_depth_internal_format is internal format for depth buffer (one of GL_DEPTH_COMPONENT16,
	 *		GL_DEPTH_COMPONENT24 or GL_DEPTH_COMPONENT32); only valid if b_depth_buffer is set and
	 *		b_depth_texture_target is not set
	 *	@param[in] n_depth_multisample_num is numbers of depth samples per depth buffer pixel;
	 *		0 = no multisampling, or 1 to n_Max_Sample_Num() to enable multisampling; only valid if
	 *		b_depth_buffer is set and b_depth_texture_target is not set
	 *
	 *	@param[in] b_stencil_buffer is stencil buffer requirement flag. if not set, following stencil-related
	 *		parameters are ignored
	 *	@param[in] b_stencil_texture_target is stencil texture target flag (if set, stencil values
	 *		will be rendered to texture, otherwise to render-buffer); only valid if b_stencil_buffer is set
	 *	@param[in] n_stencil_internal_format is internal format for stencil buffer (one of GL_STENCIL_INDEX1_EXT,
	 *		GL_STENCIL_INDEX4_EXT, GL_STENCIL_INDEX8_EXT or GL_STENCIL_INDEX16_EXT); only valid if
	 *		b_stencil_buffer is set and b_stencil_texture_target is not set
	 *	@param[in] n_stencil_multisample_num is numbers of stencil samples per stencil buffer pixel;
	 *		0 = no multisampling, or 1 to n_Max_Sample_Num() to enable multisampling; only valid if
	 *		b_stencil_buffer is set and b_stencil_texture_target is not set
	 *
	 *	@note Constructor may or may not succeed, it is recommended to call b_Status()
	 *		immediately after constructor returns.
	 */
	inline CGLFrameBufferObject(int n_width, int n_height,
		int n_color_buffer_num, const bool *p_color_texture_target,
		const GLenum *p_color_internal_format, const int *p_color_multisample_num,
		bool b_depth_buffer, bool b_depth_texture_target,
		GLenum n_depth_internal_format, int n_depth_multisample_num,
		bool b_stencil_buffer, bool b_stencil_texture_target,
		GLenum n_stencil_internal_format, int n_stencil_multisample_num)
	{
		Construct(n_width, n_height, n_color_buffer_num, p_color_texture_target,
			p_color_internal_format, p_color_multisample_num, b_depth_buffer, b_depth_texture_target,
			n_depth_internal_format, n_depth_multisample_num, false, b_stencil_buffer,
			b_stencil_texture_target, n_stencil_internal_format, n_stencil_multisample_num);
	}

	/**
	 *	@brief packed depth-stencil constructor with all color buffers having the same properties
	 *
	 *	Creates FBO with n_width per n_height pixels (note dimensions of individual images in a single FBO may
	 *		be different, but it is impossible to set here; it is however posible to render to textures of
	 *		different resolutions). FBO may contain several color images, those may be stored in textures,
	 *		or in render-buffers. All render-buffers have the same format and multi-sampling. FBO may have
	 *		packed depth/stencil buffer, stored in texture / render-buffer, it is possible to select
	 *		render-buffer format and multi-sampling again.
	 *
	 *	@param[in] n_width is framebuffer width, in pixels
	 *	@param[in] n_height is framebuffer height, in pixels
	 *
	 *	@param[in] n_color_buffer_num is number of color buffers (may be 0)
	 *	@param[in] b_color_texture_target chooses between color buffers stored in textures (true),
	 *		or color buffers stored in render-buffers (false)
	 *	@param[in] n_color_internal_format contains internal format of color render-buffer(s);
	 *		value is ignored if b_color_texture_target is true.
	 *	@param[in] n_color_multisample_num contains number of samples per pixel for
	 *		color render-buffer(s); 0 = no multisampling, or 1 to n_Max_Sample_Num() to enable
	 *		multisampling; value is ignored if b_color_texture_target is true.
	 *
	 *	@param[in] b_depth_stencil_buffer is depth/stencil buffer requirement flag. if not set,
	 *		all the following parameters are ignored
	 *	@param[in] b_depth_stencil_texture_target is depth/stencil texture target flag (if set, depth/stencil values
	 *		will be rendered to texture, otherwise to render-buffer); only valid if b_depth_stencil_buffer is set
	 *	@param[in] n_depth_stencil_internal_format is internal format for depth/stencil buffer (GL_DEPTH24_STENCIL8);
	 *		only valid if b_depth_stencil_buffer is set and b_depth_stencil_texture_target is not set
	 *	@param[in] n_depth_stencil_multisample_num is numbers of depth/stencil samples per pixel;
	 *		0 = no multisampling, or 1 to n_Max_Sample_Num() to enable multisampling; only valid if
	 *		b_depth_stencil_buffer is set and b_depth_stencil_texture_target is not set
	 *
	 *	@note Constructor may or may not succeed, it is recommended to call b_Status()
	 *		immediately after constructor returns.
	 */
	inline CGLFrameBufferObject(int n_width, int n_height,
		int n_color_buffer_num, bool b_color_texture_target,
		GLenum n_color_internal_format, int n_color_multisample_num,
		bool b_depth_stencil_buffer, bool b_depth_stencil_texture_target,
		GLenum n_depth_stencil_internal_format, int n_depth_stencil_multisample_num)
	{
		bool p_color_texture_target[max_DrawBuffer_Num];
		GLenum p_color_internal_format[max_DrawBuffer_Num];
		int p_color_multisample_num[max_DrawBuffer_Num];
		_ASSERTE(n_color_buffer_num <= max_DrawBuffer_Num);
		for(int i = 0; i < n_color_buffer_num; ++ i) {
			p_color_texture_target[i] = b_color_texture_target;
			p_color_internal_format[i] = n_color_internal_format;
			p_color_multisample_num[i] = n_color_multisample_num;
		}
		// expand constant parameters to array

		Construct(n_width, n_height, n_color_buffer_num, p_color_texture_target,
			p_color_internal_format, p_color_multisample_num, b_depth_stencil_buffer,
			b_depth_stencil_texture_target, n_depth_stencil_internal_format,
			n_depth_stencil_multisample_num, true, false, false, 0, 0);
	}

	/**
	 *	@brief separate depth stencil constructor with all color buffers having the same properties
	 *
	 *	Creates FBO with n_width per n_height pixels (note dimensions of individual images in a single FBO may
	 *		be different, but it is impossible to set here; it is however posible to render to textures of
	 *		different resolutions). FBO may contain several color images, those may be stored in textures,
	 *		or in render-buffers. All render-buffers have the same format and multi-sampling. FBO may have
	 *		depth buffer and/or stencil buffer (note currently only way to have both is to use packed
	 *		depth/stencil). Each of those may be stored in texture / render buffer, each render buffer may have
	 *		different format and multi-sampling again.
	 *
	 *	@param[in] n_width is framebuffer width, in pixels
	 *	@param[in] n_height is framebuffer height, in pixels
	 *
	 *	@param[in] n_color_buffer_num is number of color buffers (may be 0)
	 *	@param[in] b_color_texture_target chooses between color buffers stored in textures (true),
	 *		or color buffers stored in render-buffers (false)
	 *	@param[in] n_color_internal_format contains internal format of color render-buffer(s);
	 *		value is ignored if b_color_texture_target is true.
	 *	@param[in] n_color_multisample_num contains number of samples per pixel for
	 *		color render-buffer(s); 0 = no multisampling, or 1 to n_Max_Sample_Num() to enable
	 *		multisampling; value is ignored if b_color_texture_target is true.
	 *
	 *	@param[in] b_depth_buffer is depth buffer requirement flag. if not set, following depth-related
	 *		parameters are ignored
	 *	@param[in] b_depth_texture_target is depth texture target flag (if set, depth values
	 *		will be rendered to texture, otherwise to render-buffer); only valid if b_depth_buffer is set
	 *	@param[in] n_depth_internal_format is internal format for depth buffer (one of GL_DEPTH_COMPONENT16,
	 *		GL_DEPTH_COMPONENT24 or GL_DEPTH_COMPONENT32); only valid if b_depth_buffer is set and
	 *		b_depth_texture_target is not set
	 *	@param[in] n_depth_multisample_num is numbers of depth samples per depth buffer pixel;
	 *		0 = no multisampling, or 1 to n_Max_Sample_Num() to enable multisampling; only valid if
	 *		b_depth_buffer is set and b_depth_texture_target is not set
	 *
	 *	@param[in] b_stencil_buffer is stencil buffer requirement flag. if not set, following stencil-related
	 *		parameters are ignored
	 *	@param[in] b_stencil_texture_target is stencil texture target flag (if set, stencil values
	 *		will be rendered to texture, otherwise to render-buffer); only valid if b_stencil_buffer is set
	 *	@param[in] n_stencil_internal_format is internal format for stencil buffer (one of GL_STENCIL_INDEX1_EXT,
	 *		GL_STENCIL_INDEX4_EXT, GL_STENCIL_INDEX8_EXT or GL_STENCIL_INDEX16_EXT); only valid if
	 *		b_stencil_buffer is set and b_stencil_texture_target is not set
	 *	@param[in] n_stencil_multisample_num is numbers of stencil samples per stencil buffer pixel;
	 *		0 = no multisampling, or 1 to n_Max_Sample_Num() to enable multisampling; only valid if
	 *		b_stencil_buffer is set and b_stencil_texture_target is not set
	 *
	 *	@note Constructor may or may not succeed, it is recommended to call b_Status()
	 *		immediately after constructor returns.
	 */
	inline CGLFrameBufferObject(int n_width, int n_height,
		int n_color_buffer_num, bool b_color_texture_target,
		GLenum n_color_internal_format, int n_color_multisample_num,
		bool b_depth_buffer, bool b_depth_texture_target,
		GLenum n_depth_internal_format, int n_depth_multisample_num,
		bool b_stencil_buffer, bool b_stencil_texture_target,
		GLenum n_stencil_internal_format, int n_stencil_multisample_num)
	{
		bool p_color_texture_target[max_DrawBuffer_Num];
		GLenum p_color_internal_format[max_DrawBuffer_Num];
		int p_color_multisample_num[max_DrawBuffer_Num];
		_ASSERTE(n_color_buffer_num <= max_DrawBuffer_Num);
		for(int i = 0; i < n_color_buffer_num; ++ i) {
			p_color_texture_target[i] = b_color_texture_target;
			p_color_internal_format[i] = n_color_internal_format;
			p_color_multisample_num[i] = n_color_multisample_num;
		}
		// expand constant parameters to array

		Construct(n_width, n_height, n_color_buffer_num, p_color_texture_target,
			p_color_internal_format, p_color_multisample_num, b_depth_buffer, b_depth_texture_target,
			n_depth_internal_format, n_depth_multisample_num, false, b_stencil_buffer,
			b_stencil_texture_target, n_stencil_internal_format, n_stencil_multisample_num);
	}
	
	/**
	 *	@brief destructor
	 *
	 *	Takes care of cleaning up allocated OpenGL objects.
	 */
	~CGLFrameBufferObject();

	/**
	 *	@return Returns width of the render buffer, in pixels.
	 */
	inline int n_Width() const
	{
		return m_n_width;
	}

	/**
	 *	@return Returns height of the render buffer, in pixels.
	 */
	inline int n_Height() const
	{
		return m_n_height;
	}

	/**
	 *	@return Returns number of color (draw) buffers.
	 */
	inline int n_Color_Buffer_Num() const
	{
		return m_n_color_buffer_num;
	}

	/**
	 *	@param[in] n_draw_buffer_index is zero-based index of draw buffer (0 to n_Color_Buffer_Num() - 1)
	 *
	 *	@return Returns OpenGL name of color buffer with zero based index n_draw_buffer_index.
	 *
	 *	@note Useful for manual copying of results of rendering to multiple buffers.
	 */
	inline GLenum n_Draw_Buffer(int n_draw_buffer_index) const
	{
		return GL_COLOR_ATTACHMENT0 + n_draw_buffer_index;
	}

	/**
	 *	@param[in] n_draw_buffer_index is zero-based index of draw buffer (0 to n_Color_Buffer_Num() - 1)
	 *
	 *	@return Returns true if color buffer with zero based index n_draw_buffer_index is stored
	 *		in texture, otherwise returns false (color buffer is stored in render-buffer).
	 */
	inline bool b_Color_TextureTarget(int n_buffer_index) const
	{
		return m_p_color_buffer[n_buffer_index].b_texture_target;
	}

	/**
	 *	@param[in] n_draw_buffer_index is zero-based index of draw buffer (0 to n_Color_Buffer_Num() - 1)
	 *
	 *	@return Returns OpenGL internal format of color buffer with zero based index n_draw_buffer_index.
	 *		In case it is stored in texture, returns 0 (internal format depends on bound texture).
	 */
	inline GLenum n_Color_Format(int n_draw_buffer_index) const
	{
		return m_p_color_buffer[n_draw_buffer_index].n_internal_format;
	}

	/**
	 *	@param[in] n_draw_buffer_index is zero-based index of draw buffer (0 to n_Color_Buffer_Num() - 1)
	 *
	 *	@return Returns number of samples per pixel of color buffer with zero based index n_draw_buffer_index.
	 *		In case it is stored in texture, returns 0 (number of samples depends on bound texture).
	 */
	inline int n_Color_MultiSample_Sample_Num(int n_draw_buffer_index) const
	{
		return m_p_color_buffer[n_draw_buffer_index].n_multisample_sample_num;
	}

	/**
	 *	@return Returns true in case frame buffer has depth buffer (even in texture), otherwise false.
	 */
	inline bool b_Depth_Buffer() const
	{
		return m_b_depth_buffer;
	}

	/**
	 *	@return Returns true in case depth is to be rendered to depth texture,
	 *		otherwise false (depth is either not present or rendered to render-buffer).
	 */
	inline bool b_Depth_TextureTarget() const
	{
		return m_t_depth_buffer.b_texture_target;
	}

	/**
	 *	@return Returns depth buffer internal format (only valid in case depth buffer is present).
	 *		In case depth buffer is stored in texture, returns 0 (internal format depends on bound texture).
	 */
	inline GLenum n_Depth_Format() const
	{
		return m_t_depth_buffer.n_internal_format;
	}

	/**
	 *	@return Returns number of samples per pixel of depth buffer (only valid in case depth buffer is present).
	 *		In case depth buffer is stored in texture, returns 0 (number of samples depends on bound texture).
	 */
	inline int n_Depth_MultiSample_Sample_Num() const
	{
		return m_t_depth_buffer.n_multisample_sample_num;
	}

	/**
	 *	@return Returns true if depth and stencil fragments are stored in a single buffer,
	 *		using packed depth/stencil pixel format, otherwise returns false.
	 */
	inline bool b_Packed_DepthStencil_Buffer() const
	{
		return m_b_depth_stencil_buffer_packed;
	}

	/**
	 *	@return Returns true in case frame buffer has stencil buffer (even in texture), otherwise false.
	 */
	inline bool b_Stencil_Buffer() const
	{
		return m_b_stencil_buffer;
	}

	/**
	 *	@return Returns true in case stencil is to be rendered to stencil texture,
	 *		otherwise false (stencil is either not present or rendered to render-buffer).
	 */
	inline bool b_Stencil_TextureTarget() const
	{
		return m_t_stencil_buffer.b_texture_target;
	}

	/**
	 *	@return Returns stencil buffer internal format (only valid in case stencil buffer is present).
	 *		In case stencil buffer is stored in texture, returns 0 (internal format depends on bound texture).
	 */
	inline GLenum n_Stencil_Format() const
	{
		return m_t_stencil_buffer.n_internal_format;
	}

	/**
	 *	@return Returns number of samples per pixel of stencil buffer (only valid in case stencil buffer is present).
	 *		In case stencil buffer is stored in texture, returns 0 (number of samples depends on bound texture).
	 */
	inline int n_Stencil_MultiSample_Sample_Num() const
	{
		return m_t_stencil_buffer.n_multisample_sample_num;
	}

	/**
	 *	@brief binds the framebuffer buffer as OpenGL output buffer
	 *
	 *	@return Returns true on success or false on failure
	 */
	bool Bind();

	/**
	 *	@brief checks frame-buffer status
	 *
	 *	@return Returns true in case frame-buffer, specified by constructor parameters
	 *		was successfuly created and is ready to be used (glCheckFramebufferStatus()
	 *		returns GL_FRAMEBUFFER_COMPLETE).
	 *
	 *	@note glCheckFramebufferStatus() is only called if frame-buffer is bound.
	 */
	bool b_Status() const;

	/**
	 *	@brief checks frame-buffer status
	 *
	 *	@return Returns result of glCheckFramebufferStatus() (should be GL_FRAMEBUFFER_COMPLETE).
	 *	@return Returns 0 in case FBO isn't currently bound, or failed to be created.
	 */
	GLenum n_Status() const;

	/**
	 *	@brief binds one-dimensional color texture
	 *
	 *	Binds one-dimensional color texture n_texture of type n_target (should be GL_TEXTURE_1D)
	 *		to color attachment (draw buffer) with zero-based index n_attachment_index.
	 *		Frame-buffer must be bound to do this. It fails in case attachment n_attachment_index
	 *		was not specified as texture target in constructor (renders to render-buffer).
	 *		Binding texture with id 0 releases previously bound texture, as does binding
	 *		different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] n_target is texture target (GL_TEXTURE_1D)
	 *	@param[in] n_texture is OpenGL texture object id
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	bool Bind_ColorTexture1D(int n_attachment_index, GLenum n_target, GLuint n_texture, int n_level = 0);

	/**
	 *	@brief binds two-dimensional color texture
	 *
	 *	Binds two-dimensional color texture n_texture of type n_target (should be
	 *		one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) to color attachment (draw buffer) with
	 *		zero-based index n_attachment_index. Frame-buffer must be bound to do this.
	 *		It fails in case attachment n_attachment_index was not specified as texture
	 *		target in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] n_target is texture target (one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	 *	@param[in] n_texture is OpenGL texture object id
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	bool Bind_ColorTexture2D(int n_attachment_index, GLenum n_target, GLuint n_texture, int n_level = 0);

	/**
	 *	@brief binds three-dimensional color texture
	 *
	 *	Binds one layer of three-dimensional color texture n_texture of type n_target
	 *		(should be GL_TEXTURE_3D) to color attachment (draw buffer) with
	 *		zero-based index n_attachment_index. Frame-buffer must be bound to do this.
	 *		It fails in case attachment n_attachment_index was not specified as texture
	 *		target in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] n_target is texture target (GL_TEXTURE_3D)
	 *	@param[in] n_texture is OpenGL texture object id
	 *	@param[in] n_level is texture mip-map level
	 *	@param[in] n_layer is 3D texture layer (zero-based index of 3D texture slice to render to)
	 *
	 *	@return Returns true on success, false on failure.
	 */
	bool Bind_ColorTexture3D(int n_attachment_index, GLenum n_target, GLuint n_texture, int n_level, int n_layer);

	/**
	 *	@brief binds two-dimensional depth texture
	 *
	 *	Binds two-dimensional depth texture n_texture of type n_target (should be
	 *		one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z). Frame-buffer must be bound to do this.
	 *		It fails in case depth attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was specified in constructor,
	 *		Bind_DepthStencilTexture2D() must be used in such case.
	 *
	 *	@param[in] n_target is texture target (one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	 *	@param[in] n_texture is OpenGL texture object id
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	bool Bind_DepthTexture2D(GLenum n_target, GLuint n_texture, int n_level);

	/**
	 *	@brief binds two-dimensional stencil texture
	 *
	 *	Binds two-dimensional stencil texture n_texture of type n_target (should be
	 *		one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z). Frame-buffer must be bound to do this.
	 *		It fails in case stencil attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was specified in constructor,
	 *		Bind_DepthStencilTexture2D() must be used in such case.
	 *
	 *	@param[in] n_target is texture target (one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	 *	@param[in] n_texture is OpenGL texture object id
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	bool Bind_StencilTexture2D(GLenum n_target, GLuint n_texture, int n_level);

	/**
	 *	@brief binds two-dimensional packed depth/stencil texture
	 *
	 *	Binds two-dimensional depth/stencil texture n_texture of type n_target (should be
	 *		one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z). Frame-buffer must be bound to do this.
	 *		It fails in case depth attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was not specified in constructor,
	 *		Bind_DepthTexture2D() and Bind_StencilTexture2D() must be used in such case
	 *		(with the same texture depth/stencil, or with depth texture and another
	 *		stencil texture).
	 *
	 *	@param[in] n_target is texture target (one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	 *	@param[in] n_texture is OpenGL texture object id
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	bool Bind_DepthStencilTexture2D(GLenum n_target, GLuint n_texture, int n_level);

#if 0
	/**
	 *	@brief binds one-dimensional color texture
	 *
	 *	Binds one-dimensional color texture r_texture of type n_target (should be GL_TEXTURE_1D)
	 *		to color attachment (draw buffer) with zero-based index n_attachment_index.
	 *		Frame-buffer must be bound to do this. It fails in case attachment n_attachment_index
	 *		was not specified as texture target in constructor (renders to render-buffer).
	 *		Binding texture with id 0 releases previously bound texture, as does binding
	 *		different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] n_target is texture target (GL_TEXTURE_1D)
	 *	@param[in] r_texture is reference to OpenGL texture object
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_ColorTexture1D(int n_attachment_index, GLenum n_target, CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_ColorTexture1D(n_attachment_index, n_target, r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds two-dimensional color texture
	 *
	 *	Binds two-dimensional color texture r_texture of type n_target (should be
	 *		one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) to color attachment (draw buffer) with
	 *		zero-based index n_attachment_index. Frame-buffer must be bound to do this.
	 *		It fails in case attachment n_attachment_index was not specified as texture
	 *		target in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] n_target is texture target (one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	 *	@param[in] r_texture is reference to OpenGL texture object
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_ColorTexture2D(int n_attachment_index, GLenum n_target, CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_ColorTexture2D(n_attachment_index, n_target, r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds three-dimensional color texture
	 *
	 *	Binds one layer of three-dimensional color texture r_texture of type n_target
	 *		(should be GL_TEXTURE_3D) to color attachment (draw buffer) with
	 *		zero-based index n_attachment_index. Frame-buffer must be bound to do this.
	 *		It fails in case attachment n_attachment_index was not specified as texture
	 *		target in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] n_target is texture target (GL_TEXTURE_3D)
	 *	@param[in] r_texture is reference to OpenGL texture object
	 *	@param[in] n_level is texture mip-map level
	 *	@param[in] n_layer is 3D texture layer (zero-based index of 3D texture slice to render to)
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_ColorTexture3D(int n_attachment_index, GLenum n_target, CGLTexture &r_texture, int n_level, int n_layer)
	{
		return Bind_ColorTexture3D(n_attachment_index, n_target, r_texture.n_Id(), n_level, n_layer);
	}

	/**
	 *	@brief binds two-dimensional depth texture
	 *
	 *	Binds two-dimensional depth texture r_texture of type n_target (should be
	 *		one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z). Frame-buffer must be bound to do this.
	 *		It fails in case depth attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was specified in constructor,
	 *		Bind_DepthStencilTexture2D() must be used in such case.
	 *
	 *	@param[in] n_target is texture target (one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	 *	@param[in] r_texture is reference to OpenGL texture object
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_DepthTexture2D(GLenum n_target, CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_DepthTexture2D(n_target, r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds two-dimensional stencil texture
	 *
	 *	Binds two-dimensional stencil texture r_texture of type n_target (should be
	 *		one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z). Frame-buffer must be bound to do this.
	 *		It fails in case stencil attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was specified in constructor,
	 *		Bind_DepthStencilTexture2D() must be used in such case.
	 *
	 *	@param[in] n_target is texture target (one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	 *	@param[in] r_texture is reference to OpenGL texture object
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_StencilTexture2D(GLenum n_target, CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_StencilTexture2D(n_target, r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds two-dimensional packed depth/stencil texture
	 *
	 *	Binds two-dimensional depth/stencil texture r_texture of type n_target (should be
	 *		one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z). Frame-buffer must be bound to do this.
	 *		It fails in case depth attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was not specified in constructor,
	 *		Bind_DepthTexture2D() and Bind_StencilTexture2D() must be used in such case
	 *		(with the same texture depth/stencil, or with depth texture and another
	 *		stencil texture).
	 *
	 *	@param[in] n_target is texture target (one of GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 *		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	 *		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, or GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	 *	@param[in] r_texture is reference to OpenGL texture object
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_DepthStencilTexture2D(GLenum n_target, CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_DepthStencilTexture2D(n_target, r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds one-dimensional color texture
	 *
	 *	Binds one-dimensional color texture r_texture to color attachment (draw buffer) with
	 *		zero-based index n_attachment_index.
	 *		Frame-buffer must be bound to do this. It fails in case attachment n_attachment_index
	 *		was not specified as texture target in constructor (renders to render-buffer).
	 *		Binding texture with id 0 releases previously bound texture, as does binding
	 *		different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] r_texture is reference to OpenGL texture object (must be 1D)
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_ColorTexture1D(int n_attachment_index, CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_ColorTexture1D(n_attachment_index, r_texture.n_Target(), r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds two-dimensional color texture
	 *
	 *	Binds two-dimensional color texture r_texture to color attachment (draw buffer) with
	 *		zero-based index n_attachment_index. Frame-buffer must be bound to do this.
	 *		It fails in case attachment n_attachment_index was not specified as texture
	 *		target in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] r_texture is reference to OpenGL texture object (must be 2D, not cube-map)
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_ColorTexture2D(int n_attachment_index, CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_ColorTexture2D(n_attachment_index, r_texture.n_Target(), r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds three-dimensional color texture
	 *
	 *	Binds one layer of three-dimensional color texture r_texture to color attachment (draw buffer) with
	 *		zero-based index n_attachment_index. Frame-buffer must be bound to do this.
	 *		It fails in case attachment n_attachment_index was not specified as texture
	 *		target in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *
	 *	@param[in] n_attachment_index is color attachment index (0 to n_Color_Buffer_Num() - 1)
	 *	@param[in] r_texture is reference to OpenGL texture object (must be 3D)
	 *	@param[in] n_level is texture mip-map level
	 *	@param[in] n_layer is 3D texture layer (zero-based index of 3D texture slice to render to)
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_ColorTexture3D(int n_attachment_index, CGLTexture &r_texture, int n_level, int n_layer)
	{
		return Bind_ColorTexture3D(n_attachment_index, r_texture.n_Target(), r_texture.n_Id(), n_level, n_layer);
	}

	/**
	 *	@brief binds two-dimensional depth texture
	 *
	 *	Binds two-dimensional depth texture r_texture. Frame-buffer must be bound to do this.
	 *		It fails in case depth attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was specified in constructor,
	 *		Bind_DepthStencilTexture2D() must be used in such case.
	 *
	 *	@param[in] r_texture is reference to OpenGL texture object (must be 2D, not cube-map)
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_DepthTexture2D(CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_DepthTexture2D(r_texture.n_Target(), r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds two-dimensional stencil texture
	 *
	 *	Binds two-dimensional stencil texture r_texture. Frame-buffer must be bound to do this.
	 *		It fails in case stencil attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was specified in constructor,
	 *		Bind_DepthStencilTexture2D() must be used in such case.
	 *
	 *	@param[in] r_texture is reference to OpenGL texture object (must be 2D, not cube-map)
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_StencilTexture2D(CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_StencilTexture2D(r_texture.n_Target(), r_texture.n_Id(), n_level);
	}

	/**
	 *	@brief binds two-dimensional packed depth/stencil texture
	 *
	 *	Binds two-dimensional depth/stencil texture r_texture. Frame-buffer must be bound to do this.
	 *		It fails in case depth attachment was not specified as texture target
	 *		in constructor (renders to render-buffer). Binding texture with id 0
	 *		releases previously bound texture, as does binding different texture.
	 *		It also fails, if packed depth stencil was not specified in constructor,
	 *		Bind_DepthTexture2D() and Bind_StencilTexture2D() must be used in such case
	 *		(with the same texture depth/stencil, or with depth texture and another
	 *		stencil texture).
	 *
	 *	@param[in] r_texture is reference to OpenGL texture object (must be 2D, not cube-map)
	 *	@param[in] n_level is texture mip-map level
	 *
	 *	@return Returns true on success, false on failure.
	 */
	inline bool Bind_DepthStencilTexture2D(CGLTexture &r_texture, int n_level = 0)
	{
		return Bind_DepthStencilTexture2D(r_texture.n_Target(), r_texture.n_Id(), n_level);
	}
#endif

	/**
	 *	@brief releases all bound textures
	 *
	 *	Releases all bound textures. Frame-buffer must be bound to do this.
	 *		The same effect is accomplished by binding textures with id 0 to all
	 *		texture targets (color / depth / stencil) specified for this frame-buffer.
	 *
	 *	@return Returns true on success, false on failure.
	 *
	 *	@note It is usually not necessary to release textures, it might be useful for debugging.
	 */
	bool ReleaseAllTextures();

	/**
	 *	@brief releases the render buffer from OpenGL output buffer binding
	 *
	 *	@return Returns true on success, false on failure.
	 */
	bool Release();

	/**
	 *	@brief gets maximal render-buffer size
	 *
	 *	@return Returns maximal render-buffer size in pixels.
	 *
	 *	@note This involves calling glGetIntegerv(), and may result in unwanted CPU / GPU synchronization.
	 */
	static int n_Max_Size();

	/**
	 *	@brief gets maximal number of samples per pixel
	 *
	 *	@return Returns maximal number of samples per pixel for multisampling.
	 *
	 *	@note This involves calling glGetIntegerv(), and may result in unwanted CPU / GPU synchronization.
	 */
	static int n_Max_Sample_Num();

	/**
	 *	@brief gets maximal number of samples per pixel with integer format
	 *
	 *	@return Returns maximal number of samples per pixel for multisampling, this applies
	 *		to pixels with integer formats.
	 *
	 *	@note This involves calling glGetIntegerv(), and may result in unwanted CPU / GPU synchronization.
	 *
	 *	@todo Clarify what kinds of formats are integer formats (GL_STENCIL_INDEX? GL_DEPTH_COMPONENT? GL_RGBA32I?).
	 */
	static int n_Max_IntegerSample_Num();

	/**
	 *	@brief gets maximal number of draw buffers
	 *
	 *	@return Returns maximal number of OpenGL draw buffers (1 where not supported, otherwise
	 *		1, 2, 4; 8 on GeForce 8800, up to 16 in future).
	 *
	 *	@note The value is cached, so this is fast (doesn't call glGetIntegerv()), but it is only valid
	 *		in case constructor succeeded.
	 */
	inline int n_Max_DrawBuffer_Num()
	{
		return m_n_my_max_draw_buffer_num;
	}

private:
	void Construct(int n_width, int n_height, int n_color_buffer_num, const bool *p_color_texture_target,
		const GLenum *p_color_internal_format, const int *p_color_multisample_num, bool b_depth_buffer,
		bool b_depth_texture_target, GLenum n_depth_internal_format, int n_depth_multisample_num,
		bool b_depth_stencil_packed, bool b_stencil_buffer, bool b_stencil_texture_target,
		GLenum n_stencil_internal_format, int n_stencil_multisample_num);
	bool Create();
};
/*
 *								=== ~CGLFrameBufferObject ===
 */

#endif //__RENDER_BUFFER2_INCLUDED
