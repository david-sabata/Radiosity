/*
								+--------------------------------+
								|                                |
								|     ***   OpenGL 3.0   ***     |
								|                                |
								|  Copyright © -tHE SWINe- 2009  |
								|                                |
								|        FrameBuffer.cpp         |
								|                                |
								+--------------------------------+
*/

/**
 *	@file FrameBuffer.cpp
 *	@author -tHE SWINe-
 *	@brief OpenGL 3.0 frame-buffer object
 */

#include "FrameBuffer.h"

/*
 *								=== CGLFrameBufferObject ===
 */

void CGLFrameBufferObject::Construct(int n_width, int n_height,
	int n_color_buffer_num, const bool *p_color_texture_target,
	const GLenum *p_color_internal_format, const int *p_color_multisample_num,
	bool b_depth_buffer, bool b_depth_texture_target,
	GLenum n_depth_internal_format, int n_depth_multisample_num,
	bool b_depth_stencil_packed, bool b_stencil_buffer, bool b_stencil_texture_target,
	GLenum n_stencil_internal_format, int n_stencil_multisample_num)
{
	//__FuncGuard("CGLFrameBufferObject::Construct");

	_ASSERTE(m_n_color_buffer_num <= max_DrawBuffer_Num);

	m_n_width = n_width;
	m_n_height = n_height;
	// set dimensions

	m_n_color_buffer_num = n_color_buffer_num;
	for(int i = 0; i < m_n_color_buffer_num; ++ i) {
		if(m_p_color_buffer[i].b_texture_target = p_color_texture_target[i]) {
			m_p_color_buffer[i].n_internal_format = 0;
			m_p_color_buffer[i].n_multisample_sample_num = 0; // ignored, if texture target
		} else {
			m_p_color_buffer[i].n_internal_format = p_color_internal_format[i];
			m_p_color_buffer[i].n_multisample_sample_num = p_color_multisample_num[i];
		}
	}
	// set color buffers

	m_b_depth_buffer = b_depth_buffer;
	if(m_t_depth_buffer.b_texture_target = b_depth_texture_target) {
		m_t_depth_buffer.n_internal_format = 0;
		m_t_depth_buffer.n_multisample_sample_num = 0; // ignored, if texture target
	} else {
		m_t_depth_buffer.n_internal_format = n_depth_internal_format;
		m_t_depth_buffer.n_multisample_sample_num = n_depth_multisample_num;
	}
	// set depth buffer

	if(m_b_depth_stencil_buffer_packed = b_depth_stencil_packed) {
		m_b_stencil_buffer = m_b_depth_buffer;
		m_t_stencil_buffer = m_t_depth_buffer; // same as depth buffer then
	} else {
		m_b_stencil_buffer = b_stencil_buffer;
		if((m_t_stencil_buffer.b_texture_target = b_stencil_texture_target)) {
			m_t_stencil_buffer.n_internal_format = 0;
			m_t_stencil_buffer.n_multisample_sample_num = 0; // ignored, if texture target
		} else {
			m_t_stencil_buffer.n_internal_format = n_stencil_internal_format;
			m_t_stencil_buffer.n_multisample_sample_num = n_stencil_multisample_num;
		}
	}
	// set stencil buffer

	memset(m_p_bound_color_texture, 0, max_DrawBuffer_Num * sizeof(TTextureBinding));
	m_n_bound_color_texture_num = 0;
	memset(&m_t_bound_depth_texture, 0, sizeof(TTextureBinding));
	memset(&m_t_bound_stencil_texture, 0, sizeof(TTextureBinding));
	// nothing is bound

	m_n_framebuffer = 0;
	m_n_depth_rb = 0;
	m_n_stencil_rb = 0;
	memset(m_p_color_rb, 0, max_DrawBuffer_Num * sizeof(GLuint));
	// no framebuffers / renderbuffers are created

	m_n_my_max_draw_buffer_num = 0;
	// this is unknown

	m_b_active = false;
	m_b_status = Create();
	// create a new FBO

	if(m_b_active) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_b_active = false;
	}
	// might remain active on error
}

CGLFrameBufferObject::~CGLFrameBufferObject()
{
	//__FuncGuard("CGLFrameBufferObject::~CGLFrameBufferObject");

	if(m_b_status) {
		if(m_n_color_buffer_num) {
			for(int i = 0; i < m_n_color_buffer_num; ++ i) {
				if(!m_p_color_buffer[i].b_texture_target)
					glDeleteRenderbuffers(1, &m_p_color_rb[i]);
			}
		}
		if(m_b_depth_buffer && !m_t_depth_buffer.b_texture_target)
			glDeleteRenderbuffers(1, &m_n_depth_rb);
		if(m_b_stencil_buffer && !m_t_stencil_buffer.b_texture_target && !m_b_depth_stencil_buffer_packed)
			glDeleteRenderbuffers(1, &m_n_stencil_rb);
		glDeleteFramebuffers(1, &m_n_framebuffer);
	}
}

