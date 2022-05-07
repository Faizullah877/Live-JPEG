# DPCMed-JPEG

DPCMed-JPEG is a video codec based on JPEG-1. Basic Concept is that the discrete cosine tranform (DCT) coefficients of next frame are subtracted from the DCT coeffieients of previous frame and then the resultant coefficients are encoded using entropy coding used same in JPEG. 
----------------------------
### DPCMed-JPEG Command line Usage
 

#### Encoding
Example command:

```
DPCMed_JPEG.exe -i input_file_name.yuv -o output_encoded_file_name.jpg -w 1234 -h 1234 -q 75 -isf 444P -pix_fmt YUV -if 450

DPCMed_JPEG.exe -i input_file_name.yuv -o output_encoded_file_name.jpg -w 1234 -h 1234 -q 75 -isf 444P -pix_fmt YUV -if 450 -of 10 -arith 1 -sub_samp 444

DPCMed_JPEG.exe -i AI_Building_3840x1920_YUV444P_450f.yuv -o AI_Building_3840x1920_YUV444P_450f_q75_420_huff.jpg -w 3840 -h 1920 -isf 444P -pix_fmt YUV -if 450

DPCMed_JPEG.exe -f jpg_images_folder -o DPCMed_JPEG_enocoded_file.jpg -q 85 -arith 1
```

------------------------------------
#### Encoding Parameters:

| S. No	| Argument	| 	Argument description	| 			Possible values		| 		Default value |
| ----- | -------- | --------------------- | ------------------- | --------------- |
| 1* | `-t`  |  work type | encode/decode | encode |
| 2	| `-i`		| 	Input raw `.yuv` video file name		| 	string (max 255 characters) with `.yuv` extension | `input.yuv` |
| 3 | `-f`  |  Source Folder name containing `.jpg` images | string (max 255 characters) | `test` | 
| 4	| `-o`		| 	Output encoded file name. 				| string (max 255 characters) with .jpg extension		| `output.jpg` |
| 5	| `-isf`	| Input raw pixels packing | `444I` for 444 interleaved,  `444P` for 444 planner | `444I` |
| 6	| `-if`	| 	No of frames in input file		| 	| `0` |
| 7	| `-of`	| 	No of frames to be encoded		| Any number <= "-if" | `0` |
| 8	| `-w`		| Width in pixels of each input video frame		| width | `0` |
| 9	| `-h`	 |	Height in pixels of each input video frame		| height |  `0` |
| 10	| `-q`	| Quantization parameter same as in JPEG 	| 	`0-100`	|	`95` |
| 11	| `-pix_fmt`	| Input pixels color space			| `RGB`/`YUV` |		`RGB` |
| 12	| `-wsf`	| 	Write each frame as separate JPEG file		| `1` / `0` |		`0` |
| 13	| `-wdf`	| 	Write each difference frame as separate JPEG file	| `0` or `1`	| 	`0` |
| 14	| `-sub_samp` | 	JEPG encoding Subsampling format		| `420`/`444`	|			`420` |
| 15	| `-arith`	| Enables Arithmetic entropy encoding 		| `0` / `1`	| 	`0` |




### Decoding
Example command
```
DPCMed_JPEG.exe -i AI_Building_3840x1920_YUV444P_450f_q75_420_huff.jpg -o De_AI_Building_3840x1920_YUV444P_450f.yuv -pix_fmt YUV -osf 444P
```
#### Decoding Parameters

| S. No |	Argument	| Argument description	            | Possible values	| Default value |
| ----- | -------- | -------------------------------- | --------------- | ------------- |
| 1     | `-t`       | work type                        | `encode`/`decode`   | `encode`        | 
| 2     | `-i`	      | DPCMed_JPEG encoded .jpg file name | `abc.jpg`         | Null          |
| 3     |	`-o`	      | decoded raw .yuv file name       | `output.yuv`      | Null          |
| 4	    | `-of`	     | No of frames to decode	          | Any number <= total no of frames |	Total number of frames |
| 5   	 | `-wsf`	    | Write each frame as separate JPEG file	| `0`/`1` | `0` |  
| 6	    | `-wdf`    	| Write each difference frame as separate JPEG file | `0`/`1`  | `0` |  	
| 7	    | `-osf`	    | Output pixels packing 	| `444I` for 444 interleaved | `444P` for 444 planner |  `444I` | 	
| 8	    | `-pix_fmt` | Output color space	| `RGB` / `YUV`	| `RGB` | 


Note: 
1.	Some of the sample raw videos are \\210.107.229.81\shared\Faizullah\Sample_videos\Raw_Sample_Videos 
2.	The program differentiates b/w the encoding and decoding process by input file name extension. If input file name extension is .yuv, encoding process will take place. If input file name extension is .jpg, Decoding will be processed.
3.	The program saves its encoding history and log in a text file named as DPCMed_JPEG_Encoding_history.txt created automatically in same directory as DPCMed_JPEG.exe
4.	Similarly decoding history is saved in DPCMed_JPEG_Decoding_history.txt.
