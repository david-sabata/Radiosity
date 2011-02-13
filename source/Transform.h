/*
								+--------------------------------+
								|                                |
								| *** OpenGL transform utils *** |
								|                                |
								|  Copyright © -tHE SWINe- 2008  |
								|                                |
								|          Transform.h           |
								|                                |
								+--------------------------------+
*/

/**
 *	@file Transform.h
 *	@author -tHE SWINe-
 *	@brief Implementation of fixed-function OpenGL transformation matrix math
 *
 *	@date 2008-08-08
 *
 *	added \#ifdef for windows 64, added \#define for GL_GLEXT_LEGACY (for linux builds)
 *
 *	@date 2009-05-03
 *
 *	added CGLTransform::PerspectiveTile() and CGLTransform::OrthoTile() functions
 *	(tiled rendering support)
 *
 *	@date 2009-12-18
 *
 *	Rewritten to be usable under OpenGL 3.0 (glLoadMatrix(), glMultMatrix(), glOrtho()
 *	and glFrustum() are all deprecated, so output must be to Matrix4f, which is later
 *	used as parameter of vertex shader).
 *
 *	@date 2010-09-22
 *
 *	removed oblique frustum culling functions
 *
 */

#ifndef __OPENGL_TRANSFORM_UTILITIES_INCLUDED
#define __OPENGL_TRANSFORM_UTILITIES_INCLUDED

#include "Vector.h"

/**
 *	@brief OpenGL transformation state
 *
 *	Older OpenGL versions implemented fixed-function transformation state, shaders
 *	could access to transform vertex coordinates, etc. This state is now deprecated,
 *	meaning applications need to implement transformation state themselves. This is
 *	very simple transformation state which, compared to fixed-function state doesn't
 *	contain texture matrices.
 */
class CGL3TransformState {
public:
	/**
	 *	@brief matrix names
	 */
	enum {
		mat_ModelView = 0, /**< modelview matrix */
		mat_Projection, /**< projection matrix */

		mat_ModelView_Projection, /**< modelview-projection matrix */
		mat_ModelView_Projection_Inverse, /**< modelview-projection matrix inverse */
		mat_ModelView_Projection_Transpose, /**< modelview-projection matrix transpose */
		mat_ModelView_Projection_Inverse_Transpose, /**< modelview-projection matrix inverse transpose */

		mat_ModelView_Inverse, /**< inverse of modelview matrix */
		mat_ModelView_Transpose, /**< transpose of modelview matrix */
		mat_ModelView_Inverse_Transpose, /**< inverse transpose of modelview matrix */

		mat_Normal = mat_ModelView_Inverse_Transpose, /**< normal matrix (inverse transpose of modelview matrix) */

		mat_Projection_Inverse, /**< inverse of projection matrix */
		mat_Projection_Transpose, /**< transpose of projection matrix */
		mat_Projection_Inverse_Transpose, /**< inverse transpose of projection matrix */

		mat_Matrix_Num /**< number of matrices (must be last), has no semantical value */
	};

private:
	mutable Matrix4f m_p_matrix[mat_Matrix_Num];
	mutable bool m_b_matrix_dirty[mat_Matrix_Num - 2];

public:
	/**
	 *	@brief default constructor
	 *
	 *	Initializes all matrices to identity.
	 */
	CGL3TransformState();

	/**
	 *	@brief loads identity matrix
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 */
	void LoadIdentity(int n_name);

	/**
	 *	@brief loads matrix from float array
	 *
	 *	@todo Is the array column-major, or row major?
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] p_matrix is array of 16 matrix coefficients
	 */
	void LoadMatrix(int n_name, const float *p_matrix);

	/**
	 *	@brief multiplies target matrix by matrix from float array
	 *
	 *	@todo Is the array column-major, or row major?
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] p_matrix is array of 16 matrix coefficients
	 */
	void MultMatrix(int n_name, const float *p_matrix);

	/**
	 *	@brief loads transpose of matrix from float array
	 *
	 *	@todo Is the array column-major, or row major?
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] p_matrix is array of 16 matrix coefficients
	 */
	void LoadTransposeMatrix(int n_name, const float *p_matrix);

	/**
	 *	@brief multiplies target matrix by transpose of matrix from float array
	 *
	 *	@todo Is the array column-major, or row major?
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] p_matrix is array of 16 matrix coefficients
	 */
	void MultTransposeMatrix(int n_name, const float *p_matrix);