int CGLFrameBufferObject::n_Max_Size()
{
	int n_max_size = 0;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &n_max_size);
	return n_max_size;
}

int CGLFrameBufferObject::n_Max_Sample_Num()
{
	int n_max_samples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &n_max_samples);
	return n_max_samples;
}

int CGLFrameBufferObject::n_Max_IntegerSample_Num()
{
	int n_max_samples = 0;
	glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &n_max_samples);
	return n_max_samples;
}

bool CGLFrameBufferObject::Create()
{
	//__FuncGuard("CGLFrameBufferObject::Create");

	m_b_active = false;

	glGenFramebuffers(1, &m_n_framebuffer);
	if(!m_n_framebuffer)
		return false;
	// create a new framebuffer object

	glBindFramebuffer(GL_FRAMEBUFFER, m_n_framebuffer);
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false;
	}
	m_b_active = true;
	// bind it

	for(int i = 0; i < m_n_color_buffer_num; ++ i) {
		if(m_p_color_buffer[i].b_texture_target)
			continue;
		m_p_color_rb[i] = m_p_color_buffer[i].Create_RenderBuffer(m_n_width, m_n_height, GL_COLOR_ATTACHMENT0 + i);
		if(!m_p_color_rb[i] || glGetError() != GL_NO_ERROR)
			return false;
	}
	// create color buffer(s) (only in case texture is not going to be bound)

	if(m_b_depth_stencil_buffer_packed) {
		_ASSERTE(m_b_depth_buffer == m_b_stencil_buffer);
		_ASSERTE(m_t_depth_buffer.b_texture_target == m_t_stencil_buffer.b_texture_target);
		_ASSERTE(m_t_depth_buffer.n_internal_format == m_t_stencil_buffer.n_internal_format);
		_ASSERTE(m_t_depth_buffer.n_multisample_sample_num == m_t_stencil_buffer.n_multisample_sample_num);
		// format should be set to the same

		if(m_b_depth_buffer && !m_t_depth_buffer.b_texture_target) {
			m_n_depth_rb = m_t_depth_buffer.Create_RenderBuffer(m_n_width, m_n_height, GL_DEPTH_STENCIL_ATTACHMENT);
			if(!m_n_depth_rb || glGetError() != GL_NO_ERROR)
				return false;
		}
	} else {
		if(m_b_depth_buffer && !m_t_depth_buffer.b_texture_target) {
			m_n_depth_rb = m_t_depth_buffer.Create_RenderBuffer(m_n_width, m_n_height, GL_DEPTH_ATTACHMENT);
			if(!m_n_depth_rb || glGetError() != GL_NO_ERROR)
				return false;
		}
		if(m_b_stencil_buffer && !m_t_stencil_buffer.b_texture_target) {
			m_n_stencil_rb = m_t_stencil_buffer.Create_RenderBuffer(m_n_width, m_n_height, GL_STENCIL_ATTACHMENT);
			if(!m_n_stencil_rb || glGetError() != GL_NO_ERROR)
				return false;
		}
	}
	// create depth/stencil buffers (only in case depth/stencil texture is not going to be bound)

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &m_n_my_max_draw_buffer_num);
	int n_tmp;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &n_tmp);
	if(m_n_my_max_draw_buffer_num > n_tmp) // GeForce 6600 has 8 color attachments but only 4 draw buffers
		m_n_my_max_draw_buffer_num = n_tmp;
	_ASSERTE(m_n_my_max_draw_buffer_num <= max_DrawBuffer_Num);
	// get maximal number of draw buffers while bound (as it might depend on FBO format)

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_b_active = false;
	// unbind now

	if(m_n_color_buffer_num > m_n_my_max_draw_buffer_num)
		return false;
	// can't bind as many draw buffers (but maybe can create as many color attachments)

	return true;
}

bool CGLFrameBufferObject::Bind()
{
	//__FuncGuard("CGLFrameBufferObject::Bind");

	if(!m_b_status)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, m_n_framebuffer);
	// bind framebuffer

	if(m_n_color_buffer_num) {
		static const GLenum p_draw_buffers[] = {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
			GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
			GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7,
			GL_COLOR_ATTACHMENT8, GL_COLOR_ATTACHMENT9,
			GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
			GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13,
			GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15
		};
		_ASSERTE(sizeof(p_draw_buffers) / sizeof(p_draw_buffers[0]) >= max_DrawBuffer_Num); // make sure it won't overflow
		glDrawBuffers(m_n_color_buffer_num, p_draw_buffers);
	} else
		glDrawBuffer(GL_NONE);
	// set new draw buffers state

	m_b_active = true;

	return true;
}

