#include "FormFactors.h"

#define Pi (3.1415926535897932384626433832795028841931f)

static int n_hemicube_size = 0; // = Config::HEMICUBE_W();
//static float f_hemicube_max = 1;

float* p_hemicube_tmp_formfactor = NULL;
float* p_hemicube_tmp_formfactor_side = NULL;

float* p_hemicube_formfactors = NULL;


/**
 * Spocita factory pohledu vpred a do strany
 * a naplni jimi p_hemicube_tmp_formfactor a p_hemicube_tmp_formfactor_side
 */
void Calc_HemicubeFormFactors()
{	
	p_hemicube_tmp_formfactor = new float[n_hemicube_size * n_hemicube_size];
	p_hemicube_tmp_formfactor_side = new float[n_hemicube_size * n_hemicube_size / 2];

    float f_half_pixel_width;
    float f_pixel_area;
    float dx, dy;
    int x, y;
    float f;

    f_half_pixel_width = (1.0f / n_hemicube_size);
    //
    f_pixel_area = (2.0f / n_hemicube_size);
    f_pixel_area *= f_pixel_area;
    // 2 je velikost hemicube

    for(x = 0; x < n_hemicube_size; x ++) {
        for (y = 0; y < n_hemicube_size; y ++) {
            dx = ((x - n_hemicube_size / 2) / (n_hemicube_size / 2.0f)) +
                f_half_pixel_width;
            dy = ((y - n_hemicube_size / 2) / (n_hemicube_size / 2.0f)) +
                f_half_pixel_width;

            f = (dx * dx + dy * dy + 1);
            f *= f * Pi;

            p_hemicube_tmp_formfactor[x + y * n_hemicube_size] = f_pixel_area / f;
        }
    }
    // vrchni ...

    for(x = 0; x < n_hemicube_size; x ++) {
        for(y = 0; y < n_hemicube_size / 2; y ++) {
            dx = (x - n_hemicube_size / 2) / (n_hemicube_size / 2.0f) +
                f_half_pixel_width;
            dy = (n_hemicube_size / 2 - 1 - y) / (n_hemicube_size / 2.0f) +
                f_half_pixel_width;

            f = (dx * dx + dy * dy + 1);
            f *= f * Pi;

            p_hemicube_tmp_formfactor_side[x + y * n_hemicube_size] = (f_pixel_area * (dy + f_half_pixel_width)) / f;
        }
    }
    // strana

	/*
    for(x = 0, f_hemicube_max = 0; x < n_hemicube_size; x ++) {
        for(y = 0; y < n_hemicube_size / 2; y ++) {
            if(f_hemicube_max < p_hemicube_tmp_formfactor_side[x + y * n_hemicube_size])
                f_hemicube_max = p_hemicube_tmp_formfactor_side[x + y * n_hemicube_size];
        }
    }
    //
    for(x = 0; x < n_hemicube_size; x ++) {
        for(y = 0; y < n_hemicube_size; y ++) {
            if(f_hemicube_max < p_hemicube_tmp_formfactor[x + y * n_hemicube_size])
                f_hemicube_max = p_hemicube_tmp_formfactor[x + y * n_hemicube_size];
        }
    }
    // hleda nejvetsi polozku
	*/

//#define __SUM_TEST
#ifdef __SUM_TEST
        float f_sum;

        for(y = 0, f_sum = 0; y < n_hemicube_size; y ++) {
            for(x = 0; x < n_hemicube_size / 2; x ++)
                f_sum += p_hemicube_tmp_formfactor_side[y + x * n_hemicube_size];
        }
        // strana

        f_sum *= 4;
        // strany jsou 4

        for(x = 0; x < n_hemicube_size; x ++) {
            for(y = 0; y < n_hemicube_size; y ++)
                f_sum += p_hemicube_tmp_formfactor[x + y * n_hemicube_size];
        }
        // vrch

        // tady musi byt f_sum == 1, mne to vyjde 1.00435f
		cout << "ff sum: " << f_sum << endl;
#endif
}



