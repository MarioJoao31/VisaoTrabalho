//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS
#define Max(a,b) (a>b? a:b)
#define Min(a,b) (a<b? a:b)
#define Max3(a,b,c) (a>b? (a>c?a:c):(b>c?b:c))
#define Min3(a,b,c) (a<b? (a<c?a:c):(b<c?b:c))



#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vc.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *)malloc(sizeof(IVC));

	if (image == NULL)
		return NULL;
	if ((levels <= 0) || (levels > 255))
		return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

/*
	Desc:Libertar imagem da memoria
*/
IVC *vc_image_free(IVC *image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)))
			;
		if (c != '#')
			break;
		do
			c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF)
			break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#')
			ungetc(c, file);
	}

	*t = 0;

	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				// datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

/*
	Desc: le a imagem
*/
IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0)
		{
			channels = 1;
			levels = 1;
		} // Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0)
			channels = 1; // Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0)
			channels = 3; // Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

/*
	DESC:cria uma imagem

*/
int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL)
		return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

// converte em negativo uma imagem em grayscale so com 1 channel
IVC *vc_gray_negative(IVC *image)
{

	unsigned char *data = (unsigned char *)image->data;
	int width = image->width;
	int height = image->height;
	int bytesperline = image->width * image->height;
	int channels = image->channels;
	int x, y;
	long int pos;

	// verificação de erros
	if ((image->width <= 0) || (image->height <= 0) || (image->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	// ciclo para inverter imagem
	// ciclo primeiro o width
	// ciclo segundo height
	for (size_t x = 0; x < width; x++)
	{
		for (size_t y = 0; y < height; y++)
		{
			// formula para ter a posição
			pos = y * image->bytesperline + x * image->channels;

			// inverte a cor em modo grayscale
			data[pos] = 255 - data[pos];
		}
	}

	return image;
}

//converte uma imagem de rgb para negativo
IVC *vc_rgb_negative(IVC *image)
{
	unsigned char *data = (unsigned char *)image->data;
	int width = image->width;
	int height = image->height;
	int bytesperline = image->width * image->height;
	int channels = image->channels;
	int x, y;
	long int pos;

	// verificações
	if ((image->width <= 0) || (image->height <= 0) || (image->data == NULL))
		return 0;
	if (channels != 3)
		return 0;

	// ciclo para inverter imagem
	// ciclo primeiro o width
	// ciclo segundo height
	for (size_t x = 0; x < width; x++)
	{
		for (size_t y = 0; y < height; y++)
		{
			// formula para ter a posição
			pos = y * image->bytesperline + x * image->channels;

			// inverte a cor em modo grayscale
			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}

	return image;
}

//converte uma imagem de rgb para cinzento 
//recebe dois parametros (a imagem q quero tornar cinzenta / imagem vazia do mesmo tamanho da imagem rgb para guarda a imagem em tom de cinzento)
int vc_rgb_gray(IVC *src, IVC *dst){
	//converter de rgb para gray
    unsigned char *datasrc = (unsigned char * ) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst = (unsigned char * ) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y; 
    long int pos_src, pos_dst;
    float rf, gf, bf;

    //verificalão de errors
    if((src->width <= 0 ) || (src->height <= 0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height)) return 0;
    if((src->channels != 3 ) || (dst->channels != 1))return 0 ;

    for(y=0; y<height ; y++){
        for(x=0; x<width ; x++){
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

            rf = (float) datasrc[pos_src];
            gf = (float) datasrc[pos_src + 1];
            bf = (float) datasrc[pos_src + 2];

            datadst[pos_dst]= (unsigned char) (( rf * 0.299 )+( gf * 0.587 )+( bf * 0.114 ));
        }
    }
	//retorna a imagem em tom de cinzneto
    return 1;
}

int vc_rgb_to_hsv(IVC *src, IVC *dst){
	//converter de rgb para gray
    unsigned char *datasrc = (unsigned char * ) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst = (unsigned char * ) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y; 
	float max,min,hue,sat,valor;
    long int pos_src, pos_dst;
    float rf, gf, bf;

	//verificalão de errors
    if((src->width <= 0 ) || (src->height <= 0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height)) return 0;
    if((src->channels != 3 ) || (dst->channels != 3))return 0 ;

	for(y=0; y<height ; y++){
        for(x=0; x<width ; x++){
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

            rf = (float) datasrc[pos_src];
            gf = (float) datasrc[pos_src + 1];
            bf = (float) datasrc[pos_src + 2];

			//calcular value
			//find max 
			max = rf;
			if(gf>max){
				max = gf;
			}
			if(bf>max){
				max=bf;
			}

			//find min
			min = rf;
			if(gf<min){
				min = gf;
			}
			if(bf<min){
				min=bf;
			}

			//calcular value
			valor = max;

			//calcular saturação 
			if(max==0 || max == min){
				sat=0;
				hue=0;
			}else{
				sat=(max-min)/valor;
				//calcular hue 
				// Quando o vermelho é o maior, Hue será um ângulo entre 300 e 360 ou entre 0 e 60
				if(rf>bf && gf >=bf ){
					hue = 60*(gf-bf)/(max-min);
				}

				if(rf>gf && bf>gf ){
					hue = 360+60 * (gf - bf) / (max-min);
				}

				if(gf>bf && gf>rf){
					hue = 120 + 60 * (bf - rf) / (max-min);
				}

				if(bf>gf && bf>rf){
					hue = 240 + 60 * (rf - gf) / ( max - min);
				}
			}

		  	
			datadst[pos_dst] = (hue / 360) * 255;
         	datadst[pos_dst +1] = sat * 255;
         	datadst[pos_dst +2] =  valor;
        }
    }


	return 1;
}

int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin,int smax, int vmin, int vmax){
//converter de hsv para segmentar
    unsigned char *datasrc = (unsigned char * ) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    unsigned char *datadst = (unsigned char * ) dst->data;
    int bytesperline_dst = dst->width * dst->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    float max,min,hue,sat,valor;
    long int pos_src, pos_dst;
    float rf, gf, bf;

    //verificalão de errors
    if((src->width <= 0 ) || (src->height <= 0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height)) return 0;
    if((src->channels != 3 ) || (dst->channels != 1))return 0 ;

    for(y=0; y<height ; y++){
        for(x=0; x<width ; x++){
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

            rf = (float) datasrc[pos_src];
            gf = (float) datasrc[pos_src + 1];
            bf = (float) datasrc[pos_src + 2];

            //calcular value
            //find max
            max = rf;
            if(gf>max){
                max = gf;
            }
            if(bf>max){
                max=bf;
            }

            //find min
            min = rf;
            if(gf<min){
                min = gf;
            }
            if(bf<min){
                min=bf;
            }

            //calcular value
            valor = max;

            //calcular saturação
            if(max==0 || max == min){
                sat=0;
                hue=0;
            }else{
                sat=(max-min)*100.0f/valor;
                //calcular hue
                // Quando o vermelho é o maior, Hue será um ângulo entre 300 e 360 ou entre 0 e 60
                if(rf==max && gf>=bf ){
                    hue = 60.0f*(gf-bf)/(max-min);
                }

                else if(rf==max && bf>gf ){
                    hue = 360+60.0f * (gf - bf) / (max-min);
                }

                else if(gf==max){
                    hue = 120 + 60.0f * (bf - rf) / (max-min);
                }

                else if(max==bf){
                    hue = 240 + 60.0f * (rf - gf) / ( max - min);
                }
            }

            //verificação para o hue
            if(hue <= hmax && hue >= hmin && sat <= smax && sat  >= smin && valor <= vmax/100.0f * 255 && valor >= vmin/100.0f * 255)
                datadst[pos_dst] = 255;
            else
                datadst[pos_dst] = 0;

        }
    }
    return 1;

}

int vc_scale_gray_to_rgb(IVC* src, IVC* dst)
{
    unsigned char* datasrc = (unsigned char*)src->data;
    int bytesperline_src = src->channels * src->width;
    int channels_src = src->channels;
    unsigned char* datadst = (unsigned char*)dst->data;
    int bytesperline_dst = dst->channels * dst->width;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src, pos_dst;
    
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 1) || (dst->channels != 3)) return 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x * channels_dst;

			//primeiro quarteto 
            if (datasrc[pos_src] < 64)
            {
                datadst[pos_dst] = 0;
                datadst[pos_dst + 1] = datasrc[pos_src] * 4;
                datadst[pos_dst + 2] = 255;
            }
			//segundo qaurteto
            else if (datasrc[pos_src] < 128)
            {
                datadst[pos_dst] = 0;
                datadst[pos_dst + 1] = 255;
                datadst[pos_dst + 2] = 255 - ((datasrc[pos_src] - 64) * 4);
            }
			//terceiro qaurteto 
            else if (datasrc[pos_src] < 192)
            { 
                datadst[pos_dst] = (datasrc[pos_src] - 128) * 4;
                datadst[pos_dst + 1] = 255;
                datadst[pos_dst + 2] = 0;
            }
			//quarto quarteto
            else 
            {
                datadst[pos_dst] = 255;
                datadst[pos_dst + 1] = 255 - ((datasrc[pos_src] - 192) * 4);;
                datadst[pos_dst + 2] = 0;
            }

        }
    }
    return 1;
}

