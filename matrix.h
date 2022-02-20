#ifndef _VECTOR_H_
#define _VECTOR_H_

typedef float Flt;
typedef Flt Vec[3];
typedef Flt Matrix[4][4];
#define PI 3.14159265358979323846264338327950288419716939937510582097494459230
#define degrees_to_radians(d)  ((d) * PI / 180.0)
//some macros to for vectors
#define MakeVector(x, y, z, v) (v)[0] = (x), \
							   (v)[1] = (y), \
							   (v)[2] = (z)

#define VecZero(v) (v)[0] = 0.0; \
				   (v)[1] = 0.0; \
				   (v)[2] = 0.0

#define VecNegate(a) (a)[0] = (-(a)[0]); \
					 (a)[1] = (-(a)[1]); \
					 (a)[2] = (-(a)[2])

#define VecDot(a,b) ((a)[0]*(b)[0]+ \
					 (a)[1]*(b)[1]+ \
					 (a)[2]*(b)[2])

#define VecLen(a) ((a)[0] * (a)[0] + \
				   (a)[1] * (a)[1] + \
				   (a)[2] * (a)[2])

#define VecLenSq(a) sqrt(                 \
						(a)[0] * (a)[0] + \
						(a)[1] * (a)[1] + \
						(a)[2] * (a)[2])

#define VecCopy(a,b) (b)[0]=(a)[0]; \
					 (b)[1]=(a)[1]; \
					 (b)[2]=(a)[2]

#define VecCopyNeg(a,b) (b)[0] = -(a)[0]; \
						(b)[1] = -(a)[1]; \
						(b)[2] = -(a)[2]

#define VecAdd(a,b,c) (c)[0] = (a)[0] + (b)[0]; \
					  (c)[1] = (a)[1] + (b)[1]; \
					  (c)[2] = (a)[2] + (b)[2]

#define VecSub(a,b,c) (c)[0] = (a)[0] - (b)[0]; \
					  (c)[1] = (a)[1] - (b)[1]; \
					  (c)[2] = (a)[2] - (b)[2]

#define VecS(A,a,b) (b)[0] = (A) * (a)[0]; \
					(b)[1] = (A) * (a)[1]; \
					(b)[2] = (A) * (a)[2]

#define VecAddS(A,a,b,c) (c)[0] = (A) * (a)[0] + (b)[0]; \
						 (c)[1] = (A) * (a)[1] + (b)[1]; \
						 (c)[2] = (A) * (a)[2] + (b)[2]

#define VecCross(a,b,c) (c)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1]; \
						(c)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2]; \
						(c)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0]

//This macro combines 2 vectors
#define VecComb(A,a,B,b,c) (c)[0] = (A) * (a)[0] + (B) * (b)[0]; \
						   (c)[1] = (A) * (a)[1] + (B) * (b)[1]; \
						   (c)[2] = (A) * (a)[2] + (B) * (b)[2]

//Absolute values
#define VecABS(v) (v)[0] = ((v)[0] < 0.0) ? -(v)[0] : (v)[0]; \
				  (v)[1] = ((v)[1] < 0.0) ? -(v)[1] : (v)[1]; \
				  (v)[2] = ((v)[2] < 0.0) ? -(v)[2] : (v)[2]

#define ClampRGB(rgb) (rgb)[0]=((rgb)[0]>1.0f)?1.0f:(rgb)[0]; \
					  (rgb)[1]=((rgb)[1]>1.0f)?1.0f:(rgb)[1]; \
					  (rgb)[2]=((rgb)[2]>1.0f)?1.0f:(rgb)[2]

//extern void VecNormalize(Vec vec);
//extern Flt VecNormalizeL(Vec vec);
//extern void make_unit_normal_from_verts(Vec tp[], Vec norm);
//extern void make_unit_normal_from_verts3(Vec p0, Vec p1, Vec p2, Vec norm);
//extern void make_normal_from_verts(Vec tp[], Vec norm);
extern void identity(Matrix mat);
extern void matrix_cat(Matrix m1, Matrix m2, Matrix dest);
extern void trans_vector(Matrix mat, Vec in, Vec out);
extern void trans_normal(Matrix mat, Vec in, Vec out);
//extern void concatenate_to_matrix(Transform *t);
extern void concatenate_to_matrix(Matrix mat, Matrix dest);
extern void matrix_inverse(Matrix in, Matrix out);
extern void matrix_copy(Matrix m1, Matrix m2);
extern void make_view_matrix(Vec p1, Vec p2, Matrix m);
extern void translate(Vec translate, Matrix mat);
extern void rotate(Vec rot, Matrix mat);
extern void scale(Vec scale, Matrix mat);

#endif //_VECTOR_H_

