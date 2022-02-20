//program: matrix.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: cs371 Fall 2013
//         This is a library of matrix functions.
//
//
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//#include <X11/Xlib.h>
#include "log.h"
#include "matrix.h"

void matrix_copy(Matrix m1, Matrix m2)
{
	memcpy(m2, m1, sizeof(Matrix));
}

void concatenate_to_matrix(Matrix mat, Matrix dest)
{
	//Concatenate new transform with our existing matrix.
	Matrix m;
	matrix_cat(mat, dest, m);
	matrix_copy(m, dest);
}

void translate(Vec translate, Matrix mat)
{
	identity(mat);
	//Move translation into our new matrix
	//
	//      x     y     z
	//   |-----|-----|-----|-----|
	// x |  1  |     |     |     |
	//   |-----|-----|-----|-----|
	// y |     |  1  |     |     |
	//   |-----|-----|-----|-----|
	// z |     |     |  1  |     |
	//   |-----|-----|-----|-----|
	//   | tx  | ty  | tz  |  1  |
	//   |-----|-----|-----|-----|
	//
	mat[3][0] = translate[0];
	mat[3][1] = translate[1];
	mat[3][2] = translate[2];
}

void rotate(Vec rot, Matrix mat)
{
	identity(mat);
	//Move rotation into our new matrix
	//This code is not optimized.
	if (rot[0] != 0.0) {
		//
		//rotate on x-axis
		//
		//      x     y     z
		//   |-----|-----|-----|-----|
		// x |  1  |     |     |     |
		//   |-----|-----|-----|-----|
		// y |     | cos | sin |     |
		//   |-----|-----|-----|-----|
		// z |     |-sin | cos |     |
		//   |-----|-----|-----|-----|
		//   |     |     |     |  1  |
		//   |-----|-----|-----|-----|
		//
		mat[1][1] =  cos(rot[0]);
		mat[1][2] =  sin(rot[0]);
		mat[2][1] = -sin(rot[0]);
		mat[2][2] =  cos(rot[0]);
	}
	if (rot[1] != 0.0) {
		//
		//rotate on y-axis
		//
		//      x     y     z
		//   |-----|-----|-----|-----|
		// x | cos |     |-sin |     |
		//   |-----|-----|-----|-----|
		// y |     |  1  |     |     |
		//   |-----|-----|-----|-----|
		// z | sin |     | cos |     |
		//   |-----|-----|-----|-----|
		//   |     |     |     |  1  |
		//   |-----|-----|-----|-----|
		//
		mat[0][0] =  cos(rot[1]);
		mat[0][2] = -sin(rot[1]);
		mat[2][0] =  sin(rot[1]);
		mat[2][2] =  cos(rot[1]);
	}
	if (rot[2] != 0.0) {
		//
		//rotate on z-axis
		//
		//      x     y     z
		//   |-----|-----|-----|-----|
		// x | cos | sin |     |     |
		//   |-----|-----|-----|-----|
		// y |-sin | cos |     |     |
		//   |-----|-----|-----|-----|
		// z |     |     |  1  |     |
		//   |-----|-----|-----|-----|
		//   |     |     |     |  1  |
		//   |-----|-----|-----|-----|
		//
		mat[0][0] =  cos(rot[2]);
		mat[0][1] =  sin(rot[2]);
		mat[1][0] = -sin(rot[2]);
		mat[1][1] =  cos(rot[2]);
	}
}

void scale(Vec scale, Matrix mat)
{
	identity(mat);
	//Move scale into our new matrix
	//
	//      x     y     z
	//   |-----|-----|-----|-----|
	// x | sx  |     |     |     |
	//   |-----|-----|-----|-----|
	// y |     | sy  |     |     |
	//   |-----|-----|-----|-----|
	// z |     |     | sz  |     |
	//   |-----|-----|-----|-----|
	//   |     |     |     |  1  |
	//   |-----|-----|-----|-----|
	//
	mat[0][0] = scale[0];
	mat[1][1] = scale[1];
	mat[2][2] = scale[2];
}

void identity(Matrix mat)
{
	int i, j;
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			//mat[i][j] = 0.0;
			//if (i==j)
			//	mat[i][j] = 1.0;
			mat[i][j] = (i==j) ? 1.0 : 0.0;
		}
	}
}

