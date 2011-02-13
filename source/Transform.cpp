/*
								+--------------------------------+
								|                                |
								| *** OpenGL transform utils *** |
								|                                |
								|  Copyright © -tHE SWINe- 2008  |
								|                                |
								|         Transform.cpp          |
								|                                |
								+--------------------------------+
*/

/**
 *	@file Transform.cpp
 *	@author -tHE SWINe-
 *	@brief Implementation of fixed-function OpenGL transformation matrix math
 */

#include <math.h>
#include "Transform.h"

/*
 *								=== CGLTransform ===
 */

void CGLTransform::Frustum(Matrix4f &r_t_matrix, float f_left, float f_right,
	float f_bottom, float f_top, float f_near, float f_far)
{
	r_t_matrix[0][0] = 2 * f_near / (f_right - f_left);
	r_t_matrix[1][0] = 0;
	r_t_matrix[2][0] = (f_right + f_left) / (f_right - f_left);
	r_t_matrix[3][0] = 0;
	r_t_matrix[0][1] = 0;
	r_t_matrix[1][1] = 2 * f_near / (f_top - f_bottom);
	r_t_matrix[2][1] = (f_top + f_bottom) / (f_top - f_bottom);
	r_t_matrix[3][1] = 0;
	r_t_matrix[0][2] = 0;
	r_t_matrix[1][2] = 0;
	r_t_matrix[2][2] = -(f_far + f_near) / (f_far - f_near);
	r_t_matrix[3][2] = -2 * f_far * f_near / (f_far - f_near);
	r_t_matrix[0][3] = 0;
	r_t_matrix[1][3] = 0;
	r_t_matrix[2][3] = -1;
	r_t_matrix[3][3] = 0;
	// assuming column-major order here (OpenGL compatible)
}

void CGLTransform::Ortho(Matrix4f &r_t_matrix, float f_left, float f_right,
	float f_bottom, float f_top, float f_near, float f_far)
{
	r_t_matrix[0][0] = 2 / (f_right - f_left);
	r_t_matrix[1][0] = 0;
	r_t_matrix[2][0] = 0;
	r_t_matrix[3][0] = -(f_right + f_left) / (f_right - f_left);
	r_t_matrix[0][1] = 0;
	r_t_matrix[1][1] = 2 / (f_top - f_bottom);
	r_t_matrix[2][1] = 0;
	r_t_matrix[3][1] = -(f_top + f_bottom) / (f_top - f_bottom);
	r_t_matrix[0][2] = 0;
	r_t_matrix[1][2] = 0;
	r_t_matrix[2][2] = -2 / (f_far - f_near);
	r_t_matrix[3][2] = -(f_far + f_near) / (f_far - f_near);
	r_t_matrix[0][3] = 0;
	r_t_matrix[1][3] = 0;
	r_t_matrix[2][3] = 0;
	r_t_matrix[3][3] = 1;
	// assuming column-major order here (OpenGL compatible)
}

void CGLTransform::Perspective(Matrix4f &r_t_matrix, float f_fov, float f_aspect, float f_near, float f_far)
{
	float f_w, f_h;

	f_h = float(tan(f_fov * f_pi / 180 * .5f)) * f_near;
	f_w = f_h * f_aspect;
	// calc half width of frustum in x-y plane at z = f_near

	Frustum(r_t_matrix, -f_w, f_w, -f_h, f_h, f_near, f_far);
	// calculate frustum
}

void CGLTransform::PerspectiveTile(Matrix4f &r_t_matrix, int n_total_width, int n_total_height,
	int n_tile_x, int n_tile_y, int n_tile_width, int n_tile_height,
	float f_fov, float f_aspect, float f_near, float f_far)
{
	float f_half_w, f_half_h;
	f_half_h = float(tan(f_fov * f_pi / 180 * .5f)) * f_near;
	f_half_w = f_half_h * f_aspect;
	// calc half width of frustum in x-y plane at z = f_near

	float f_left = -f_half_w;
	float f_right = f_half_w;
	float f_width = f_half_w + f_half_w;
	float f_bottom = -f_half_h;
	float f_top = f_half_h;
	float f_height = f_half_h + f_half_h;
	// regular frustum

	float f_tile_left = f_left + f_width * n_tile_x / n_total_width;
	float f_tile_right = f_tile_left + f_width * n_tile_width / n_total_width;
	float f_tile_bottom = f_bottom + f_height * n_tile_y / n_total_height;
	float f_tile_top = f_tile_bottom + f_height * n_tile_height / n_total_height;
	// calculate tile frustum

	Frustum(r_t_matrix, f_tile_left, f_tile_right, f_tile_bottom, f_tile_top, f_near, f_far);
	// set frustum
}

