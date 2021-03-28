#ifndef _DE_CODESTREAM_
#define _DE_CODESTREAM_
#include <iostream>
#include "Header.h"
#include <vector>
#include "Box.h"
using namespace std;


	sj_jumbf_contiguous_codestream_box_ptr create_codestream_box(unsigned char* uuid_data, unsigned int codestreamSize);
	sj_jumbf_super_box_ptr sj_create_jumbf_codestream_box(unsigned char* uuid_data, unsigned int codestreamSize, char* lebel);
	sj_jumbf_super_box_ptr sj_create_jumbf_sosL_box(unsigned int* sos_len_arr, unsigned int total_diff_frames, char* label);

#endif	