int vc_gray_to_binary(IVC *src, IVC *dst, int threshold) {
    unsigned char *datasrc = (unsigned char*)src->data;
    int bytesperline_src = src->width*src->channels;
    int channels_src = src->channels;
    unsigned char *datadst = (unsigned char*)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline_dst = dst->width*dst->channels;
    int channels_dst = dst->channels;
    int x, y;
    long int pos_src, pos_dst;

    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
    if ((src->width != dst->width) || (src->height != dst->height))return 0;
    if ((src->channels != 1) || (dst->channels != 1))return 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos_src = y*bytesperline_src + x*channels_src;
            pos_dst = y*bytesperline_dst + x*channels_dst;

            if (datasrc[pos_src] > threshold) {
                datadst[pos_dst] = 255;
            }
            else
            {
                datadst[pos_dst] = 0;
            }

        }
    }

    return 1;
}

//deteta o kernel automaticamente
int vc_gray_to_binary_media(IVC *src, IVC *dst) {
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;
	int total=0;
	float threshold;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y*bytesperline_src + x*channels_src;

			total += datasrc[pos_src];
		}
	}

	threshold = (float)total/(width*height);

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y*bytesperline_src + x*channels_src;
			pos_dst = y*bytesperline_dst + x*channels_dst;

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

	return 1;
}