	/**
	 *	@brief loads matrix
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] r_t_matrix is value to be set
	 */
	void LoadMatrix(int n_name, const Matrix4f &r_t_matrix);

	/**
	 *	@brief multiplies target matrix by matrix
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] r_t_matrix is value to be multiplied by
	 */
	void MultMatrix(int n_name, const Matrix4f &r_t_matrix);

	/**
	 *	@brief applies rotation transformation
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] f_angle_degrees is rotation angle, in degrees
	 *	@param[in] x is x-component of rotation axis
	 *	@param[in] y is y-component of rotation axis
	 *	@param[in] z is z-component of rotation axis
	 *
	 *	@note Rotation axis doesn't have to be normalized (use
	 *		RotateNormalized() for better performance in case it is).
	 */
	void Rotate(int n_name, float f_angle_degrees, float x, float y, float z);

	/**
	 *	@brief applies rotation transformation
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] f_angle_degrees is rotation angle, in degrees
	 *	@param[in] x is x-component of normalized rotation axis
	 *	@param[in] y is y-component of normalized rotation axis
	 *	@param[in] z is z-component of normalized rotation axis
	 *
	 *	@note Rotation axis has to be normalized (use Rotate() in case it isn't).
	 */
	void RotateNormalized(int n_name, float f_angle_degrees, float x, float y, float z);

	/**
	 *	@brief applies translation transformation
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] x is offset along x-axis
	 *	@param[in] y is offset along y-axis
	 *	@param[in] z is offset along z-axis
	 */
	void Translate(int n_name, float x, float y, float z);

	/**
	 *	@brief applies uniform scaling transformation
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] s is uniform scaling coefficient
	 */
	void Scale(int n_name, float s);

	/**
	 *	@brief applies scaling transformation
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] x is coefficient of scaling along x-axis
	 *	@param[in] y is coefficient of scaling along y-axis
	 *	@param[in] z is coefficient of scaling along z-axis
	 */
	void Scale(int n_name, float x, float y, float z);

	/**
	 *	@brief applies frustum matrix
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] f_left is position of left clip-plane
	 *	@param[in] f_right is position of right clip-plane
	 *	@param[in] f_bottom is position of bottom clip-plane
	 *	@param[in] f_top is position of top clip-plane
	 *	@param[in] f_near is depth of near clip plane
	 *	@param[in] f_far is depth of far clip plane
	 */
	void Frustum(int n_name, float f_left, float f_right,
		float f_bottom, float f_top, float f_near, float f_far);

	/**
	 *	@brief applies orthographic projection matrix
	 *
	 *	@param[in] n_name is target matrix name (mat_ModelView or mat_Projection)
	 *	@param[in] f_left is position of left clip-plane
	 *	@param[in] f_right is position of right clip-plane
	 *	@param[in] f_bottom is position of bottom clip-plane
	 *	@param[in] f_top is position of top clip-plane
	 *	@param[in] f_near is depth of near clip plane
	 *	@param[in] f_far is depth of far clip plane
	 */
	void Ortho(int n_name, float f_left, float f_right,
		float f_bottom, float f_top, float f_near, float f_far);

	/**
	 *	@brief gets selected matrix as column-major float array
	 *
	 *	@param[in] n_name is target matrix name (mat_*)
	 *
	 *	@return Returns pointer to 16 float values (column-major), containing matrix coefficients.
	 */
	const float *p_GetMatrix(int n_name) const;

	/**
	 *	@brief gets selected matrix
	 *
	 *	@param[in] n_name is target matrix name (mat_*)
	 *
	 *	@return Returns const reference to selected matrix.
	 */
	const Matrix4f &t_GetMatrix(int n_name) const;

	/**
	 *	@brief gets upper-left 3x3 coefficents of selected matrix as column-major float array
	 *
	 *	This is particularily useful in combination with mat_Normal,
	 *		which is, in fact, 3 by 3 matrix.
	 *
	 *	@param[in] n_name is target matrix name (mat_*)
	 *
	 *	@return Returns pointer to 9 float values (column-major), containing matrix coefficients.
	 *
	 *	@note Returned pointer points to a static variable, which gets overwritten
	 *		by next call to this function. Use immediately, or copy away.
	 *	@note This actually involves copying coefficients from selected matrix to temp variable.
	 */
	const float *p_GetMatrix3x3(int n_name) const;

private:
	void UpdateDerivedMatrix(int n_name) const;
	void MarkDerivedDirty(int n_name);
};