void matrix_cat(Matrix m1, Matrix m2, Matrix dest)
{
	//concatenate m1 with m2
	int i, j, k;
	Matrix m3;
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			m3[i][j] = 0;
			for (k=0; k<4; k++) {
				m3[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			dest[i][j] = m3[i][j];
		}
	}
}

void trans_vector(Matrix mat, Vec in, Vec out)
{
	//transform a vector by the matrix, mat
	Flt t, f[4];
	f[0] = mat[0][0]*in[0] + mat[1][0]*in[1] + mat[2][0]*in[2] + mat[3][0];
	f[1] = mat[0][1]*in[0] + mat[1][1]*in[1] + mat[2][1]*in[2] + mat[3][1];
	f[2] = mat[0][2]*in[0] + mat[1][2]*in[1] + mat[2][2]*in[2] + mat[3][2];
	f[3] = mat[0][3]*in[0] + mat[1][3]*in[1] + mat[2][3]*in[2] + mat[3][3];
	t = 1.0 / f[3];
	out[0] = f[0] * t;
	out[1] = f[1] * t;
	out[2] = f[2] * t;
}

void trans_normal(Matrix mat, Vec in, Vec out)
{
	//transform a normal
	//Do not apply any translation
	//int i;
	Vec temp;
	//for (i=0; i<3; i++) {
	//	temp[i] = in[0] * mat[0][i] +
	//			  in[1] * mat[1][i] +
	//			  in[2] * mat[2][i];
	//}
	//for (i=0; i<3; i++) {
	//	out[i] = temp[i];
	//}
	temp[0] = in[0] * mat[0][0] + in[1] * mat[1][0] + in[2] * mat[2][0];
	temp[1] = in[0] * mat[0][1] + in[1] * mat[1][1] + in[2] * mat[2][1];
	temp[2] = in[0] * mat[0][2] + in[1] * mat[1][2] + in[2] * mat[2][2];
	out[0] = temp[0];
	out[1] = temp[1];
	out[2] = temp[2];
}


