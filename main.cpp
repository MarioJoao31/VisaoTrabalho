#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

extern "C" {
#include "vc.h"
}


void vc_timer(void) {
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running) {
		running = true;
	}
	else {
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}


int main(void) {
	
	// codigo para abrir camera
	
	cv::VideoCapture capture;

	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de v�deo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi dever� estar localizado no mesmo direct�rio que o ficheiro de c�digo fonte.
	*/
	capture.open(0);

	/* Em alternativa, abrir captura de v�deo pela Webcam #0 */
	//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi poss�vel abrir o ficheiro de v�deo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	/* N�mero total de frames no v�deo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do v�deo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolu��o do v�deo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o v�deo */
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();

 	//TODO: por aqui a criar a imagem para ter mais framerate
	IVC *image = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageTempBlue = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageBlue = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageRed = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageGreen = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageBlobsBlue = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageBlobsRed = vc_image_new(video.width, video.height, 1, 255);
    IVC *imagetemp2 = vc_image_new(video.width, video.height, 1, 255);
    IVC *image3 = vc_image_new(video.width, video.height, 3, 255);
    IVC *image4 = vc_image_new(video.width, video.height, 3, 255);
    int nblobs = 0;
    OVC *blobs;

    cv::Rect rect(0, 0, 50, 50);

    cv::Mat frame;
	while (key != 'q') {
		/* Leitura de uma frame do v�deo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		/* Exemplo de inser��o texto na frame */
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);


		memcpy(image->data, frame.data, video.width * video.height * 3);
        //converte o frame q esta em bgr para rgb
        vc_convert_to_rgb(image);

        /////////////////////AZUL
        //teste faz o segment de algumas cores
		vc_hsv_segmentation(image, imageBlue, 200,240,70,100,70,100);
        //vc_binary_open(imageTempBlue,imageBlue,7,2);
        //faz o blob labeling
        blobs = vc_binary_blob_labelling(imageBlue,imageBlobsBlue, &nblobs);
        float circularidade = 0.000;
        if (blobs != NULL){
            //da a informação das blobs
            vc_binary_blob_info(imageBlobsBlue,blobs, nblobs);

            for (size_t i = 0; i < nblobs; i++)
            {
                if( blobs[i].area > 7000){

                    printf("label %d | area %d \n", blobs[i].label, blobs[i].area);

                    str = std::string(" !!! AZUL !!! ");
                    cv::putText(frame, str, cv::Point(blobs->x, blobs->y - 20), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 0, 0), 2);
                    cv::putText(frame, str, cv::Point(blobs->x, blobs->y - 20), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 0, 0), 1);

                    //circular ?
                    circularidade= 4.00 * M_PI * blobs->area/(blobs->perimeter*blobs->perimeter);
                    printf("circularidade: %f\n", circularidade);
                    if(circularidade > 0.20){
                        //TODO: falta fazer a verificação do centro de massa
                        printf("É UM CIRCULO AZULL!!!\n");

                        int imageCenterX = blobs->width / 2;
                        int imageCenterY = blobs->height / 2;
                        printf("xc:%d\n", blobs->xc);
                        printf("yc:%d\n", blobs->yc);
                        printf("imageCenterX:%d\n", imageCenterX);
                        printf("imageCenterY:%d\n", imageCenterY);
                        /*
                        int imageCenterX = blobs->width / 2;
                        if (blobs->xc > imageCenterX) {
                            printf("Image is to the left\n");
                        } else if (blobs->xc < imageCenterX) {
                            printf("Image is to right\n");
                        } else {
                            printf("Image is in the center!\n");
                        }
                        */

                    }
                    // Draw a rectangle around the object
                    // and its top left corner...
                    cv::Point pt1(blobs->x, blobs->y);
                    // and its bottom right corner.
                    cv::Point pt2(blobs->x + blobs->width, blobs->y + blobs->height);
                    // These two calls...
                    cv::rectangle(frame, pt1, pt2, cv::Scalar(255, 0, 0),5);

                    // Draw a rectangle around the center of mass
                    // and its top left corner...
                    cv::Point pt3(blobs->xc, blobs->yc);
                    // and its bottom right corner.
                    cv::Point pt4(blobs->xc + 5 , blobs->yc + 5);
                    // These two calls...
                    cv::rectangle(frame, pt3, pt4, cv::Scalar(255, 0, 0),5);
                }
            }
            free(blobs);
        }

        /////////////////////vermelho
        //teste faz o segment de algumas cores
        vc_hsv_segmentation(image, imageRed, 345,10,70,100,70,100);
        //faz o blob labeling
        blobs = vc_binary_blob_labelling(imageRed,imageBlobsRed, &nblobs);
        circularidade = 0.00;

        if (blobs != NULL){
            //da a informação das blobs
            vc_binary_blob_info(imageBlobsRed,blobs, nblobs);

            for (size_t i = 0; i < nblobs; i++)
            {
                if( blobs[i].area > 7000){

                    printf("label %d | area %d \n", blobs[i].label, blobs[i].area);

                    //circular ?
                    circularidade= 4.00 * M_PI * blobs->area/(blobs->perimeter*blobs->perimeter);
                    printf("circularidade: %f\n", circularidade);
                    //TODO: melhorar a verificação para nao enganhar com o stop
                    if(circularidade > 0.22){
                        printf("É um sentido proibido");
                        str = std::string("SENTIDO PROIBIDO");
                        cv::putText(frame, str, cv::Point(blobs->x, blobs->y - 20), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
                        cv::putText(frame, str, cv::Point(blobs->x, blobs->y - 20), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 1);

                    }else{
                        str = std::string("STOP");
                        cv::putText(frame, str, cv::Point(blobs->x, blobs->y - 20), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
                        cv::putText(frame, str, cv::Point(blobs->x, blobs->y - 20), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 1);

                    }


                    // Draw a rectangle around the object
                    // and its top left corner...
                    cv::Point pt1(blobs->x, blobs->y);
                    // and its bottom right corner.
                    cv::Point pt2(blobs->x + blobs->width, blobs->y + blobs->height);
                    // These two calls...
                    cv::rectangle(frame, pt1, pt2, cv::Scalar(0, 0, 255),5);
                }
            }
            free(blobs);
        }


        //converter no final e amostrar e para dar debug ao segment
        //vc_convertToThreeChannels(imageBlue,image3);
		//memcpy(frame.data, image3->data, video.width * video.height * 3);

		/* Exibe a frame */
		cv::imshow("BIMBOLANDIA", frame);

		/* Sai da aplica��o, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}
	// Liberta a mem�ria da imagem IVC que havia sido criada
	vc_image_free(image);
	vc_image_free(image3);
	vc_image_free(image4);

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("BIMBOLANDIA");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}
