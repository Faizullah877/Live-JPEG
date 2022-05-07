#pragma once
#include "Header.h"
int encode_live_jpeg_folder(
	char* executable_name,
	const char* In_folder,
	const char* out_file_name,
	uint quality,
	bool ycbcr,
	int subsamp,
	bool wsf_flag,
	bool wdf_flag,
	bool arithmetic_flag,
	bool enable_log_file
);