/**
 *	@brief implements misc transformation-related utilities for OpenGL
 */
class CGLTransform {
public:
	/**
	 *	@brief calculates frustum matrix
	 *
	 *	To achieve the same effect as deprecated fixed-functionality glFrustum,
	 *	one needs to call:
	 *<code>	Matrix4f t_modelview, t_projection; // OpenGL transformation state
	 *
	 *	Matrix4f t_frustum;
	 *	Frustum(t_frustum, -1, 1, -1, 1, -1, 1); // calculate "some" frustum
	 *
	 *	t_projection *= t_frustum; // multiply projection</code>
	 *
	 *	@param[out] r_t_matrix is ouput matrix (overwritten with result, not multiplied by result)
	 *	@param[in] f_left is position of left clip-plane
	 *	@param[in] f_right is position of right clip-plane
	 *	@param[in] f_bottom is position of bottom clip-plane
	 *	@param[in] f_top is position of top clip-plane
	 *	@param[in] f_near is depth of near clip plane
	 *	@param[in] f_far is depth of far clip plane
	 */
	static void Frustum(Matrix4f &r_t_matrix, float f_left, float f_right,
		float f_bottom, float f_top, float f_near, float f_far);

	/**
	 *	@brief calculates ortographic projection matrix
	 *
	 *	To achieve the same effect as deprecated fixed-functionality glOrtho,
	 *	one needs to call:
	 *<code>	Matrix4f t_modelview, t_projection; // OpenGL transformation state
	 *
	 *	Matrix4f t_ortho;
	 *	Frustum(t_ortho, -1, 1, -1, 1, -1, 1); // calculate "some" ortographic projection
	 *
	 *	t_projection *= t_ortho; // multiply projection</code>
	 *
	 *	@param[out] r_t_matrix is ouput matrix (overwritten with result, not multiplied by result)
	 *	@param[in] f_left is position of left clip-plane
	 *	@param[in] f_right is position of right clip-plane
	 *	@param[in] f_bottom is position of bottom clip-plane
	 *	@param[in] f_top is position of top clip-plane
	 *	@param[in] f_near is depth of near clip plane
	 *	@param[in] f_far is depth of far clip plane
	 */
	static void Ortho(Matrix4f &r_t_matrix, float f_left, float f_right,
		float f_bottom, float f_top, float f_near, float f_far);

	/**
	 *	@brief calculates perspective projection matrix
	 *
	 *	@param[out] r_t_matrix is ouput matrix (overwritten with result, not multiplied by result)
	 *	@param[in] f_fov is field-of-view in degrees
	 *	@param[in] f_aspect is aspect (viewport height / width)
	 *	@param[in] f_near is depth of near clip plane
	 *	@param[in] f_far is depth of far clip plane
	 */
	static void Perspective(Matrix4f &r_t_matrix, float f_fov, float f_aspect, float f_near, float f_far);

	/**
	 *	@brief calculates perspective projection matrix for tiled rendering
	 *
	 *	Alias for gluPerspective, except this adds support for tile rendering
	 *		(rendering of rectangular window of perspective viewport in
	 *		purpose of rendering images, larger than maximal viewport size).
	 *
	 *		Tiles don't have to be square or power-of-two. and can overlap
	 *		(that is actually required for correct rendering of points and lines,
	 *		and can be useful to blend image seams when using reflection maps).
	 *
	 *		Projection should be multiplied by this matrix.
	 *
	 *		Example use:
	 *<code>		const int n_tile_size = 512;
	 *		const int n_target_width = 4000;
	 *		const int n_target_height = 3000;
	 *
	 *		for(int y = 0; y < n_target_height; y += n_tile_size) {
	 *			for(int x = 0; x < n_target_width; x += n_tile_size) {
	 *				glMatrixMode(GL_PROJECTION);
	 *				glLoadIdentity();
	 *				PerspectiveTile(n_target_width, n_target_height, x, y, n_tile_size, n_tile_size,
	 *					90, float(n_target_width) / n_target_height, .1f, 10000);
	 *				RenderScene();
	 *			}
	 *		}</code>
	 *
	 *	@todo See if example use code fragment works correctly.
	 *
	 *	@param[out] r_t_matrix is ouput matrix (overwritten with result, not multiplied by result)
	 *	@param[in] n_total_width is target image width (for example 4000)
	 *	@param[in] n_total_height is target image height (for example 3000)
	 *	@param[in] n_tile_x is horizontal offset of tile in target image
	 *	@param[in] n_tile_y is vertical offset of tile in target image
	 *	@param[in] n_tile_width is tile width (for example 512)
	 *	@param[in] n_tile_height is tile height (for example 512)
	 *	@param[in] f_fov is field-of-view in degrees
	 *	@param[in] f_aspect is aspect (viewport height / width)
	 *	@param[in] f_near is depth of near clip plane
	 *	@param[in] f_far is depth of far clip plane
	 *
	 *	@note n_tile_width, n_tile_height should be constant size, even if
	 *		<tt>n_tile_x + n_tile_width > n_total_width</tt> (or <tt>n_tile_y +
	 *		n_tile_height > n_total_height</tt>, respectively).
	 */
	static void PerspectiveTile(Matrix4f &r_t_matrix, int n_total_width, int n_total_height,
		int n_tile_x, int n_tile_y, int n_tile_width, int n_tile_height,
		float f_fov, float f_aspect, float f_near, float f_far);

