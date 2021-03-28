#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <fstream>
#include <jpeg/include/jpeglib.h>
//#include <libjpeg-turbo/include/jpeglib.h>
//#include <libjpeg-turbo/include/jerror.h>
#include <jpeg/include/jerror.h>

#include "Header.h"
#include "dct_coefficients.h"
#include "Box.h"
#include "codestream_box.h"
#include "write_jpeg.h"
#include "read_jumb.h"
#include "pix_format_converter.h"

using namespace std;


void extract_key_frame_jpg_buf(byte* data, uint size, byte** keyFrameJPG, uint* keyFrameJPGsize) {

	byte* fdata = data;

	vector<byte> header;
	header.push_back(0xFF);
	header.push_back(0xD8);


	byte last = sj_get_byte(&fdata);
	byte current = sj_get_byte(&fdata);
	if (last != 0xFF || current != M_SOI) {
		cout << "Image is not JPEG" << endl;
		return;
	}
	last = sj_get_byte(&fdata);
	current = sj_get_byte(&fdata);

	//reading markers;
	while (1)
	{
		if (last != 0xFF)
		{
			std::cout << "Error - Expected a marker\n";
			return;
		}
		if (current != M_APP11)
		{
			int length = sj_get_2byte(&fdata);
			fdata -= 4;
			for (int i = 0; i < length + 2; ++i)
			{
				header.push_back(sj_get_byte(&fdata));
			}
			if (current == M_SOS)
				break;
		}
		/*if (current == M_DQT || current == M_DHT || current == M_SOS || current == M_SOF0)
		{
			int length = sj_get_2byte(&fdata);
			fdata -= 4;
			for (int i = 0; i < length + 2; ++i)
			{
				header.push_back(sj_get_byte(&fdata));
			}
			if (current == M_SOS)
				break;
		}*/
		//any number of 0xFF in a row is allowed and should be ignored
		else if (current == 0xFF) {
			current = sj_get_byte(&fdata);
			continue;
		}
		else {
			unsigned long int length = sj_get_2byte(&fdata);
			fdata += (length - 2);
		}

		last = sj_get_byte(&fdata);
		current = sj_get_byte(&fdata);
	}
	
	while (1) {
		last = sj_get_byte(&fdata);
		header.push_back(last);
		if (last == 0xFF) {
			current = sj_get_byte(&fdata);
			header.push_back(current);
			if (current == M_EOI)
			{
				break;
			}
		}

	}



	*keyFrameJPGsize = header.size();
	*keyFrameJPG = new byte[*keyFrameJPGsize];
	memcpy(*keyFrameJPG, header.data(), *keyFrameJPGsize);


}

struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */

	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr)cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

int		read_jpeg_memory(unsigned char* mem, unsigned long mem_size, bool ycbcr, JSAMPLE* image_buffer)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	/* More stuff */
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);

		return 0;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, mem, mem_size);
	(void)jpeg_read_header(&cinfo, TRUE);
	if (ycbcr == true) {
		cinfo.out_color_space = JCS_YCbCr;
	}

	(void)jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height) {
		(void)jpeg_read_scanlines(&cinfo, buffer, 1);
		JSAMPLE* ptr = buffer[0];
		for (int i = 0; i < row_stride; i += 3) {
			*image_buffer++ = *ptr++;
			*image_buffer++ = *ptr++;
			*image_buffer++ = *ptr++;
		}
	}
	(void)jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 1;
}


void ReadBytestream2(const char* InputFile, DataBuffer* pBuffer) {
	std::ifstream jpg(InputFile, std::ifstream::binary);
	if (jpg) {
		// get length of file:
		jpg.seekg(0, jpg.end);
		unsigned long int length = jpg.tellg();
		jpg.seekg(0, jpg.beg);
		//cout << "File size is : " << length << endl;
		char* buffer = new char[length];
		jpg.read(buffer, length);
		if (jpg) {
			//std::cout << "all characters read successfully.";
			pBuffer->data = (unsigned char*)buffer;
			pBuffer->size = length;
		}
		else
			std::cout << "error: only " << jpg.gcount() << " could be read from input file";
		jpg.close();
	}

}


