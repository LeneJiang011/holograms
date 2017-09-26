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
#define DATA_NAME "rect.bmp"//入力画像
#define RESULT_NAME "rect_fourier.png"//出力画像
#define IMAGE_Y 512
#define IMAGE_X 512

//入力画像を格納する配列
static unsigned char Image[IMAGE_Y][IMAGE_X];
static unsigned char tImage[IMAGE_Y * 2][IMAGE_X * 2];

//パワースペクトルを格納する配列
static double g[IMAGE_Y * 2][IMAGE_X * 2];

//出力画像を格納する配列
static unsigned char result[IMAGE_Y * 2][IMAGE_X * 2];
static unsigned char trimming[IMAGE_Y][IMAGE_X];

//象限変換に必要な配列
static double outr[IMAGE_Y * 2][IMAGE_X * 2], outi[IMAGE_Y * 2][IMAGE_X * 2];
static double buff1[IMAGE_Y * 2][IMAGE_X * 2], buff2[IMAGE_Y * 2][IMAGE_X * 2];
static double outr_u[IMAGE_Y * 2][IMAGE_X * 2], outi_u[IMAGE_Y * 2][IMAGE_X * 2];
static double buff1_u[IMAGE_Y * 2][IMAGE_X * 2], buff2_u[IMAGE_Y * 2][IMAGE_X * 2];
static double outr_h[IMAGE_Y * 2][IMAGE_X * 2], outi_h[IMAGE_Y * 2][IMAGE_X * 2];
static double buff1_h[IMAGE_Y * 2][IMAGE_X * 2], buff2_h[IMAGE_Y * 2][IMAGE_X * 2];

double x[IMAGE_X*2], y[IMAGE_Y*2];


