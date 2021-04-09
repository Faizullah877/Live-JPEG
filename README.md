# live_jpeg_v02
This version of code is submitted to Professor Kwon. This is a Live-JPEG codec in working status. The last results presented in the lab meetings are generated using this executable
Live-JPEG Command line Usage
 

Encoding Example:
Live_jpeg_v02.exe -i input_file_name.yuv -o output_encoded_file_name.jpg -w 1234 -h 1234 -q 75 -isf 444P -pix_fmt YUV -if 450 
Live_jpeg_v02.exe -i input_file_name.yuv -o output_encoded_file_name.jpg -w 1234 -h 1234 -q 75 -isf 444P -pix_fmt YUV -if 450 -of 10 -arith 1 -sub_samp 444
live_jpeg_v02.exe -i AI_Building_3840x1920_YUV444P_450f.yuv -o AI_Building_3840x1920_YUV444P_450f_q75_420_huff.jpg -w 3840 -h 1920 -isf 444P -pix_fmt YUV -if 450

Encoding Parameters
S. No	Argument	Argument description	Possible values	Default value
1	-i	Input raw video file name
File should be .yuv
Frames packing should be interleaved or planner 444-pixel format		
2	-o	Output file name.
Must have .jpg extension		
3	-isf	Input file packing format
Interleaved or planner?
	444I
444P	444I
4	-if	No of frames in input file		0
5	-of	No of frames user want to encode	Any number smaller than ‘-if’ value	0
6	-w	Width in pixels of each input video frame		0
7	-h	Height in pixels of each input video frame		0
8	-q	Quality of each frame	0-100	95
9	-pix_fmt	Input frames color space	RGB
YUV	RGB
10	-wsf	Write each frame as separate JPEG file	0
1	0
11	-wdf	Write each difference frame as separate JPEG file	0
1	0
12	-sub_samp	Each frame encoding Subsampling format	420
444	420
13	-arith	Enables Arithmetic entropy encoding for each frame	0
1	0







Decoding Example
live_jpeg_v02.exe -i AI_Building_3840x1920_YUV444P_450f_q75_420_huff.jpg -o De_AI_Building_3840x1920_YUV444P_450f.yuv -pix_fmt YUV -osf 444P
Decoding Parameters
S. No	Argument	Argument description	Possible values	Default value
1*	-i	Input live-JPEG encoded file name
File should be .jpg		
2*	-o	Output decoded raw file name.
Must have .yuv extension		
3	-of	No of frames user want to decode	Any number smaller than total no of frames	Total number of frames
4	-wsf	Write each frame as separate JPEG file	0
1	0
5	-wdf	Write each difference frame as separate JPEG file	0
1	0
6	-osf	Output file frame packing format	444I
444P	444I
7	-pix_fmt	Output raw frames color space	RGB
YUV	RGB
8	-wsf	decode each frame as separate JPEG file	0
1	0
9	-wdf	Decode each difference frame as separate JPEG file	0
1	0


Note: 
1.	Some of the sample raw videos are \\210.107.229.81\shared\Faizullah\Sample_videos\Raw_Sample_Videos 
2.	The program differentiates b/w the encoding and decoding process by input file name extension. If input file name extension is .yuv, encoding process will take place. If input file name extension is .jpg, Decoding will be processed.
3.	The program saves its encoding history and log in a text file named as Live_jpeg_Encoding_history.txt created automatically in same directory as live_jpeg_v02.exe
4.	Similarly decoding history is saved in Live_jpeg_Decoding_history.txt.
