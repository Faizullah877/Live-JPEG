#pragma once
//#include <libjpeg-turbo/include/jpeglib.h>
#include <setjmp.h>
#include <jpeg/jpeg.h>
#include <jpeg/include/jpeglib.h>
#include "Header.h"
#include <jpeg/include/jerror.h>

	void extractCoefficients_f(j_decompress_ptr srcinfo, jvirt_barray_ptr* src_coef_arrays, short** pCoeffBuffer);
	void get_coefficients_f(byte* data, uint size, HANDLE_JPEG& hjpeg, short* coeff_buffer);
	short* get_coefficients1(byte* data, uint size, HANDLE_JPEG& hjpeg, int* coeff_buf_size);

	int extractCoefficients1(j_decompress_ptr srcinfo, jvirt_barray_ptr* src_coef_arrays, short** pCoeffBuffer);

	void find_coeff_diff(short* PreviousCoeff, short* nextCoeff, short* diff, int coeff_buf_size);
	
	unsigned long write_jpeg_memory_coefficients(DataBuffer* pBuffer, HANDLE_JPEG& hjpeg, bool arithmetic_flag);