void ReadBytestream(FILE* fp, DataBuffer* pBuffer)
{
	fseek(fp, 0L, SEEK_END);
	unsigned long int filesize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	//cout << "File size : " << filesize << endl;
	if (filesize) {
		pBuffer->data = new unsigned char[filesize];
		if (pBuffer->data) {
			unsigned long int readsize = (int)fread(pBuffer->data, sizeof(char), filesize, fp);
			fclose(fp);

			if (readsize != filesize) {
				delete[] pBuffer->data;
				pBuffer->data = 0;
				pBuffer->size = 0;
			}
			pBuffer->size = filesize;
		}
		else {
			fclose(fp);
		}
	}
}

void decode_live_jpeg(
	char* executable_name,
	const char* InputFile, 
	const char* out_file_name, 
	bool ycbcr,
	MEDIA_FORMAT output_format, 
	bool wsf_flag, 
	bool wdf_flag
) {
	auto start1 = chrono::high_resolution_clock::now();
	auto start_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	FILE* resulttxt;
	errno_t err3 = fopen_s(&resulttxt, "Live_jpeg_Decoding_history.txt", "a+");
	if (err3 == 0)
	{
		//printf("\Input File  : %s ", In_file_name);
		//cout << endl;
	}
	else
	{
		printf("Live_jpeg_Decoding_history.txt  was not opened. ");
		cout << endl;
		//return;
	}

	printf("Running                       : %s\n", executable_name);
	cout << "Process ====>>>>>> Live_jpeg Decoding" << endl;
	cout << "Source properties" << endl;
	cout << "\tSource File Name            : " << InputFile << endl;
	cout << "Decoding Paramenters" << endl;
	cout << "\tOutput pixels Color Space    : ";
	if (ycbcr) cout << "YCbCr" << endl; else cout << "RGB" << endl;

	if (err3 == 0) {
		char str1[26];
		ctime_s(str1, sizeof str1, &start_time);
		fprintf(resulttxt, "******************************************************************\n");
		fprintf(resulttxt, "Running                       : %s\n", executable_name);
		fprintf(resulttxt, "Start Time                    : %s", str1);
		fprintf(resulttxt, "\nProcess ====>>>>>> Live_jpeg decoding.\n");
		fprintf(resulttxt, "\nSource properties.\n");
		fprintf(resulttxt, "\tSource File Name             : %s\n", InputFile);
		fprintf(resulttxt, "\tSource pixels Color Space   : ");
		if (ycbcr) fprintf(resulttxt, "YCbCr\n"); else fprintf(resulttxt, "RGB\n");
	}

	DataBuffer* pBuffer = new DataBuffer();
	DataBuffer* ptr = new DataBuffer();
	FILE* InFile;
	errno_t err = fopen_s(&InFile, InputFile, "rb");
	if (err == 0)
	{
		//printf("\Input File  : %s ", InputFile);
		//cout << endl;
	}
	else
	{
		fprintf(resulttxt, "\nError -> Failed to open source file. Name            : %s\n", InputFile);
		printf("The %s for Input was not opened ", InputFile);
		cout << endl;
		return;
	}

	//ReadBytestream(InFile, ptr);
	ReadBytestream2(InputFile, ptr);

	FILE* outFile;
	errno_t err1 = fopen_s(&outFile, out_file_name, "wb");
	if (err1 == 0)
	{
		//printf("\Input File  : %s ", InputFile);
		//cout << endl;
	}
	else
	{
		fprintf(resulttxt, "\nError -> Failed to open output file. Name            : %s\n", out_file_name);
		printf("The %s for Output was not opened ", out_file_name);
		cout << endl;
		return;
	}
	byte* keyFrameJPG = NULL;
	uint keyFrameJPGsize = 0;
	byte* dst_data = NULL;
	uint dst_size = 0;
	byte* header_buf;
	uint header_size;
	uint frame_counter = 0;
	byte* sosL_payload;
	uint sosL_payload_len;
	uint width;
	uint height;
	uint components;

	copy_jpeg_header(ptr->data, ptr->size, &header_buf, &header_size);

	extract_key_frame_jpg_buf(ptr->data, ptr->size, &keyFrameJPG, &keyFrameJPGsize);

	struct jpeg_decompress_struct srcinfo;
	struct jpeg_error_mgr jsrcerr;
	jpeg_create_decompress(&srcinfo);
	srcinfo.err = jpeg_std_error(&jsrcerr);

	jpeg_mem_src(&srcinfo, keyFrameJPG, keyFrameJPGsize);
	(void)jpeg_read_header(&srcinfo, TRUE);

	width = srcinfo.image_width;
	height = srcinfo.image_height;
	components = srcinfo.num_components;

	/*FILE* keyFrame;
	errno_t errkey = fopen_s(&keyFrame, "keyFrame.jpg", "wb");
	fwrite(keyFrameJPG, sizeof(unsigned char), keyFrameJPGsize, keyFrame);
	fclose(keyFrame);*/


	//cout << "Header buffer size is : " << header_size << endl;
	extract_sosL_box(ptr->data, ptr->size, &sosL_payload, &sosL_payload_len);

	uint* sosL_arr = (uint*)sosL_payload;
	uint total_frames = sosL_arr[0];
	/*for (uint i = 0; i < 6; i++)
		cout << "diff frame : " << i << "  sos size is : " << sosL_arr[i] << endl;*/

	/*cout << "Total Frames present : " << sosL_arr[0];
	for (uint i = 0; i < 10; ++i)
		cout << "diff frame : " << i << " sos size is : " << sosL_arr[i + 1] << endl;*/

	byte* diff_sos_buf = NULL;
	double diff_sos_buf_len;
	extract_contiguous_cs_box(ptr->data, ptr->size, &diff_sos_buf, &diff_sos_buf_len);
	byte* sos_data = diff_sos_buf;


	if(wdf_flag){


		byte* sos_data1 = sos_data;
		for (int i = 0; i < total_frames-1; ++i) {
			FILE *fff;
			errno_t err;
			char fileN[100];
			sprintf_s(fileN, 100, "Decodedd_diff_Frame_%d.jpg", i + 1);
			err = fopen_s(&fff, (char*)fileN, "wb");
			//fwrite(header_buf, sizeof(unsigned char), header_size, fff);
			fwrite(sos_data1, sizeof(unsigned char), sosL_arr[i + 1], fff);
			//cout << "abcd " <<hex<< (int)sos_data1[0] << "  " << (int)sos_data1[1] << endl;
			//cout << "abcd " << (int)sos_data1[sosL_arr[i + 1] - 2] << "  " << (int)sos_data1[sosL_arr[i+1]-1] <<dec<< endl;
			//putc(0xFF, fff);
			//putc(0xD9, fff);
			sos_data1 += sosL_arr[i + 1];
			fclose(fff);
		}
	}
	//exit(1);
	byte* mem = ptr->data;
	uint mem_size = ptr->size;

	HANDLE_JPEG hjpeg1;
	int comp0_width_in_blocks = (width + 7) / 8;
	int comp0_height_in_blocks = (height + 7) / 8;
	uint coeff_buf_size = sizeof(short) * DCTSIZE2 * comp0_width_in_blocks * comp0_height_in_blocks * components;
	//cout << "Coeff_buf_size 1 : " << coeff_buf_size << endl;
	short* PreviousCoeff = new short[coeff_buf_size];
	//short * 

	get_coefficients_f(keyFrameJPG, keyFrameJPGsize, hjpeg1, PreviousCoeff);
	//PreviousCoeff = get_coefficients1(ptr->data, ptr->size, hjpeg1, &coeff_buf_size);
	//cout << "Coeff_buf_size 2 : " << coeff_buf_size << endl;

	short* diff_coeff = new short[coeff_buf_size];
	short* curr_frame_coeff = new short[coeff_buf_size];
	uint raw_size = width * height * components;
	JSAMPLE* raw_img = new unsigned char[raw_size];
	read_jpeg_memory(keyFrameJPG, keyFrameJPGsize, ycbcr, raw_img);

	if (output_format == MEDIA_YUV444P) {
		convert_yuv444I_to_yuv444p(raw_img, raw_size, width, height);
	}

	fwrite(raw_img, sizeof(unsigned char), raw_size, outFile);
	frame_counter++;
	for (uint i = 1; i < total_frames; ++i) {
		uint curr_jpeg_size = sosL_arr[i];
		byte* curr_jpeg_buf = new byte[curr_jpeg_size];
		//memcpy(curr_jpeg_buf, header_buf, header_size);
		//curr_jpeg_buf += header_size;
		memcpy(curr_jpeg_buf, sos_data, sosL_arr[i]);
		sos_data += sosL_arr[i];
		//curr_jpeg_buf -= header_size;
		//curr_jpeg_buf[curr_jpeg_size - 2] = 0xFF;
		//curr_jpeg_buf[curr_jpeg_size - 1] = 0xD9;
		int coeff_buf_size1;
		//diff_coeff = get_coefficients1(curr_jpeg_buf, curr_jpeg_size, hjpeg1, &coeff_buf_size1);
		get_coefficients_f(curr_jpeg_buf, curr_jpeg_size, hjpeg1, diff_coeff);
		//find_coeff_diff(PreviousCoeff, diff_coeff, curr_frame_coeff, coeff_buf_size);
		//for (int i = 0; i < coeff_buf_size; i++) {
		//	curr_frame_coeff[i] = PreviousCoeff[i] - diff_coeff[i];
		//}
		
		for (uint c = 0; c < coeff_buf_size; c++) {
			*(curr_frame_coeff + c) = *(PreviousCoeff + c) - *(diff_coeff + c);
		}

		int curr_frame_jpeg_size;
		pBuffer->additional_data = (unsigned char*)curr_frame_coeff;
		//cout << "pBuffer size before coeff encoding : " << pBuffer->size << endl;
		curr_frame_jpeg_size = write_jpeg_memory_coefficients(pBuffer, hjpeg1, 0);
		int max = 0;
		if (wsf_flag) { // write each frame in jpeg seperate
			FILE* f;
			errno_t err;
			char fileN[100];
			sprintf_s(fileN, 100, "Decoded_Frame_%d.jpg", i + 1);
			err = fopen_s(&f, (char*)fileN, "wb");
			fwrite(pBuffer->data, sizeof(unsigned char), curr_frame_jpeg_size, f);
			fclose(f);
		}
		//PreviousCoeff = curr_frame_coeff;
		memcpy(PreviousCoeff, curr_frame_coeff, coeff_buf_size);

		read_jpeg_memory(pBuffer->data, curr_frame_jpeg_size, ycbcr, raw_img);
		//cout << "here\n\n";
		if (output_format == MEDIA_YUV444P) {
			convert_yuv444I_to_yuv444p(raw_img, raw_size, width, height);
		}
		fwrite(raw_img, sizeof(unsigned char), raw_size, outFile);
		frame_counter++;

		printf("\r                Frame Decoding...           : %d", frame_counter);
		fflush(stdout);
		delete[] curr_jpeg_buf;
	}
	fclose(outFile);

	//cout << "Total frames Encountered : " << frame_counter << endl;


	delete[] PreviousCoeff;
	delete[] curr_frame_coeff;
	delete[] diff_coeff;

	cout << "\nOutput properties" << endl;
	cout << "\tDecoded No of Frames        : " << frame_counter << endl;
	cout << "\tOutput File Name             : " << out_file_name << endl;
	auto finish = chrono::high_resolution_clock::now();
	auto elapsed = finish - start1;
	cout << "\tDecoding Time               : " << elapsed / std::chrono::milliseconds(1)/1000.0 << " sec" << endl;

	auto end_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	float time_elapsed = elapsed / std::chrono::milliseconds(1)/1000.0;
	if (err3 == 0) {
		char str2[26];
		ctime_s(str2, sizeof str2, &end_time);
		fprintf(resulttxt, "Output Properties.\n");
		fprintf(resulttxt, "\tTotal frames decoded        : %d\n", frame_counter);
		fprintf(resulttxt, "\tOutput File Name             : %s\n", out_file_name);
		fprintf(resulttxt, "\tDecoding Time                : %f sec.\n", time_elapsed);
		fprintf(resulttxt, "End   Time                    : %s", str2);
		fprintf(resulttxt, "***************************************************************\n");
	}
}