/*
 *	virtual bool CGLFrameBufferObject::Release()
 *		- releases the render buffer from OpenGL output buffer binding
 *		- returns true on success or false on failure
 */
bool CGLFrameBufferObject::Release()
{
	//__FuncGuard("CGLFrameBufferObject::Release");

	if(!m_b_active)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// restore draw buffers state

	m_b_active = false;

	return true;
}

bool CGLFrameBufferObject::Bind_ColorTexture1D(int n_attachment_index, GLenum n_target, GLuint n_texture, int n_level)
{
	//__FuncGuard("CGLFrameBufferObject::BindColorTexture_1D");

	_ASSERTE(n_target == GL_TEXTURE_1D);

	if(!m_b_active)
		return false;
	if(n_attachment_index < 0 || n_attachment_index >= m_n_color_buffer_num)
		return false;
	if(!m_p_color_buffer[n_attachment_index].b_texture_target)
		return false;

	glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n_attachment_index, n_target, n_texture, n_level);

	if(!m_p_bound_color_texture[n_attachment_index].n_texture_id && n_texture)
		m_n_bound_color_texture_num ++;
	else if(m_p_bound_color_texture[n_attachment_index].n_texture_id && !n_texture)
		m_n_bound_color_texture_num --;
	// maintain number of bound textures

	m_p_bound_color_texture[n_attachment_index].n_texture_id = n_texture;
	m_p_bound_color_texture[n_attachment_index].n_target = n_target;

	return true;
}

bool CGLFrameBufferObject::Bind_ColorTexture2D(int n_attachment_index, GLenum n_target, GLuint n_texture, int n_level)
{
	//__FuncGuard("CGLFrameBufferObject::BindColorTexture_2D");

	_ASSERTE(n_target == GL_TEXTURE_2D || n_target == GL_TEXTURE_RECTANGLE ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_X ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z);

	if(!m_b_active)
		return false;
	if(n_attachment_index < 0 || n_attachment_index >= m_n_color_buffer_num)
		return false;
	if(!m_p_color_buffer[n_attachment_index].b_texture_target)
		return false;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n_attachment_index, n_target, n_texture, n_level);

	if(!m_p_bound_color_texture[n_attachment_index].n_texture_id && n_texture)
		m_n_bound_color_texture_num ++;
	else if(m_p_bound_color_texture[n_attachment_index].n_texture_id && !n_texture)
		m_n_bound_color_texture_num --;
	// maintain number of bound textures

	m_p_bound_color_texture[n_attachment_index].n_texture_id = n_texture;
	m_p_bound_color_texture[n_attachment_index].n_target = n_target;

	return true;
}

bool CGLFrameBufferObject::Bind_ColorTexture3D(int n_attachment_index, GLenum n_target, GLuint n_texture, int n_level, int n_layer)
{
	//__FuncGuard("CGLFrameBufferObject::BindColorTexture_3D");

	_ASSERTE(n_target == GL_TEXTURE_3D);

	if(!m_b_active)
		return false;
	if(n_attachment_index < 0 || n_attachment_index >= m_n_color_buffer_num)
		return false;
	if(!m_p_color_buffer[n_attachment_index].b_texture_target)
		return false;

	glFramebufferTexture3D(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + n_attachment_index, n_target, n_texture, n_level, n_layer);

	if(!m_p_bound_color_texture[n_attachment_index].n_texture_id && n_texture)
		m_n_bound_color_texture_num ++;
	else if(m_p_bound_color_texture[n_attachment_index].n_texture_id && !n_texture)
		m_n_bound_color_texture_num --;
	// maintain number of bound textures

	m_p_bound_color_texture[n_attachment_index].n_texture_id = n_texture;
	m_p_bound_color_texture[n_attachment_index].n_target = n_target;
	//m_p_bound_color_texture[n_index].n_3d_tex_slice = n_z_offset; // unnecessary

	return true;
}

bool CGLFrameBufferObject::Bind_DepthTexture2D(GLenum n_target, GLuint n_texture, int n_level)
{
	//__FuncGuard("CGLFrameBufferObject::Bind_DepthTexture2D");

	_ASSERTE(n_target == GL_TEXTURE_2D || n_target == GL_TEXTURE_RECTANGLE ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_X ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z);

	if(m_b_depth_stencil_buffer_packed)
		return false;

	if(!m_b_active || !m_b_depth_buffer || !m_t_depth_buffer.b_texture_target)
		return false;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, n_target, n_texture, n_level);

	m_t_bound_depth_texture.n_texture_id = n_texture;
	m_t_bound_depth_texture.n_target = n_target;

	return true;
}