void CGLTransform::OrthoTile(Matrix4f &r_t_matrix, int n_total_width, int n_total_height,
	int n_tile_x, int n_tile_y, int n_tile_width, int n_tile_height,
	float f_left, float f_right, float f_bottom, float f_top, float f_near, float f_far)
{
	float f_width = f_right - f_left;
	float f_height = f_top - f_bottom;
	// regular frustum given by caller

	float f_tile_left = f_left + f_width * n_tile_x / n_total_width;
	float f_tile_right = f_tile_left + f_width * n_tile_width / n_total_width;
	float f_tile_bottom = f_bottom + f_height * n_tile_y / n_total_height;
	float f_tile_top = f_tile_bottom + f_height * n_tile_height / n_total_height;
	// calculate tile frustum

	Ortho(r_t_matrix, f_tile_left, f_tile_right, f_tile_bottom, f_tile_top, f_near, f_far);
	// set frustum
}

void CGLTransform::LookAt(Matrix4f &r_t_matrix, Vector3f v_eye, Vector3f v_target, Vector3f v_up)
{
	Vector3f v_dir(v_target - v_eye);
	v_dir.Normalize();
	Vector3f v_right(v_dir.v_Cross(v_up));
	v_right.Normalize();
	v_up = v_right.v_Cross(v_dir);
	// calculate complete perpendicular coord frame

	for(int i = 0; i < 3; ++ i)
		r_t_matrix[i][0] = v_right[i];
	for(int i = 0; i < 3; ++ i)
		r_t_matrix[i][1] = v_up[i];
	for(int i = 0; i < 3; ++ i)
		r_t_matrix[i][2] = -v_dir[i];
	for(int i = 0; i < 3; ++ i) {
		r_t_matrix[i][3] = 0;
		r_t_matrix[3][i] = 0;
	}
	r_t_matrix[3][3] = 1;
	// copy it to matrix

	/** @todo - this might be column-major, check it! */

	r_t_matrix.Translate(-v_eye.x, -v_eye.y, -v_eye.z);
	// apply translation

	/** @todo - does this work correctly? doesn't it skew matrices or stuff?
		could it be incorporated to the matrix itself? */
}

void CGLTransform::Calc_ViewRay(const Matrix4f &r_modelview_projection_inverse_transpose,
	Vector2f v_screen_point, Vector3f &r_v_org, Vector3f &r_v_dir)
{
	Vector4f v_far4 = r_modelview_projection_inverse_transpose *
		Vector4f(v_screen_point.x, v_screen_point.y, 1, 1);
	Vector4f v_near4 = r_modelview_projection_inverse_transpose *
		Vector4f(v_screen_point.x, v_screen_point.y, -1, 1);
	// z needs to be normalized to [-1, 1] range, just as x and y are

	Vector3f v_far(v_far4.x, v_far4.y, v_far4.z); v_far /= v_far4.w;
	Vector3f v_near(v_near4.x, v_near4.y, v_near4.z); v_near /= v_near4.w;
	// dehomogenize

	r_v_org = v_near;
	r_v_dir = v_far - v_near;
	// output
}