/*
BMP *p_Hemicube_Test()
{
	BMP *p_temp;
	int x, y, n;

	if(!(p_temp = (BMP*)malloc(sizeof(BMP))))
		return 0;

	p_temp->n_Width = Config::PATCHVIEW_TEX_W();
	p_temp->n_Height = Config::PATCHVIEW_TEX_H();

	if(!(p_temp->bytes = (unsigned __int32*)malloc(p_temp->n_Width *
	   p_temp->n_Height * sizeof(unsigned __int32))))
		return 0;

	for(x = 0; x < Config::PATCHVIEW_TEX_W(); x ++) {
		for(y = 0; y < Config::PATCHVIEW_TEX_H(); y ++) {
			n = (int)(p_hemicube_formfactors[x + y * Config::PATCHVIEW_TEX_W()] / f_hemicube_max * 0xff);
			p_temp->bytes[x + y * Config::PATCHVIEW_TEX_W()] = RGB(n, n, n);
		}
	}	
	// vytvoøí grayscale bitmapu ...

	return p_temp;
}


int	Save_TrueColor_BMP(char *filename, BMP *bmp)
{
	unsigned long ByteCount;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	unsigned char *lpbBuf;
	unsigned __int32 bBuf;
	unsigned int BuffLen;
	int i, j, x;
	FILE *BMPfp;

	BuffLen = ((3 * bmp->n_Width + 3) >> 2) << 2;
	lpbBuf =  (unsigned char*) malloc(BuffLen);
	// compute lenght of line-buffer

	//if((BMPfp = fopen(filename, "wb")) == NULL)
	if((BMPfp = fopen(filename, "wb")) == NULL)
		return 0;
	// opens file

	bmfh.bfType = ('B'|('M'<<8)); // "BM"
	bmfh.bfSize = 0;              // necessary to modify
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER);
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = bmp->n_Width;
	bmih.biHeight = bmp->n_Height;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;	      // True Color
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;         // necessary to modify
	bmih.biXPelsPerMeter = 96;
	bmih.biYPelsPerMeter = 96;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	// create header

	fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, BMPfp);
	fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1, BMPfp);
	// writing header

	ByteCount = 0;
	for(i = bmp->n_Height - 1; i >= 0; i --) {
		for(j = 0, x = 0; j < bmp->n_Width; j ++, x ++) {
			bBuf = bmp->bytes[i * bmp->n_Width + j];
			lpbBuf[x * 3] = (char)((bBuf >> 16) & 0xff);
			lpbBuf[x * 3 + 1] = (char)((bBuf >> 8) & 0xff);
			lpbBuf[x * 3 + 2] = (char)(bBuf & 0xff);
			// saved in BGR, not RGB
			ByteCount += 3;
		}

		if(fwrite(lpbBuf, 1, BuffLen, BMPfp) != BuffLen) {
			free(lpbBuf);
			return 0;
		}
	}

	// repair header
	bmih.biSizeImage = ByteCount;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) +
	   ByteCount + sizeof(BITMAPINFOHEADER);
	fseek(BMPfp, 0, SEEK_SET);
	fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, BMPfp);
	fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1, BMPfp);

	fclose(BMPfp);

	free(lpbBuf);

	return 1;
}
*/


/*
    0        256       512
384	* - - - - * - - - - * 384
	|         |         |
	|   UP    |  DOWN   |
256	* - - - - * - - - - * 256
	|     |       |     |
	| LFT | FRONT | RGT |
	|     |       |     |
  0	* - - * - - - * - - * 0
    0    128     384   512

        0        H_W       H_W*2
H_H*1.5 * - - - - * - - - - * H_H*1.5
        |         |         |
        |   UP    |  DOWN   |
    H_H * - - - - * - - - - * H_H
        |     |       |     |
        | LFT | FRONT | RGT |
        |     |       |     |
      0 * - - * - - - * - - * 0
        0   H_W/2  H_W*1.5  H_W*2
*/



/**
 * Zkopiruje form factory pro pohledu vpred a do strany do dynymickeho bufferu o strukture odpovidajici texture a vraci ukazatel na nej
 * Pri lookupu bude pote stacit sahat na stejne misto v bufferu jako ze ktereho se cte textura
 */
float* precomputeHemicubeFormFactors() {
	unsigned int HEMICUBE_W = Config::HEMICUBE_W();
	unsigned int HEMICUBE_H = Config::HEMICUBE_H();
	unsigned int PATCHVIEW_TEX_RES = Config::PATCHVIEW_TEX_RES();
	unsigned int PATCHVIEW_TEX_W = Config::PATCHVIEW_TEX_W();
	unsigned int PATCHVIEW_TEX_H = Config::PATCHVIEW_TEX_H();

	n_hemicube_size = Config::HEMICUBE_W();
	p_hemicube_formfactors = new float[PATCHVIEW_TEX_RES];

	// predpocita factory pro pohled vpred a do strany
	Calc_HemicubeFormFactors();

	// zkopiruje pohledy do jedineho pole
	for (unsigned int i = 0; i < PATCHVIEW_TEX_RES; i++) {
		unsigned int x = i % PATCHVIEW_TEX_W;
		unsigned int y = i / PATCHVIEW_TEX_W;

		// pohled dopredu
		if (x >= HEMICUBE_W/2 && x < HEMICUBE_W*1.5 && y < HEMICUBE_H)
			p_hemicube_formfactors[i] = p_hemicube_tmp_formfactor[ y * HEMICUBE_W + (x - HEMICUBE_W/2) ];				
		// pohled vlevo
		else if (x < HEMICUBE_W/2 && y < HEMICUBE_H)
			p_hemicube_formfactors[i] = p_hemicube_tmp_formfactor_side[ (HEMICUBE_H/2 - x) * HEMICUBE_W - y - 1 ];		
		// pohled vpravo
		else if (x >= HEMICUBE_W*1.5 && y < HEMICUBE_H)
			p_hemicube_formfactors[i] = p_hemicube_tmp_formfactor_side[ (x - unsigned int(HEMICUBE_W*1.5)) * HEMICUBE_W + y ];		
		// pohled nahoru
		else if (x < HEMICUBE_W && y >= HEMICUBE_H)
			p_hemicube_formfactors[i] = p_hemicube_tmp_formfactor_side[ (y - HEMICUBE_H) * HEMICUBE_W + x ];
		// pohled dolu
		else if (x >= HEMICUBE_W && y >= HEMICUBE_H)
			p_hemicube_formfactors[i] = p_hemicube_tmp_formfactor_side[ ((HEMICUBE_H/2-1) - (y - HEMICUBE_H)) * HEMICUBE_W + (x - HEMICUBE_W) ];
	}


	/*
	// test
	BMP* img = p_Hemicube_Test();
	Save_TrueColor_BMP("test.bmp", img);
	*/
	
	delete[] p_hemicube_tmp_formfactor;
	delete[] p_hemicube_tmp_formfactor_side;
	
	return p_hemicube_formfactors;
}