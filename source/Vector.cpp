/*
								+----------------------------------+
								|                                  |
								|    ***  Vector math v2.0  ***    |
								|                                  |
								|   Copyright © -tHE SWINe- 2003   |
								|                                  |
								|            Vector.cpp            |
								|                                  |
								+----------------------------------+
*/

/*
 *	passed code revision at 2006-05-16
 *
 *	optimized some parts of code
 *	all integers storing just true of false were replaced with bool
 *
 *	passed code revision at 2006-07-02
 *
 *	added swizzle operators to vector classes
 *	added redlect / refract / fresnel functions to Vector3<T> class
 *	renamed Vector3<T>::Len to Vector3<T>::f_Length, Vector3<T>::Len2 to Vector3<T>::f_Length2
 *	added some Matrix3f methods, renamed Matrix3f::Init() to Matrix3f::Identity()
 *	renamed Planef to Plane3f
 *	renamed Pi to f_pi
 *	general cleanup of code
 *
 *	2006-07-17
 *
 *	renamed CPolygon::r_Vertex(int) to CPolygon::r_t_Vertex(int)
 *	renamed CPolygon::r_Normal() to CPolygon::r_t_Normal()
 *
 *	2006-07-20
 *
 *	added CPolygon::b_MT_RayHit_CullFrontfaces and CPolygon::b_MT_RayHit_CullBackFaces
 *
 *	2006-07-24
 *
 *	renamed TVertexStruct to TVertex3f
 *	added class CVectorMath with ray distances and bounding box / line (ray) intersection tests
 *
 *	2007-01-23
 *
 *	fixed std::vector handling in template CPolygon<TVertex3f> (not-enough-memory checks)
 *	remade TVertex3f base class and CPolygon<TVertex3f> a bit (added + and * operators,
 *	renamed Lerp() to t_Lerp())
 *
 *	2007-03-07
 *
 *	added complete swizzling functions set to vectors
 *
 *	2007-06-04
 *
 *	added missing const keyword to input swizzle functions
 *
 *	2007-08-03
 *
 *	added quaternion class with basic quaternion operations
 *
 *	2007-10-27
 *
 *	added convenience vector / constant operators and component-wise vector * vector
 *	and vector / vector operators to all vector templates
 *	added Vector4f Matrix4f::operator *(const Vector4f &) function for natural matrix-vertex
 *	multiplication as alternative to v_Transform_Pos() and v_Transform_Dir()
 *
 *	2007-11-26
 *
 *	added Matrix4f::f_Subdet, Matrix4f::f_Determinant, Matrix4f::FullInvert and
 *	Matrix4f::t_FullInverse
 *	added convenience vector + constant and vector - constant operators to all vector templates
 *
 *	2008-08-21
 *
 *	fixed Matrix4f::Scale(), added non-Vector3f variants of Matrix4f::Scale(),
 *	Matrix4f::Translate() and Matrix4f::Rotate()
 *
 *	2008-08-23
 *
 *	documented Matrix4f class, added functions for generating transformation
 *	matrices (not applying transformation on the matrix), namely Matrix4f::Translation(),
 *	Matrix4f::Scaling(), Matrix4f::RotationX(), Matrix4f::RotationY(), Matrix4f::RotationZ(),
 *	and Matrix4f::Rotation(). added component-wise matrix by scalar multiplication (overloads
 *	of Matrix4f::operator *() and Matrix4f::operator *=()) and finaly added transposition
 *	function Matrix4f::Transposition()
 *
 *	2009-05-04
 *
 *	fixed mixed windows / linux line endings
 *
 *	2009-05-12
 *
 *	fixed out-of-bounds index in TObjectOrientedBox::b_Intersect_Ray()
 *
 *	2009-05-23
 *
 *	removed all instances of std::vector::reserve and replaced them by stl_ut::Reserve_*
 *
 */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include "Vector.h"

const float f_pi = 3.1415926535897932384626433832795028841971691075f;

