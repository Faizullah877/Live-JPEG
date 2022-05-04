#include "pix_format_converter.h"


void convert_yuv444p_to_yuv444I(Byte* rawFrameData, uint rawFrameSize, uint width, uint height) {

	Byte* odata = new Byte[rawFrameSize];
	for (int i = 0; i < rawFrameSize; ++i)
	{
		if (i % 3 == 0) odata[i] = rawFrameData[i / 3];
		if (i % 3 == 1) odata[i] = rawFrameData[(i / 3) + (width * height)];
		if (i % 3 == 2) odata[i] = rawFrameData[(i / 3) + (width * height * 2)];
	}

	memcpy(rawFrameData, odata, rawFrameSize);
	delete[] odata;
	//rawFrameData = odata;
}


void convert_yuv444I_to_yuv444p(Byte* rawFrameData, uint rawFrameSize, uint width, uint height) {
	Byte* odata = new Byte[rawFrameSize];
	for (int i = 0; i < rawFrameSize; ++i)
	{
		if (i % 3 == 0) odata[i / 3] = rawFrameData[i];
		if (i % 3 == 1) odata[(i / 3) + (width * height)] = rawFrameData[i];
		if (i % 3 == 2) odata[(i / 3) + (width * height * 2)] = rawFrameData[i];
	}
	memcpy(rawFrameData, odata, rawFrameSize);
	delete[] odata;
}
void convert_420planer_to_444Interleave(Byte* inDatabuffer, uint outDatabuffer) {
	//if (inDatabuffer->data == NULL) cout << "empty input buffer " << endl;
	//if (inDatabuffer->info.width == 0 || inDatabuffer->info.height == 0) cout << "Width or height not correct" << endl;
	//int inframeSize = inDatabuffer->info.width * inDatabuffer->info.height * 1.5;
	//int outFrameSize = inDatabuffer->info.width * inDatabuffer->info.height * 3;

	//outDatabuffer->size = inDatabuffer->info.width * inDatabuffer->info.height * inDatabuffer->info.num_comp * outDatabuffer->info.frame_count;
	//byte* indata = inDatabuffer->data;

	//byte* odata = new byte[outDatabuffer->size];


	//int width = inDatabuffer->info.width;
	//int height = inDatabuffer->info.height; 
	//int wy = width;
	//int hy = height;
	//int wuv = wy / 2;
	//int huv = hy / 2;
	//int sizeY = wy * hy;
	//int sizeUV = wuv * huv;

	//unsigned char* Y;
	//unsigned char* U;
	//unsigned char* V;

	//unsigned char* Un = new byte[sizeY];
	//unsigned char* Vn = new byte[sizeY];

	//for (int f = 0; f < outDatabuffer->info.frame_count; f++)
	//{
	//	Y = indata;
	//	U = indata + sizeY;
	//	V = indata + sizeY + sizeUV;

	//	for (int r = 0; r < hy; ++r) {
	//		for (int c = 0; c < wy; ++c)
	//		{
	//			Un[r * wy + c] = U[(r / 2) * wuv + (c / 2)];
	//			Vn[r * wy + c] = V[(r / 2) * wuv + (c / 2)];

	//		}
	//	}

	//	for (uint i = 0; i < sizeY; ++i) {
	//		*odata++ = Y[i];
	//		*odata++ = Un[i];
	//		*odata++ = Vn[i];
	//	}
	//	indata += inframeSize;
	//}


	//odata -= (outFrameSize * outDatabuffer->info.frame_count);
	//outDatabuffer->data = odata;
	//outDatabuffer->info.width = inDatabuffer->info.width;
	//outDatabuffer->info.height = inDatabuffer->info.height;
	//outDatabuffer->info.num_comp = inDatabuffer->info.num_comp;
}