void CGLTransform::Mirror(Matrix4f &r_t_new_proj, const Plane3f &r_t_plane)
{
	const float nx = r_t_plane.v_normal.x, ny = r_t_plane.v_normal.y,
		nz = r_t_plane.v_normal.z, d = r_t_plane.f_dist;

	/*float p_mirror_matrix[4][4] = {
		{1-2 * nx * nx	, -2 * ny * nx	, -2 * nz * nx	, 0},
		{-2 * nx * ny	, 1-2 * ny * ny	, -2 * nz * ny	, 0},
		{-2 * nx * nz	, -2 * ny * nz	, 1-2 * nz * nz	, 0},
		{-2 * nx * d	, -2 * ny * d	, -2 * nz * d	, 1}
	};*/
	// calculate mirroring matrix

	r_t_new_proj[0][0] = 1 - 2 * nx * nx;
	r_t_new_proj[0][1] = -2 * nx * ny;
	r_t_new_proj[0][2] = -2 * nx * nz;
	r_t_new_proj[0][3] = -2 * nx * d;
	r_t_new_proj[1][0] = -2 * ny * nx;
	r_t_new_proj[1][1] = 1 - 2 * ny * ny;
	r_t_new_proj[1][2] = -2 * ny * nz;
	r_t_new_proj[1][3] = -2 * ny * d;
	r_t_new_proj[2][0] = -2 * nz * nx;
	r_t_new_proj[2][1] = -2 * nz * ny;
	r_t_new_proj[2][2] = 1 - 2 * nz * nz;
	r_t_new_proj[2][3] = -2 * nz * d;
	r_t_new_proj[3][0] = 0;
	r_t_new_proj[3][1] = 0;
	r_t_new_proj[3][2] = 0;
	r_t_new_proj[3][3] = 1;
	// calculate mirroring matrix

	/** @todo - this might be column-major, check it! */
}

Vector3f CGLTransform::v_UnTransform(const Matrix4f &r_modelview_projection_inverse_transpose,
	Vector2f v_screen_point, float f_z)
{
	Vector4f v_pt4 = r_modelview_projection_inverse_transpose *
		Vector4f(v_screen_point.x, v_screen_point.y, f_z, 1);
	return Vector3f(v_pt4.x, v_pt4.y, v_pt4.z) / v_pt4.w;
}

Vector3f CGLTransform::v_Transform(const Matrix4f &r_modelview_projection,
	Vector3f v_world_point)
{
	Vector4f v_pt4 = r_modelview_projection *
		Vector4f(v_world_point.x, v_world_point.y, v_world_point.z, 1);
	return Vector3f(v_pt4.x, v_pt4.y, v_pt4.z) / v_pt4.w;
}

/*
 *								=== ~CGLTransform ===
 */

/*
 *								=== CGL3TransformState ===
 */

CGL3TransformState::CGL3TransformState()
{
	for(int i = 0; i < 2; ++ i)
		m_p_matrix[i].Identity();
}

void CGL3TransformState::LoadIdentity(int n_name)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	m_p_matrix[n_name].Identity();
	// load identity matrix

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::LoadMatrix(int n_name, const float *p_matrix)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	memcpy(m_p_matrix[n_name].p_Data(), p_matrix, 16 * sizeof(float));
	// copy matrix coeffs

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::MultMatrix(int n_name, const float *p_matrix)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	Matrix4f t_tmp;
	memcpy(t_tmp.p_Data(), p_matrix, 16 * sizeof(float));
	// copy to matrix struct

	// @todo - multiply directly from float data

	m_p_matrix[n_name] *= t_tmp;
	// multiply it

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::LoadTransposeMatrix(int n_name, const float *p_matrix)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	memcpy(m_p_matrix[n_name].p_Data(), p_matrix, 16 * sizeof(float));
	m_p_matrix[n_name].Transpose();
	// copy matrix coeffs

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::MultTransposeMatrix(int n_name, const float *p_matrix)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	Matrix4f t_tmp;
	memcpy(t_tmp.p_Data(), p_matrix, 16 * sizeof(float));
	t_tmp.Transpose();
	// copy to matrix struct

	// @todo - multiply directly from float data

	m_p_matrix[n_name] *= t_tmp;
	// multiply it

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::LoadMatrix(int n_name, const Matrix4f &r_t_matrix)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	m_p_matrix[n_name] = r_t_matrix;
	// copy matrix coeffs

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::MultMatrix(int n_name, const Matrix4f &r_t_matrix)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	m_p_matrix[n_name] *= r_t_matrix;
	// multiply matrices

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::Rotate(int n_name, float f_angle_degrees, float x, float y, float z)
{
	float f = sqrt(x * x + y * y + z * z);
	if(f > 0) {
		f = 1 / f;
		x *= f;
		y *= f;
		z *= f;
	}
	// normalize rotation axis!

	RotateNormalized(n_name, f_angle_degrees, x, y, z);
	// mark derived matrices as dirty
}

