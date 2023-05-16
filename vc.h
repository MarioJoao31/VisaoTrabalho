//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define VC_DEBUG

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct
{
	unsigned char *data;
	int width, height;
	int channels;	  // Bin�rio/Cinzentos=1; RGB=3
	int levels;		  // Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline; // width * channels
} IVC;

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// �rea
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Per�metro
	int label;					// Etiqueta
} OVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
// cria imagem
IVC *vc_image_new(int width, int height, int channels, int levels);
// libertar da memoria a imagem
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
// ler a imagem
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

// passa uma imagem em grayscale para negativo
IVC *vc_gray_negative(IVC *image);
// passa uma imagem rgb para negativo;
IVC *vc_rgb_negative(IVC *image);
// converte uma imagem de rgb para cinzento
int vc_rgb_gray(IVC *src, IVC *dst);
// converte uma imagem de rgb para hsv
int vc_rgb_to_hsv(IVC *src, IVC *dst);
//converte uma imagem RGB e segmenta dependendo dos parametros 
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin,int smax, int vmin, int vmax);
//converte uma imagem grey para rgb 
int vc_scale_gray_to_rgb(IVC *src, IVC *dst);
//calcula a percentagem de pixeis na imagem 
int ativadade_cerebro(IVC *src);


//cinzento para duas cores atraves do certo threshold
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold) ;
//deteta o kernel automaticamente
int vc_gray_to_binary_media(IVC *src, IVC *dst);
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel);
int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int Cmin);
//converte de um canal para 3 canais!
int vc_convertToThreeChannels(IVC *graysrc,IVC *graydst);

//serve para dilatar os pixeis  
int vc_binary_dilate(IVC *src, IVC *dst, int kernel);
int vc_binary_erode(IVC *src, IVC *dst, int kernel);

//open = dilata e depois faz erode 
int vc_binary_open(IVC *src, IVC *dst, int kernelErode, int kernelDilate);
//close = erose e depois dilata 
int vc_binary_close(IVC *src, IVC *dst,int kernelErode, int kernelDilate);

//Através da aplicação de técnicas de binarização de imagens, é possível segmentar regiões, e contar las 
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);

//histogramas equaliza as cores da imagem 
int vc_gray_histogram_equalization(IVC *src, IVC *dst);

// Detecção de contornos pelos operadores Prewitt
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);

//converter bgr para rgb
IVC* vc_convert_to_rgb(IVC *src);