bool CGLFrameBufferObject::Bind_StencilTexture2D(GLenum n_target, GLuint n_texture, int n_level)
{
	//__FuncGuard("CGLFrameBufferObject::Bind_StencilTexture2D");

	_ASSERTE(n_target == GL_TEXTURE_2D || n_target == GL_TEXTURE_RECTANGLE ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_X ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z);

	if(m_b_depth_stencil_buffer_packed)
		return false;

	if(!m_b_active || !m_b_stencil_buffer || !m_t_stencil_buffer.b_texture_target)
		return false;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, n_target, n_texture, n_level);

	m_t_bound_stencil_texture.n_texture_id = n_texture;
	m_t_bound_stencil_texture.n_target = n_target;

	return true;
}

bool CGLFrameBufferObject::Bind_DepthStencilTexture2D(GLenum n_target, GLuint n_texture, int n_level)
{
	//__FuncGuard("CGLFrameBufferObject::Bind_DepthStencilTexture2D");

	_ASSERTE(n_target == GL_TEXTURE_2D || n_target == GL_TEXTURE_RECTANGLE ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_X ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y ||
		n_target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ||
		n_target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z);

	if(!m_b_depth_stencil_buffer_packed)
		return false;

	_ASSERTE(m_b_depth_buffer == m_b_stencil_buffer);
	_ASSERTE(m_t_depth_buffer.b_texture_target == m_t_stencil_buffer.b_texture_target);
	_ASSERTE(m_t_depth_buffer.n_internal_format == m_t_stencil_buffer.n_internal_format);
	_ASSERTE(m_t_depth_buffer.n_multisample_sample_num == m_t_stencil_buffer.n_multisample_sample_num);
	// format should be set to the same

	if(!m_b_active || !m_b_depth_buffer || !m_t_depth_buffer.b_texture_target)
		return false;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, n_target, n_texture, n_level);

	m_t_bound_depth_texture.n_texture_id = n_texture;
	m_t_bound_depth_texture.n_target = n_target;

	return true;
}

bool CGLFrameBufferObject::ReleaseAllTextures()
{
	//__FuncGuard("CGLFrameBufferObject::ReleaseAllTextures");

	if(!m_b_active)
		return false;

	for(int i = 0; i < m_n_color_buffer_num && m_n_bound_color_texture_num > 0; ++ i) {
		if(m_p_bound_color_texture[i].n_texture_id) {
			if(m_p_bound_color_texture[i].n_target == GL_TEXTURE_3D)
				glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_3D, 0, 0, 0);
			else if(m_p_bound_color_texture[i].n_target == GL_TEXTURE_1D)
				glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_1D, 0, 0);
			else // 2D / RECT / CUBE_MAP_(POSI|NEGA)TIVE_[XYZ]
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, m_p_bound_color_texture[i].n_target, 0, 0);
			m_p_bound_color_texture[i].n_texture_id = 0;
			m_n_bound_color_texture_num --;
		}
	}
	_ASSERTE(!m_n_bound_color_texture_num);
	// release color textures

	if(m_b_depth_stencil_buffer_packed) {
		_ASSERTE(m_b_depth_buffer == m_b_stencil_buffer);
		_ASSERTE(m_t_depth_buffer.b_texture_target == m_t_stencil_buffer.b_texture_target);
		_ASSERTE(m_t_depth_buffer.n_internal_format == m_t_stencil_buffer.n_internal_format);
		_ASSERTE(m_t_depth_buffer.n_multisample_sample_num == m_t_stencil_buffer.n_multisample_sample_num);
		// format should be set to the same

		_ASSERTE(!m_t_bound_stencil_texture.n_texture_id);
		// stencil texture should be unused

		if(m_b_depth_buffer && m_t_depth_buffer.b_texture_target && m_t_bound_depth_texture.n_texture_id) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
				m_t_bound_depth_texture.n_target, 0, 0);
			m_t_bound_depth_texture.n_texture_id = 0;
		}
	} else {
		if(m_b_depth_buffer && m_t_depth_buffer.b_texture_target && m_t_bound_depth_texture.n_texture_id) {
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT, m_t_bound_depth_texture.n_target, 0, 0);
			m_t_bound_depth_texture.n_texture_id = 0;
		}
		if(m_b_stencil_buffer && m_t_stencil_buffer.b_texture_target && m_t_bound_stencil_texture.n_texture_id) {
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT, m_t_bound_stencil_texture.n_target, 0, 0);
			m_t_bound_stencil_texture.n_texture_id = 0;
		}
	}
	// release depth/stencil textures

	return true;
}

bool CGLFrameBufferObject::b_Status() const
{
	//__FuncGuard("CGLFrameBufferObject::b_Status");

	if(!m_b_active)
		return m_b_status;
	int n_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	return n_status == GL_FRAMEBUFFER_COMPLETE;
}

GLenum CGLFrameBufferObject::n_Status() const
{
	//__FuncGuard("CGLFrameBufferObject::n_Status");

	if(!m_b_active)
		return 0;
	return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

/*
 *								=== ~CGLFrameBufferObject ===
 */
