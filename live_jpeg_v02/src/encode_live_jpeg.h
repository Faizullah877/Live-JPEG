#pragma once
#include "Header.h"
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
	int sub_samp, 
	bool wsf_flag, 
	bool wdf_flag,
	bool arithmetic_flag,
	bool enable_log_file
);