/*
 *        === Matrix4f ===
 */

/*
 *	void Matrix4f::Identity()
 *		- creates unit matrix (identity transformation)
 *		- note this is not done automatically by constructor (there's none)
 */
void Matrix4f::Identity()
{
	for(int j = 0; j < 4; ++ j) {		
		for(int i = 0; i < 4; ++ i)
			f[i][j] = (float)(i == j);
	}
}

/*
 *	void Matrix4f::Translation(const Vector3f &r_v_translate)
 *		- creates translation matrix; r_v_translate is translation vector
 */
void Matrix4f::Translation(const Vector3f &r_v_translate)
{
	for(int j = 0; j < 4; ++ j) {		
		for(int i = 0; i < 3; ++ i)
			f[i][j] = (float)(i == j);
	}
	f[3][0] = r_v_translate.x;
    f[3][1] = r_v_translate.y;
    f[3][2] = r_v_translate.z;
	f[3][3] = 1;
}

/*
 *	void Matrix4f::Translation(float f_translate_x, float f_translate_y, float f_translate_z)
 *		- creates translation matrix;
 *		  (f_translate_x, f_translate_y, f_translate_z) is translation vector
 */
void Matrix4f::Translation(float f_translate_x, float f_translate_y, float f_translate_z)
{
	for(int j = 0; j < 4; ++ j) {		
		for(int i = 0; i < 3; ++ i)
			f[i][j] = (float)(i == j);
	}
	f[3][0] = f_translate_x;
    f[3][1] = f_translate_y;
    f[3][2] = f_translate_z;
	f[3][3] = 1;
}

/*
 *	void Matrix4f::Scaling(float f_scale)
 *		- creates scaling matrix; f_scale is scaling factor (same for x, y and z)
 */
void Matrix4f::Scaling(float f_scale)
{
    Identity();
    f[0][0] = f_scale;
    f[1][1] = f_scale;
    f[2][2] = f_scale;
}

/*
 *	void Matrix4f::Scaling(float f_scale_x, float f_scale_y, float f_scale_z)
 *		- creates scaling matrix; f_scale_x, f_scale_y and f_scale_z are
 *		  scaling factors for x, y and z, respectively
 */
void Matrix4f::Scaling(float f_scale_x, float f_scale_y, float f_scale_z)
{
    Identity();
    f[0][0] = f_scale_x;
    f[1][1] = f_scale_y;
    f[2][2] = f_scale_z;
}

/*
 *	void Matrix4f::Scaling(const Vector3f &r_v_scale)
 *		- creates scaling matrix; r_v_scale contains scaling factors for x, y and z
 */
void Matrix4f::Scaling(const Vector3f &r_v_scale)
{
    Identity();
    f[0][0] = r_v_scale.x;
    f[1][1] = r_v_scale.y;
    f[2][2] = r_v_scale.z;
}

/*
 *	void Matrix4f::RotationX(float f_angle)
 *		- creates matrix for rotation arround x-axis; f_angle is angle in radians
 */
void Matrix4f::RotationX(float f_angle)
{
    Identity();
	float f_sin = float(sin(f_angle));
	float f_cos = float(cos(f_angle));
    f[2][1] = -f_sin;
    f[2][2] = f_cos;
    f[1][1] = f_cos;
    f[1][2] = f_sin;
}

/*
 *	void Matrix4f::RotationY(float f_angle)
 *		- creates matrix for rotation arround y-axis; f_angle is angle in radians
 */
void Matrix4f::RotationY(float f_angle)
{
    Identity();
	float f_sin = float(sin(f_angle));
	float f_cos = float(cos(f_angle));
    f[0][0] = f_cos;
    f[0][2] = -f_sin;
    f[2][2] = f_cos;
    f[2][0] = f_sin;
}

/*
 *	void Matrix4f::RotationZ(float f_angle)
 *		- creates matrix for rotation arround z-axis; f_angle is angle in radians
 */
