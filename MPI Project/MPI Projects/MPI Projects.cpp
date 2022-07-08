#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <mpi.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int *Red = new int[BM.Height * BM.Width];
	int *Green = new int[BM.Height * BM.Width];
	int *Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height*BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i*BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}

	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i*width + j] < 0)
			{
				image[i*width + j] = 0;
			}
			if (image[i*width + j] > 255)
			{
				image[i*width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//outputRes" + index + ".png");
	cout << "result Image Saved " << index << endl;
}


int main()
{
	int ImageWidth = 4, ImageHeight = 4;

	int start_s, stop_s, TotalTime = 0;
	MPI_Init(NULL, NULL);
	int rank;
	int size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int* imageData = new int[10];
	int* sum, *count, *mean = new int[4];
	if (rank == 0)
	{
		System::String^ imagePath;
		std::string img;
		img = "..//Data//Input//test.png";
		imagePath = marshal_as<System::String^>(img);
		imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);
		start_s = clock();
		int n = 4;
		sum = new int[n];
		count = new int[n];
		mean = new int[n];
		for (int i = 0; i < 4; i++) {
			sum[i] = 0;
			count[i] = 0;
			mean[i] = 0;
		}
		for (int i = 0; i < ImageWidth*ImageHeight; i++) {
			if (imageData[i] >= 0 && imageData[i] <= 64) {
				sum[0] += imageData[i];
				count[0] ++;
			}
			else if (imageData[i] > 64 && imageData[i] <= 128) {
				sum[1] += imageData[i];
				count[1] ++;
			}
			else if (imageData[i] > 128 && imageData[i] <= 192) {
				sum[2] += imageData[i];
				count[2] ++;
			}
			else if (imageData[i] > 192 && imageData[i] <= 256) {
				sum[3] += imageData[i];
				count[3] ++;
			}
		}
		for (int i = 0; i < n; i++) {
			mean[i] = sum[i] / count[i];
		}
		/*for (int i = 0; i < 4; i++)
			cout << mean[i] << endl;*/


	}
	MPI_Bcast(&ImageWidth, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&ImageHeight, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(mean, 4, MPI_INT, 0, MPI_COMM_WORLD);
	int s = ImageWidth * ImageHeight;
	int *subImg = new int[s / size];
	int* newsubImg = new int[s / size];
	MPI_Scatter(imageData, s / size, MPI_INT, subImg, s / size, MPI_INT, 0, MPI_COMM_WORLD);
	for (int i = 0; i < 4; i++)
		cout << mean[i] << endl;
	for (int i = 0; i < s / size; i++) {
		if (subImg[i] >= 0 && subImg[i] <= 64) {
			newsubImg[i] = mean[0];
		}
		else if (subImg[i] > 64 && subImg[i] <= 128) {
			newsubImg[i] = mean[1];
		}
		else if (subImg[i] > 128 && subImg[i] <= 192) {
			newsubImg[i] = mean[2];
		}
		else if (subImg[i] > 192 && subImg[i] < 256) {
			newsubImg[i] = mean[3];
		}
	}
	int* newImg = new int[s];
	MPI_Gather(newsubImg, s / size, MPI_INT, newImg, s / size, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0) {
		createImage(newImg, ImageWidth, ImageHeight, 0);
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << "time: " << TotalTime << endl;
	}

	free(imageData);
	MPI_Finalize();
	return 0;

}



