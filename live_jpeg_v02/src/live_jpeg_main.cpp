// live_jpeg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "encode_live_jpeg.h"
#include "Header.h"
#include "decode_live_jpeg.h"
#include "psnr.h"

using namespace std;

static const std::string opt_ref = "-r";
static const std::string opt_input = "-i";
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


#if 1

int main(int argc, char* argv[])
{
	char input_filename[MAX_PATH] = {};
	char ref_filename[MAX_PATH] = {};
	char output_filename[MAX_PATH] = {};
	int quality = 95;
	uint width = 0;
	uint height = 0;
	uint input_frame_count = 0;
	bool output_frame_count_flag = false;
	uint output_frame_count = 0;
	MEDIA_FORMAT input_format = MEDIA_YUV444I;
	MEDIA_FORMAT out_format = MEDIA_YUV444I;
	bool encode = false;
	bool decode = false;
	bool ycbcr = false;
	int sub_samp = 420;
	bool wsf_flag = false; // write each frame as seperate jpeg file.
	bool wdf_flag = false; // write each difference frame as seperate jpeg file.
	bool arithmetic_flag = false; // to arithmetic encoding.
	std::string work_type = "null";

	if (argc < 3) {
		print_usage();
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i++];
		if (arg == opt_input) {
			snprintf(input_filename, MAX_PATH, argv[i]);
		}
		else if (arg == opt_ref) {
			snprintf(ref_filename, MAX_PATH, argv[i]);
		}
		else if (arg == opt_output) {
			snprintf(output_filename, MAX_PATH, argv[i]);
		}
		else if (arg == opt_work) {
			work_type = argv[i];
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
	string ifname(input_filename);
	std::size_t a = ifname.rfind(".jpg");
	if (a != std::string::npos)
	{
		decode = true;
	}
	std::size_t aa = ifname.rfind(".yuv");
	if (aa != std::string::npos)
	{
		encode = true;
	}

	if (work_type == "psnr") {
		find_psnr(
			ref_filename,
			input_filename,
			width,
			height,
			input_frame_count,
			input_format,
			ycbcr);
		return 1;

	}

	if (encode) {

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
	if (decode) {
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

#else

#include <jpeg/include/jpeglib.h>

int main(int argc, char* argv[])
{


	FILE* In, * Out;
	fopen_s(&In, "VID_42_3840x1920_RGB24_0.yuv", "rb");
	fopen_s(&Out, "a123_arith.jpg", "wb");

	int width = 3840;
	int height = 1920;
	int quality = 100;

	std::cout << "Hello World!\n";
	unsigned int size = width * height * 3;
	unsigned int outsize = size;

	unsigned char* data = new unsigned char[size];
	unsigned char* out_data = new unsigned char[size];
	fread_s(data, size, sizeof(unsigned char), size, In);


	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &out_data, (unsigned long*)&outsize);

	cinfo.image_width = width; 	/* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	//cinfo.raw_data_in = true;
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	cinfo.optimize_coding = TRUE;


	jpeg_set_defaults(&cinfo);
	cinfo.comp_info[0].h_samp_factor = 1;
	cinfo.comp_info[0].v_samp_factor = 1;
	cinfo.comp_info[1].h_samp_factor = 1;
	cinfo.comp_info[1].v_samp_factor = 1;
	cinfo.comp_info[2].h_samp_factor = 1;
	cinfo.comp_info[2].v_samp_factor = 1;
	cinfo.arith_code = TRUE;

	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = cinfo.image_width * 3;	/* JSAMPLEs per row in image_buffer */
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &data[cinfo.next_scanline * row_stride];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);

	//jvirt_barray_ptr* dst_coef_arrays= NULL;
	//jpeg_start_compress(&cinfo, TRUE);


	//prepare_coefficients(&cinfo, dst_coef_arrays, (short*)mcus);


	//jpeg_write_coefficients(&cinfo, (jvirt_barray_ptr*)(dct_coeff_buff));

	// 	jpeg_write_coefficients(&cinfo, dst_coef_arrays);


	jpeg_destroy_compress(&cinfo);

	fwrite(out_data, outsize, sizeof(unsigned char), Out);
	fclose(In);
	fclose(Out);


}


#endif