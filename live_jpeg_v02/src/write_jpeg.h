#ifndef _WRITE_DEJPEG_
#define _WRITE_DEJPEG_
#include <iostream>
#include <vector>
#include "Box.h"
using namespace std;

	bool jpeg_write_sos_jumbf(unsigned char* input_jpeg, unsigned int input_jpeg_size, sj_jumbf_super_box* sos_super_box, unsigned char** dst_buf, unsigned int* dst_size);
	bool jpeg_write_jumbf(unsigned char* primary_frame, unsigned int primary_frame_size, sj_jumbf_super_box* superbox, unsigned char** dst_buf, unsigned int* dst_size);


#endif	
