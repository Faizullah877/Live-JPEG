#include <iostream>
#include <stdio.h>
#include <vector>
#include <chrono>
#include <jpeg/include/jpeglib.h>
#include <jpeg/jpeg.h>
#include "Header.h"
#include "dct_coefficients.h"
#include "Box.h"
#include "codestream_box.h"
#include "write_jpeg.h"
#include "jpeg/src/turbojpeg.h"
#include "pix_format_converter.h"
#include "encode_live_jpeg_folder.h"

#include <fstream>
#include <iomanip>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;
struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */

	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr kf_info)
{
	my_error_ptr myerr = (my_error_ptr)kf_info->err;
	(*kf_info->err->output_message) (kf_info);
	longjmp(myerr->setjmp_buffer, 1);
}

int	read_jpeg_memory1(unsigned char* mem, unsigned long mem_size, bool ycbcr, JSAMPLE* image_buffer)
{
	struct jpeg_decompress_struct decode_info;
	struct my_error_mgr jerr1;
	/* More stuff */
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride_f;		/* physical row width in output buffer */
	decode_info.err = jpeg_std_error(&jerr1.pub);
	jerr1.pub.error_exit = my_error_exit;
	if (setjmp(jerr1.setjmp_buffer)) {
		jpeg_destroy_decompress(&decode_info);
		return 0;
	}
	jpeg_create_decompress(&decode_info);
	jpeg_mem_src(&decode_info, mem, mem_size);
	(void)jpeg_read_header(&decode_info, TRUE);
	if (ycbcr == true) {
		decode_info.out_color_space = JCS_YCbCr;
	}
	(void)jpeg_start_decompress(&decode_info);
	row_stride_f = decode_info.output_width * decode_info.output_components;
	buffer = (*decode_info.mem->alloc_sarray)
		((j_common_ptr)&decode_info, JPOOL_IMAGE, row_stride_f, 1);
	while (decode_info.output_scanline < decode_info.output_height) {
		(void)jpeg_read_scanlines(&decode_info, buffer, 1);
		JSAMPLE* ptr = buffer[0];
		for (int i = 0; i < row_stride_f; i += 3) {
			*image_buffer++ = *ptr++;
			*image_buffer++ = *ptr++;
			*image_buffer++ = *ptr++;
		}
	}
	(void)jpeg_finish_decompress(&decode_info);
	jpeg_destroy_decompress(&decode_info);
	return 1;
}

