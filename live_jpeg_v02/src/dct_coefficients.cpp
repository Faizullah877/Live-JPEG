#include "dct_coefficients.h"

short* get_coefficients1(Byte* data, uint size, HANDLE_JPEG& hjpeg, int* coeff_buf_size)
{
	struct jpeg_error_mgr jsrcerr;
	hjpeg.srcinfo.err = jpeg_std_error(&jsrcerr);
	jpeg_create_decompress(&hjpeg.srcinfo);
	jpeg_mem_src(&hjpeg.srcinfo, data, size);
	(void)jpeg_read_header(&hjpeg.srcinfo, TRUE);
	hjpeg.src_coef_arrays = jpeg_read_coefficients(&hjpeg.srcinfo);
	short* coeff_buffer = 0;
	*coeff_buf_size = extractCoefficients1(&hjpeg.srcinfo, hjpeg.src_coef_arrays, (short**)&coeff_buffer);
	return coeff_buffer;
}



void get_coefficients_f(Byte* data, uint size, HANDLE_JPEG& hjpeg, short* coeff_buffer)
{
	struct jpeg_error_mgr jsrcerr;
	hjpeg.srcinfo.err = jpeg_std_error(&jsrcerr);
	jpeg_create_decompress(&hjpeg.srcinfo);
	jpeg_mem_src(&hjpeg.srcinfo, data, size);
	(void)jpeg_read_header(&hjpeg.srcinfo, TRUE);
	hjpeg.src_coef_arrays = jpeg_read_coefficients(&hjpeg.srcinfo);
	//short* coeff_buffer = 0;
	extractCoefficients_f(&hjpeg.srcinfo, hjpeg.src_coef_arrays, (short**)&coeff_buffer);
	//return coeff_buffer;
}

