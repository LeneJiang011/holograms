#define _CRT_SECURE_NO_WARNINGS
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#define _USE_MATH_DEFINES
#include<math.h>


double max(double, double);
double min(double, double);


#pragma pack(1)
typedef struct tagBITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long  bf0ffBits;
}BITMAPFILEHEADER;
#pragma pack()

typedef struct tagBITMAPINFOHEADER
{
	unsigned long   biSize;
	long			biWidth;
	long			biHeight;
	unsigned short  biPlanes;
	unsigned short  biBitCount;
	unsigned long   biCompression;
	unsigned long   biSizeImage;
	long			biXPelsPerMeter;
	long			biYPelsPerMeter;
	unsigned long   biCirUsed;
	unsigned long   biCirImportant;
}BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
	unsigned char  rgbBlue;
	unsigned char  rgbGreen;
	unsigned char  rgbRed;
	unsigned char  rgbReserved;
}RGBQUAD;


unsigned char img[1920 * 1080] = { 0 };
double fin[1920 * 1080] = { 0 };
double U1[1920 * 1080] = { 0 };
double U2[1920 * 1080] = { 0 };

int main()
{
	clock_t start, end;
	FILE *fp, *f;
	int x[284], y[284];
	double z[284];
	int i, j, number;
	double maximum, medium, minimum;
	double k = (2 * M_PI) / ((520)* pow(10.0, -9));
	double p = 8.0*pow(10.0, -6);
	int num[284 * 3 + 1];

	BITMAPFILEHEADER    BmpFileHeader;
	BITMAPINFOHEADER    BmpInfoHeader;
	RGBQUAD             RGBQuad[256];

	BmpFileHeader.bfType = 19778;
	BmpFileHeader.bfSize = 14 + 40 + 1024 + (1920 * 1080);
	BmpFileHeader.bfReserved1 = 0;
	BmpFileHeader.bfReserved2 = 0;
	BmpFileHeader.bf0ffBits = 14 + 40 + 1024;

	BmpInfoHeader.biSize = 40;
	BmpInfoHeader.biWidth = 1920;
	BmpInfoHeader.biHeight = 1080;
	BmpInfoHeader.biPlanes = 1;
	BmpInfoHeader.biBitCount = 8;
	BmpInfoHeader.biCompression = 0L;
	BmpInfoHeader.biSizeImage = 0L;
	BmpInfoHeader.biXPelsPerMeter = 0L;
	BmpInfoHeader.biYPelsPerMeter = 0L;
	BmpInfoHeader.biCirUsed = 0L;
	BmpInfoHeader.biCirImportant = 0L;

	for (i = 0; i < 256; i++){
		RGBQuad[i].rgbBlue = i;
		RGBQuad[i].rgbGreen = i;
		RGBQuad[i].rgbRed = i;
		RGBQuad[i].rgbReserved = 0;
	}


	//ﾊﾞｲﾅﾘﾋﾞｯﾄの出力
	fp = fopen("cube284.3d", "rb");
	fread(num, sizeof(int), 284 * 3 + 1, fp);

	for (i = 0; i<284; i++)
	{
		x[i] = num[3 * i + 1] * 40 + 960;
		y[i] = num[3 * i + 2] * 40 + 540;
		z[i] = num[3 * i + 3] * 40 + 1 / p;
	}

	start = clock();
	//光強度用の配列I
	for (i = 0; i < 1920; i++)
	{
		for (j = 0; j < 1080; j++)
		{
			for (number = 0; number < num[0]; number++){
				U1[i + 1920 * j] += sin(k*p*sqrt((i - x[number])* (i - x[number]) + (j - y[number]) * (j - y[number]) + z[number] * z[number]));
				U2[i + 1920 * j] += cos(k*p*sqrt((i - x[number])* (i - x[number]) + (j - y[number]) * (j - y[number]) + z[number] * z[number]));
			}
			fin[i + 1920 * j] = atan2(U1[i + 1920 * j] , U2[i + 1920 * j]);
		}
	}
	end = clock();
	printf("%.2f 秒\n", (double)(end - start) / CLOCKS_PER_SEC);

	printf("%lf", fin[0]);
	//光強度Iの最大値と最小値を求める
	minimum = fin[0];
	maximum = fin[0];
	for (i = 0; i < 1920; i++)
	{
		for (j = 0; j < 1080; j++)
		{
			minimum = min(minimum, fin[i + 1920 * j]);
			maximum = max(maximum, fin[i + 1920 * j]);
		}
	}
	medium = (maximum + minimum) / 2;
	printf("medium = %f \n", medium);
	printf("maximum = %f \n", maximum);
	printf("minimum = %f \n", minimum);

	//ビットマップ用配列を作り
	for (i = 0; i < 1920; i++)
	{
		for (j = 0; j < 1080; j++)
		{
			if (fin[i + 1920 * j] < medium)
				img[i + 1920 * j] = 0;
			else if (fin[i + 1920 * j] > medium)
				img[i + 1920 * j] = 255;
		}
	}

	f = fopen("CGH4.bmp", "wb");
	fwrite(&BmpFileHeader, sizeof(BmpFileHeader), 1, f);
	fwrite(&BmpInfoHeader, sizeof(BmpInfoHeader), 1, f);
	fwrite(&RGBQuad, sizeof(RGBQuad), 1, f);
	fwrite(&img, sizeof(img), 1, f);


	fclose(fp);
	fclose(f);
	return 0;

}


double max(double n, double m)
{
	if (n < m)
		return m;
	else
		return n;
}

double min(double n, double m)
{
	if (n < m)
		return n;
	else
		return m;
}