void Matrix4f::RotationZ(float f_angle)
{
    Identity();
	float f_sin = float(sin(f_angle));
	float f_cos = float(cos(f_angle));
    f[0][0] = f_cos;
    f[0][1] = f_sin;
    f[1][1] = f_cos;
    f[1][0] = -f_sin;
}

/*
 *	void Matrix4f::Rotation(float f_angle, float f_axis_x, float f_axis_y, float f_axis_z)
 *		- creates matrix for rotation arround axis given
 *		  by (f_axis_x, f_axis_y, f_axis_z), f_angle is angle in radians
 */
void Matrix4f::Rotation(float f_angle, float f_axis_x, float f_axis_y, float f_axis_z)
{
	// formula for rotation matrix around arbitrary axis:
	// R = uuT + cos(f_angle) * (I - uuT) + sin(f_angle)S

	float f_cos = float(cos(f_angle));
	float f_o_m_cos = 1 - f_cos;
	float f_axis_x_o_m_cos = f_axis_x * f_o_m_cos;
	float f_axis_y_o_m_cos = f_axis_y * f_o_m_cos;
	float f_axis_z_o_m_cos = f_axis_z * f_o_m_cos;
    f[0][0] = f_axis_x * f_axis_x_o_m_cos + f_cos;
    f[0][1] = f_axis_x * f_axis_y_o_m_cos;
    f[0][2] = f_axis_x * f_axis_z_o_m_cos;
	f[0][3] = 0;
    f[1][0] = f_axis_y * f_axis_x_o_m_cos;
    f[1][1] = f_axis_y * f_axis_y_o_m_cos + f_cos;
    f[1][2] = f_axis_y * f_axis_z_o_m_cos;
	f[1][3] = 0;
    f[2][0] = f_axis_z * f_axis_x_o_m_cos;
    f[2][1] = f_axis_z * f_axis_y_o_m_cos;
    f[2][2] = f_axis_z * f_axis_z_o_m_cos + f_cos;
	f[2][3] = 0;
	f[3][0] = 0;
	f[3][1] = 0;
	f[3][2] = 0;
	f[3][3] = 1;
	// R = uu^T * (1 - cos(f_angle)) + cos(f_angle) * I + ...

	float f_sin = float(sin(f_angle));
	f_axis_x *= f_sin;
	f_axis_y *= f_sin;
	f_axis_z *= f_sin;
	f[1][0] -= f_axis_z;
	f[0][1] += f_axis_z;
	f[2][0] += f_axis_y;
	f[0][2] -= f_axis_y;
	f[2][1] -= f_axis_x;
	f[1][2] += f_axis_x;
	// ...  + sin(f_angle)S
}

/*
 *	void Matrix4f::Rotation(float f_angle, const Vector3f &r_v_axis)
 *		- creates matrix for rotation arround axis given by r_v_axis,
 *		  f_angle is angle in radians
 */
void Matrix4f::Rotation(float f_angle, const Vector3f &r_v_axis)
{
	// formula for rotation matrix around arbitrary axis:
	// R = uuT + cos(f_angle) * (I - uuT) + sin(f_angle)S

	float f_cos = float(cos(f_angle));
	Vector3f v_u_o_m_cos(r_v_axis * (1 - f_cos));
	for(int i = 0; i < 3; ++ i) {
        for(int j = 0; j < 3; ++ j)
            f[i][j] = r_v_axis[i] * v_u_o_m_cos[j] + ((i == j)? f_cos : 0);
		f[i][3] = 0;
		f[3][i] = 0;
    }
	f[3][3] = 1;
	// R = uu^T * (1 - cos(f_angle)) + cos(f_angle) * I + ...

	Vector3f v_s(r_v_axis * float(sin(f_angle)));
	f[1][0] -= v_s.z;
	f[0][1] += v_s.z;
	f[2][0] += v_s.y;
	f[0][2] -= v_s.y;
	f[2][1] -= v_s.x;
	f[1][2] += v_s.x;
	// ...  + sin(f_angle)S
}

