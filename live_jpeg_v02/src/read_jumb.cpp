#include "read_jumb.h"
#include <vector>
using namespace std;

	static uint previos_Z = 0;
	static uint previous_Z_sos = 0;
	bool isJthBitSet_1(byte n, int j)
	{
		if ((n >> (j - 1)) & 1)
			return true;
		else
			return false;
	}


	void extract_sosL_box(byte* fdata, uint size, byte** sosL_array, uint* sosL_payload_len) {
		byte* data = fdata;
		sj_jumbf_sos_lengths_box_ptr sosL_box;
		sosL_box = (sj_jumbf_sos_lengths_box_ptr)malloc(sizeof(sj_jumbf_sos_lengths_box));
		memset(sosL_box, 0, sizeof(sj_jumbf_sos_lengths_box));
		vector<byte> sosL_payload;
		byte last = sj_get_byte(&data);
		byte current = sj_get_byte(&data);
		if (last != 0xFF || current != M_SOI) {
			cout << "Image is not JPEG" << endl;
			return;
		}
		last = sj_get_byte(&data);
		current = sj_get_byte(&data);
		uint app11_counter = 0;
		//reading markers;
		while (1)
		{

			if (last != 0xFF)
			{
				std::cout << "Error - Expected a marker\n";
				break;
			}
			else if (current == M_SOS) {
				//std::cout << "meet Start of Scan Marker :\n";
				break;
			}
			else if (current == M_APP11)
			{
				app11_counter++;
				//cout << "APP11 Marker No : " << app11_counter << endl;
				int len = sj_get_2byte(&data);
				byte* app11_data = data - 4;
				readAPP11_4sosL(app11_data, sosL_payload, sosL_box);
				data += (len - 2);
			}

			else if (current == M_SOI) {
				std::cout << "Error - Embedded JPGs not supported\n";
				break;
			}
			else if (current == M_EOI) {
				std::cout << "Error - EOI detected before SOS\n";
				break;
			}
			else {
				int len = sj_get_2byte(&data);
				data += (len - 2);
			}
			last = sj_get_byte(&data);
			current = sj_get_byte(&data);
		}
		*sosL_array = new byte[sosL_payload.size()];
		*sosL_payload_len = sosL_payload.size();
		memcpy(*sosL_array, &sosL_payload[0], sosL_payload.size());
	}

	void readAPP11_4sosL(byte* app11_data, vector<byte>& sosL_payload, sj_jumbf_sos_lengths_box_ptr sosL_box)
	{

		unsigned char type_sosL[16] = { 0x65, 0x79, 0xD6, 0xFB, 0xDB, 0xA2, 0x44, 0x6B, 0xB2, 0xAC, 0x1B, 0x82, 0x66, 0x61, 0x69, 0x7a }; // faiz
		uint marker = sj_get_2byte(&app11_data);
		if (marker != 0xFFEB) {
			cout << "marker is not APP11" << endl;
			return;
		}

		uint length = sj_get_2byte(&app11_data);
		uint CI = sj_get_2byte(&app11_data);
		uint En = sj_get_2byte(&app11_data);
		uint Z = sj_get_4byte(&app11_data);
		if (Z - previous_Z_sos == 1)
		{
			previous_Z_sos = Z;
		}
		uint LBox = sj_get_4byte(&app11_data);
		uint TBox = sj_get_4byte(&app11_data);

		bool sosL_boxFlag = false;

		if (TBox == BOX_TYPE_JUMB)
		{
			uint jumdlength = sj_get_4byte(&app11_data);
			uint jumdtype = sj_get_4byte(&app11_data);
			byte jumdID[16] = { 0 };
			for (uint k = 0; k < 16; k++)
				jumdID[k] = sj_get_byte(&app11_data);

			for (uint k = 0; k < 16; ++k) {
				if (jumdID[k] != type_sosL[k]) {
					sosL_boxFlag = false;
					break;
				}
				else {
					sosL_boxFlag = true;
				}
			}

			byte toggles = sj_get_byte(&app11_data);
			bool label_present = isJthBitSet_1(toggles, 2);
			bool ID_present = isJthBitSet_1(toggles, 3);
			bool Sign_present = isJthBitSet_1(toggles, 4);
			char* label;
			if (label_present) {
				uint label_length = jumdlength - 8 - 16 - 1 - (ID_present ? 4 : 0) - (Sign_present ? 256 : 0);
				label = new char[label_length];
				for (uint k = 0; k < label_length; ++k)
					label[k] = sj_get_byte(&app11_data);
			}
			if (ID_present)
				int id = sj_get_4byte(&app11_data);
			char* signature;
			if (Sign_present) {
				signature = new char[256];
				for (uint k = 0; k < 256; ++k)
					signature[k] = sj_get_byte(&app11_data);
			}
			else
				signature = NULL;
			if (sosL_boxFlag) {

				uint content_box_length = sj_get_4byte(&app11_data);
				uint content_box_type = sj_get_4byte(&app11_data);
				if (content_box_type == BOX_TYPE_FAIZ)
				{
					sosL_box->length = content_box_length - 8 + sosL_payload.size();
					sosL_box->box_type = BOX_TYPE_FAIZ;
					sosL_payload.insert(sosL_payload.end(), &app11_data[0], &app11_data[content_box_length - 8]);
					app11_data += (content_box_length - 8);
				}
				else {
					cout << "Content Box other than codestream Type (" << BOX_TYPE_FAIZ << ") is not supported" << endl;
				}
			}
			else
				return;
		}
		else
		{
			cout << "Box_Type other than JUMBF (" << BOX_TYPE_JUMB << ") is not supported" << endl;
		}


	}


	void extract_contiguous_cs_box(byte* data, uint size, byte** dataa, double* size1) {

		sj_jumbf_contiguous_codestream_box_ptr jp2c_box;
		jp2c_box = (sj_jumbf_contiguous_codestream_box_ptr)malloc(sizeof(sj_jumbf_contiguous_codestream_box));
		memset(jp2c_box, 0, sizeof(sj_jumbf_contiguous_codestream_box));
		vector<byte> codestream_data; ///concatenated
		byte last = sj_get_byte(&data);
		byte current = sj_get_byte(&data);
		if (last != 0xFF || current != M_SOI) {
			cout << "Image is not JPEG" << endl;
			return;
		}
		last = sj_get_byte(&data);
		current = sj_get_byte(&data);
		uint app11_counter = 0;
		//reading markers;
		while (1)
		{

			if (last != 0xFF)
			{
				std::cout << "Error - Expected a marker\n";
				break;
			}
			else if (current == M_SOS) {
				//std::cout << "meet Start of Scan Marker :\n";
				break;
			}
			else if (current == M_APP11)
			{
				app11_counter++;
				//cout << "APP11 Marker No : " << app11_counter << endl;
				int len = sj_get_2byte(&data);
				byte* app11_data = data - 4;
				readAPP11_f(app11_data, codestream_data, jp2c_box);
				data += (len - 2);
			}

			else if (current == M_SOI) {
				std::cout << "Error - Embedded JPGs not supported\n";
				break;
			}
			else if (current == M_EOI) {
				std::cout << "Error - EOI detected before SOS\n";
				break;
			}
			else {
				int len = sj_get_2byte(&data);
				data += (len - 2);
			}
			last = sj_get_byte(&data);
			current = sj_get_byte(&data);
		}
		*dataa = new byte[codestream_data.size()];
		*size1 = codestream_data.size();
		memcpy(*dataa, &codestream_data[0], codestream_data.size());
		//jp2c_box->codestream = codestream_data.data();
		//cout << "Codestream box length : " << jp2c_box->length << endl;
		//cout << "Codestream length vector :" << codestream_data.size();

		//return jp2c_box;

	}

	void readAPP11_f(byte* app11_data, vector<byte>& codestream_data, sj_jumbf_contiguous_codestream_box_ptr codestream_box)
	{

		uint marker = sj_get_2byte(&app11_data);
		if (marker != 0xFFEB) {
			cout << "marker is not APP11" << endl;
			return;
		}

		uint length = sj_get_2byte(&app11_data);
		uint CI = sj_get_2byte(&app11_data);
		uint En = sj_get_2byte(&app11_data);
		uint Z = sj_get_4byte(&app11_data);
		if (Z - previos_Z == 1)
		{
			previos_Z = Z;
		}
		uint LBox = sj_get_4byte(&app11_data);
		uint TBox = sj_get_4byte(&app11_data);
		bool jp2c_flag = false;

		unsigned char type_jp2c[16] = { 0x65, 0x79, 0xD6, 0xFB, 0xDB, 0xA2, 0x44, 0x6B, 0xB2, 0xAC, 0x1B, 0x82, 0xFE, 0xEB, 0x89, 0xD1 };
		if (TBox == BOX_TYPE_JUMB)
		{
			uint jumdlength = sj_get_4byte(&app11_data);
			uint jumdtype = sj_get_4byte(&app11_data);
			byte jumdID[16] = { 0 };
			for (uint k = 0; k < 16; k++)
				jumdID[k] = sj_get_byte(&app11_data);

			for (uint k = 0; k < 16; ++k) {
				if (jumdID[k] != type_jp2c[k]) {
					jp2c_flag = false;
					break;
				}
				else {
					jp2c_flag = true;
				}
			}

			byte toggles = sj_get_byte(&app11_data);
			bool label_present = isJthBitSet_1(toggles, 2);
			bool ID_present = isJthBitSet_1(toggles, 3);
			bool Sign_present = isJthBitSet_1(toggles, 4);
			char* label;
			if (label_present) {
				uint label_length = jumdlength - 8 - 16 - 1 - (ID_present ? 4 : 0) - (Sign_present ? 256 : 0);
				label = new char[label_length];
				for (uint k = 0; k < label_length; ++k)
					label[k] = sj_get_byte(&app11_data);
			}
			if (ID_present)
				int id = sj_get_4byte(&app11_data);
			char* signature;
			if (Sign_present) {
				signature = new char[256];
				for (uint k = 0; k < 256; ++k)
					signature[k] = sj_get_byte(&app11_data);
			}
			else
				signature = NULL;
			if (jp2c_flag) {
				uint content_box_length = sj_get_4byte(&app11_data);
				uint content_box_type = sj_get_4byte(&app11_data);
				if (content_box_type == BOX_TYPE_CODESTREAM)
				{
					codestream_box->length = content_box_length - 8 + codestream_data.size();
					codestream_box->box_type = BOX_TYPE_CODESTREAM;
					codestream_data.insert(codestream_data.end(), &app11_data[0], &app11_data[content_box_length - 8]);
					app11_data += (content_box_length - 8);
				}
				else {
					cout << "Content Box other than codestream Type (" << BOX_TYPE_CODESTREAM << ") is not supported" << endl;
				}
			}

		}
		else
		{
			cout << "Box_Type other than JUMBF (" << BOX_TYPE_JUMB << ") is not supported" << endl;
		}


	}

	//sj_jumbf_super_box_ptr extract_jumbf_super_box(byte *data, uint size)
	//{
	//	sj_jumbf_super_box_ptr super_box1 = (sj_jumbf_super_box_ptr)malloc(sizeof(sj_jumbf_super_box));
	//	memset(super_box1, 0, sizeof(sj_jumbf_super_box));
	//	sj_jumbf_desc_box_ptr desc_box = (sj_jumbf_desc_box_ptr)malloc(sizeof(sj_jumbf_desc_box));
	//	memset(desc_box, 0, sizeof(sj_jumbf_desc_box));
	//	sj_jumbf_contiguous_codestream_box_ptr jp2c_box;
	//	jp2c_box = (sj_jumbf_contiguous_codestream_box_ptr)malloc(sizeof(sj_jumbf_contiguous_codestream_box));
	//	memset(jp2c_box, 0, sizeof(sj_jumbf_contiguous_codestream_box));
	//	read_app11_marker(data, size, super_box1);
	//	return super_box1;
	//}	
	//sj_jumbf_super_box_ptr extract_jumbf_super_box(byte* data, uint size) {
	//	sj_jumbf_super_box_ptr super_box1 = (sj_jumbf_super_box_ptr)malloc(sizeof(sj_jumbf_super_box));
	//	memset(super_box1, 0, sizeof(sj_jumbf_super_box));
	//	sj_jumbf_desc_box_ptr desc_box = (sj_jumbf_desc_box_ptr)malloc(sizeof(sj_jumbf_desc_box));
	//	memset(desc_box, 0, sizeof(sj_jumbf_desc_box));
	//	sj_jumbf_contiguous_codestream_box_ptr jp2c_box;
	//	jp2c_box = (sj_jumbf_contiguous_codestream_box_ptr)malloc(sizeof(sj_jumbf_contiguous_codestream_box));
	//	memset(jp2c_box, 0, sizeof(sj_jumbf_contiguous_codestream_box));
	//	vector<byte> codestream_data; ///concatenated
	//	byte last = sj_get_byte(&data);
	//	byte current = sj_get_byte(&data);
	//	if (last != 0xFF || current != M_SOI) {
	//		cout << "Image is not JPEG" << endl;
	//		return false;
	//	}
	//	last = sj_get_byte(&data);
	//	current =sj_get_byte(&data);
	//	uint app11_counter = 0;
	//	//reading markers;
	//	while (1)
	//	{
	//		if (last != 0xFF)
	//		{
	//			std::cout << "Error - Expected a marker\n";
	//			break;
	//		}
	//		else if (current == M_SOS) {
	//			std::cout << "meet Start of Scan Marker :\n";
	//			break;
	//		}
	//		else if (current == M_APP11)
	//		{
	//			app11_counter++;
	//			cout << "APP11 Marker No : " << app11_counter << endl;
	//			int len = sj_get_2byte(&data);
	//			byte * app11_data = data - 4;
	//			readAPP11(app11_data, codestream_data, super_box1, desc_box, jp2c_box);
	//			cout << "Codestream_data Vector Size after adding new marker values : " << codestream_data.size() << endl;
	//			data += (len - 2);
	//		}
	//				
	//		else if (current == M_SOI) {
	//			std::cout << "Error - Embedded JPGs not supported\n";
	//			break;
	//		}
	//		else if (current == M_EOI) {
	//			std::cout << "Error - EOI detected before SOS\n";
	//			break;
	//		}
	//		else {
	//			int len = sj_get_2byte(&data);
	//			data += (len-2);
	//		}
	//		last = sj_get_byte(&data);
	//		current = sj_get_byte(&data);
	//	}
	//	jp2c_box->codestream = codestream_data.data();
	//	super_box1->next_box = (sj_jumbf_box_ptr)desc_box;
	//	desc_box->next_box = (sj_jumbf_box_ptr)jp2c_box;
	//	jp2c_box->next = NULL;
	//	
	//	return super_box1;
	//}

	//void readAPP11(byte * app11_data, vector<byte> &codestream_data, sj_jumbf_super_box_ptr super_box, sj_jumbf_desc_box_ptr desc_box, sj_jumbf_contiguous_codestream_box_ptr codestream_box)
	//{
	//
	//	uint marker = sj_get_2byte(&app11_data);
	//	if (marker != 0xFFEB) {
	//		cout << "marker is not APP11" << endl;
	//		return;
	//	}
	//	
	//	uint length = sj_get_2byte(&app11_data);
	//	uint CI = sj_get_2byte(&app11_data);
	//	uint En = sj_get_2byte(&app11_data);
	//	uint Z = sj_get_4byte(&app11_data);
	//	if (Z - previos_Z == 1)
	//	{
	//		cout << "Need to concatenate with previos" << endl;
	//		previos_Z = Z;
	//	}
	//	else
	//		cout << "Not to catenate : " << endl;
	//	//previos_Z = Z;
	//	uint LBox = sj_get_4byte(&app11_data);
	//	cout << "Superbox Length is : " << LBox << endl;
	//	uint TBox = sj_get_4byte(&app11_data);
	//	super_box->length = LBox;
	//	super_box->box_type = TBox;
	//	if (TBox == BOX_TYPE_JUMB)
	//	{
	//		cout << "JUMBF box found : " << endl;
	//		uint jumdlength = sj_get_4byte(&app11_data);
	//		cout << "JUMD Length is : " << jumdlength << endl;
	//		uint jumdtype = sj_get_4byte(&app11_data);
	//		desc_box->length = jumdlength;
	//		desc_box->box_type = jumdtype;
	//		//super_box->next_box = (sj_jumbf_box_ptr)desc_box;
	//		for (uint k = 0; k < 16; k++)
	//			desc_box->type[k] = sj_get_byte(&app11_data);
	//		desc_box->toggles = sj_get_byte(&app11_data);
	//		bool label_present = isJthBitSet_1(desc_box->toggles, 2);
	//		bool ID_present = isJthBitSet_1(desc_box->toggles, 3);
	//		bool Sign_present = isJthBitSet_1(desc_box->toggles, 4);
	//		if (label_present) {
	//			uint label_length = jumdlength - 8 - 16 - 1 - (ID_present ? 4 : 0) - (Sign_present ? 256 : 0);
	//			desc_box->label = new char[label_length];
	//			for (uint k = 0; k < label_length; ++k)
	//				desc_box->label[k] = sj_get_byte(&app11_data);
	//		}
	//		if (ID_present)
	//			desc_box->id = sj_get_4byte(&app11_data);
	//		if (Sign_present) {
	//			desc_box->signature = new char[256];
	//			for (uint k = 0; k < 256; ++k)
	//				desc_box->signature[k] = sj_get_byte(&app11_data);
	//		}
	//		else
	//			desc_box->signature = NULL;
	//		//if (Z == 1) {
	//		//	jp2c_box = (sj_jumbf_contiguous_codestream_box_ptr)malloc(sizeof(sj_jumbf_contiguous_codestream_box));
	//		//	memset(jp2c_box, 0, sizeof(sj_jumbf_contiguous_codestream_box));
	//		//	jp2c_box->length = LBox - desc_box->length - 8;
	//		//	jp2c_box->box_type = BOX_TYPE_CODESTREAM;
	//		//	jp2c_box->codestream = new byte[jp2c_box->length - 8];
	//		//	jp2c_box->next = NULL;
	//		//}
	//		uint content_box_length = sj_get_4byte(&app11_data);
	//		cout << "Current Marker Content Box lenght : " << content_box_length << endl;
	//		uint content_box_type = sj_get_4byte(&app11_data);
	//		if (content_box_type == BOX_TYPE_CODESTREAM)
	//		{				
	//			codestream_box->length = content_box_length-8 + codestream_data.size();
	//			//jp2c_box->box_type = content_box_type;
	//			//jp2c_box->codestream = new byte[jp2c_box->length - 8];
	//			//memcpy(jp2c_box->codestream, app11_data, content_box_length - 8);
	//			codestream_data.insert(codestream_data.end(), &app11_data[0], &app11_data[content_box_length - 8]);
	//			//jp2c_box->codestream += (content_box_length - 8);
	//			app11_data += (content_box_length - 8);
	//		}
	//		else {
	//			cout << "Content Box other than codestream Type (" << BOX_TYPE_CODESTREAM << ") is not supported" << endl;
	//		}
	//	}
	//	else
	//	{
	//		cout << "Box_Type other than JUMBF ("<<BOX_TYPE_JUMB<<") is not supported" << endl;
	//	}
	//	
	//	
	//}