void matrix_inverse(Matrix in, Matrix out)
{
	//source
	//Book: Photorealism And Ray Tracing In C
	//
	Flt d00, d01, d02, d03;
	Flt d10, d11, d12, d13;
	Flt d20, d21, d22, d23;
	Flt d30, d31, d32, d33;
	Flt m00, m01, m02, m03;
	Flt m10, m11, m12, m13;
	Flt m20, m21, m22, m23;
	Flt m30, m31, m32, m33;
	Flt d;
	//
	m00 = in[0][0];  m01 = in[0][1];  m02 = in[0][2];  m03 = in[0][3];
	m10 = in[1][0];  m11 = in[1][1];  m12 = in[1][2];  m13 = in[1][3];
	m20 = in[2][0];  m21 = in[2][1];  m22 = in[2][2];  m23 = in[2][3];
	m30 = in[3][0];  m31 = in[3][1];  m32 = in[3][2];  m33 = in[3][3];
	//
	d00 = m11*m22*m33 + m12*m23*m31 + m13*m21*m32 -
		m31*m22*m13 - m32*m23*m11 - m33*m21*m12;
	d01 = m10*m22*m33 + m12*m23*m30 + m13*m20*m32 -
		m30*m22*m13 - m32*m23*m10 - m33*m20*m12;
	d02 = m10*m21*m33 + m11*m23*m30 + m13*m20*m31 -
		m30*m21*m13 - m31*m23*m10 - m33*m20*m11;
	d03 = m10*m21*m32 + m11*m22*m30 + m12*m20*m31 -
		m30*m21*m12 - m31*m22*m10 - m32*m20*m11;
	//
	d10 = m01*m22*m33 + m02*m23*m31 + m03*m21*m32 -
		m31*m22*m03 - m32*m23*m01 - m33*m21*m02;
	d11 = m00*m22*m33 + m02*m23*m30 + m03*m20*m32 -
		m30*m22*m03 - m32*m23*m00 - m33*m20*m02;
	d12 = m00*m21*m33 + m01*m23*m30 + m03*m20*m31 -
		m30*m21*m03 - m31*m23*m00 - m33*m20*m01;
	d13 = m00*m21*m32 + m01*m22*m30 + m02*m20*m31 -
		m30*m21*m02 - m31*m22*m00 - m32*m20*m01;
	//
	d20 = m01*m12*m33 + m02*m13*m31 + m03*m11*m32 -
		m31*m12*m03 - m32*m13*m01 - m33*m11*m02;
	d21 = m00*m12*m33 + m02*m13*m30 + m03*m10*m32 -
		m30*m12*m03 - m32*m13*m00 - m33*m10*m02;
	d22 = m00*m11*m33 + m01*m13*m30 + m03*m10*m31 -
		m30*m11*m03 - m31*m13*m00 - m33*m10*m01;
	d23 = m00*m11*m32 + m01*m12*m30 + m02*m10*m31 -
		m30*m11*m02 - m31*m12*m00 - m32*m10*m01;
	//
	d30 = m01*m12*m23 + m02*m13*m21 + m03*m11*m22 -
		m21*m12*m03 - m22*m13*m01 - m23*m11*m02;
	d31 = m00*m12*m23 + m02*m13*m20 + m03*m10*m22 -
		m20*m12*m03 - m22*m13*m00 - m23*m10*m02;
	d32 = m00*m11*m23 + m01*m13*m20 + m03*m10*m21 -
		m20*m11*m03 - m21*m13*m00 - m23*m10*m01;
	d33 = m00*m11*m22 + m01*m12*m20 + m02*m10*m21 -
		m20*m11*m02 - m21*m12*m00 - m22*m10*m01;
	//
	d = m00*d00 - m01*d01 + m02*d02 - m03*d03;
	//
	if (d == 0.0) {
		Log("ERROR - matrix_inverse() Singular matrix.\n");
	}
	//
	d = 1.0 / d;
	out[0][0] =  d00 * d;
	out[0][1] = -d10 * d;
	out[0][2] =  d20 * d;
	out[0][3] = -d30 * d;
	out[1][0] = -d01 * d;
	out[1][1] =  d11 * d;
	out[1][2] = -d21 * d;
	out[1][3] =  d31 * d;
	out[2][0] =  d02 * d;
	out[2][1] = -d12 * d;
	out[2][2] =  d22 * d;
	out[2][3] = -d32 * d;
	out[3][0] = -d03 * d;
	out[3][1] =  d13 * d;
	out[3][2] = -d23 * d;
	out[3][3] =  d33 * d;
}

void make_view_matrix(Vec p1, Vec p2, Matrix m)
{
	Flt d, len, l1;
	Vec up, out, left;
	Vec upv={0,1,0};

	//Log( "make_view_matrix()...\n");
	//Line between p1 and p2 form a LOS Line-of-sight.
	//A rotation matrix is built to transform objects to this LOS.
	//
	//Credit: Diana Gruber
	//http://www.makegames.com/3Drotation/
	//
	m[0][0]=1.0; m[0][1]=0.0; m[0][2] =0.0;
	m[1][0]=0.0; m[1][1]=1.0; m[1][2] =0.0;
	m[2][0]=0.0; m[2][1]=0.0; m[2][2] =1.0;
	VecSub(p2, p1, out);
	//
	len = out[0]*out[0] + out[1]*out[1] + out[2]*out[2];
	if (len == 0.0) {
		MakeVector(0.0,0.0,1.0,out);
	}
	else {
		l1 = 1.0 / sqrt(len);
		out[0] *= l1;
		out[1] *= l1;
		out[2] *= l1;
	}
	//
	m[2][0] = out[0];
	m[2][1] = out[1];
	m[2][2] = out[2];
	//
	d = out[0] * upv[0] + out[1] * upv[1] + out[2] * upv[2]; 
	up[0] = upv[0] - d * out[0];
	up[1] = upv[1] - d * out[1];
	up[2] = upv[2] - d * out[2];
	len = up[0]*up[0] + up[1]*up[1] + up[2]*up[2];
	if (len == 0.0) {
		MakeVector(0, 0, 1, up);
	} else {
		l1 = 1.0 / sqrt(len);
		up[0] *= l1;
		up[1] *= l1;
		up[2] *= l1;
	}
	m[1][0] = up[0];
	m[1][1] = up[1];
	m[1][2] = up[2];
	//make left vector.
	VecCross(up, out, left);
	m[0][0] = left[0];
	m[0][1] = left[1];
	m[0][2] = left[2];
}