int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel)
{
 	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->channels * src->width;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->channels * dst->width;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y, vY, vX;
	long int pos_src, pos_dst;
	float threshold;
	float vMax = 0, vMin = 256;

	//Percorre uma imagem 
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			vY = kernel /2;
			vX = kernel /2;
			


			int xx, yy;
			
			for (vMax = 0, vMin = 255, yy = y - vY; yy <= y + vY; yy++)
			{
				
				for (xx = x - vX ; xx <= x + vX; xx++)
				{
					
					if(yy >= 0 && yy < height && xx >=0 && xx < width)
					{ 
					
						pos_src = yy * bytesperline_src + xx * channels_src;

						if (datasrc[pos_src] > vMax)
						{
							vMax = datasrc[pos_src];
						}
						if (datasrc[pos_src] < vMin)
						{
							vMin = datasrc[pos_src];
						}

					}
				}
			}
		
			threshold = (vMin + vMax) / 2;
		
			if (datasrc[pos_src] > threshold)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}	
	return 1;

}

int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int Cmin)
{
 	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->channels * src->width;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->channels * dst->width;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y, vY, vX;
	long int pos_src, pos_dst;
	float threshold;
	float vMax = 0, vMin = 256;

	//Percorre uma imagem pyxel a pyxel
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			vY = kernel /2;
			vX = kernel /2;
			


			int xx, yy;
			
			//Percorre a vizinhança e descobre o Vmax e Vmin da vizinhança e faz o threshould
			for (vMax = 0, vMin = 255, yy = y - vY; yy <= y + vY; yy++)
			{
				
				for (xx = x - vX ; xx <= x + vX; xx++)
				{
					//verificação para não sair fora da imagem
					if(yy >= 0 && yy < height && xx >=0 && xx < width)
					{ 
						//Calcula o threshould da vizinhança
						pos_src = yy * bytesperline_src + xx * channels_src;

						if (datasrc[pos_src] > vMax)
						{
							vMax = datasrc[pos_src];
						}
						if (datasrc[pos_src] < vMin)
						{
							vMin = datasrc[pos_src];
						}

					}
				}
			}

			//Caso a variação 
			if ((vMax - vMin) < Cmin)
			{
				threshold = 255/2;
			}
			else
			{
				threshold = (vMin + vMax) / 2;
			}
			
		
			if (datasrc[pos_src] > threshold)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}	
	return 1;
}

