#pragma once

void decode_live_jpeg(
	char* executable_name,
	const char* InputFile, 
	const char* outFile, 
	bool ycbcr,
	MEDIA_FORMAT output_format, 
	bool wsf_flag, 
	bool wdf_flag
);