void extractCoefficients_f(j_decompress_ptr srcinfo, jvirt_barray_ptr* src_coef_arrays, short** pCoeffBuffer)
{
	//unsigned int coef_buffer_size = sizeof(short) * DCTSIZE2 * srcinfo->comp_info[0].width_in_blocks * srcinfo->comp_info[0].height_in_blocks * srcinfo->num_components;
	//*pCoeffBuffer = new short[coef_buffer_size];
	jpeg_component_info* pInfo = srcinfo->comp_info;
	MEDIA_FORMAT format;
	// check sumbsampling mode; 420, 422_h, 422_v, and 444
	// 420: 2x2, 1x1, 1x1 format
	if (pInfo[0].h_samp_factor == 2 && pInfo[0].v_samp_factor == 2 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr420;

	// 422_v: 2x1, 1x1, 1x1 format
	else if (pInfo[0].h_samp_factor == 2 && pInfo[0].v_samp_factor == 1 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr422V;

	// 422_h: 1x2, 1x1, 1x1  format
	else if (pInfo[0].h_samp_factor == 1 && pInfo[0].v_samp_factor == 2 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr422H;

	// 444: 1x1, 1x1, 1x1  format
	else if (pInfo[0].h_samp_factor == 1 && pInfo[0].v_samp_factor == 1 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr444;


	int ysize = srcinfo->image_width * srcinfo->image_height;
	int cbsize = 0, crsize = 0;

	ysize = DCTSIZE2 * srcinfo->comp_info[0].width_in_blocks * srcinfo->comp_info[0].height_in_blocks;

	switch (format) {
	case MEDIA_YCbCr420:
		cbsize = crsize = ysize / 4;
		break;

	case MEDIA_YCbCr444:
		cbsize = crsize = ysize;
		break;

	case MEDIA_YCbCr422H:
	case MEDIA_YCbCr422V:
		cbsize = crsize = ysize / 2;
		break;
	}

	short* ptr_comp[3];

	ptr_comp[0] = *pCoeffBuffer;
	ptr_comp[1] = ptr_comp[0] + ysize;
	ptr_comp[2] = ptr_comp[1] + cbsize;

	JBLOCKARRAY row_ptrs;

	//For each component,
	for (int index = 0; index < srcinfo->num_components; index++)
	{
		//...iterate over rows,
		for (int row = 0; row < srcinfo->comp_info[index].height_in_blocks; row++)
		{
			row_ptrs = ((srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[index], row, (JDIMENSION)1, FALSE);
			//...and for each block in a row,
			for (int bi = 0; bi < srcinfo->comp_info[index].width_in_blocks; bi++) {
				//...iterate over DCT coefficients
				for (int i = 0; i < DCTSIZE2; i++)
				{
					*ptr_comp[index] = row_ptrs[0][bi][i];
					ptr_comp[index]++;
				}
			}
		}
	}
	//return coef_buffer_size;
}
int extractCoefficients1(j_decompress_ptr srcinfo, jvirt_barray_ptr* src_coef_arrays, short** pCoeffBuffer)
{
	unsigned int coef_buffer_size = sizeof(short) * DCTSIZE2 * srcinfo->comp_info[0].width_in_blocks * srcinfo->comp_info[0].height_in_blocks * srcinfo->num_components;
	*pCoeffBuffer = new short[coef_buffer_size];
	jpeg_component_info* pInfo = srcinfo->comp_info;
	MEDIA_FORMAT format;
	// check sumbsampling mode; 420, 422_h, 422_v, and 444
	// 420: 2x2, 1x1, 1x1 format
	if (pInfo[0].h_samp_factor == 2 && pInfo[0].v_samp_factor == 2 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr420;

	// 422_v: 2x1, 1x1, 1x1 format
	else if (pInfo[0].h_samp_factor == 2 && pInfo[0].v_samp_factor == 1 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr422V;

	// 422_h: 1x2, 1x1, 1x1  format
	else if (pInfo[0].h_samp_factor == 1 && pInfo[0].v_samp_factor == 2 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr422H;

	// 444: 1x1, 1x1, 1x1  format
	else if (pInfo[0].h_samp_factor == 1 && pInfo[0].v_samp_factor == 1 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr444;


	int ysize = srcinfo->image_width * srcinfo->image_height;
	int cbsize = 0, crsize = 0;

	ysize = DCTSIZE2 * srcinfo->comp_info[0].width_in_blocks * srcinfo->comp_info[0].height_in_blocks;

	switch (format) {
	case MEDIA_YCbCr420:
		cbsize = crsize = ysize / 4;
		break;

	case MEDIA_YCbCr444:
		cbsize = crsize = ysize;
		break;

	case MEDIA_YCbCr422H:
	case MEDIA_YCbCr422V:
		cbsize = crsize = ysize / 2;
		break;
	}

	short* ptr_comp[3];

	ptr_comp[0] = *pCoeffBuffer;
	ptr_comp[1] = ptr_comp[0] + ysize;
	ptr_comp[2] = ptr_comp[1] + cbsize;

	JBLOCKARRAY row_ptrs;

	//For each component,
	for (int index = 0; index < srcinfo->num_components; index++)
	{
		//...iterate over rows,
		for (int row = 0; row < srcinfo->comp_info[index].height_in_blocks; row++)
		{
			row_ptrs = ((srcinfo)->mem->access_virt_barray)((j_common_ptr)&srcinfo, src_coef_arrays[index], row, (JDIMENSION)1, FALSE);
			//...and for each block in a row,
			for (int bi = 0; bi < srcinfo->comp_info[index].width_in_blocks; bi++) {
				//...iterate over DCT coefficients
				for (int i = 0; i < DCTSIZE2; i++)
				{
					*ptr_comp[index] = row_ptrs[0][bi][i];
					ptr_comp[index]++;
				}
			}
		}
	}
	return coef_buffer_size;
}

void find_coeff_diff(short* PreviousCoeff, short* nextCoeff, short* diff, int coeff_buf_size) {
	//short * diff1 = new short[coeff_buf_size];
	for (int i = 0; i < coeff_buf_size; i++) {
		diff[i] = PreviousCoeff[i] - nextCoeff[i];
	}
	//*diff = diff1;
//	delete[] diff1;
	//	return diff;
}

void jcopy_huff_table(struct jpeg_compress_struct* dstinfo, struct jpeg_decompress_struct* srcinfo)
{
	dstinfo->optimize_coding = false;

	JHUFF_TBL** dchtblptr;
	JHUFF_TBL** achtblptr;
	int tblno, ci, coefi;
	/*Coopy HUFF table*/
	for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++) {
		if (srcinfo->dc_huff_tbl_ptrs[tblno] != NULL) {
			dchtblptr = &dstinfo->dc_huff_tbl_ptrs[tblno];

			if (*dchtblptr == NULL)
				*dchtblptr = jpeg_alloc_huff_table((j_common_ptr)dstinfo);
			memcpy((*dchtblptr)->bits, srcinfo->dc_huff_tbl_ptrs[tblno]->bits, sizeof((*dchtblptr)->bits));
			memcpy((*dchtblptr)->huffval, srcinfo->dc_huff_tbl_ptrs[tblno]->huffval,
				sizeof((*dchtblptr)->huffval));
			(*dchtblptr)->sent_table = FALSE;

		}
		if (srcinfo->ac_huff_tbl_ptrs[tblno] != NULL) {
			achtblptr = &dstinfo->ac_huff_tbl_ptrs[tblno];
			if (*achtblptr == NULL)
				*achtblptr = jpeg_alloc_huff_table((j_common_ptr)dstinfo);
			memcpy((*achtblptr)->bits, srcinfo->ac_huff_tbl_ptrs[tblno]->bits, sizeof((*achtblptr)->bits));
			memcpy((*achtblptr)->huffval, srcinfo->ac_huff_tbl_ptrs[tblno]->huffval,
				sizeof((*achtblptr)->huffval));
			(*achtblptr)->sent_table = FALSE;
		}
	}

}

void prepareCoefficients(j_decompress_ptr srcinfo, j_compress_ptr dstinfo, jvirt_barray_ptr* src_coef_arrays, short* pCoeffBuffer)
{
	jpeg_component_info* pInfo = srcinfo->comp_info;

	MEDIA_FORMAT format;

	// check sumbsampling mode; 420, 422_h, 422_v, and 444
	// 420: 2x2, 1x1, 1x1 format
	if (pInfo[0].h_samp_factor == 2 && pInfo[0].v_samp_factor == 2 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr420;

	// 422_v: 2x1, 1x1, 1x1 format
	else if (pInfo[0].h_samp_factor == 2 && pInfo[0].v_samp_factor == 1 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr422V;

	// 422_h: 1x2, 1x1, 1x1  format
	else if (pInfo[0].h_samp_factor == 1 && pInfo[0].v_samp_factor == 2 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr422H;

	// 444: 1x1, 1x1, 1x1  format
	else if (pInfo[0].h_samp_factor == 1 && pInfo[0].v_samp_factor == 1 &&
		pInfo[1].h_samp_factor == 1 && pInfo[1].v_samp_factor == 1 &&
		pInfo[2].h_samp_factor == 1 && pInfo[2].v_samp_factor == 1)
		format = MEDIA_YCbCr444;


	int ysize = srcinfo->image_width * srcinfo->image_height;
	int cbsize = 0, crsize = 0;
	ysize = DCTSIZE2 * srcinfo->comp_info[0].width_in_blocks * srcinfo->comp_info[0].height_in_blocks;

	switch (format) {
	case MEDIA_YCbCr420:
		cbsize = crsize = ysize / 4;
		break;

	case MEDIA_YCbCr444:
		cbsize = crsize = ysize;
		break;

	case MEDIA_YCbCr422H:
	case MEDIA_YCbCr422V:
		cbsize = crsize = ysize / 2;
		break;
	}

	short* ptr_comp[3];
	ptr_comp[0] = pCoeffBuffer;
	ptr_comp[1] = ptr_comp[0] + ysize;
	ptr_comp[2] = ptr_comp[1] + cbsize;

	JBLOCKARRAY row_ptrs;

	//Save the changes
	//For each component,
	for (int index = 0; index < srcinfo->num_components; index++) {
		//...iterate over rows
		for (int row = 0; row < srcinfo->comp_info[index].height_in_blocks; row++) {
			row_ptrs = (dstinfo->mem->access_virt_barray)((j_common_ptr)dstinfo, src_coef_arrays[index], row, (JDIMENSION)1, TRUE);
			// for each block in a row
			for (int bi = 0; bi < srcinfo->comp_info[index].width_in_blocks; bi++) {
				//...iterate over DCT coefficients
				for (int i = 0; i < DCTSIZE2; i++) {
					row_ptrs[0][bi][i] = *ptr_comp[index];
					ptr_comp[index]++;
				}
			}
		}
	}
}

unsigned long write_jpeg_memory_coefficients(DataBuffer* pBuffer, HANDLE_JPEG& hjpeg, bool arithmetic_flag)
{
	struct jpeg_compress_struct dstinfo;
	struct jpeg_error_mgr jdsterr;
	jvirt_barray_ptr* dst_coef_arrays;
	/* Initialize the JPEG compression object with default error handling. */
	dstinfo.err = jpeg_std_error(&jdsterr);
	jpeg_create_compress(&dstinfo);

	jpeg_copy_critical_parameters(&hjpeg.srcinfo, &dstinfo);
	if(arithmetic_flag)
		dstinfo.arith_code = TRUE;

	dstinfo.write_JFIF_header = false;
	// moveDCTAround(&hjpeg.srcinfo, &dstinfo, hjpeg.src_coef_arrays);
	prepareCoefficients(&hjpeg.srcinfo, &dstinfo, hjpeg.src_coef_arrays, (short*)pBuffer->additional_data);

	/* ..when done with DCT, do this: */
	// dst_coef_arrays = jtransform_adjust_parameters(&hjpeg.srcinfo, &dstinfo, hjpeg.src_coef_arrays, &hjpeg.transformoption);
	dst_coef_arrays = hjpeg.src_coef_arrays;

	/* Specify data destination for compression */
	delete[] pBuffer->data;

	pBuffer->size *= 2;
	pBuffer->data = new unsigned char[pBuffer->size];
	memset(pBuffer->data, 0, pBuffer->size);
	//dst
	jpeg_mem_dest(&dstinfo, &pBuffer->data, (unsigned long*)&pBuffer->size);
	//jcopy_huff_table(&dstinfo, &hjpeg.srcinfo);
	/* Start compressor (note no image data is actually written here) */


	jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

	/* Copy to the output file any extra markers that we want to preserve */
	// need to check
	// jcopy_markers_execute(&hjpeg.srcinfo, &dstinfo, JCOPYOPT_ALL);

	jpeg_finish_compress(&dstinfo);
	jpeg_destroy_compress(&dstinfo);
	(void)jpeg_finish_decompress(&hjpeg.srcinfo);
	jpeg_destroy_decompress(&hjpeg.srcinfo);

	return (unsigned long)pBuffer->size;
}