int vc_binary_dilate(IVC *src, IVC *dst, int kernel){
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int xk, yk;
	int i, j;
	long int pos, posk;
	int s1, s2;
	unsigned char pixel;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	s2 = (kernel - 1) / 2;
	s1 = -(s2);

	memcpy(datadst, datasrc, bytesperline * height);

	// Cálculo da dilatacao
	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;

			pixel = datasrc[pos];

			for (yk = s1; yk <= s2; yk++)
			{
				j = y + yk;

				if ((j < 0) || (j >= height)) continue;

				for (xk = s1; xk <= s2; xk++)
				{
					i = x + xk;

					if ((i < 0) || (i >= width)) continue;

					posk = j * bytesperline + i * channels;
					//aqui a unica diference entre erode ou dilate
					//se encontrar um pixel a branco mete o pixel central a branco 

					pixel |= datasrc[posk];
				}
			}

			// Se um qualquer pixel da vizinhança, na imagem de origem, for de plano de fundo, então o pixel central
			// na imagem de destino é também definido como plano de fundo.
			if (pixel == 255) datadst[pos] = 255;
		}
	}
	return 1;
}
// Erosão binária
int vc_binary_erode(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int xk, yk;
	int i, j;
	long int pos, posk;
	int s1, s2;
	unsigned char pixel;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	s2 = (kernel - 1) / 2;
	s1 = -(s2);

	memcpy(datadst, datasrc, bytesperline * height);

	// Cálculo da erosão
	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			pos = y * bytesperline + x * channels;

			pixel = datasrc[pos];

			for (yk = s1; yk <= s2; yk++)
			{
				j = y + yk;

				if ((j < 0) || (j >= height)) continue;

				for (xk = s1; xk <= s2; xk++)
				{
					i = x + xk;

					if ((i < 0) || (i >= width)) continue;

					posk = j * bytesperline + i * channels;
					//aqui a unica diference entre erode ou dilate
					//se encontrar um pixel a branco mete o pixel central a branco 

					pixel &= datasrc[posk];
				}
			}

			// Se um qualquer pixel da vizinhança, na imagem de origem, for de plano de fundo, então o pixel central
			// na imagem de destino é também definido como plano de fundo.
			if (pixel == 0) datadst[pos] = 0;
		}
	}
	

	return 1;
}
//serve para fazer contornos ( erodir a imagem para depois contornar para simplificar a imagem )
int vc_binary_open(IVC *src, IVC *dst, int kernelErode, int kernelDilate){

	IVC *temp;
	temp = vc_image_new(src->width,src->height,1,255);

	vc_binary_erode(src,temp,kernelErode);
	
	vc_binary_dilate(temp,dst,kernelDilate);

	vc_image_free(temp);
	
	return 1;
}


int vc_binary_close(IVC *src, IVC *dst, int kernelErode, int kernelDilate){

	IVC *temp;
	temp = vc_image_new(src->width,src->height,1,255);

	vc_binary_dilate(src,temp,kernelDilate);

	vc_binary_erode(temp,dst,kernelErode);
	

	vc_image_free(temp);
	
	return 1;
}

// Etiquetagem de blobs
// src		: Imagem bin�ria de entrada
// dst		: Imagem grayscale (ir� conter as etiquetas)
// nlabels	: Endere�o de mem�ria de uma vari�vel, onde ser� armazenado o n�mero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}
	//codigo acima faz a estiquetagem 

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / Max(blobs[i].area, 1);
		blobs[i].yc = sumy / Max(blobs[i].area, 1);
	}

	return 1;
}

int vc_gray_histogram_equalization(IVC *src, IVC *dst) {
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x,y;
	long int pos_src, pos_dst;
	int n[256] = {0};
	float pdf[256] = { 0 },cdf[256] = { 0 };
	float size = width*height;
	float min = 0;

	for(int x=0; x<height*width ; x++){
		n[datasrc[x]]++;
	}
	//calcular pdf
	for (x = 0; x <= 255; x++)
	{
		pdf[x] = (((float)n[x]) / size);
	}
	//calcular cdf
	for(int y=0; y<=255; y++){
		if(y == 0){
			cdf[y] = pdf[y];
		}else{
			cdf[y]= cdf[y - 1]+ pdf[y];
		}
	}

	for(int x=0; x <=255; x++){
		if (cdf[x] != 0) { 
			min = pdf[x]; 
			break; 
		}
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_dst = y*bytesperline_src + x*channels_src;
			datadst[pos_dst] = (cdf[datasrc[pos_dst]] - min) / (1.0 - min) * (256-1);
		}
	}

	return 1;
}

