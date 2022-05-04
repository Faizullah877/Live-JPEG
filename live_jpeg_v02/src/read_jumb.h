#pragma once
#include "Header.h"
#include <iostream>
#include <vector>
#include "Box.h"
	//sj_jumbf_super_box_ptr extract_jumbf_super_box(byte *data, uint size);
	/*sj_jumbf_super_box_ptr extract_jumbf_super_box(byte* data, uint size);
	void readAPP11(byte * app11_data, std::vector<byte> &codestream_data, sj_jumbf_super_box_ptr super_box, sj_jumbf_desc_box_ptr desc_box, sj_jumbf_contiguous_codestream_box_ptr codestream_box);*/
	void readAPP11_4sosL(Byte* app11_data, std::vector<Byte>& sosL_payload, sj_jumbf_sos_lengths_box_ptr sosL_box);
	void extract_contiguous_cs_box(Byte* data, uint size, Byte** data1, double* size1);
	void readAPP11_f(Byte* app11_data, std::vector<Byte>& codestream_data, sj_jumbf_contiguous_codestream_box_ptr codestream_box);
	void extract_sosL_box(Byte* fdata, uint size, Byte** sosL_array, uint* sosL_payload_len);