	/**
	 *	@brief calculates ortoghraphic projection matrix for tiled rendering
	 *
	 *	Alias for gluOrtho, except this adds support for tile rendering
	 *		(rendering of rectangular window of perspective viewport in
	 *		purpose of rendering images, larger than maximal viewport size).
	 *		This is intended for 2D / GUI rendering in tiled images.
	 *
	 *		Tiles don't have to be square or power-of-two. and can overlap
	 *		(that is actually required for correct rendering of points and lines,
	 *		and can be useful to blend image seams when using reflection maps).
	 *
	 *		Projection should be multiplied by this matrix.
	 *
	 *		Example use:
	 *<code>		const int n_tile_size = 512;
	 *		const int n_target_width = 4000;
	 *		const int n_target_height = 3000;
	 *
	 *		for(int y = 0; y < n_target_height; y += n_tile_size) {
	 *			for(int x = 0; x < n_target_width; x += n_tile_size) {
	 *				glMatrixMode(GL_PROJECTION);
	 *				glLoadIdentity();
	 *				OrthoTile(n_target_width, n_target_height, x, y,
	 *					n_tile_size, n_tile_size, -1, 1, -1, 1, .1f, 10000);
	 *				RenderScene();
	 *			}
	 *		}</code>
	 *
	 *	@todo See if example use code fragment works correctly.
	 *
	 *	@param[out] r_t_matrix is ouput matrix (overwritten with result, not multiplied by result)
	 *	@param[in] n_total_width is target image width (for example 4000)
	 *	@param[in] n_total_height is target image height (for example 3000)
	 *	@param[in] n_tile_x is horizontal offset of tile in target image
	 *	@param[in] n_tile_y is vertical offset of tile in target image
	 *	@param[in] n_tile_width is tile width (for example 512)
	 *	@param[in] n_tile_height is tile height (for example 512)
	 *	@param[in] f_left is position of left clip-plane
	 *	@param[in] f_right is position of right clip-plane
	 *	@param[in] f_bottom is position of bottom clip-plane
	 *	@param[in] f_top is position of top clip-plane
	 *	@param[in] f_near is depth of near clip plane
	 *	@param[in] f_far is depth of far clip plane
	 *
	 *	@note n_tile_width, n_tile_height should be constant size, even if
	 *		<tt>n_tile_x + n_tile_width > n_total_width</tt> (or <tt>n_tile_y +
	 *		n_tile_height > n_total_height</tt>, respectively).
	 */
	static void OrthoTile(Matrix4f &r_t_matrix, int n_total_width, int n_total_height,
		int n_tile_x, int n_tile_y, int n_tile_width, int n_tile_height,
		float f_left, float f_right, float f_bottom, float f_top,
		float f_near, float f_far);

	/**
	 *	alias for gluLookAt
	 *
	 *	Calculates matrix of camera placed at v_eye, looking towards v_target,
	 *		while v_up is collinear with (0, 1) when projected using the camera matrix.
	 *		Modelview should be multiplied by this matrix.
	 *
	 *	@param[out] r_t_matrix is ouput matrix (overwritten with result, not multiplied by result)
	 *	@param[in] v_eye is position of the eye (camera)
	 *	@param[in] v_target is position of the eye (camera) target
	 *	@param[in] v_up is direction of up-vector
	 *
	 *	@note This doesn't work when up-vector is colinear with view vector.
	 */
	static void LookAt(Matrix4f &r_t_matrix, Vector3f v_eye, Vector3f v_target, Vector3f v_up);