// Detecção de contornos pelos operadores Prewitt
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th) // th = [0.001, 1.000]
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	float histmax;
	int histthreshold;
	int sumx, sumy;
	float hist[256] = { 0.0f };

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	size = width * height;

	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// PosA PosB PosC
			// PosD PosX PosE
			// PosF PosG PosH

			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			// PosA*(-1) PosB*0 PosC*(1)
			// PosD*(-1) PosX*0 PosE*(1)
			// PosF*(-1) PosG*0 PosH*(1)

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1

			// PosA*(-1) PosB*(-1) PosC*(-1)
			// PosD*0    PosX*0    PosE*0
			// PosF*(1)  PosG*(1)  PosH*(1)

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1

			//datadst[posX] = (unsigned char)sqrt((double)(sumx*sumx + sumy*sumy));
			datadst[posX] = (unsigned char) (sqrt((double) (sumx*sumx + sumy*sumy)) / sqrt(2.0));
			// Explicação:
			// Queremos que no caso do pior cenário, em que sumx = sumy = 255, o resultado
			// da operação se mantenha no intervalo de valores admitido, isto é, entre [0, 255].
			// Se se considerar que:
			// max = 255
			// Então,
			// sqrt(pow(max,2) + pow(max,2)) * k = max <=> sqrt(2*pow(max,2)) * k = max <=> k = max / (sqrt(2) * max) <=> 
			// k = 1 / sqrt(2)
		}
	}

	// Calcular o histograma com o valor das magnitudes
	for (i = 0; i < size; i++)
	{
		hist[datadst[i]]++;
	}

	// Definir o threshold.
	// O threshold é definido pelo nível de intensidade (das magnitudes)
	// quando se atinge uma determinada percentagem de pixeis, definida pelo utilizador.
	// Por exemplo, se o parâmetro 'th' tiver valor 0.8, significa the o threshold será o 
	// nível de magnitude, abaixo do qual estão pelo menos 80% dos pixeis.
	histmax = 0.0f;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th)) break;
	}
	histthreshold = i == 0 ? 1 : i;

	// Aplicada o threshold
	for (i = 0; i < size; i++)
	{
		if (datadst[i] >= (unsigned char) histthreshold) datadst[i] = 255;
		else datadst[i] = 0;
	}

	return 1;
}
/*
int vc_convertToThreeChannels(IVC *graysrc,IVC *graydst) {
    //verificações
    if (!graysrc || !graysrc->data || !graydst || !graydst->data) {
        return -1; // Invalid parameters
    }

    if (graysrc->channels != 1) {
        return -2; // Input image should have only one channel
    }

    if (graydst->channels != 3) {
        return -3; // Output image should have three channels
    }

    if (graysrc->width != graydst->width || graysrc->height != graydst->height) {
        return -4; // Input and output images should have the same size
    }

    // Copy image header information
    graydst->width = graysrc->width;
    graydst->height = graysrc->height;
    graydst->channels = 3;
    graydst->levels = graysrc->levels;
    graydst->bytesperline = 3 * graysrc->width;
    graydst->data = (unsigned char*) malloc(graydst->channels * graydst->width * graydst->height * sizeof(unsigned char));

    //BUG: muda as cores se eu passar como paramentro vermelho ele muda para reconhecer azul

    // Copiar os valores de pixel da imagem em escala de cinza para cada um dos três canais da nova imagem
    for (int i = 0; i < graysrc->height; i++) {
        for (int j = 0; j < graysrc->width; j++) {
            int binary_idx = i * graysrc->bytesperline + j / 8;
            int binary_bit = 7 - j % 8;  // Invert bit order because of endianness
            int color_idx = i * graydst->bytesperline + j * graydst->channels;

            unsigned char binary_value = (graysrc->data[binary_idx] >> binary_bit) & 0x01;
            unsigned char color_value = binary_value * 255;

            graydst->data[color_idx] = color_value;
            graydst->data[color_idx + 1] = color_value;
            graydst->data[color_idx + 2] = color_value;
        }
    }

    return 1;
}
 */

int vc_convertToThreeChannels(IVC* binary_image, IVC* color_image) {
    int i, j;

    // Check that input image has only one channel
    if (binary_image->channels != 1) {
        printf("Error: input image must have only one channel.\n");
        return 0;
    }

    // Allocate memory for color image
    color_image->data = (unsigned char*)malloc(color_image->bytesperline * color_image->height * sizeof(unsigned char));
    color_image->channels = 3;

    // Copy pixel values from black and white image to color image
    for (i = 0; i < binary_image->height; i++) {
        for (j = 0; j < binary_image->width; j++) {
            int bw_idx = i * binary_image->bytesperline + j;
            int color_idx = i * color_image->bytesperline + j * color_image->channels;

            unsigned char bw_value = binary_image->data[bw_idx];
            unsigned char color_value = bw_value;

            color_image->data[color_idx] = color_value;
            color_image->data[color_idx + 1] = color_value;
            color_image->data[color_idx + 2] = color_value;
        }
    }
    return 1;
}

IVC* vc_convert_to_rgb(IVC *image){
    unsigned char* data = image->data;
    int width = image->width;
    int height = image->height;
    int channels = image->channels;
    int bytesperline = image->bytesperline;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * bytesperline + x * channels;
            unsigned char blue = data[index];
            data[index] = data[index + 2];
            data[index + 2] = blue;
        }
    }
    return image;
}