/*
 *	void Matrix4f::Translate(float f_translate_x, float f_translate_y, float f_translate_z)
 *		- applies translation on this matrix; translation vector
 *		  is given by (f_translate_x, f_translate_y, f_translate_z)
 */
void Matrix4f::Translate(float f_translate_x, float f_translate_y, float f_translate_z)
{
	Matrix4f t_tmp;
    t_tmp.Translation(f_translate_x, f_translate_y, f_translate_z);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::Translate(const Vector3f &r_v_translate)
 *		- applies translation on this matrix; r_v_translate is translation vector
 */
void Matrix4f::Translate(const Vector3f &r_v_translate)
{
	Matrix4f t_tmp;
    t_tmp.Translation(r_v_translate);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::Scale(float f_scale)
 *		- applies scaling on this matrix; f_scale is scaling factor (for all x, y and z)
 */
void Matrix4f::Scale(float f_scale)
{
	Matrix4f t_tmp;
	t_tmp.Scaling(f_scale);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::Scale(float f_scale_x, float f_scale_y, float f_scale_z)
 *		- applies scaling on this matrix; f_scale_x, f_scale_y, f_scale_z
 *		  are scaling factors for x, y and z, respectively
 */
void Matrix4f::Scale(float f_scale_x, float f_scale_y, float f_scale_z)
{
	Matrix4f t_tmp;
	t_tmp.Scaling(f_scale_x, f_scale_y, f_scale_z);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::Scale(const Vector3f &r_v_scale)
 *		- applies scaling on this matrix; r_v_scale contains scaling factors for x, y and z
 */
void Matrix4f::Scale(const Vector3f &r_v_scale)
{
	Matrix4f t_tmp;
	t_tmp.Scaling(r_v_scale);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::RotateX(float f_angle)
 *		- applies rotation f_angle radians arround x-axis to this matrix
 */
void Matrix4f::RotateX(float f_angle)
{
	Matrix4f t_tmp;
    t_tmp.RotationX(f_angle);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::RotateY(float f_angle)
 *		- applies rotation f_angle radians arround y-axis to this matrix
 */
void Matrix4f::RotateY(float f_angle)
{
	Matrix4f t_tmp;
    t_tmp.RotationY(f_angle);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::RotateZ(float f_angle)
 *		- applies rotation f_angle radians arround z-axis to this matrix
 */
void Matrix4f::RotateZ(float f_angle)
{
	Matrix4f t_tmp;
    t_tmp.RotationZ(f_angle);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::Rotate(float f_angle, float f_axis_x, float f_axis_y, float f_axis_z)
 *		- applies rotation f_angle radians arround axis given
 *		  by (f_axis_x, f_axis_y, f_axis_z) to this matrix
 */
void Matrix4f::Rotate(float f_angle, float f_axis_x, float f_axis_y, float f_axis_z)
{
	Matrix4f t_tmp;
    t_tmp.Rotation(f_angle, f_axis_x, f_axis_y, f_axis_z);
	*this *= t_tmp;
}

/*
 *	void Matrix4f::Rotate(float f_angle, const Vector3f &r_v_axis)
 *		- applies rotation f_angle radians arround axis given by r_v_axis to this matrix
 */
void Matrix4f::Rotate(float f_angle, const Vector3f &r_v_axis)
{
	Matrix4f t_tmp;
    t_tmp.Rotation(f_angle, r_v_axis);
	*this *= t_tmp;
}

/*
 *	Matrix4f Matrix4f::operator *(float f_factor) const
 *		- returns element-wise multiplication of matrix and f_factor
 */
void Matrix4f::ProductOf(const Matrix4f &r_t_mat1, float f_factor)
{
    for(int i = 0; i < 4; ++ i) {
        for(int j = 0; j < 4; ++ j)
            f[i][j] = r_t_mat1.f[i][j] * f_factor;
    }
}

void Matrix4f::ProductOf(const Matrix4f &r_t_mat1, const Matrix4f &r_t_mat2)
{
	_ASSERTE(this != &r_t_mat1);
	_ASSERTE(this != &r_t_mat2); // have to use temp matrix, or operator *=()

	for(int i = 0; i < 4; ++ i) {
		for(int j = 0; j < 4; ++ j) {
			f[i][j] = r_t_mat1.f[0][j] * r_t_mat2.f[i][0] +
					  r_t_mat1.f[1][j] * r_t_mat2.f[i][1] +
					  r_t_mat1.f[2][j] * r_t_mat2.f[i][2] +
					  r_t_mat1.f[3][j] * r_t_mat2.f[i][3];
		}
	}
}

/*
 *	Matrix4f Matrix4f::operator *=(float f_factor) const
 *		- element-wise multiplies this matrix by
 *		  f_factor and returns reference to this
 */
Matrix4f &Matrix4f::operator *=(float f_factor)
{
    for(int i = 0; i < 4; ++ i) {
        for(int j = 0; j < 4; ++ j)
            f[i][j] *= f_factor;
    }
	return *this;
}

/*
 *	Matrix4f Matrix4f::&operator *=(const Matrix4f &r_t_mat)
 *		- multiplies this matrix by r_t_mat and returns reference to this
 */
Matrix4f &Matrix4f::operator *=(const Matrix4f &r_t_mat)
{
	float f_temp_1_0 = f[0][0] * r_t_mat.f[1][0] + f[1][0] * r_t_mat.f[1][1] +
		f[2][0] * r_t_mat.f[1][2] + f[3][0] * r_t_mat.f[1][3];
	float f_temp_2_0 = f[0][0] * r_t_mat.f[2][0] + f[1][0] * r_t_mat.f[2][1] +
		f[2][0] * r_t_mat.f[2][2] + f[3][0] * r_t_mat.f[2][3];
	float f_temp_3_0 = f[0][0] * r_t_mat.f[3][0] + f[1][0] * r_t_mat.f[3][1] +
		f[2][0] * r_t_mat.f[3][2] + f[3][0] * r_t_mat.f[3][3];
	f[0][0] = f[0][0] * r_t_mat.f[0][0] + f[1][0] * r_t_mat.f[0][1] +
		f[2][0] * r_t_mat.f[0][2] + f[3][0] * r_t_mat.f[0][3];
	f[1][0] = f_temp_1_0;
	f[2][0] = f_temp_2_0;
	f[3][0] = f_temp_3_0;
	float f_temp_1_1 = f[0][1] * r_t_mat.f[1][0] + f[1][1] * r_t_mat.f[1][1] +
		f[2][1] * r_t_mat.f[1][2] + f[3][1] * r_t_mat.f[1][3];
	float f_temp_2_1 = f[0][1] * r_t_mat.f[2][0] + f[1][1] * r_t_mat.f[2][1] +
		f[2][1] * r_t_mat.f[2][2] + f[3][1] * r_t_mat.f[2][3];
	float f_temp_3_1 = f[0][1] * r_t_mat.f[3][0] + f[1][1] * r_t_mat.f[3][1] +
		f[2][1] * r_t_mat.f[3][2] + f[3][1] * r_t_mat.f[3][3];
	f[0][1] = f[0][1] * r_t_mat.f[0][0] + f[1][1] * r_t_mat.f[0][1] +
		f[2][1] * r_t_mat.f[0][2] + f[3][1] * r_t_mat.f[0][3];
	f[1][1] = f_temp_1_1;
	f[2][1] = f_temp_2_1;
	f[3][1] = f_temp_3_1;
	float f_temp_1_2 = f[0][2] * r_t_mat.f[1][0] + f[1][2] * r_t_mat.f[1][1] +
		f[2][2] * r_t_mat.f[1][2] + f[3][2] * r_t_mat.f[1][3];
	float f_temp_2_2 = f[0][2] * r_t_mat.f[2][0] + f[1][2] * r_t_mat.f[2][1] +
		f[2][2] * r_t_mat.f[2][2] + f[3][2] * r_t_mat.f[2][3];
	float f_temp_3_2 = f[0][2] * r_t_mat.f[3][0] + f[1][2] * r_t_mat.f[3][1] +
		f[2][2] * r_t_mat.f[3][2] + f[3][2] * r_t_mat.f[3][3];
	f[0][2] = f[0][2] * r_t_mat.f[0][0] + f[1][2] * r_t_mat.f[0][1] +
		f[2][2] * r_t_mat.f[0][2] + f[3][2] * r_t_mat.f[0][3];
	f[1][2] = f_temp_1_2;
	f[2][2] = f_temp_2_2;
	f[3][2] = f_temp_3_2;
	float f_temp_1_3 = f[0][3] * r_t_mat.f[1][0] + f[1][3] * r_t_mat.f[1][1] +
		f[2][3] * r_t_mat.f[1][2] + f[3][3] * r_t_mat.f[1][3];
	float f_temp_2_3 = f[0][3] * r_t_mat.f[2][0] + f[1][3] * r_t_mat.f[2][1] +
		f[2][3] * r_t_mat.f[2][2] + f[3][3] * r_t_mat.f[2][3];
	float f_temp_3_3 = f[0][3] * r_t_mat.f[3][0] + f[1][3] * r_t_mat.f[3][1] +
		f[2][3] * r_t_mat.f[3][2] + f[3][3] * r_t_mat.f[3][3];
	f[0][3] = f[0][3] * r_t_mat.f[0][0] + f[1][3] * r_t_mat.f[0][1] +
		f[2][3] * r_t_mat.f[0][2] + f[3][3] * r_t_mat.f[0][3];
	f[1][3] = f_temp_1_3;
	f[2][3] = f_temp_2_3;
	f[3][3] = f_temp_3_3;
	// somewhat better optimized, requires less copying

	return *this;
}

/*
 *	Vector4f Matrix4f::operator *(const Vector4f &r_v_vec) const
 *		- vector-matrix multiplication
 *		- returns this * r_v_vec
 */
Vector4f Matrix4f::operator *(const Vector4f &r_v_vec) const
{
	return Vector4f(r_v_vec.x * f[0][0] + r_v_vec.y * f[1][0] +
					r_v_vec.z * f[2][0] + r_v_vec.w * f[3][0],
				    r_v_vec.x * f[0][1] + r_v_vec.y * f[1][1] +
					r_v_vec.z * f[2][1] + r_v_vec.w * f[3][1],
				    r_v_vec.x * f[0][2] + r_v_vec.y * f[1][2] +
					r_v_vec.z * f[2][2] + r_v_vec.w * f[3][2],
				    r_v_vec.x * f[0][3] + r_v_vec.y * f[1][3] +
					r_v_vec.z * f[2][3] + r_v_vec.w * f[3][3]);
}

/*
 *	Vector3f Matrix4f::v_Transform_Pos(const Vector3f &r_v_vec) const
 *		- transforms position r_v_vec by this matrix
 *		- equivalent to multiplying this matrix by Vector4f(r_v_vec, 1)
 */
Vector3f Matrix4f::v_Transform_Pos(const Vector3f &r_v_vec) const
{
	return Vector3f(r_v_vec.x * f[0][0] + r_v_vec.y * f[1][0] + r_v_vec.z * f[2][0] + f[3][0],
				    r_v_vec.x * f[0][1] + r_v_vec.y * f[1][1] + r_v_vec.z * f[2][1] + f[3][1],
				    r_v_vec.x * f[0][2] + r_v_vec.y * f[1][2] + r_v_vec.z * f[2][2] + f[3][2]);
}

/*
 *	Vector3f Matrix4f::v_Transform_Dir(const Vector3f &r_v_vec) const
 *		- transforms direction r_v_vec by this matrix
 *		- equivalent to multiplying this matrix by Vector4f(r_v_vec, 0)
 */
Vector3f Matrix4f::v_Transform_Dir(const Vector3f &r_v_vec) const
{
	return Vector3f(r_v_vec.x * f[0][0] + r_v_vec.y * f[1][0] + r_v_vec.z * f[2][0],
				    r_v_vec.x * f[0][1] + r_v_vec.y * f[1][1] + r_v_vec.z * f[2][1],
				    r_v_vec.x * f[0][2] + r_v_vec.y * f[1][2] + r_v_vec.z * f[2][2]);
}

void Matrix4f::FastInverseTo(Matrix4f &r_dest) const
{
	_ASSERTE(&r_dest != this); // use FastInvert() instead

	_ASSERTE(f[0][3] == 0 && f[1][3] == 0 && f[2][3] == 0 && f[3][3] == 1);
	// bottom row must be equal to 0 0 0 1

    float f_det = f[0][0] * f[1][1] * f[2][2] +
            f[1][0] * f[2][1] * f[0][2] +
            f[0][1] * f[1][2] * f[2][0] -
            f[0][2] * f[1][1] * f[2][0] -
            f[0][1] * f[1][0] * f[2][2] -
            f[0][0] * f[1][2] * f[2][1];

    r_dest.f[0][0] = f[1][1] * f[2][2] - f[1][2] * f[2][1];
    r_dest.f[1][0] =-f[1][0] * f[2][2] + f[1][2] * f[2][0];
    r_dest.f[2][0] = f[1][0] * f[2][1] - f[1][1] * f[2][0];
    r_dest.f[3][0] =-f[1][0] * f[2][1] * f[3][2] -
		f[1][1] * f[2][2] * f[3][0] -
		f[2][0] * f[3][1] * f[1][2] +
		f[1][2] * f[2][1] * f[3][0] +
		f[1][1] * f[2][0] * f[3][2] +
		f[2][2] * f[1][0] * f[3][1];
    r_dest.f[0][1] =-f[0][1] * f[2][2] + f[0][2] * f[2][1];
    r_dest.f[1][1] = f[0][0] * f[2][2] - f[0][2] * f[2][0];
    r_dest.f[2][1] =-f[0][0] * f[2][1] + f[0][1] * f[2][0];
	r_dest.f[3][1] = f[0][0] * f[2][1] * f[3][2] +
		f[0][1] * f[2][2] * f[3][0] +
		f[2][0] * f[3][1] * f[0][2] -
		f[0][2] * f[2][1] * f[3][0] -
		f[0][1] * f[2][0] * f[3][2] -
		f[2][2] * f[0][0] * f[3][1];
    r_dest.f[0][2] = f[0][1] * f[1][2] - f[0][2] * f[1][1];
    r_dest.f[1][2] =-f[0][0] * f[1][2] + f[0][2] * f[1][0];
    r_dest.f[2][2] = f[0][0] * f[1][1] - f[0][1] * f[1][0];
    r_dest.f[3][2] =-f[0][0] * f[1][1] * f[3][2] -
		f[0][1] * f[1][2] * f[3][0] -
		f[1][0] * f[3][1] * f[0][2] +
		f[0][2] * f[1][1] * f[3][0] +
		f[0][1] * f[1][0] * f[3][2] +
		f[1][2] * f[0][0] * f[3][1];

	f_det = 1.0f / f_det;
    for(int j = 0; j < 4; ++ j) {
        for(int i = 0; i < 3; ++ i)
            r_dest.f[j][i] *= f_det;
	}

    r_dest.f[0][3] = 0;
    r_dest.f[1][3] = 0;
    r_dest.f[2][3] = 0;
    r_dest.f[3][3] = 1;
}

/*
 *	float Matrix4f::f_Subdet(int n_col, int n_row) const
 *		- returns determinant of this matrix with column n_col and row
 *		  n_row left out (so it calculates 3x3 matrix determinant)
 *		- note the result is not multiplied by (-1)^(n_col + n_row)
 */
float Matrix4f::f_Subdet(int n_col, int n_row) const
{
	int i0 = (n_row <= 0) + 0, i1 = (n_row <= 1) + 1, i2 = (n_row <= 2) + 2;
	int j0 = (n_col <= 0) + 0, j1 = (n_col <= 1) + 1, j2 = (n_col <= 2) + 2;

	return (f[j0][i0] * f[j1][i1] * f[j2][i2] +
		   f[j1][i0] * f[j2][i1] * f[j0][i2] +
		   f[j0][i1] * f[j1][i2] * f[j2][i0]) -
		   (f[j0][i2] * f[j1][i1] * f[j2][i0] +
		   f[j0][i1] * f[j1][i0] * f[j2][i2] +
		   f[j0][i0] * f[j1][i2] * f[j2][i1]);
}

/*
 *	float Matrix4f::f_Determinant() const
 *		- returns determinant of this matrix
 *		- note it uses subdeterminants, it is optimized for matrices
 *		  having zeros in the last row (common transformation matrices)
 */
float Matrix4f::f_Determinant() const
{
	float f_result = 0;
	float f_sign = 1;
	for(int i = 0; i < 4; ++ i, f_sign *= -1) {
		if(f[i][3]) // last row is sometimes zero-prone
			f_result += f_sign * f_Subdet(i, 3) * f[i][3];
	}
	return f_result;
}

/*
 *	Matrix4f Matrix4f::t_FullInverse() const
 *		- inverts this matrix (uses adjunged matrix method)
 *		- note full here means unoptimized, t_FastInverse() can be used to invert
 *		  matrices with bottom row equal to 0 0 0 1 (common transformation
 *		  matrices) more optimally
 */
void Matrix4f::FullInverseTo(Matrix4f &r_dest) const
{
	_ASSERTE(&r_dest != this); // use FullInvert() instead

	float f_inv_det = 1 / f_Determinant();
	for(int i = 0; i < 4; ++ i) {
		for(int j = 0; j < 4; ++ j)
			r_dest.f[j][i] = (1 - 2 * ((i + j) & 1)) * f_Subdet(i, j) * f_inv_det;
	}
}

void Matrix4f::FullInverseNoTransposeTo(Matrix4f &r_dest) const
{
	_ASSERTE(&r_dest != this); // use FullInvert() instead

	float f_inv_det = 1 / f_Determinant();
	for(int i = 0; i < 4; ++ i) {
		for(int j = 0; j < 4; ++ j)
			r_dest.f[i][j] = (1 - 2 * ((i + j) & 1)) * f_Subdet(i, j) * f_inv_det;
	}
}

/*
 *	void Matrix4f::Transpose()
 *		- transposes this matrix
 */
void Matrix4f::Transpose()
{
	float f_tmp0 = f[0][1];
    f[0][1] = f[1][0];
    f[1][0] = f_tmp0;
	float f_tmp1 = f[0][2];
    f[0][2] = f[2][0];
    f[2][0] = f_tmp1;
	float f_tmp2 = f[0][3];
    f[0][3] = f[3][0];
    f[3][0] = f_tmp2;
	float f_tmp3 = f[1][2];
    f[1][2] = f[2][1];
    f[2][1] = f_tmp3;
	float f_tmp4 = f[1][3];
    f[1][3] = f[3][1];
    f[3][1] = f_tmp4;
	float f_tmp5 = f[2][3];
    f[2][3] = f[3][2];
    f[3][2] = f_tmp5;
}

/*
 *	void Matrix4f::Transpose()
 *		- transposes this matrix
 */
void Matrix4f::TransposeTo(Matrix4f &r_dest) const
{
	_ASSERTE(&r_dest != this); // use Transpose() instead

    for(int i = 0; i < 4; ++ i) {
        for(int j = 0; j < 4; ++ j)
            r_dest.f[i][j] = f[j][i];
    }
}

/*
 *        === ~Matrix4f ===
 */
