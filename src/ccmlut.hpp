#ifndef _CCMLUT_HPP_
#define _CCMLUT_HPP_


#include <stdio.h>
#include <iostream>


#ifndef MAX
#define MAX(a,b) (a)>(b)?(a):(b)
#endif
#ifndef MIN
#define MIN(a,b) (a)<(b)?(a):(b)
#endif


struct CCMLUT
{
	unsigned char *pdata;

	CCMLUT():pdata(nullptr){}
};


// gammaÒ»°ãÊÇ 0.45
static void use_ccm(double *ccm, unsigned char* gamma_lut, unsigned char* degamma_lut,int r, int g, int b, unsigned char* rgb)
{
	// ÑÕÉ«degamma -> linear rgb
	double rr = degamma_lut[r] / 255.0;
	double gg = degamma_lut[g] / 255.0;
	double bb = degamma_lut[b] / 255.0;

	// linear rgb -> xyz
	double x = 0.4124564 * rr + 0.3575761 * gg + 0.1804375 * bb;
	double y = 0.2126729 * rr + 0.7151522 * gg + 0.0721750 * bb;
	double z = 0.0193339 * rr + 0.1191920 * gg + 0.9503041 * bb;

	// xyz -> ccm -> xyz2
	double xx = x * ccm[0] + y * ccm[1 * 3 + 0] + z * ccm[2 * 3 + 0] + ccm[3 * 3 + 0];
	double yy = x * ccm[1] + y * ccm[1 * 3 + 1] + z * ccm[2 * 3 + 1] + ccm[3 * 3 + 1];
	double zz = x * ccm[2] + y * ccm[1 * 3 + 2] + z * ccm[2 * 3 + 2] + ccm[3 * 3 + 2];

	// xyz -> rgb
	rr = 3.24045 * xx + -1.53714 * yy - 0.498532 * zz;
	gg = -0.969266 * xx + 1.87601 * yy + 0.0415561 * zz;
	bb = 0.0556434 * xx - 0.204026 * yy + 1.05723 * zz;

	// 1 -> 255
	rr = MIN(MAX(rr * 255, 0), 255);
	gg = MIN(MAX(gg * 255, 0), 255);
	bb = MIN(MAX(bb * 255, 0), 255);

	// regamma
	r = (int)round(rr);
	g = (int)round(gg);
	b = (int)round(bb);

	r = gamma_lut[r];
	g = gamma_lut[g];
	b = gamma_lut[b];

	rgb[0] = (unsigned char)r;
	rgb[1] = (unsigned char)g;
	rgb[2] = (unsigned char)b;
}


bool clut_create(double *ccm, double gamma, CCMLUT& lut)
{
	if (ccm == nullptr) return false;

	double degamma = 1.0 / gamma;
	unsigned char gamma_lut[256];
	unsigned char degamma_lut[256];
	
	for (int i = 0; i < 256; i++)
	{
		int lv = (int)round(pow((float)i / 255.0, gamma) * 255.0);
		gamma_lut[i] = (unsigned char)MAX(MIN(lv, 255), 0);

		lv = (int)round(pow((float)i / 255.0, degamma) * 255.0);
		degamma_lut[i] = (unsigned char)MAX(MIN(lv, 255), 0);
	}
	

	const int lut_size = 256 * 256 * 256 * 3;
	if(lut.pdata == nullptr)
	{
		lut.pdata = new unsigned char[lut_size];
	}

	for (int r = 0; r < 256; r++)
	{
		for (int g = 0; g < 256; g++)
		{
			for (int b = 0; b < 256; b++)
			{
				int pos = (r * 256 * 256 + g * 256 + b) * 3;
				use_ccm(ccm, gamma_lut, degamma_lut, r, g, b, lut.pdata + pos);
			}
		}
	}

	return true;
}

bool clut_lookup(const CCMLUT& lut, int r, int g, int b, int &or, int &og, int &ob)
{
	if (lut.pdata == nullptr) return false;

	int pos = (r * 256 * 256 + g * 256 + b) * 3;
	or = lut.pdata[pos];
	og = lut.pdata[pos+1];
	ob = lut.pdata[pos+2];
	
	return true;
}

void clut_destroy(CCMLUT&lut)
{
	if (lut.pdata)delete[]lut.pdata;
	lut.pdata = nullptr;
}


#endif // _CCMLUT_HPP_
