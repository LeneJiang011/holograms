#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <windows.h>

#include <fftw3.h>
#pragma comment(lib, "libfftw3-3.lib")
#pragma comment(lib, "libfftw3f-3.lib")
#pragma comment(lib, "libfftw3l-3.lib")

#define sqr(x) (x)*(x)

//画像のパラメータ
#define DATA_NAME "cube284.3d"//入力画像
#define RESULT_NAME "reCGH.bmp"//出力画像
#define IMAGE_Y 1080
#define IMAGE_X 1920

//入力画像を格納する配列
static unsigned char Image[IMAGE_Y*IMAGE_X];

double fin[IMAGE_Y*IMAGE_X];
double U1[IMAGE_Y*IMAGE_X];
double U2[IMAGE_Y*IMAGE_X];

//パワースペクトルを格納する配列
static double g[IMAGE_Y][IMAGE_X];

//出力画像を格納する配列
static unsigned char result[IMAGE_Y][IMAGE_X];

//象限変換に必要な配列
static double outr[IMAGE_Y][IMAGE_X], outi[IMAGE_Y][IMAGE_X];
static double buff1[IMAGE_Y][IMAGE_X], buff2[IMAGE_Y][IMAGE_X];
static double outr_u[IMAGE_Y][IMAGE_X], outi_u[IMAGE_Y][IMAGE_X];
static double buff1_u[IMAGE_Y][IMAGE_X], buff2_u[IMAGE_Y][IMAGE_X];
static double outr_h[IMAGE_Y][IMAGE_X], outi_h[IMAGE_Y][IMAGE_X];
static double buff1_h[IMAGE_Y][IMAGE_X], buff2_h[IMAGE_Y][IMAGE_X];

double x_h[IMAGE_X], y_h[IMAGE_Y];

