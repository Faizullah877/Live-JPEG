#pragma once
#include <iostream>

typedef unsigned char Byte;
typedef unsigned int uint;
#define MAX_PATH (256)
void print_usage();
typedef enum {                  /* JPEG marker codes */
	M_SOF0 = 0xc0,
	M_SOF1 = 0xc1,
	M_SOF2 = 0xc2,
	M_SOF3 = 0xc3,

	M_SOF5 = 0xc5,
	M_SOF6 = 0xc6,
	M_SOF7 = 0xc7,

	M_JPG = 0xc8,
	M_SOF9 = 0xc9,
	M_SOF10 = 0xca,
	M_SOF11 = 0xcb,

	M_SOF13 = 0xcd,
	M_SOF14 = 0xce,
	M_SOF15 = 0xcf,

	M_DHT = 0xc4,

	M_DAC = 0xcc,

	M_RST0 = 0xd0,
	M_RST1 = 0xd1,
	M_RST2 = 0xd2,
	M_RST3 = 0xd3,
	M_RST4 = 0xd4,
	M_RST5 = 0xd5,
	M_RST6 = 0xd6,
	M_RST7 = 0xd7,

	M_SOI = 0xd8,
	M_EOI = 0xd9,
	M_SOS = 0xda,
	M_DQT = 0xdb,
	M_DNL = 0xdc,
	M_DRI = 0xdd,
	M_DHP = 0xde,
	M_EXP = 0xdf,

	M_APP0 = 0xe0,
	M_APP1 = 0xe1,
	M_APP2 = 0xe2,
	M_APP3 = 0xe3,
	M_APP4 = 0xe4,
	M_APP5 = 0xe5,
	M_APP6 = 0xe6,
	M_APP7 = 0xe7,
	M_APP8 = 0xe8,
	M_APP9 = 0xe9,
	M_APP10 = 0xea,
	M_APP11 = 0xeb,
	M_APP12 = 0xec,
	M_APP13 = 0xed,
	M_APP14 = 0xee,
	M_APP15 = 0xef,

	M_JPG0 = 0xf0,
	M_JPG13 = 0xfd,
	M_COM = 0xfe,

	M_TEM = 0x01,

	M_ERROR = 0x100
} JPEG_MARKER;

#define SIZE_OF_SIGNATURE	32

#define SIZE_SEGMENT_ELEMENT 20
#define SIZE_LABEL_ELEMENT 5

#define METHOD_SIGNATURE 1
#define METHOD_WATERMARK 2
#define METHOD_WATERMARK_MODIFIED_REGION 3
typedef struct SJ_SEGMENT_ELEMENT {
	bool selected = false;
	unsigned int offset;
	unsigned int length;
	unsigned int ID;

}  * SJ_SEGMENT_ELEMENT_PTR;

typedef struct SJ_SEGMENT_STRUCT {
	SJ_SEGMENT_ELEMENT_PTR* pSegment_element;
	char size = -1;
}* SJ_SEGMENT_STRUCT_PTR;


typedef struct SJ_LABEL_ELEMENT {
	bool selected = false;
	unsigned char* label;
	unsigned int offset;
	unsigned int length;
	char size;
}*SJ_LABEL_ELEMENT_PTR;


typedef struct SJ_LABEL{
	SJ_LABEL_ELEMENT_PTR* pLabel;
	char size = -1;
} * SJ_LABEL_PTR;


typedef struct {


	SJ_LABEL m_label;

	SJ_SEGMENT_STRUCT m_segment;


}SJ_SEGMENT, * SJ_SEGMENT_PTR;



//extern unsigned int sj_get_byte(unsigned char** buf);
//extern unsigned int sj_get_2byte(unsigned char** buf);
//extern long sj_get_4byte(unsigned char** buf);


typedef enum {
	MEDIA_PPM,
	MEDIA_PFM,
	MEDIA_BMP,
	MEDIA_JPEG,
	MEDIA_JPEG2000,

	MEDIA_RGB444I,
	MEDIA_RGB444P,
	MEDIA_YUV420I,
	MEDIA_YUV444I,

	MEDIA_YUV420P,      // Planar
	MEDIA_YUV444P,      // Planar
	MEDIA_YCbCr420,     // Planar
	MEDIA_YCbCr422H,    // Planar
	MEDIA_YCbCr422V,    // Planar
	MEDIA_YCbCr444,     // Planar
	MEDIA_RGB888,       // Interleaved
	MEDIA_RGB565,       // Interleaved
	MEDIA_RGB32F,       // Interleaved
	MEDIA_GRAY,
	MEDIA_BYTESTREAM,
	MEDIA_NULL = -1
} MEDIA_FORMAT;



typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int padded_width; // only padded pixels
    unsigned int padded_height;
    unsigned int num_comp;
    MEDIA_FORMAT format;
} MEDIA_INFO;

