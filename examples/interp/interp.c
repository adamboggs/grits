#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(a,min,max) MIN(MAX((a),(min)),(max))

/* Print functions */
void print(float in)
{
	printf("%7.4f ", in);
}
void print1(float *in, int x)
{
	for (int i = 0; i < x; i++)
		print(in[i]);
	printf("\n");
}
void print2(void *_data, int x, int y)
{
	float (*data)[x] = _data;
	for (int yi=0; yi < y; yi++)
		print1(data[yi], x);
	printf("\n");
}

/*
 * y    ,+---+---+---+---+ 
 * |  ,+---+---+---+---+'|   z
 *  ,+---+'| 0 | 0 | 0 | + .' 
 * +---+'|,+---+---+---+'|
 * | 0 | +---+---+'| 0 | +
 * +---+'| 1 | 1 |,+---+'|
 * | 0 |,+---+---+---+'| +
 * +---+---+---+---+'| +'
 * | 0 | 0 | 0 | 0 | +'
 * +---+---+---+---+' -- x
 */

/* Nearest neighbor interpolation */
void nearest2(void *_in, void *_out, int x, int y, int s)
{
	float (*in )[y*1] = _in;
	float (*out)[y*s] = _out;
	for (int yi=0; yi < y*s; yi++)
	for (int xi=0; xi < x*s; xi++) {
		out[xi][yi] = in[xi/s][yi/s];
	}
}

/* Linear interpolation */
void linear2(void *_in, void *_out, int x, int y, int s)
{
	float (*in )[y*1] = _in;
	float (*out)[y*s] = _out;
	for (int yi=s/2; yi+s/2 < y*s; yi++)
	for (int xi=s/2; xi+s/2 < x*s; xi++) {
		float xf  = (xi+0.5)/s;
		float yf  = (yi+0.5)/s;
		int   xl  = xf - 0.5;
		int   xh  = xf + 0.5;
		int   yl  = yf - 0.5;
		int   yh  = yf + 0.5;
		float f00 = in[xl][yl];
		float f01 = in[xl][yh];
		float f10 = in[xh][yl];
		float f11 = in[xh][yh];
		float xs  = xf - (xl+0.5);
		float ys  = yf - (yl+0.5);
		out[xi][yi] =
			f00*(1-xs)*(1-ys) +
			f01*(1-xs)*(  ys) +
			f10*(  xs)*(1-ys) +
			f11*(  xs)*(  ys);
	}
}

/**
 * Cubic interpolation (Catmull–Rom)
 *
 *        val[k+1] - val[k-1]    val[k+1] - val[k-1]
 * m[k] = ------------------- == -------------------
 *        pos[k+1] - pos[k-1]             2
 *
 * pl-.          .-p1-.
 *     '.      o'      '-.
 *       '-p0-'|          'ph 
 *          '--t---'         '-.
 */
float cubic(float t, float pl, float p0, float p1, float ph)
{
	float m0 = (p1-pl)/2;
	float m1 = (ph-p0)/2;
	//printf("%5.2f %5.2f   %5.2f %5.2f - %5.2f", p0, p1, m0, m1, t);
	return (1+2*t) * (1-t) *  (1-t)  * p0 +
	          t    * (1-t) *  (1-t)  * m0 +
	          t    *   t   * (3-2*t) * p1 +
	          t    *   t   *  (t-1)  * m1;
}

void cubic1(float *in, float *out, int x, int s)
{
	for (int xi=0; xi < x*s; xi++) {
		float xf  = (xi+0.5)/s;
		int   xll = MAX(xf - 1.5, 0  );
		int   xl  = MAX(xf - 0.5, 0  );
		int   xh  = MIN(xf + 0.5, x-1);
		int   xhh = MIN(xf + 1.5, x-1);
		float xt  = (xf+0.5) - (int)(xf+0.5);
		//printf("%2d:  %d %d %d %d - %6.3f   | ", xi, xll, xl, xh, xhh, xt);
		out[xi] = cubic(xt, in[xll], in[xl], in[xh], in[xhh]);
		//printf("   | %5.2f\n", out[xi]);
		//    0        1        1        0
		// 0 0 0 0  1 1 1 1  1 1 1 1  0 0 0 0
		// 0 1 2 3  4 5 6 7  8 9 a b  c d e f
		//          .
	}
}