void CGL3TransformState::RotateNormalized(int n_name, float f_angle_degrees, float x, float y, float z)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	m_p_matrix[n_name].Rotate(f_angle_degrees / 180 * f_pi, x, y, z);
	// apply rotation

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::Translate(int n_name, float x, float y, float z)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	m_p_matrix[n_name].Translate(x, y, z);
	// apply rotation

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::Scale(int n_name, float s)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	m_p_matrix[n_name].Scale(s);
	// apply rotation

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::Scale(int n_name, float x, float y, float z)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	m_p_matrix[n_name].Scale(x, y, z);
	// apply rotation

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::Frustum(int n_name, float f_left, float f_right,
	float f_bottom, float f_top, float f_near, float f_far)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	Matrix4f t_frustum;
	CGLTransform::Frustum(t_frustum, f_left, f_right, f_bottom, f_top, f_near, f_far);
	// create frustum matrix

	m_p_matrix[n_name] *= t_frustum;
	// multiply

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

void CGL3TransformState::Ortho(int n_name, float f_left, float f_right,
	float f_bottom, float f_top, float f_near, float f_far)
{
	
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);

	Matrix4f t_ortho;
	CGLTransform::Ortho(t_ortho, f_left, f_right, f_bottom, f_top, f_near, f_far);
	// create orthographic projection matrix

	m_p_matrix[n_name] *= t_ortho;
	// multiply

	MarkDerivedDirty(n_name);
	// mark derived matrices as dirty
}

const float *CGL3TransformState::p_GetMatrix(int n_name) const
{
	return t_GetMatrix(n_name).p_Data(); // don't repeat same code twice
}

const float *CGL3TransformState::p_GetMatrix3x3(int n_name) const
{
	static float p_coeffs[3 * 3];

	const Matrix4f &r_t_matrix = t_GetMatrix(n_name); // don't repeat same code twice
	for(int y = 0; y < 3; ++ y) {
		for(int x = 0; x < 3; ++ x)
			p_coeffs[3 * x + y] = r_t_matrix[x][y];
	}
	// copy 3x3 part

	return p_coeffs;
}

const Matrix4f &CGL3TransformState::t_GetMatrix(int n_name) const
{
	_ASSERTE(n_name >= mat_ModelView && n_name < mat_Matrix_Num);

	if(n_name < 2)
		return m_p_matrix[n_name]; // modelview / projection
	else {
		// another matrix, derived from modelview / projection

		UpdateDerivedMatrix(n_name);
		// check if matrix is dirty

		return m_p_matrix[n_name];
		// return matrix value
	}
}