class DataBuffer {
public:
    void* owner;
    Byte* data;
    void* additional_data;
    unsigned int    size; // size in a byte type

    MEDIA_INFO      info;

    // for JPEG/JPEGXT

public:
    DataBuffer() : owner(0) { reset(); }
    virtual ~DataBuffer() {}

    void reset() {
        owner = 0;
        data = 0;
        additional_data = 0;
        size = 0;

        info.width = info.height = 0;
        info.padded_width = info.padded_height = 0;
        info.num_comp = 0;
    }

    DataBuffer& operator=(const DataBuffer* ptr) {
        this->info.width = ptr->info.width;
        this->info.height = ptr->info.height;
        this->info.padded_height = ptr->info.padded_height;
        this->info.padded_width = ptr->info.padded_width;

        return *this;
    }
};

typedef struct {
	int JPEG360Version;
	char* MediaType;
	char* ProjectionType;
	float PhiMin;
	float PhiMax;
	float ThetaMax;
	float ThetaMin;
	float PhiGravity;
	float ThetaGravity;
	float CompassPhi;
	char* BoxReference;
}SJ_360ImageMeta, * SJ_360ImageMeta_Ptr;

typedef struct {
	int JPEG360ViewportNumber;
	float ViewportPhi;
	float ViewportTheta;
	float ViewportPhiFOV;
	float ViewportThetaFOV;
	float ViewportRoll;
}SJ_360ViewMeta_ELE, * SJ_360ViewMeta_ELE_Ptr;

typedef struct {
	SJ_360ViewMeta_ELE_Ptr* metadata;
	int viewportnumber;
}SJ_360ViewMeta, * SJ_360ViewMeta_Ptr;


typedef struct {
	SJ_360ImageMeta_Ptr JPEG360ImageMetadata;
	SJ_360ViewMeta_Ptr JPEG360VivewportMetadata;
}SJ_360Meta, * SJ_360Meta_Ptr;


#define MAX_VP 10

	struct ColorComponent {
		Byte horizontalSamplingFactor = 1;
		Byte verticalSamplingFactor = 1;
		Byte quantizationTableID = 0;
		Byte huffmanDCTableID = 0;
		Byte huffmanACTableID = 0;
		bool used = false;
	};

	struct xmlContent {
		bool used = false;
		unsigned int xmlLength;
		int boxType;
		unsigned char* data;
	};
	struct jumbfSupBoxInfo {
		bool used = false;
		int boxType;
		int length;
	};
	struct jumbfDesBox {
		bool used = false;
		int jumd_lenght = 0;
		int type = 0;
		int UUID[16] = { 0 };
		int toggle = 0;
		bool requestable = false;
		bool label_present = false;
		unsigned char lable[256] = { 0 };
		bool ID_present = false;
		int ID = 0;
		bool sign_present = false;
		int signature[32] = { 0 };
	};


	struct APP11_marker
	{
		bool APP11Flag = false;
		int markerOffst = 0;
		int Le = 0;
		int CI_dec = 0;
		int En = 0;
		int Z = 0;
		int LBox = 0;
		int Tbox;
		bool XLBox_present = false;
		unsigned long int XLBox = 0;
	};

	struct Camera
	{
		std::string make;
		std::string model;
	};
	struct Header {
		Byte frameType = 0;
		uint height = 0;
		uint width = 0;
		short numComponents = 0;
		bool ERP_flag = false;
		bool zeroBased = false;
		bool Camera360_flag = false;
		ColorComponent colorComponents[3];
		bool valid = true;
		std::string projectionTypeGPano;
		bool UsePanoramaViewerGPano = false;
		bool isJPEG360 = false;
		APP11_marker app11;
		Camera camera;
		jumbfSupBoxInfo superbox;
		jumbfDesBox jumdbox;
		xmlContent xmlbox;
		SJ_360ViewMeta_ELE VP_metadata[MAX_VP];
		SJ_360ImageMeta IMG_metadata;
	};


	typedef enum
	{
		APP1_EXIF = 0x45786966,
		APP1_XMP = 0x68747470
	}APP1_TYPES;

	unsigned int sj_get_2IIbyte(unsigned char** buf);
	unsigned int sj_get_byte(unsigned char** buf);

	unsigned int sj_get_2byte(unsigned char** buf);

	long sj_get_4byte(unsigned char** buf);


	void decodeIFD_IIfield(Byte** buf, Header* header, unsigned char* TIFF_data);
	void decodeIFD_MMfield(Byte** buf, Header* header, unsigned char* TIFF_data);
	void copy_jpeg_header(Byte* image_data, uint img_size, Byte** header_buf, uint* header_size);
	Header* read_image_metadata(Byte* fdata, uint fsize);

	void ReadBytestream2(const char* InputFile, DataBuffer* pBuffer);
	

