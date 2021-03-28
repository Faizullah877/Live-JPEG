#include "codestream_box.h"
//#include "Box.h"

	sj_jumbf_sos_lengths_box_ptr create_sosL_box(unsigned int* sos_len_arr, unsigned int total_diff_frames)
	{
		sj_jumbf_sos_lengths_box_ptr sosL_box = (sj_jumbf_sos_lengths_box_ptr)malloc(sizeof(sj_jumbf_sos_lengths_box));
		memset(sosL_box, 0, sizeof(sj_jumbf_sos_lengths_box));
		unsigned int length = (total_diff_frames * sizeof(unsigned int)) + 8; //length 4 byte, box type 4 bytpe, 문자열 끝 \0 1byte
		unsigned int box_type = BOX_TYPE_FAIZ;
		sosL_box->codestream = (unsigned char*)calloc(length - 8, sizeof(unsigned char));
		sosL_box->length = length;
		sosL_box->box_type = box_type;
		//sosL_box->total_diff_frames = total_diff_frames;
		memcpy(sosL_box->codestream, sos_len_arr, length - 8);
		sosL_box->next = NULL;
		return sosL_box;
	}
	sj_jumbf_super_box_ptr sj_create_jumbf_sosL_box(unsigned int* sos_len_arr, unsigned int total_diff_frames, char* label)
	{
		sj_jumbf_super_box_ptr jumbf_box = (sj_jumbf_super_box_ptr)malloc(sizeof(sj_jumbf_super_box));
		memset(jumbf_box, 0, sizeof(sj_jumbf_super_box));
		sj_jumbf_sos_lengths_box_ptr sosL_box = create_sosL_box(sos_len_arr, total_diff_frames);

		sj_jumbf_desc_box_ptr desc_box = create_desc_box(TYPE_SOSL, label, 0, NULL);
		desc_box->next_box = (sj_jumbf_box_ptr)sosL_box;
		jumbf_box->length = sosL_box->length + desc_box->length + 8;
		jumbf_box->box_type = BOX_TYPE_JUMB;
		jumbf_box->next_box = (sj_jumbf_box_ptr)desc_box;
		return jumbf_box;
	}
	sj_jumbf_super_box_ptr sj_create_jumbf_codestream_box(unsigned char* codestream_data, unsigned int codestreamSize, char* label)
	{
		sj_jumbf_super_box_ptr jumbf_box = (sj_jumbf_super_box_ptr)malloc(sizeof(sj_jumbf_super_box));
		memset(jumbf_box, 0, sizeof(sj_jumbf_super_box));
		sj_jumbf_contiguous_codestream_box_ptr codestream_box = create_codestream_box(codestream_data, codestreamSize);

		sj_jumbf_desc_box_ptr desc_box = create_desc_box(TYPE_JP2C, label, 0, NULL);
		desc_box->next_box = (sj_jumbf_box_ptr)codestream_box;
		jumbf_box->length = codestream_box->length + desc_box->length + 8;
		jumbf_box->box_type = BOX_TYPE_JUMB;
		jumbf_box->next_box = (sj_jumbf_box_ptr)desc_box;
		return jumbf_box;
	}

	sj_jumbf_contiguous_codestream_box_ptr create_codestream_box(unsigned char* codestream_data, unsigned int codestreamSize)
	{
		sj_jumbf_contiguous_codestream_box_ptr codestream_box = (sj_jumbf_contiguous_codestream_box_ptr)malloc(sizeof(sj_jumbf_contiguous_codestream_box));
		memset(codestream_box, 0, sizeof(sj_jumbf_contiguous_codestream_box));
		unsigned int length = codestreamSize + 8; //length 4 byte, box type 4 bytpe, 문자열 끝 \0 1byte
		unsigned int box_type = BOX_TYPE_CODESTREAM;
		codestream_box->codestream = (unsigned char*)calloc(length - 8, sizeof(unsigned char));
		codestream_box->length = length;
		codestream_box->box_type = box_type;

		memcpy(codestream_box->codestream, codestream_data, length - 8);
		codestream_box->next = NULL;
		return codestream_box;
	}
