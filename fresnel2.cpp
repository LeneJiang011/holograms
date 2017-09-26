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
double I[1920 * 1080] = { 0 };

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


	//�޲���ޯĂ̏o��
	fp = fopen("cube284.3d", "rb");
	fread(num, sizeof(int), 284 * 3 + 1, fp);

	for (i = 0; i<284; i++)
	{
		x[i] = num[3 * i + 1] * 40 + 960;
		y[i] = num[3 * i + 2] * 40 + 540;
		z[i] = num[3 * i + 3] * 40 + 1 / p;
	}

	printf("x[10] = %d", x[10]);

	start = clock();
	//�����x�p�̔z��I
	for (i = 0; i < 1920; i++)
	{
		for (j = 0; j < 1080; j++)
		{
			for (number = 0; number < num[0]; number++){
					I[i + 1920 * j] += cos(k*p*(z[number] + ((i - x[number])*(i - x[number]) + (j - y[number])*(j - y[number])) / (2 * z[number])));
			}
		}
	}
	end = clock();
	printf("%.2f �b\n", (double)(end - start) / CLOCKS_PER_SEC);

	printf("%lf", I[0]);
	//�����xI�̍ő�l�ƍŏ��l�����߂�
	minimum = I[0];
	maximum = I[0];
	for (i = 0; i < 1920; i++)
	{
		for (j = 0; j < 1080; j++)
		{
			minimum = min(minimum, I[i + 1920 * j]);
			maximum = max(maximum, I[i + 1920 * j]);
		}
	}
	medium = (maximum + minimum) / 2;
	printf("medium = %f \n", medium);
	printf("maximum = %f \n", maximum);
	printf("minimum = %f \n", minimum);

	//�r�b�g�}�b�v�p�z������
	for (i = 0; i < 1920; i++)
	{
		for (j = 0; j < 1080; j++)
		{
			if (I[i + 1920 * j] < medium)
				img[i + 1920 * j] = 0;
			else if (I[i + 1920 * j] > medium)
				img[i + 1920 * j] = 255;
		}
	}

	f = fopen("CGH6.bmp", "wb");
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


