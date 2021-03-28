#pragma once
#include "Header.h"

void find_psnr(
	const char* ref_filename,
	const char* input_filename,
	uint width,
	uint height,
	uint input_frame_count,
	MEDIA_FORMAT input_format,
	bool ycbcr
	);