#pragma once
#include "Header.h"
void convert_yuv444p_to_yuv444I(byte* rawFrameData, uint rawFrameSize, uint width, uint height);
void convert_yuv444I_to_yuv444p(byte* rawFrameData, uint rawFrameSize, uint width, uint height);
void convert_420planer_to_444Interleave(byte* inDatabuffer, uint outDatabuffer);