void cubic2(void *_in, void *_out, int x, int y, int s)
{
	float (*in )[x*1] = _in;
	float (*out)[x*s] = _out;

	/* Interpolate in y direction */
	for (int yi=0; yi < y; yi++)
		cubic1(in[yi], out[yi*s], x, s);

	/* Interpolate in x direction */
	/* Todo: use a stride, etc instead of copying */
	float yin [y*1];
	float yout[y*s];
	for (int xi=0; xi < x*s; xi++) {
		for (int yi = 0; yi < y; yi++)
			yin[yi] = out[yi*s][xi];
		cubic1(yin, yout, y, s);
		for (int yi = 0; yi < y*s; yi++)
			out[yi][xi] = yout[yi];
	}
}


int main()
{
	//float orig  [ 3][ 4] = {};
	//float smooth[15][20] = {};

	//orig[1][1] = 1;
	//orig[1][2] = 1;

	//binearest(orig, smooth, 4, 3, 4);
	//printf("Binearest:\n");
	//print(orig,    4,  3);
	//print(smooth, 16, 12);

	//bilinear(orig, smooth, 4, 3, 4);
	//printf("Bilinear:\n");
	//print(orig,    4,  3);
	//print(smooth, 16, 12);

	//printf("cubic 2:\n");
	//cubic2(orig, smooth, 4, 3, 5);
	//print2(orig,    4,  3);
	//print2(smooth, 20, 15);

	//float lorig  [4]  = {0, 1, 1, 0};
	//float lsmooth[16] = {};
	//printf("Cubic1:\n");
	//cubic1(lorig, lsmooth, 4, 4);
	//print1(lorig,   4 );
	//print1(lsmooth, 16);


	/* Test with matlab */
	//float smooth[80][80] = {};
	//float orig  [ 4][ 4] =
	//	{{1,2,4,1},
	//	 {6,3,5,2},
	//	 {4,2,1,5},
	//	 {5,4,2,3}};
	//cubic2(orig, smooth, 4, 4, 20);
	//print2(smooth, 80, 80);
	// data = textread('<output>');
	// [xs ys] = meshgrid(linspace(0,3,size(data,2)), linspace(0,3,size(data,1)));
	// surf(xs, ys, data);
	// shading flat;
	// colormap(jet);
	// view(0, 90)

	/* interpolatecolormaps */
	float smooth[3][80] = {};
	float orig  [3][16] =
		{{0x00, 0x04, 0x01, 0x03,  0x02, 0x01, 0x00, 0xfd,
		  0xe5, 0xfd, 0xfd, 0xd4,  0xbc, 0xf8, 0x98, 0xfd},
		 {0x00, 0xe9, 0x9f, 0x00,  0xfd, 0xc5, 0x8e, 0xf8,
		  0xbc, 0x95, 0x00, 0x00,  0x00, 0x00, 0x54, 0xfd},
		 {0x00, 0xe7, 0xf4, 0xf4,  0x02, 0x01, 0x00, 0x02,
		  0x00, 0x00, 0x00, 0x00,  0x00, 0xfd, 0xc6, 0xfd}};
	cubic1(orig[0], smooth[0], 16, 5);
	cubic1(orig[1], smooth[1], 16, 5);
	cubic1(orig[2], smooth[2], 16, 5);
	for (int i = 0; i < 80; i++)
		printf("{0x%02x,0x%02x,0x%02x}\n",
			(uint8_t)round(CLAMP(smooth[0][i], 0x00, 0xff)),
			(uint8_t)round(CLAMP(smooth[1][i], 0x00, 0xff)),
			(uint8_t)round(CLAMP(smooth[2][i], 0x00, 0xff)));
}