void CGL3TransformState::UpdateDerivedMatrix(int n_name) const
{
	_ASSERTE(n_name > mat_Projection && n_name < mat_Matrix_Num);

	if(m_b_matrix_dirty[n_name - 2]) {
		m_b_matrix_dirty[n_name - 2] = false;

		switch(n_name) {
		case mat_ModelView_Projection:
			if(!m_b_matrix_dirty[mat_ModelView_Projection_Transpose - 2])
				m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_ModelView_Projection_Transpose]); // reuse MvPT
			else
				m_p_matrix[n_name].ProductOf(m_p_matrix[mat_Projection], m_p_matrix[mat_ModelView]); // calculate
			break;
		case mat_ModelView_Projection_Inverse:
			if(!m_b_matrix_dirty[mat_ModelView_Projection_Inverse_Transpose - 2])
				m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_ModelView_Projection_Inverse_Transpose]); // reuse MvPIT?
			else if(!m_b_matrix_dirty[mat_ModelView_Projection - 2])
				m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_ModelView_Projection].t_FullInverse()); // reuse MvP
			else {
				m_p_matrix[n_name].ProductOf(m_p_matrix[mat_Projection], m_p_matrix[mat_ModelView]);
				m_p_matrix[n_name].FullInvertNoTranspose(); // calculate
			}
			break;
		case mat_ModelView_Projection_Transpose:
			if(!m_b_matrix_dirty[mat_ModelView_Projection - 2])
				m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_ModelView_Projection]); // reuse MvP
			else {
				m_p_matrix[n_name].ProductOf(m_p_matrix[mat_Projection], m_p_matrix[mat_ModelView]);
				m_p_matrix[n_name].Transpose(); // calculate
			}
			break;
		case mat_ModelView_Projection_Inverse_Transpose:
			if(!m_b_matrix_dirty[mat_ModelView_Projection - 2])
				m_p_matrix[n_name].FullInverseOf(m_p_matrix[mat_ModelView_Projection]); // reuse MvP
			else {
				m_p_matrix[n_name].ProductOf(m_p_matrix[mat_Projection], m_p_matrix[mat_ModelView]);
				m_p_matrix[n_name].FullInvert(); // calculate
			}
			break;

		case mat_ModelView_Inverse:
			if(!m_b_matrix_dirty[mat_ModelView_Inverse_Transpose - 2])
				m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_ModelView_Inverse_Transpose]); // reuse MvIT
			else
				m_p_matrix[n_name].FullInverseNoTransposeOf(m_p_matrix[mat_ModelView]); // calculate
			break;
		case mat_ModelView_Transpose:
			m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_ModelView]); // always calculate
			break;
		case mat_ModelView_Inverse_Transpose:
			if(!m_b_matrix_dirty[mat_ModelView_Inverse - 2])
				m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_ModelView_Inverse]); // reuse MvI
			else
				m_p_matrix[n_name].FullInverseOf(m_p_matrix[mat_ModelView]); // calculate
			break;

		case mat_Projection_Inverse:
			if(!m_b_matrix_dirty[mat_Projection_Inverse_Transpose - 2])
				m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_Projection_Inverse_Transpose]); // reuse PIT
			else
				m_p_matrix[n_name].FullInverseNoTransposeOf(m_p_matrix[mat_Projection]); // calculate
			break;
		case mat_Projection_Transpose:
			m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_Projection]); // always calculate
			break;
		case mat_Projection_Inverse_Transpose:
			if(!m_b_matrix_dirty[mat_Projection_Inverse - 2])
				m_p_matrix[n_name].TransposeOf(m_p_matrix[mat_Projection_Inverse]); // reuse PI
			else
				m_p_matrix[n_name].FullInverseOf(m_p_matrix[mat_Projection]); // calculate
			break;
		};
	}
	// matrix is dirty, need to recalculate it
}

void CGL3TransformState::MarkDerivedDirty(int n_name)
{
	_ASSERTE(n_name == mat_ModelView || n_name == mat_Projection);
	if(n_name == mat_ModelView) {
		m_b_matrix_dirty[mat_ModelView_Projection] = true;
		m_b_matrix_dirty[mat_ModelView_Projection_Inverse] = true;
		m_b_matrix_dirty[mat_ModelView_Projection_Transpose] = true;
		m_b_matrix_dirty[mat_ModelView_Projection_Inverse_Transpose] = true;
		m_b_matrix_dirty[mat_ModelView_Inverse] = true;
		m_b_matrix_dirty[mat_ModelView_Transpose] = true;
		m_b_matrix_dirty[mat_ModelView_Inverse_Transpose] = true;
	} else /*if(n_name == mat_Projection)*/ {
		m_b_matrix_dirty[mat_ModelView_Projection] = true;
		m_b_matrix_dirty[mat_ModelView_Projection_Inverse] = true;
		m_b_matrix_dirty[mat_ModelView_Projection_Transpose] = true;
		m_b_matrix_dirty[mat_ModelView_Projection_Inverse_Transpose] = true;
		m_b_matrix_dirty[mat_Projection_Inverse] = true;
		m_b_matrix_dirty[mat_Projection_Transpose] = true;
		m_b_matrix_dirty[mat_Projection_Inverse_Transpose] = true;
	}
}

/*
 *								=== ~CGL3TransformState ===
 */