int main()
{
	int n, m, i, number,color[1024];
	FILE *fp1, *fp2;

	double lambda = 520.0*pow(10.0, (-9));
	double k = (2 * M_PI) / lambda;
	double p_u = 8.0 * pow(10.0, -6);

	BITMAPFILEHEADER BmpFileHeader;
	BITMAPINFOHEADER BmpInfoHeader;
	RGBQUAD			 RGBQuad[256];

	fp1 = fopen(DATA_NAME, "rb");

	int x_u[284], y_u[284];
	double z_u[284];
	int num[284 * 3 + 1];

	fread(num, sizeof(int), 284 * 3 + 1, fp1);
	fread(&BmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp1);
	fread(&BmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp1);
	fread(&color, 1024, 1, fp1);

	for (i = 0; i < 284; i++){
		x_u[i] = num[3 * i + 1] * 40 + 960;
		y_u[i] = num[3 * i + 2] * 40 + 540;
		z_u[i] = num[3 * i + 3] * 40 + 1 / p_u;
	}

	fclose(fp1);

	fftw_complex *in_u, *out_u, *in_h, *out_h, *in, *out;
	fftw_plan p;

	/* 領域の確保 */
	in_u = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * sizeof(fftw_complex));
	out_u = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * sizeof(fftw_complex));
	in_h = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * sizeof(fftw_complex));
	out_h = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * sizeof(fftw_complex));
	in = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * sizeof(fftw_complex));
	out = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * sizeof(fftw_complex));


	/* 入力データの格納 */
	for (n = 0; n < IMAGE_Y; n++){
		for (m = 0; m < IMAGE_X; m++){
			for (number = 0; number < num[0]; number++){
				U1[m + n*IMAGE_X] += sin(k*p_u*sqrt(sqr(m - x_u[number]) + sqr(n - y_u[number]) + sqr(z_u[number])));
				U2[m + n*IMAGE_X] += cos(k*p_u*sqrt(sqr(m - x_u[number]) + sqr(n - y_u[number]) + sqr(z_u[number])));
			}
			fin[m + n*IMAGE_X] = atan2(U1[m + n*IMAGE_X], U2[m + n*IMAGE_X]);
		}
	}

	double maximum, minimum, mid;
	maximum = fin[0];
	minimum = fin[0];
	for (n = 0; n < IMAGE_Y; n++){
		for (m = 0; m < IMAGE_X; m++){
			if (fin[m + n*IMAGE_X] < minimum)
				minimum = fin[m + n*IMAGE_X];
			else if (fin[m + n*IMAGE_X] > maximum)
				maximum = fin[m + n*IMAGE_X];
		}
	}
	mid = (maximum + minimum) / 2;

	for (n = 0; n < IMAGE_Y; n++){
		for (m = 0; m < IMAGE_X; m++){
			if (fin[m + n*IMAGE_X] < mid)
				Image[m + n*IMAGE_X] = 0;
			else if (fin[m + n*IMAGE_X] > mid)
				Image[m + n*IMAGE_X] = 255;
		}
	}

	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			in_u[m + n*IMAGE_X][0] = Image[m + n*IMAGE_X];
			in_u[m + n*IMAGE_X][1] = 0.0;
		}
	}

	/* plan作成 */
	p = fftw_plan_dft_2d(IMAGE_Y, IMAGE_X, in_u, out_u, FFTW_FORWARD, FFTW_ESTIMATE);

	/* FFT実行 */
	fftw_execute(p);

	/* 正規化 */
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			out_u[m + n*IMAGE_X][0] /= (IMAGE_Y*IMAGE_X);
			out_u[m + n*IMAGE_X][1] /= (IMAGE_Y*IMAGE_X);
		}
	}

	/* 象限変換 */
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			outr_u[n][m] = out_u[m + n*IMAGE_X][0];
			outi_u[n][m] = out_u[m + n*IMAGE_X][1];
		}
	}
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			buff1_u[n][m] = outr_u[n][m];
			buff2_u[n][m] = outi_u[n][m];
		}
	}
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			if (m<IMAGE_X / 2  && n<IMAGE_Y / 2){
				outr_u[n][m] = outr_u[n + IMAGE_Y / 2][m + IMAGE_X / 2];
				outr_u[n + IMAGE_Y / 2][m + IMAGE_X / 2] = buff1_u[n][m];
				outi_u[n][m] = outi_u[n + IMAGE_Y / 2][m + IMAGE_X / 2];
				outi_u[n + IMAGE_Y / 2][m + IMAGE_X / 2] = buff2_u[n][m];
			}
			else if (m >= IMAGE_X / 2 && n<IMAGE_Y / 2){
				outr_u[n][m] = outr_u[n + IMAGE_Y / 2][m - IMAGE_X / 2];
				outr_u[n + IMAGE_Y / 2][m - IMAGE_X / 2] = buff1_u[n][m];
				outi_u[n][m] = outi_u[n + IMAGE_Y / 2][m - IMAGE_X / 2];
				outi_u[n + IMAGE_Y / 2][m - IMAGE_X / 2] = buff2_u[n][m];
			}
		}
	}

	double pitch_x = 8.0*pow(10.0, (-6));
	double pitch_y = 8.0*pow(10.0, (-6));
	double z = 1;

	/* 入力データの格納 */
	for (n = 0; n < IMAGE_Y; n++){
		y_h[n] = ((-1)*IMAGE_Y / 2 + n)*pitch_y;
	}
	for (m = 0; m < IMAGE_X; m++){
		x_h[m] = ((-1)*IMAGE_X / 2 + m)*pitch_x;
	}
	for (n = 0; n < IMAGE_Y; n++){
		for (m = 0; m < IMAGE_X; m++){
			in_h[m + n*IMAGE_X][0] = (1 / (lambda*z))*sin(k*z + (k / (2 * z))*(sqr(x_h[m]) + sqr(y_h[n])));
			in_h[m + n*IMAGE_X][1] = (-1)* (1 / (lambda*z))*cos(k*z + (k / (2 * z))*(sqr(x_h[m]) + sqr(y_h[n])));
		}
	}

	/* plan作成 */
	p = fftw_plan_dft_2d(IMAGE_Y, IMAGE_X, in_h, out_h, FFTW_FORWARD, FFTW_ESTIMATE);

	/* FFT実行 */
	fftw_execute(p);

	/* 正規化 */
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			out_h[m + n*IMAGE_X][0] /= (IMAGE_Y*IMAGE_X);
			out_h[m + n*IMAGE_X][1] /= (IMAGE_Y*IMAGE_X);
		}
	}

	/* 象限変換 */
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			outr_h[n][m] = out_h[m + n*IMAGE_X][0];
			outi_h[n][m] = out_h[m + n*IMAGE_X][1];
		}
	}
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			buff1_h[n][m] = outr_h[n][m];
			buff2_h[n][m] = outi_h[n][m];
		}
	}
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			if (m<IMAGE_X / 2 && n<IMAGE_Y / 2){
				outr_h[n][m] = outr_h[n + IMAGE_Y / 2][m + IMAGE_X / 2];
				outr_h[n + IMAGE_Y / 2][m + IMAGE_X / 2] = buff1_h[n][m];
				outi_h[n][m] = outi_h[n + IMAGE_Y / 2][m + IMAGE_X / 2];
				outi_h[n + IMAGE_Y / 2][m + IMAGE_X / 2] = buff2_h[n][m];
			}
			else if (m >= IMAGE_X / 2 && n<IMAGE_Y / 2){
				outr_h[n][m] = outr_h[n + IMAGE_Y / 2][m - IMAGE_X / 2];
				outr_h[n + IMAGE_Y / 2][m - IMAGE_X / 2] = buff1_h[n][m];
				outi_h[n][m] = outi_h[n + IMAGE_Y / 2][m - IMAGE_X / 2];
				outi_h[n + IMAGE_Y / 2][m - IMAGE_X / 2] = buff2_h[n][m];
			}
		}
	}


	/* 入力データの格納 */
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			in[m + n*IMAGE_X][0] = outr_u[n][m] * outr_h[n][m] - outi_u[n][m] * outi_h[n][m];
			in[m + n*IMAGE_X][1] = outi_u[n][m] * outr_h[n][m] + outr_u[n][m] * outi_h[n][m];
		}
	}

	/* plan作成 */
	p = fftw_plan_dft_2d(IMAGE_Y, IMAGE_X, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

	/* FFT実行 */
	fftw_execute(p);

	/* 正規化 */
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			out[m + n*IMAGE_X][0] /= (IMAGE_Y*IMAGE_X);
			out[m + n*IMAGE_X][1] /= (IMAGE_Y*IMAGE_X);
		}
	}

	/* 象限変換 */
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			outr[n][m] = out[m + n*IMAGE_X][0];
			outi[n][m] = out[m + n*IMAGE_X][1];
		}
	}
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			buff1[n][m] = outr[n][m];
			buff2[n][m] = outi[n][m];
		}
	}
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			if (m<IMAGE_X / 2 && n<IMAGE_Y / 2){
				outr[n][m] = outr[n + IMAGE_Y / 2][m + IMAGE_X / 2];
				outr[n + IMAGE_Y / 2][m + IMAGE_X / 2] = buff1[n][m];
				outi[n][m] = outi[n + IMAGE_Y / 2][m + IMAGE_X / 2];
				outi[n + IMAGE_Y / 2][m + IMAGE_X / 2] = buff2[n][m];
			}
			else if (m >= IMAGE_X / 2 && n<IMAGE_Y / 2){
				outr[n][m] = outr[n + IMAGE_Y / 2][m - IMAGE_X / 2];
				outr[n + IMAGE_Y / 2][m - IMAGE_X / 2] = buff1[n][m];
				outi[n][m] = outi[n + IMAGE_Y / 2][m - IMAGE_X / 2];
				outi[n + IMAGE_Y / 2][m - IMAGE_X / 2] = buff2[n][m];
			}
		}
	}

	/* パワースペクトル計算 */
	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			g[n][m] = sqrt(sqr(outr[n][m]) + sqr(outi[n][m]));
		}
	}


	//256諧調化
	double out_max = 0, out_min;

	out_min = g[0][0];

	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			if (g[n][m]>out_max)
				out_max = g[n][m];
			if (g[n][m]<out_min)
				out_min = g[n][m];
		}
	}

	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			result[n][m] = (unsigned char)((255 * (g[n][m] - out_min)) / (out_max - out_min));
		}
	}

	/* plan破棄 */
	fftw_destroy_plan(p);

	/* メモリの開放 */
	fftw_free(in_u);
	fftw_free(out_u);
	fftw_free(in_h);
	fftw_free(out_h);
	fftw_free(in);
	fftw_free(out);

	BmpFileHeader.bfType = 19778;
	BmpFileHeader.bfSize = 14 + 40 + IMAGE_Y*IMAGE_X;
	BmpFileHeader.bfReserved1 = 0;
	BmpFileHeader.bfReserved2 = 0;
	BmpFileHeader.bfOffBits = 14 + 40 + 1024;

	BmpInfoHeader.biSize = 40;
	BmpInfoHeader.biWidth = IMAGE_X;
	BmpInfoHeader.biHeight = IMAGE_Y;
	BmpInfoHeader.biPlanes = 1;
	BmpInfoHeader.biBitCount = 8;
	BmpInfoHeader.biCompression = 0L;
	BmpInfoHeader.biSizeImage = 0L;
	BmpInfoHeader.biXPelsPerMeter = 0L;
	BmpInfoHeader.biYPelsPerMeter = 0L;
	BmpInfoHeader.biClrUsed = 0L;
	BmpInfoHeader.biClrImportant = 0L;

	for (i = 0; i<256; i++){
		RGBQuad[i].rgbBlue = i;
		RGBQuad[i].rgbGreen = i;
		RGBQuad[i].rgbRed = i;
		RGBQuad[i].rgbReserved = 0;
	}

	fp2 = fopen(RESULT_NAME, "wb");

	fwrite(&BmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp2);
	fwrite(&BmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp2);

	for (i = 0; i<256; i++)
		fwrite(&RGBQuad[i], sizeof(RGBQUAD), 1, fp2);

	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			fwrite(&result[n][m], 1, 1, fp2);
		}
	}

	fclose(fp1);
	fclose(fp2);

	return 0;
}