	/**
	 *	alias for gluLookAt
	 *
	 *	Calculates matrix of camera placed at f_eye_[xyz], looking towards f_target_[xyz],
	 *		while f_up_[xyz] is collinear with (0, 1) when projected using the camera matrix.
	 *		Modelview should be multiplied by this matrix.
	 *
	 *	@param[out] r_t_matrix is ouput matrix (overwritten with result, not multiplied by result)
	 *	@param[in] f_eye_x is x-position of the eye (camera)
	 *	@param[in] f_eye_y is y-position of the eye (camera)
	 *	@param[in] f_eye_z is z-position of the eye (camera)
	 *	@param[in] f_target_x is x-position of the eye (camera) target
	 *	@param[in] f_target_y is y-position of the eye (camera) target
	 *	@param[in] f_target_z is z-position of the eye (camera) target
	 *	@param[in] f_up_x is x-direction of up-vector
	 *	@param[in] f_up_y is y-direction of up-vector
	 *	@param[in] f_up_z is z-direction of up-vector
	 *
	 *	@note This doesn't work when up-vector is colinear with view vector.
	 *	@note This calls Vector3f version of LookAt().
	 */
	static inline void LookAt(Matrix4f &r_t_matrix, float f_eye_x, float f_eye_y, float f_eye_z,
		float f_target_x, float f_target_y, float f_target_z,
		float f_up_x, float f_up_y, float f_up_z)
	{
		LookAt(r_t_matrix, Vector3f(f_eye_x, f_eye_y, f_eye_z),
			   Vector3f(f_target_x, f_target_y, f_target_z),
			   Vector3f(f_up_x, f_up_y, f_up_z));
	}

	/**
	 *	@brief calculates worldspace ray for given pixel under given projection
	 *
	 *	Calculates worldspace ray under position v_point (in normalized [-1, 1] OpenGL
	 *		screen-space coordinates), useful for object selection using raytracing,
	 *		or for raytracing parts of picture, coherent with rasterized image.
	 *
	 *	@param[in] r_modelview_projection_inverse_transpose is modelview projection
	 *		inverse transpose matrix (use (t_modelview * t_projection).t_FullInverse())
	 *	@param[in] v_screen_point is point in normalized [-1, 1] OpenGL screen-space coordinates
	 *	@param[out] r_v_org is ray origin
	 *	@param[out] r_v_dir is ray direction (not normalized)
	 *
	 *	@note Ray direction doesn't come out normalized.
	 */
	static void Calc_ViewRay(const Matrix4f &r_modelview_projection_inverse_transpose,
		Vector2f v_screen_point, Vector3f &r_v_org, Vector3f &r_v_dir);

	/**
	 *	@brief mirrors camera arround plane
	 *
	 *	Mirrors camera arround r_t_plane (needs to be normalized).
	 *		This is used to render mirrors, it is also required to
	 *		set proper clipping plane, so the geometry stays "behind
	 *		the mirror". Modelview matrix should be multiplied by
	 *		result matrix.
	 *
	 *	@param[out] r_t_matrix is ouput matrix (overwritten with result, not multiplied by result)
	 *	@param[in] r_t_plane is mirror plane
	 */
	static void Mirror(Matrix4f &r_t_matrix, const Plane3f &r_t_plane);

	/**
	 *	@brief calculates screenspace to worldspace transformation
	 *
	 *	@param[in] r_modelview_projection_inverse_transpose is modelview projection
	 *		inverse transpose matrix (use (t_modelview * t_projection).t_FullInverse())
	 *	@param[in] v_screen_point is point in normalized [-1, 1] OpenGL screen-space coordinates
	 *	@param[in] f_z is (normalized) screen point depth (-1 = point at near plane)
	 *
	 *	@return Returns worldspace point, corresponding to given screenspace coordinates.
	 */
	static Vector3f v_UnTransform(const Matrix4f &r_modelview_projection_inverse_transpose,
		Vector2f v_screen_point, float f_z = -1);

	/**
	 *	@brief calculates worldspace to screenspace transformation
	 *
	 *	@param[in] r_modelview_projection is modelview-projection matrix
	 *	@param[in] v_world_point is point in world-space coordinates
	 *
	 *	@return Returns screenspace point, corresponding to worldspace point.
	 */
	static Vector3f v_Transform(const Matrix4f &r_modelview_projection,
		Vector3f v_world_point);
};

#endif //__OPENGL_TRANSFORM_UTILITIES_INCLUDED
