// live_jpeg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "encode_live_jpeg.h"
#include "encode_live_jpeg_folder.h"
#include "Header.h"
#include "decode_live_jpeg.h"
#include "psnr.h"

using namespace std;

static const std::string opt_input = "-i";
static const std::string opt_input_folder = "-f";
static const std::string opt_output = "-o";
static const std::string opt_input_fmt = "-isf";
static const std::string opt_output_fmt = "-osf";
static const std::string opt_input_frames_count = "-if";
static const std::string opt_output_frames_count = "-of";
static const std::string opt_input_width = "-w";
static const std::string opt_input_height = "-h";
static const std::string opt_quality = "-q";
static const std::string opt_pixels_format = "-pix_fmt";
static const std::string opt_sub_sampling = "-sub_samp";
static const std::string opt_wrt_jpg_frame = "-wsf";
static const std::string opt_wrt_dff_jpg_frame = "-wdf";
static const std::string opt_work = "-t";
static const std::string opt_entropy_enc = "-arith";


int main(int argc, char* argv[])
{
	char input_filename[MAX_PATH] = {"input.yuv"};
	char output_filename[MAX_PATH] = {"test"};
	int quality = 95;
	uint width = 0;
	uint height = 0;
	uint input_frame_count = 0;
	bool output_frame_count_flag = false;
	uint output_frame_count = 0;
	MEDIA_FORMAT input_format = MEDIA_YUV444I;
	MEDIA_FORMAT out_format = MEDIA_YUV444I;
	bool ycbcr = false;
	int sub_samp = 420;
	bool wsf_flag = false; // write each frame as seperate jpeg file.
	bool wdf_flag = false; // write each difference frame as seperate jpeg file.
	bool arithmetic_flag = false; // to arithmetic encoding.
	std::string work_type = "encode";
	bool folder_mode = false; // for folder containg .jpg images

	if (argc < 3) {
		print_usage();
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i++];
		if (arg == opt_work) {
			work_type = argv[i];
		}
		else if (arg == opt_input) {
			snprintf(input_filename, MAX_PATH, argv[i]);
			folder_mode = false;
		}
		else if (arg == opt_input_folder) {
			snprintf(input_filename, MAX_PATH, argv[i]);
			folder_mode = true;
		}
		else if (arg == opt_output) {
			snprintf(output_filename, MAX_PATH, argv[i]);
		}
		else if (arg == opt_pixels_format)
		{
			std::string opt = argv[i];
			if (opt == "RGB" || opt == "rgb")
				ycbcr = false;
			else if (opt == "YUV" || opt == "yuv")
				ycbcr = true;
			else {
				cout << "Only RGB and YUV input Pixels Color space are supported\n" << endl;
				print_usage();
				return -1;
			}
		}
		else if (arg == opt_sub_sampling)
		{
			std::string opt = argv[i];
			if (opt == "420")
				sub_samp = 420;
			else if (opt == "444")
				sub_samp = 444;
			else {
				cout << "Only 420 and 444 subsamping are supported\n" << endl;
				print_usage();
				return -1;
			}
		}
		else if (arg == opt_input_fmt) {
			std::string opt = argv[i];
			if (opt == "444P" || opt == "444p")
				input_format = MEDIA_YUV444P;
			else if (opt == "420P" || opt == "420p")
				input_format = MEDIA_YUV420P;
			else if (opt == "420I" || opt == "420i")
				input_format = MEDIA_YUV420I;
			else if (opt == "444I" || opt == "444i")
				input_format = MEDIA_YUV444I;
			else
			{
				cout << "input format not recognized" << endl;
				print_usage();
				return -1;
			}
		}
		else if (arg == opt_output_fmt) {
			std::string opt = argv[i];
			if (opt == "444P" || opt == "444p")
				out_format = MEDIA_YUV444P;
			else if (opt == "420P" || opt == "420p")
				out_format = MEDIA_YUV420P;
			else if (opt == "420I" || opt == "420i")
				out_format = MEDIA_YUV420I;
			else if (opt == "444I" || opt == "444i")
				out_format = MEDIA_YUV444I;
			else
			{
				cout << "Output format not recognized" << endl;
				print_usage();
				return -1;
			}
		}
		else if (arg == opt_input_frames_count) {
			input_frame_count = atoi(argv[i]);
		}
		else if (arg == opt_output_frames_count) {
			output_frame_count = atoi(argv[i]);
			output_frame_count_flag = true;
		}
		else if (arg == opt_input_width)
			width = atoi(argv[i]);

		else if (arg == opt_input_height)
			height = atoi(argv[i]);
		else if (arg == opt_wrt_jpg_frame) {
			int a = atoi(argv[i]);
			wsf_flag = a ? true : false;
		}
		else if (arg == opt_wrt_dff_jpg_frame) {
			int b = atoi(argv[i]);
			wdf_flag = b ? true : false;
		}
		else if (arg == opt_entropy_enc) {
			int c = atoi(argv[i]);
			arithmetic_flag = c ? true : false;
		}
		else if (arg == opt_quality)
			quality = atoi(argv[i]);
		else {
			printf("Invalid argument: %s\n", argv[i - 1]);
			print_usage();
			return 0;
		}
	}
	if (!output_frame_count_flag) {
		output_frame_count = input_frame_count;
	}
	if (output_frame_count > input_frame_count) {
		printf("Error - No. of Output Frames greater than No. of Input frames. \n");
		print_usage();
		return -1;
	}
	if (quality > 100 || quality < 1) {
		printf("Error - Quality value must be 1 - 100 (low to High).\n");
		print_usage();
		return -1;
	}

	if (input_format != MEDIA_YUV444P && input_format != MEDIA_YUV444I)
	{
		printf("Error - Input file pixels packing format is not supported. (only YUVI and YUVP are supported");
		print_usage();
		return -1;
	}

	if (out_format != MEDIA_YUV444P && out_format != MEDIA_YUV444I)
	{
		printf("Error - Output file pixels packing format is not supported. (only YUVI and YUVP are supported");
		print_usage();
		return -1;
	}
	//string ifname(input_filename);
	//std::size_t a = ifname.rfind(".jpg");
	//if (a != std::string::npos)
	//{
	//	decode = true;
	//}
	//std::size_t aa = ifname.rfind(".yuv");
	//if (aa != std::string::npos)
	//{
	//	encode = true;
	//}


	if (work_type == "encode") {
		if (folder_mode)
		{
			encode_live_jpeg_folder(
				argv[0],
				input_filename,
				output_filename,
				quality,
				ycbcr,
				sub_samp,
				wsf_flag,
				wdf_flag,
				arithmetic_flag);
		}
		else {
			if (width <= 0)
			{
				cout << "Width is not set" << endl;
				print_usage();
				return -1;
			}
			if (height <= 0)
			{
				cout << "height is not set" << endl;
				print_usage();
				return -1;
			}
			encode_live_jpeg(
				argv[0],
				input_filename,
				output_filename,
				width,
				height,
				quality,
				input_frame_count,
				output_frame_count,
				input_format,  //444p or 444i
				ycbcr,
				sub_samp,
				wsf_flag,
				wdf_flag,
				arithmetic_flag);
		}
	}
	if (work_type == "decode") {
		decode_live_jpeg(
			argv[0],
			input_filename,
			output_filename,
			ycbcr,
			out_format, // 444p or 444i
			wsf_flag,
			wdf_flag);
	}

	return 1;
}