int main()
{
	int n, m, i, color[1024];
	FILE *fp;

	BITMAPFILEHEADER BmpFileHeader;
	BITMAPINFOHEADER BmpInfoHeader;
	RGBQUAD			 RGBQuad[256];

	fp = fopen(DATA_NAME, "rb");

	fread(&BmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&BmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(&color, 1024, 1, fp);

	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			fread(&Image[n][m], 1, 1, fp);
		}
	}

	fclose(fp);

	/*zero-padding*/
	for (n = 0; n < IMAGE_Y * 2; n++){
		for (m = 0; m < IMAGE_X * 2; m++){
			tImage[n][m] = 0;
		}
	}
	for (n = 0; n < IMAGE_Y; n++){
		for (m = 0; m < IMAGE_X; m++){
			tImage[n + IMAGE_Y / 2][m + IMAGE_X / 2] = Image[n][m];
		}
	}

	fftw_complex *in_u, *out_u, *in_h, *out_h, *in, *out;
	fftw_plan p;

    /* 領域の確保 */
	in_u = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * 4 * sizeof(fftw_complex));
	out_u = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * 4 * sizeof(fftw_complex));
	in_h = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * 4 * sizeof(fftw_complex));
	out_h = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * 4 * sizeof(fftw_complex));
	in = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * 4 * sizeof(fftw_complex));
	out = (fftw_complex*)fftw_malloc(IMAGE_Y * IMAGE_X * 4 * sizeof(fftw_complex));


	/* 入力データの格納 */
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			in_u[m + n*IMAGE_X * 2][0] = tImage[n][m];
			in_u[m + n*IMAGE_X * 2][1] = 0.0;
		}
	}

	/* plan作成 */
	p = fftw_plan_dft_2d(IMAGE_Y * 2, IMAGE_X * 2, in_u, out_u, FFTW_FORWARD, FFTW_ESTIMATE);

	/* FFT実行 */
	fftw_execute(p);

	/* 正規化 */
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			out_u[m + n*IMAGE_X * 2][0] /= (IMAGE_Y*IMAGE_X * 4);
			out_u[m + n*IMAGE_X * 2][1] /= (IMAGE_Y*IMAGE_X * 4);
		}
	}

	/* 象限変換 */
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			outr_u[n][m] = out_u[m + n*IMAGE_X * 2][0];
			outi_u[n][m] = out_u[m + n*IMAGE_X * 2][1];
		}
	}
	for (n = 0; n<IMAGE_Y*2; n++){
		for (m = 0; m<IMAGE_X*2; m++){
			buff1_u[n][m] = outr_u[n][m];
			buff2_u[n][m] = outi_u[n][m];
		}
	}
	for (n = 0; n<IMAGE_Y*2; n++){
		for (m = 0; m<IMAGE_X*2; m++){
			if (m<IMAGE_X  && n<IMAGE_Y ){
				outr_u[n][m] = outr_u[n + IMAGE_Y][m + IMAGE_X];
				outr_u[n + IMAGE_Y][m + IMAGE_X] = buff1_u[n][m];
				outi_u[n][m] = outi_u[n + IMAGE_Y][m + IMAGE_X];
				outi_u[n + IMAGE_Y][m + IMAGE_X] = buff2_u[n][m];
			}
			else if (m >=IMAGE_X && n<IMAGE_Y){
				outr_u[n][m] = outr_u[n + IMAGE_Y][m - IMAGE_X];
				outr_u[n + IMAGE_Y][m - IMAGE_X] = buff1_u[n][m];
				outi_u[n][m] = outi_u[n + IMAGE_Y][m - IMAGE_X];
				outi_u[n + IMAGE_Y][m - IMAGE_X] = buff2_u[n][m];
			}
		}
	}

	double pitch_x = 8.0*pow(10.0, (-6));
	double pitch_y = 8.0*pow(10.0, (-6));
	double lambda = 532.0*pow(10.0, (-9));
	double k = (2 * M_PI) / lambda;
	double z = 0.5;

	/* 入力データの格納 */
	for (n = 0; n < IMAGE_Y * 2; n++){
		y[n] = ((-1)*IMAGE_Y + n)*pitch_y;
	}
	for (m = 0; m < IMAGE_X * 2; m++){
		x[m] = ((-1)*IMAGE_X + m)*pitch_x;
	}
	for (n = 0; n < IMAGE_Y * 2; n++){
		for (m = 0; m < IMAGE_X * 2; m++){
			in_h[m + n*IMAGE_X * 2][0] = (1 / (lambda*z))*sin(k*z + (k / (2 * z))*(sqr(x[m]) + sqr(y[n])));
			in_h[m + n*IMAGE_X * 2][1] = (-1)* (1 / (lambda*z))*cos(k*z + (k / (2 * z))*(sqr(x[m]) + sqr(y[n])));
		}
	}

	/* plan作成 */
	p = fftw_plan_dft_2d(IMAGE_Y * 2, IMAGE_X * 2, in_h, out_h, FFTW_FORWARD, FFTW_ESTIMATE);

	/* FFT実行 */
	fftw_execute(p);

	/* 正規化 */
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			out_h[m + n*IMAGE_X * 2][0] /= (IMAGE_Y*IMAGE_X * 4);
			out_h[m + n*IMAGE_X * 2][1] /= (IMAGE_Y*IMAGE_X * 4);
		}
	}

	/* 象限変換 */
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			outr_h[n][m] = out_h[m + n*IMAGE_X * 2][0];
			outi_h[n][m] = out_h[m + n*IMAGE_X * 2][1];
		}
	}
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			buff1_h[n][m] = outr_h[n][m];
			buff2_h[n][m] = outi_h[n][m];
		}
	}
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			if (m<IMAGE_X  && n<IMAGE_Y){
				outr_h[n][m] = outr_h[n + IMAGE_Y][m + IMAGE_X];
				outr_h[n + IMAGE_Y][m + IMAGE_X] = buff1_h[n][m];
				outi_h[n][m] = outi_h[n + IMAGE_Y][m + IMAGE_X];
				outi_h[n + IMAGE_Y][m + IMAGE_X] = buff2_h[n][m];
			}
			else if (m >= IMAGE_X && n<IMAGE_Y){
				outr_h[n][m] = outr_h[n + IMAGE_Y][m - IMAGE_X];
				outr_h[n + IMAGE_Y][m - IMAGE_X] = buff1_h[n][m];
				outi_h[n][m] = outi_h[n + IMAGE_Y][m - IMAGE_X];
				outi_h[n + IMAGE_Y][m - IMAGE_X] = buff2_h[n][m];
			}
		}
	}


	/* 入力データの格納 */
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			in[m + n*IMAGE_X * 2][0] = outr_u[n][m] * outr_h[n][m] - outi_u[n][m] * outi_h[n][m];
			in[m + n*IMAGE_X * 2][1] = outi_u[n][m] * outr_h[n][m] + outr_u[n][m] * outi_h[n][m];
		}
	}

	/* plan作成 */
	p = fftw_plan_dft_2d(IMAGE_Y * 2, IMAGE_X * 2, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

	/* FFT実行 */
	fftw_execute(p);

	/* 正規化 */
	for (n = 0; n<IMAGE_Y*2; n++){
		for (m = 0; m<IMAGE_X*2; m++){
			out[m + n*IMAGE_X*2][0] /= (IMAGE_Y*IMAGE_X*4);
			out[m + n*IMAGE_X*2][1] /= (IMAGE_Y*IMAGE_X*4);
		}
	}

	/* 象限変換 */
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			outr[n][m] = out[m + n*IMAGE_X * 2][0];
			outi[n][m] = out[m + n*IMAGE_X * 2][1];
		}
	}
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			buff1[n][m] = outr[n][m];
			buff2[n][m] = outi[n][m];
		}
	}
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			if (m<IMAGE_X  && n<IMAGE_Y){
				outr[n][m] = outr[n + IMAGE_Y][m + IMAGE_X];
				outr[n + IMAGE_Y][m + IMAGE_X] = buff1[n][m];
				outi[n][m] = outi[n + IMAGE_Y][m + IMAGE_X];
				outi[n + IMAGE_Y][m + IMAGE_X] = buff2[n][m];
			}
			else if (m >= IMAGE_X && n<IMAGE_Y){
				outr[n][m] = outr[n + IMAGE_Y][m - IMAGE_X];
				outr[n + IMAGE_Y][m - IMAGE_X] = buff1[n][m];
				outi[n][m] = outi[n + IMAGE_Y][m - IMAGE_X];
				outi[n + IMAGE_Y][m - IMAGE_X] = buff2[n][m];
			}
		}
	}

	/* パワースペクトル計算 */
	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			g[n][m] = sqrt(sqr(outr[n][m]) + sqr(outi[n][m]));
		}
	}

    /* 象限変換 */
	/*for (n = 0; n<IMAGE_Y*2; n++){
		for (m = 0; m<IMAGE_X*2; m++){
			buff1[n][m] = outr[n][m];
			buff2[n][m] = outi[n][m];
		}
	}

	for (n = 0; n<IMAGE_Y*2; n++){
		for (m = 0; m<IMAGE_X*2; m++){
			if (m<IMAGE_X  && n<IMAGE_Y ){
				outr[n][m] = outr[n + IMAGE_Y][m + IMAGE_X];
				outr[n + IMAGE_Y][m + IMAGE_X] = buff1[n][m];
				outi[n][m] = outi[n + IMAGE_Y][m + IMAGE_X];
				outi[n + IMAGE_Y][m + IMAGE_X] = buff2[n][m];
			}
			else if (m >=IMAGE_X && n<IMAGE_Y){
				outr[n][m] = outr[n + IMAGE_Y][m - IMAGE_X];
				outr[n + IMAGE_Y][m - IMAGE_X] = buff1[n][m];
				outi[n][m] = outi[n + IMAGE_Y][m - IMAGE_X];
				outi[n + IMAGE_Y][m - IMAGE_X] = buff2[n][m];
			}
		}
	}*/

	//256諧調化
	double max = 0, min;

	min = g[0][0];

	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			if (g[n][m]>max)
				max = g[n][m];
			if (g[n][m]<min)
				min = g[n][m];
		}
	}

	for (n = 0; n<IMAGE_Y * 2; n++){
		for (m = 0; m<IMAGE_X * 2; m++){
			result[n][m] = (unsigned char)((255 * (g[n][m] - min)) / (max - min));
		}
	}

	for (n = 0; n < IMAGE_X; n++){
		for (m = 0; m < IMAGE_Y; m++){
			trimming[n][m] = result[n + IMAGE_Y / 2][m + IMAGE_X / 2];
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

	fp = fopen(RESULT_NAME, "wb");

	fwrite(&BmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&BmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

	for (i = 0; i<256; i++)
		fwrite(&RGBQuad[i], sizeof(RGBQUAD), 1, fp);

	for (n = 0; n<IMAGE_Y; n++){
		for (m = 0; m<IMAGE_X; m++){
			fwrite(&trimming[n][m], 1, 1, fp);
		}
	}

	fclose(fp);

	return 0;
}