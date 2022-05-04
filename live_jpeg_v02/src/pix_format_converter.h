#pragma once
#include "Header.h"
void convert_yuv444p_to_yuv444I(Byte* rawFrameData, uint rawFrameSize, uint width, uint height);
void convert_yuv444I_to_yuv444p(Byte* rawFrameData, uint rawFrameSize, uint width, uint height);
void convert_420planer_to_444Interleave(Byte* inDatabuffer, uint outDatabuffer);