int encode_live_jpeg_folder(
	char* executable_name,
	const char* In_folder,
	const char* out_file_name,
	uint quality,
	bool ycbcr,
	int subsamp,
	bool wsf_flag,
	bool wdf_flag,
	bool arithmetic_flag) {


	vector<string> images_names;
	if (!fs::exists(In_folder))
	{
		cout << "path does not exits" << endl;
		return 0;
	}
	for (const auto& entry : fs::directory_iterator(In_folder))
	{
		const string s = entry.path().string();
		std::size_t a = s.rfind(".jpg");
		if (a != std::string::npos)
		{
			images_names.push_back(s);
		}
		else {
			cout << "Input file : " << s << " is not JPEG." << endl;
			continue;
		}
	}
	uint no_of_images = images_names.size();
	cout << "Total Number of Input images : " << no_of_images << endl;
	vector<DataBuffer*> jpg_bytestreams;
	uint infiles_total_size = 0;

	for (uint i = 0; i < no_of_images; ++i)
	{
		DataBuffer* ptr = new DataBuffer();
		ReadBytestream2(images_names[i].c_str(), ptr);
		jpg_bytestreams.push_back(ptr);
		infiles_total_size += ptr->size;
	}
	cout << "Combined Size of all input images is : " << infiles_total_size << endl;

	Byte* keyFrameJPG = jpg_bytestreams[0]->data;  // first image of input image exactly same
	uint keyFrameJPGsize = jpg_bytestreams[0]->size;


	/*******************Start: first decode keyframe to yuv spatial domain****************************/
	struct jpeg_decompress_struct kf_info;  // key frame information
	struct my_error_mgr jerr2;
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */
	kf_info.err = jpeg_std_error(&jerr2.pub);
	jerr2.pub.error_exit = my_error_exit;
	if (setjmp(jerr2.setjmp_buffer)) {
		jpeg_destroy_decompress(&kf_info);
		return 0;
	}
	jpeg_create_decompress(&kf_info);
	jpeg_mem_src(&kf_info, keyFrameJPG, keyFrameJPGsize);
	(void)jpeg_read_header(&kf_info, TRUE);
	uint width = kf_info.image_width;
	uint height = kf_info.image_height;
	uint components = kf_info.num_components;
	uint raw_size = width * height * components;
	unsigned char* KeyFramePixels = new unsigned char[raw_size];
	read_jpeg_memory1(keyFrameJPG, keyFrameJPGsize, ycbcr, KeyFramePixels);
	//(void)jpeg_finish_decompress(&kf_info);
	//jpeg_destroy_decompress(&kf_info);
	/*********END: Key Frame decoding to ycbcr spatical domain done*****************************/


	uint output_frame_count = no_of_images;
	bool diff_frame_size_to_txt = false;
	fstream fs;
	if (diff_frame_size_to_txt) {

		fs.open("diff_frame_sizes.txt", std::fstream::in | std::fstream::out | std::fstream::app);
	}

	FILE* resulttxt;
	errno_t err3 = fopen_s(&resulttxt, "Live_jpeg_Encoding_history.txt", "a+");
	if (err3 == 0)
	{
		//printf("\Input File  : %s ", In_file_name);
		//cout << endl;
	}
	else
	{
		printf("Live_jpeg_Encoding_history.txt  was not opened ");
		cout << endl;
		//return;
	}
	auto start1 = chrono::high_resolution_clock::now();
	auto start_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	printf("Running : %s\n", executable_name);
	cout << "Process ====>>>>>> Live_jpeg Encoding" << endl;
	cout << "Source properties" << endl;
	cout << "\tSource File Name            : " << In_folder << endl;
	cout << "\tSource Frames Width         : " << width << endl;
	cout << "\tSource Frames Height        : " << height << endl;
	cout << "Encoding Paramenters" << endl;
	cout << "\tSubsampling                 : " << subsamp << endl;
	cout << "\tQuality                     : " << quality << endl;
	if (arithmetic_flag) cout << "\tEntropy Encoding Scheme     : Arithmetic\n"; else  cout << "\tEntropy Encoding Scheme    : Huffman\n";
	cout << "\tTotal input images          : " << no_of_images << endl;
	cout << "\tFrames to encode            : " << output_frame_count << endl;

	if (err3 == 0) {
		char str1[26];
		ctime_s(str1, sizeof str1, &start_time);
		fprintf(resulttxt, "******************************************************************\n");
		fprintf(resulttxt, "Running                       : %s\n", executable_name);
		fprintf(resulttxt, "Start Time                    : %s", str1);
		fprintf(resulttxt, "\nProcess ====>>>>>> Live_jpeg Encoding.\n");
		fprintf(resulttxt, "\nSource properties.\n");
		fprintf(resulttxt, "\tSource File Name             : %s\n", In_folder);
		fprintf(resulttxt, "\tSource Frames Width         : %d\n", width);
		fprintf(resulttxt, "\tSource Frames Height        : %d\n", height);
		fprintf(resulttxt, "\tSource pixels Color Space   : ");
		if (ycbcr) fprintf(resulttxt, "YCbCr\n"); else fprintf(resulttxt, "RGB\n");
		fprintf(resulttxt, "Encoding Paramenters.\n");
		fprintf(resulttxt, "\tSubsampling                    : %d\n", subsamp);
		fprintf(resulttxt, "\tQuality                            : %d\n", quality);
		if (arithmetic_flag) fprintf(resulttxt, "\tEntropy Encoding Scheme     : Arithmetic\n"); else  fprintf(resulttxt, "\tEntropy Encoding Scheme    : Huffman\n");
		fprintf(resulttxt, "\tTotal input Frames          : %d\n", no_of_images);
		fprintf(resulttxt, "\tFrames to encode..          : %d\n", output_frame_count);

	}

	DataBuffer* pBuffer = new DataBuffer();
	DataBuffer* ptr = new DataBuffer();
	Byte* keyF_gmis_jpg = NULL;
	uint keyF_gmis_jpg_size = 0;

	Byte* jpeg_buf = NULL;
	uint jpeg_size = 0;
	uint* diff_frame_sos_size = new uint[output_frame_count];
	diff_frame_sos_size[0] = output_frame_count;

	J_COLOR_SPACE in_color; 	/* colorspace of input image */

	if (ycbcr)
		in_color = JCS_YCbCr;
	else
		in_color = JCS_RGB;

	ptr->info.width = width;
	ptr->info.height = height;
	ptr->info.num_comp = 3;
	uint rawFrameSize = ptr->info.width * ptr->info.height * 3;

	/*******Start: Recompress KeyFrame to jpeg based on gmis parameters*******/
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride_ff;		/* physical row width in image buffer */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &keyF_gmis_jpg, (unsigned long*)&keyF_gmis_jpg_size);
	cinfo.image_width = width; 	/* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = in_color; 	/* colorspace of input image */
	cinfo.optimize_coding = TRUE;
	jpeg_set_defaults(&cinfo);
	cinfo.write_JFIF_header = false;
	if (subsamp == 420) {
		cinfo.comp_info[0].h_samp_factor = 2;
		cinfo.comp_info[0].v_samp_factor = 2;
		cinfo.comp_info[1].h_samp_factor = 1;
		cinfo.comp_info[1].v_samp_factor = 1;
		cinfo.comp_info[2].h_samp_factor = 1;
		cinfo.comp_info[2].v_samp_factor = 1;
	}
	else if (subsamp == 444) {
		cinfo.max_h_samp_factor = 1;
		cinfo.max_v_samp_factor = 1;
		cinfo.comp_info[0].h_samp_factor = 1;
		cinfo.comp_info[0].v_samp_factor = 1;
		cinfo.comp_info[1].h_samp_factor = 1;
		cinfo.comp_info[1].v_samp_factor = 1;
		cinfo.comp_info[2].h_samp_factor = 1;
		cinfo.comp_info[2].v_samp_factor = 1;
	}
	if (arithmetic_flag)
		cinfo.arith_code = TRUE;
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride_ff = cinfo.image_width * 3;	/* JSAMPLEs per row in image_buffer */
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &KeyFramePixels[cinfo.next_scanline * row_stride_ff];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	/**********End: Recompress KeyFrame to jpeg based on gmis parameters********/



	// Key Frame in gmis_jpeg is ready to write
	if (wsf_flag) { // write soure frame in jpeg 
		FILE* ff;
		errno_t err12;
		char fileN[100];
		sprintf_s(fileN, 100, "Frame_0.jpg");
		err12 = fopen_s(&ff, (char*)fileN, "wb");
		fwrite(keyF_gmis_jpg, sizeof(unsigned char), keyF_gmis_jpg_size, ff);
		fclose(ff);
	}

	/*************** Start: get coefficients of KeyFrame************/
	HANDLE_JPEG hjpeg1;
	uint coeff_buf_size;
	coeff_buf_size = sizeof(short) * DCTSIZE2 * cinfo.comp_info[0].width_in_blocks * cinfo.comp_info[0].height_in_blocks * cinfo.num_components;
	short* PreviousCoeff = new short[coeff_buf_size];
	get_coefficients_f(keyF_gmis_jpg, keyF_gmis_jpg_size, hjpeg1, PreviousCoeff);
	/*************** End: get coefficients of KeyFrame************/


	if (0) { // just to see extracted coeffeients re-jpeg-writing => result show both recons.. and orig are same with every quality.
		pBuffer->additional_data = (unsigned char*)PreviousCoeff;
		write_jpeg_memory_coefficients(pBuffer, hjpeg1, arithmetic_flag);
		FILE* fd;
		errno_t err111;
		char fileN111[100];
		sprintf_s(fileN111, 100, "Frame1_rec.jpg");
		err111 = fopen_s(&fd, (char*)fileN111, "wb");
		fwrite(pBuffer->data, sizeof(unsigned char), pBuffer->size, fd);
		fclose(fd);
	}


	short* nextCoeff = new short[coeff_buf_size];
	short* diff = new short[coeff_buf_size];
	delete pBuffer->data;
	pBuffer->data = new unsigned char[jpeg_size];
	memcpy(pBuffer->data, jpeg_buf, jpeg_size);
	pBuffer->size = jpeg_size;
	uint diff_jpeg_size = 0;
	Byte* jpegpool = NULL;
	uint jpegpoolsize = 0;
	vector<Byte> sos_pool;

	if (diff_frame_size_to_txt) {
		fs << " Size of frame in Bytes   :  ";
		fs << setw(4) << 0;
		fs << "  :  is     :    ";
		fs << setw(13) << jpeg_size;
		fs << endl;
	}

	for (int i = 1; i < output_frame_count; ++i) {
		unsigned char* FramePixels = new unsigned char[raw_size];
		read_jpeg_memory1(jpg_bytestreams[i]->data, jpg_bytestreams[i]->size, ycbcr, FramePixels);
		Byte* jpeg_buf1 = NULL;
		int jpeg_size1 = 0;
		struct jpeg_compress_struct cinfo1;
		struct jpeg_error_mgr jerr1;
		cinfo1.err = jpeg_std_error(&jerr1);
		jpeg_create_compress(&cinfo1);
		JSAMPROW row_pointer1[1];	/* pointer to JSAMPLE row[s] */
		int row_stride1;		/* physical row width in image buffer */
		cinfo1.err = jpeg_std_error(&jerr1);
		jpeg_create_compress(&cinfo1);
		jpeg_mem_dest(&cinfo1, &jpeg_buf1, (unsigned long*)&jpeg_size1);
		cinfo1.image_width = width; 	/* image width and height, in pixels */
		cinfo1.image_height = height;
		cinfo1.input_components = 3;		/* # of color components per pixel */
		cinfo1.in_color_space = in_color; 	/* colorspace of input image */
		//cinfo1.raw_data_in = true;
		//cinfo.arith_code = TRUE;
		cinfo1.optimize_coding = TRUE;
		jpeg_set_defaults(&cinfo1);
		cinfo1.write_JFIF_header = false;
		if (subsamp == 420) {
			cinfo1.comp_info[0].h_samp_factor = 2;
			cinfo1.comp_info[0].v_samp_factor = 2;
			cinfo1.comp_info[1].h_samp_factor = 1;
			cinfo1.comp_info[1].v_samp_factor = 1;
			cinfo1.comp_info[2].h_samp_factor = 1;
			cinfo1.comp_info[2].v_samp_factor = 1;
		}
		else if (subsamp == 444) {
			cinfo1.max_h_samp_factor = 1;
			cinfo1.max_v_samp_factor = 1;
			cinfo1.comp_info[0].h_samp_factor = 1;
			cinfo1.comp_info[0].v_samp_factor = 1;
			cinfo1.comp_info[1].h_samp_factor = 1;
			cinfo1.comp_info[1].v_samp_factor = 1;
			cinfo1.comp_info[2].h_samp_factor = 1;
			cinfo1.comp_info[2].v_samp_factor = 1;
		}
		jpeg_set_quality(&cinfo1, quality, TRUE /* limit to baseline-JPEG values */);
		if (arithmetic_flag)
			cinfo1.arith_code = TRUE;

		jpeg_start_compress(&cinfo1, TRUE);
		row_stride1 = cinfo1.image_width * 3;	/* JSAMPLEs per row in image_buffer */
		while (cinfo1.next_scanline < cinfo1.image_height) {
			row_pointer[0] = &FramePixels[cinfo1.next_scanline * row_stride1];
			(void)jpeg_write_scanlines(&cinfo1, row_pointer, 1);
		}
		jpeg_finish_compress(&cinfo1);
		if (wsf_flag) { // write each frame in jpeg seperate
			FILE* fff;
			errno_t err;
			char fileN[100];
			sprintf_s(fileN, 100, "Frame_%d.jpg", i);
			err = fopen_s(&fff, (char*)fileN, "wb");
			fwrite(jpeg_buf1, sizeof(unsigned char), jpeg_size1, fff);
			fclose(fff);
		}
		if (diff_frame_size_to_txt) {
			fs << " Size of frame in Bytes   :  ";
			fs << setw(4) << i;
			fs << "  :  is     :    ";
			fs << jpeg_size1;
		}
		delete pBuffer->data;
		pBuffer->data = new unsigned char[jpeg_size1];
		memcpy(pBuffer->data, jpeg_buf1, jpeg_size1);
		pBuffer->size = jpeg_size1;
		get_coefficients_f(jpeg_buf1, jpeg_size1, hjpeg1, nextCoeff);

		for (uint c = 0; c < coeff_buf_size; c++) {
			*(diff + c) = *(PreviousCoeff + c) - *(nextCoeff + c);
		}
		pBuffer->additional_data = (unsigned char*)diff;

		diff_jpeg_size = write_jpeg_memory_coefficients(pBuffer, hjpeg1, arithmetic_flag);
		if (diff_frame_size_to_txt) {
			fs << "     >>   diff   :   ";
			fs << setw(13) << diff_jpeg_size;
			fs << endl;
		}

		sos_pool.insert(sos_pool.end(), pBuffer->data, pBuffer->data + pBuffer->size);

		diff_frame_sos_size[i] = pBuffer->size;

		if (wdf_flag) {
			FILE* fff111;
			errno_t err111;
			char fileN111[100];
			sprintf_s(fileN111, 100, "diff_Frame_%d.jpg", i);
			err111 = fopen_s(&fff111, (char*)fileN111, "wb");
			fwrite(pBuffer->data, sizeof(unsigned char), pBuffer->size, fff111);
			fclose(fff111);
		}

		memcpy(PreviousCoeff, nextCoeff, coeff_buf_size);

		printf("\r                Frame Encoding.....              : %d", i);
		fflush(stdout);
		jpeg_destroy_compress(&cinfo1);
	}
	if (diff_frame_size_to_txt)
		fs.close();

	cout << endl;

	jpeg_destroy_compress(&cinfo);
	Byte* uuid_box_data = NULL;
	uint uuid_box_size;
	Byte* dst_data1 = NULL;
	unsigned int dst_size1;
	sj_jumbf_super_box_ptr sosL_box = sj_create_jumbf_sosL_box(diff_frame_sos_size, output_frame_count, (char*)"Diff_frames_SOS_Lengths");
	jpeg_write_sos_jumbf(keyF_gmis_jpg, keyF_gmis_jpg_size, sosL_box, &dst_data1, &dst_size1);
	unsigned char* dst_data = NULL;
	unsigned int dst_size = 0;
	sj_jumbf_super_box_ptr uuid_box = sj_create_jumbf_codestream_box(sos_pool.data(), sos_pool.size(), (char*)"LiveJPEG_SOS");
	jpeg_write_jumbf(dst_data1, dst_size1, uuid_box, &dst_data, &dst_size);
	delete pBuffer->data;
	FILE* outFile;
	errno_t err1 = fopen_s(&outFile, out_file_name, "wb");
	if (err1 == 0)
	{
		//printf("\Output File  : %s ", out_file_name);
		//cout << endl;
	}
	else
	{
		fprintf(resulttxt, "\nError -> Failed to open output file. Name            : %s\n", out_file_name);
		printf("The %s for Output was not opened ", out_file_name);
		cout << endl;
		return 0;
	}
	double Source_size = (double)width * (double)height * (double)output_frame_count;
	double dist_size = dst_size;
	float bpp = (float)((dist_size * 8.0) / Source_size);
	fwrite(dst_data, sizeof(unsigned char), dst_size, outFile);
	fclose(outFile);
	auto finish = chrono::high_resolution_clock::now();
	auto elapsed = finish - start1;
	float ISBR = double(infiles_total_size - dst_size) / double(infiles_total_size);
	cout << "Output Properties " << endl;
	cout << "\tTotal Frames Encoded        : " << output_frame_count << endl;
	cout << "\tOutput File Name            : " << out_file_name << endl;
	cout << "\tAPP11 Markers               : " << (sos_pool.size() / 65000) + 1 << endl;
	cout << "\tOutput File Size            : " << dst_size << " Bytes " << endl;
	cout << "\tbit per pixels              : " << bpp << endl;
	cout << "\tISBR                        : " << ISBR << endl;
	cout << "\tEncoding Time               : " << elapsed / std::chrono::milliseconds(1) / 1000.0 << " sec" << endl;

	auto end_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	float time_elapsed = elapsed / std::chrono::milliseconds(1) / 1000.0;
	if (err3 == 0) {
		char str2[26];
		ctime_s(str2, sizeof str2, &end_time);
		fprintf(resulttxt, "Output Properties.\n");
		fprintf(resulttxt, "\tTotal frames encoded        : %d\n", output_frame_count);
		fprintf(resulttxt, "\tOutput File Name            : %s\n", out_file_name);
		fprintf(resulttxt, "\tAPP11 Markers               : %d\n", (sos_pool.size() / 65000) + 1);
		fprintf(resulttxt, "\tOutput File Size              : %d Bytes.\n", dst_size);
		fprintf(resulttxt, "\tbit per pixels                 : %f bits per pixel.\n", bpp);
		fprintf(resulttxt, "\tISBR                           : %f \n", ISBR);
		fprintf(resulttxt, "\tEncoding Time               : %f sec.\n", time_elapsed);
		fprintf(resulttxt, "End   Time                    : %s", str2);
		fprintf(resulttxt, "***************************************************************\n\n");
	}
	fclose(resulttxt);

	return 1;
}

