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

#include <fstream>
#include <iomanip>
using namespace std;

void encode_live_jpeg(
	char* executable_name,
	const char* In_file_name,
	const char* out_file_name,
	uint width,
	uint height,
	uint quality,
	uint input_frame_count,
	uint output_frame_count,
	MEDIA_FORMAT input_format,
	bool ycbcr,
	int subsamp,
	bool wsf_flag,
	bool wdf_flag,
	bool arithmetic_flag,
	bool enable_log_file) {
	if (input_format != MEDIA_YUV444I && input_format != MEDIA_YUV444P) {
		cout << "Input Format not supported... " << endl;
		return;
	}

	bool diff_frame_size_to_txt = false;
	fstream fs;
	if (diff_frame_size_to_txt) {

		fs.open("diff_frame_sizes.txt", std::fstream::in | std::fstream::out | std::fstream::app);
	}


	auto start1 = chrono::high_resolution_clock::now();
	auto start_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	printf("Running : %s\n", executable_name);
	cout << "Process ====>>>>>> Live_jpeg Encoding" << endl;
	cout << "Source properties" << endl;
	cout << "\tSource File Name            : " << In_file_name << endl;
	cout << "\tSource Frames Width         : " << width << endl;
	cout << "\tSource Frames Height        : " << height << endl;
	cout << "\tSource pixels Color Space   : ";
	if (ycbcr) cout << "YCbCr" << endl; else cout << "RGB" << endl;
	cout << "Encoding Paramenters" << endl;
	cout << "\tSubsampling                 : " << subsamp << endl;
	cout << "\tQuality                     : " << quality << endl;
	if (arithmetic_flag) cout << "\tEntropy Encoding Scheme     : Arithmetic\n"; else  cout << "\tEntropy Encoding Scheme    : Huffman\n";
	cout << "\tTotal input frames          : " << input_frame_count << endl;
	cout << "\tFrames to encode            : " << output_frame_count << endl;

	DataBuffer* pBuffer = new DataBuffer();
	DataBuffer* ptr = new DataBuffer();
	Byte* keyFrameJPG = NULL;
	uint keyFrameJPGsize = 0;

	Byte* jpeg_buf = NULL;
	uint jpeg_size = 0;
	uint* diff_frame_sos_size = new uint[output_frame_count];
	diff_frame_sos_size[0] = output_frame_count;

	J_COLOR_SPACE in_color; 	/* colorspace of input image */

	if (ycbcr)
		in_color = JCS_YCbCr;
	else
		in_color = JCS_RGB;

	FILE* InFile;
	errno_t err = fopen_s(&InFile, In_file_name, "rb");
	if (err == 0)
	{
		//printf("\Input File  : %s ", In_file_name);
		//cout << endl;
	}
	else
	{
		printf("The %s for Input was not opened ", In_file_name);
		//fprintf(resulttxt, "\nError -> Failed to open source file. Name            : %s\n", In_file_name);
		cout << endl;
		return;
	}



	ptr->info.width = width;
	ptr->info.height = height;
	ptr->info.num_comp = 3;

	uint rawFrameSize = ptr->info.width * ptr->info.height * 3;
	Byte* rawFrameData = new Byte[rawFrameSize];
	int size = fread(rawFrameData, sizeof(unsigned char), rawFrameSize, InFile);
	if (input_format == MEDIA_YUV444P)
	{
		convert_yuv444p_to_yuv444I(rawFrameData, rawFrameSize, width, height);
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &jpeg_buf, (unsigned long*)&jpeg_size);
	cinfo.image_width = width; 	/* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = in_color; 	/* colorspace of input image */



	//cinfo.raw_data_in = true; 
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
	row_stride = cinfo.image_width * 3;	/* JSAMPLEs per row in image_buffer */
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &rawFrameData[cinfo.next_scanline * row_stride];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	Byte* header_buf = NULL;
	uint header_buf_size;

	copy_jpeg_header(jpeg_buf, jpeg_size, &header_buf, &header_buf_size);


	// Key Frame in jpeg is ready to write
	if (wsf_flag) { // write soure frame in jpeg 
		FILE* ff;
		errno_t err12;
		char fileN[100];
		sprintf_s(fileN, 100, "Frame_0.jpg");
		err12 = fopen_s(&ff, (char*)fileN, "wb");
		fwrite(jpeg_buf, sizeof(unsigned char), jpeg_size, ff);
		fclose(ff);
	}
	keyFrameJPGsize = jpeg_size;
	keyFrameJPG = new Byte[jpeg_size];
	memcpy(keyFrameJPG, jpeg_buf, jpeg_size);
	//memcpy_s(keyFrameJPG, jpeg_size, jpeg_buf, jpeg_size);

	HANDLE_JPEG hjpeg1;
	uint coeff_buf_size;
	coeff_buf_size = sizeof(short) * DCTSIZE2 * cinfo.comp_info[0].width_in_blocks * cinfo.comp_info[0].height_in_blocks * cinfo.num_components;
	short* PreviousCoeff = new short[coeff_buf_size];
	get_coefficients_f(jpeg_buf, jpeg_size, hjpeg1, PreviousCoeff);

	if (0) { // just to see extracted coeffeients re-jpeg-writing => result show both recons.. and orig are same with every quality.
		pBuffer->additional_data = (unsigned char*)PreviousCoeff;
		write_jpeg_memory_coefficients(pBuffer, hjpeg1, arithmetic_flag);
		FILE* fd;
		errno_t err111;
		char fileN111[100];
		sprintf_s(fileN111, 100, "Frame1_rec.jpg");
		err111 = fopen_s(&fd, (char*)fileN111, "wb");

		//fwrite(header_buf, sizeof(unsigned char), header_buf_size, fff111);
		fwrite(pBuffer->data, sizeof(unsigned char), pBuffer->size, fd);
		//Byte eoi[2];
	//	eoi[0] = 0xFF;
		//eoi[1] = 0xD9; 
		//fwrite(eoi, sizeof(unsigned char), 2, fff111);
		fclose(fd);
	}


	//short *PreviousCoeff= get_coefficients1(jpeg _buf, jpeg_size, hjpeg1, &coeff_buf_size);
	//short *diff
	//	cout << "Frame 1 Coeff size : " << coeff_buf_size << endl;
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
	//jpeg_destroy_compress(&cinfo);

	if (diff_frame_size_to_txt) {
		fs << " Size of frame in Bytes   :  ";
		fs << setw(4) << 0;
		fs << "  :  is     :    ";
		fs << setw(13) << jpeg_size;
		fs << endl;
	}

	for (int i = 1; i < output_frame_count; ++i) {
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


		fread(rawFrameData, sizeof(unsigned char), rawFrameSize, InFile);
		//cout << "in here\n" << ftell(InFile);
		if (input_format == MEDIA_YUV444P)
		{
			convert_yuv444p_to_yuv444I(rawFrameData, rawFrameSize, width, height);
		}

		jpeg_start_compress(&cinfo1, TRUE);
		row_stride = cinfo1.image_width * 3;	/* JSAMPLEs per row in image_buffer */
		while (cinfo1.next_scanline < cinfo1.image_height) {
			row_pointer[0] = &rawFrameData[cinfo1.next_scanline * row_stride];
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



		//int uniq_freq[3340] = { 0 };
		//int uniq_freq_counter = 0;
		//int total_data = 0;
		//int min = 0; int max = 0; int max_frq_occur = 0;
		//cout << "Frame 2 JPEG size : " << jpeg_size1 << endl;
		delete pBuffer->data;
		pBuffer->data = new unsigned char[jpeg_size1];
		memcpy(pBuffer->data, jpeg_buf1, jpeg_size1);
		pBuffer->size = jpeg_size1;
		get_coefficients_f(jpeg_buf1, jpeg_size1, hjpeg1, nextCoeff);
		// find difference of coeffients. 

		for (uint c = 0; c < coeff_buf_size; c++) {
			*(diff + c) = *(PreviousCoeff + c) - *(nextCoeff + c);
		}

		pBuffer->additional_data = (unsigned char*)diff;


		//pBuffer->additional_data = (unsigned char*)PreviousCoeff;
		diff_jpeg_size = write_jpeg_memory_coefficients(pBuffer, hjpeg1, arithmetic_flag);
		if (diff_frame_size_to_txt) {
			fs << "     >>   diff   :   ";
			fs << setw(13) << diff_jpeg_size;
			fs << endl;
		}

		//Byte* curr_sos_data = NULL;
		//unsigned int curr_sos_size = 0;
		//extract_curr_sos_data(pBuffer->data, pBuffer->size, &curr_sos_data, &curr_sos_size);
		sos_pool.insert(sos_pool.end(), pBuffer->data, pBuffer->data + pBuffer->size);

		diff_frame_sos_size[i] = pBuffer->size;
		//if (i < 5) {
		//	cout << "Current Sos_size of diff frame : " << i << " is : " << curr_sos_size << endl;
		//}

		if (wdf_flag) {
			FILE* fff111;
			errno_t err111;
			char fileN111[100];
			sprintf_s(fileN111, 100, "diff_Frame_%d.jpg", i);
			err111 = fopen_s(&fff111, (char*)fileN111, "wb");

			//fwrite(header_buf, sizeof(unsigned char), header_buf_size, fff111);
			fwrite(pBuffer->data, sizeof(unsigned char), pBuffer->size, fff111);
			//Byte eoi[2];
		//	eoi[0] = 0xFF;
			//eoi[1] = 0xD9; 
			//fwrite(eoi, sizeof(unsigned char), 2, fff111);
			fclose(fff111);
		}

		//delete jpeg_buf1;
		//PreviousCoeff = nextCoeff;
		memcpy(PreviousCoeff, nextCoeff, coeff_buf_size);
		//cout << "Diff JPEG size : " << diff_jpeg_size << endl;

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

	jpeg_write_sos_jumbf(keyFrameJPG, keyFrameJPGsize, sosL_box, &dst_data1, &dst_size1);

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
		//fprintf(resulttxt, "\nError -> Failed to open output file. Name            : %s\n", out_file_name);
		printf("The %s for Output was not opened ", out_file_name);
		cout << endl;
		return;
	}
	double Source_size = (double)width * (double)height * (double)output_frame_count;
	double dist_size = dst_size;
	float bpp = (float)((dist_size * 8.0) / Source_size);
	fwrite(dst_data, sizeof(unsigned char), dst_size, outFile);
	fclose(outFile);
	auto finish = chrono::high_resolution_clock::now();
	auto elapsed = finish - start1;

	cout << "Output Properties " << endl;
	cout << "\tTotal Frames Encoded        : " << output_frame_count << endl;
	cout << "\tOutput File Name            : " << out_file_name << endl;
	cout << "\tAPP11 Markers               : " << (sos_pool.size() / 65000) + 1 << endl;
	cout << "\tOutput File Size            : " << dst_size << " Bytes " << endl;
	cout << "\tbit per pixels              : " << bpp << endl;
	cout << "\tEncoding Time               : " << elapsed / std::chrono::milliseconds(1) / 1000.0 << " sec" << endl;

	auto end_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	float time_elapsed = elapsed / std::chrono::milliseconds(1) / 1000.0;

	if (enable_log_file) {
		FILE* resulttxt;
		errno_t err3 = fopen_s(&resulttxt, "DPCMed_JPEG_encoding_history.txt", "a+");
		if (err3 == 0)
		{
			char str1[26];
			ctime_s(str1, sizeof str1, &start_time);
			fprintf(resulttxt, "******************************************************************\n");
			fprintf(resulttxt, "Running                       : %s\n", executable_name);
			fprintf(resulttxt, "Start Time                    : %s", str1);
			fprintf(resulttxt, "\nProcess ====>>>>>> DPCMed-JPEG Encoding.\n");
			fprintf(resulttxt, "\nSource properties.\n");
			fprintf(resulttxt, "\tSource File Name            : %s\n", In_file_name);
			fprintf(resulttxt, "\tSource Frames Width         : %d\n", width);
			fprintf(resulttxt, "\tSource Frames Height        : %d\n", height);
			fprintf(resulttxt, "\tSource pixels Color Space   : ");
			if (ycbcr) fprintf(resulttxt, "YCbCr\n"); else fprintf(resulttxt, "RGB\n");
			fprintf(resulttxt, "Encoding Paramenters.\n");
			fprintf(resulttxt, "\tSubsampling                 : %d\n", subsamp);
			fprintf(resulttxt, "\tQuality                     : %d\n", quality);
			if (arithmetic_flag) fprintf(resulttxt, "\tEntropy Encoding Scheme     : Arithmetic\n"); else  fprintf(resulttxt, "\tEntropy Encoding Scheme     : Huffman\n");
			fprintf(resulttxt, "\tTotal input Frames          : %d\n", input_frame_count);
			fprintf(resulttxt, "\tFrames to encode..          : %d\n", output_frame_count);


			char str2[26];
			ctime_s(str2, sizeof str2, &end_time);
			fprintf(resulttxt, "Output Properties.\n");
			fprintf(resulttxt, "\tTotal frames encoded        : %d\n", output_frame_count);
			fprintf(resulttxt, "\tOutput File Name            : %s\n", out_file_name);
			fprintf(resulttxt, "\tAPP11 Markers               : %d\n", (sos_pool.size() / 65000) + 1);
			fprintf(resulttxt, "\tOutput File Size            : %d Bytes.\n", dst_size);
			fprintf(resulttxt, "\tbit per pixels              : %f bits per pixel.\n", bpp);
			fprintf(resulttxt, "\tEncoding Time               : %f sec.\n", time_elapsed);
			fprintf(resulttxt, "End   Time                  \t: %s", str2);
			fprintf(resulttxt, "***************************************************************\n\n");
			fclose(resulttxt);
		}
		else
		{
			printf("DPCMed_JPEG_encoding_history.txt  was not opened ");
			cout << endl;
		}

	}
}

	//
	//void extract_curr_sos_data(Byte* jpeg_data, unsigned int jpegsize, Byte** sos_data, unsigned * sos_size) {
	//	int header_len = 0;
	//
	//	Byte* jpeg_buf = jpeg_data;
	//	byte  m_ffd8 = sj_get_2byte(&jpeg_buf);
	//	header_len += 2;
	//	/*if (m_ffd8 != 0xFFD8)
	//		return;
	//*/
	//	byte last = sj_get_byte(&jpeg_buf);
	//	byte current = sj_get_byte(&jpeg_buf);
	//	header_len += 2;
	//	//reading markers;
	//	while (1)
	//	{
	//		if (last != 0xFF)
	//			cout << "marker expected : " << endl;
	//
	//
	//		if (current == M_SOS) {
	//			unsigned long int marker_length = sj_get_2byte(&jpeg_buf);
	//			header_len += marker_length;
	//			jpeg_buf += marker_length - 2;
	//			break;
	//		}
	//		else {
	//			unsigned long int marker_length = sj_get_2byte(&jpeg_buf);
	//			header_len += marker_length;
	//			jpeg_buf += marker_length - 2;
	//		}
	//
	//		last = sj_get_byte(&jpeg_buf);
	//		current = sj_get_byte(&jpeg_buf);
	//		header_len += 2;
	//	}
	//
	//	*sos_size = jpegsize - header_len - 2;
	//	*sos_data = jpeg_buf;
	//	
	//	//cout <<hex<< "\nabc : " << (int)jpeg_buf[0] << "  " << (int)jpeg_buf[1] << endl;
	//	//cout << "abc : " << (int)jpeg_buf[jpegsize - header_len - 2 -1] << "  " << (int)jpeg_buf[jpegsize - header_len - 2 -2] << endl;
	//	//cout << dec;
	//	return;
	//}