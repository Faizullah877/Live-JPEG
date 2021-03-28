#include "psnr.h"
#include <iostream>
#include <chrono>
#define pixel_range 200
using namespace std;
void find_frame_psnr444I(byte* ref_frame_data, byte* input_frame_data, uint width, uint height, uint frame_size, double* psnrC0, double* psnrC1, double* psnrC2) {
	uint comp_size = width * height;
	double tmp_mseC0f = 0.0;
	double tmp_mseC1f = 0.0;
	double tmp_mseC2f = 0.0;
	double mse_c0f = 0.0;
	double mse_c1f = 0.0;
	double mse_c2f = 0.0;
	for (int i = 0; i < frame_size; i+=3) {
		tmp_mseC0f += (ref_frame_data[i+0] - input_frame_data[i+0]) * (ref_frame_data[i+0] - input_frame_data[i+0]);
		tmp_mseC1f += (ref_frame_data[i+1] - input_frame_data[i+1]) * (ref_frame_data[i+1] - input_frame_data[i+1]);
		tmp_mseC2f += (ref_frame_data[i+2] - input_frame_data[i+2]) * (ref_frame_data[i+2] - input_frame_data[i+2]);
	}
	mse_c0f = tmp_mseC0f / (double)comp_size;
	mse_c1f = tmp_mseC1f / (double)comp_size;
	mse_c2f = tmp_mseC2f / (double)comp_size;

	*psnrC0 = 10 * log10((pixel_range * pixel_range) / mse_c0f);
	*psnrC1 = 10 * log10((pixel_range * pixel_range) / mse_c1f);
	*psnrC2 = 10 * log10((pixel_range * pixel_range) / mse_c2f);
}

void find_psnr(
	const char* ref_filename,
	const char* input_filename,
	uint width,
	uint height,
	uint input_frame_count,
	MEDIA_FORMAT input_format,
	bool ycbcr
) 
{
	if (input_format != MEDIA_YUV444I) {
		cout << "Input Format not supported... " << endl;
		return;
	}
	uint frame_size;
	FILE* resulttxt;
	errno_t err3 = fopen_s(&resulttxt, "psnr_results.txt", "a+");
	if (err3 == 0)
	{
		//printf("\Input File  : %s ", In_file_name);
		//cout << endl;
	}
	else
	{
		printf("psnr_results.txt  was not opened ");
		cout << endl;
		//return;
	}
	auto start1 = chrono::high_resolution_clock::now();
	auto start_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	cout << "\nProcess ====>>>>>> Live_jpeg psnr calcuation" << endl;
	cout << "\tInput File Name            : " << input_filename << endl;
	cout << "\tFrame Width         : " << width << endl;
	cout << "\tFrame Height        : " << height << endl;
	cout << "\tPixels Color Space   : ";
	if (ycbcr) cout << "YCbCr" << endl; else cout << "RGB" << endl;

	if (err3 == 0) {
		char str1[26];
		ctime_s(str1, sizeof str1, &start_time);
		fprintf(resulttxt, "******************************************************************\n");
		fprintf(resulttxt, "Start Time                    : %s", str1);
		fprintf(resulttxt, "\nProcess ====>>>>>> Live_jpeg psnr calcuation.\n");
		fprintf(resulttxt, "\tInput File Name             : %s\n", input_filename);
		fprintf(resulttxt, "\tReference File Name             : %s\n", ref_filename);
		fprintf(resulttxt, "\tFrame Width         : %d\n", width);
		fprintf(resulttxt, "\tFrame Height        : %d\n", height);
		fprintf(resulttxt, "\tPixels Color Space   : ");
		if (ycbcr) fprintf(resulttxt, "YCbCr\n"); else fprintf(resulttxt, "RGB\n");
	}
	FILE* refFile;
	errno_t err = fopen_s(&refFile, ref_filename, "rb");
	if (err == 0)
	{
		//printf("\Input File  : %s ", In_file_name);
		//cout << endl;
	}
	else
	{
		printf("The %s for Reference was not opened ", ref_filename);
		fprintf(resulttxt, "\nError -> Failed to open source file. Name            : %s\n", ref_filename);
		cout << endl;
		return;
	}
	FILE* inFile;
	errno_t err1 = fopen_s(&inFile, input_filename, "rb");
	if (err1 == 0)
	{
		//printf("\Input File  : %s ", In_file_name);
		//cout << endl;
	}
	else
	{
		printf("The %s for Input was not opened ", input_filename);
		fprintf(resulttxt, "\nError -> Failed to open source file. Name            : %s\n", input_filename);
		cout << endl;
		return;
	}
	if (input_format == MEDIA_RGB444I || input_format == MEDIA_RGB444P || input_format == MEDIA_YUV444I || input_format == MEDIA_YUV444P){
		frame_size = width * height * 3;
	}
	else if (input_format == MEDIA_YUV420I || input_format == MEDIA_YUV420P) {
		frame_size = width * height * 1.5;
	}
	else {
		cout << "Unsupported format " << endl;
		return;
	}

	byte* ref_frame_data = new byte[frame_size];
	byte* input_frame_data = new byte[frame_size];

	double psnrC0 = 0; // C0 = component 0 (Y or R)
	double psnrC1 = 0;
	double psnrC2 = 0;


	for (uint frame_no = 0; frame_no < input_frame_count; frame_no++) {
		double psnrC0F = 0; // C0 = component 0 (Y or R) For each frame
		double psnrC1F = 0;
		double psnrC2F = 0;
		fread(ref_frame_data, sizeof(unsigned char), frame_size, refFile);
		fread(input_frame_data, sizeof(unsigned char), frame_size, inFile);
		if (input_format == MEDIA_YUV444I || input_format == MEDIA_RGB444I) {
			find_frame_psnr444I(ref_frame_data, input_frame_data, frame_size, width, height, &psnrC0F, &psnrC1F, &psnrC2F);
			//fprintf(resulttxt, "\tFrame No    : %d", frame_no);
			if ((frame_no) % 20 == 0)
				fprintf(resulttxt, "\n\tFrame No\t\tPSNR_Y\t\tPSNR_U\t\tPSNR_V\n");
			fprintf(resulttxt, "\t\t%d\t\t%f\t%f\t%f\n", frame_no, psnrC0F, psnrC1F, psnrC2F);
			//fprintf(resulttxt, "\tPSNR Component 0   : %f", psnrC0F);
			//fprintf(resulttxt, "\tPSNR Component 1   : %f", psnrC1F);
			//fprintf(resulttxt, "\tPSNR Component 2   : %f\n", psnrC2F);
			//printf("\tFrame No    : %d", frame_no);
			//printf("\tPSNR Component 0   : %f", psnrC0F);
			//printf("\tPSNR Component 1   : %f", psnrC1F);
			//printf("\tPSNR Component 2   : %f\n", psnrC2F);		
			printf("\r Frames No              : %d", frame_no);
			fflush(stdout);
		}


	}

	char str2[26];
	auto finish = chrono::high_resolution_clock::now();
	auto elapsed = finish - start1; 
	auto end_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	float time_elapsed = elapsed / std::chrono::milliseconds(1);
	ctime_s(str2, sizeof str2, &end_time);
	fprintf(resulttxt, "\tConsumed Time               : %f ms.\n", time_elapsed);
	fprintf(resulttxt, "End   Time                    : %s", str2);
	fprintf(resulttxt, "***************************************************************\n\n");


}