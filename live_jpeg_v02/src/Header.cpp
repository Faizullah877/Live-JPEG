#include "Header.h"
#include "Box.h"
#include <vector>
using namespace std;

unsigned char type_jpeg360[16] = { 0x78, 0x5F, 0x34, 0xB7, 0x5D, 0x4B, 0x47, 0x4C, 0xB8, 0x9F, 0x1D, 0x99, 0xE0, 0xE3, 0xA8, 0xDD };



void print_usage()
{
	printf("Usage: live_jpeg_v02.exe <options>\n");
	printf("Options: \n");
	printf("    -i          Source File name            (max 255 characters).\n");
	printf("    -o          Output filename             (max 255 characters).\n");
	//printf("    -r          Reference filename          (max 255 characters).\n");
	printf("    -isf        Source samples packing format      (444P, 444I).\n");
	printf("    -osf        Output samples packing format      (444P, 444I).\n");
	printf("    -if         Source frame count.\n");
	printf("    -of         Output frames count.\n");
	printf("    -w          Source Frame width.\n");
	printf("    -h          Source Frame height.\n");
	printf("    -q          quality  (0-100)\n");
	printf("    -pix_fmt    Source pixels color space   (RGB/YUV)\n");
	printf("    -sub_samp   Frame_encoding sub-sampling (420/444)\n");
	printf("    -wsf        Write each frame as seperate jpg file (value 0 or 1).\n");
	printf("    -wdf        Write each Difference frame as seperate jpg file (value 0 or 1).\n");
	printf("    -arith      Entropy Encoding Scheme (arithmetic = 1) default is huffman.\n");
	//printf("    -t          work = find psnr (value = psnr).\n");
	printf("\n");
	return;
	//exit(1);
}
unsigned int sj_get_byte(unsigned char** buf)
/* Read next input byte; we do not support suspension in this module. */
{

	unsigned int val;
	val = *(*buf)++;
	return val;

}

unsigned int sj_get_2byte(unsigned char** buf)
{
	int val1, val2;
	val1 = (sj_get_byte(buf)) << 8;
	val2 = sj_get_byte(buf);
	return val1 + val2;

}

long sj_get_4byte(unsigned char** buf)
{
	long val1, val2;
	val1 = (sj_get_2byte(buf)) << 16;
	val2 = sj_get_2byte(buf);

	return val1 + val2;
}



unsigned int sj_get_2IIbyte(unsigned char** buf)
{
	int val1, val2;
	val1 = (sj_get_byte(buf));
	val2 = sj_get_byte(buf);
	return val1 + (val2 << 8);
}

long sj_get_4IIbyte(unsigned char** buf)
{
	int val1, val2;
	val1 = (sj_get_2IIbyte(buf));
	val2 = sj_get_2IIbyte(buf);
	return val1 + (val2 << 16);
}

long get_8IIbyte(unsigned char** buf)
{
	int val1, val2;
	val1 = (sj_get_4IIbyte(buf));
	val2 = sj_get_4IIbyte(buf);
	return val1 + (val2 << 32);
}

bool isKthBitSet_1(byte n, int k)
{
	if ((n >> (k - 1)) & 1)
		return true;
	else
		return false;
}

void decodeIFD_IIfield(byte** buf, Header* header, unsigned char* TIFF_data) {
	int tag = sj_get_2IIbyte(buf);
	int type = sj_get_2IIbyte(buf);
	uint count = sj_get_4IIbyte(buf);
	uint offset = sj_get_4IIbyte(buf);
	if (tag == 0x010F)
	{
		//std::cout << "\nCamera Make tag is present in Exif\n";
		byte* make = new byte[count + 1];
		for (int i = 0; i < count; i++)
		{
			make[i] = TIFF_data[i + offset];
		}
		make[count] = '\0';
		header->camera.make = (char*)make;
	}
	else if (tag == 0x0110) {
		//std::cout << "\nCamera Model tag is present in Exif";
		byte* model = new byte[count + 1];
		for (int i = 0; i < count; i++)
		{
			model[i] = TIFF_data[i + offset];
		}
		model[count] = '\0';
		header->camera.model = (char*)model;
	}
	else if (tag == 0x0112) {
		// std::cout<<"\nOrientation of Image information: ";

	}
}

void decodeIFD_MMfield(byte** buf, Header* header, unsigned char* TIFF_data) {
	int tag = sj_get_2byte(buf);
	int type = sj_get_2byte(buf);
	int count = sj_get_4byte(buf);
	int offset = sj_get_4byte(buf);
	if (tag == 0x010F)
	{
		//std::cout << "\nCamera Make tag is present in Exif\n";
		byte* make = new byte[count + 1];
		for (int i = 0; i < count; i++)
		{
			make[i] = TIFF_data[i + offset];
		}
		make[count] = '\0';
		header->camera.make = (char*)make;
	}
	else if (tag == 0x0110) {
		//std::cout << "\nCamera Model tag is present in Exif";
		byte* model = new byte[count + 1];
		for (int i = 0; i < count; i++)
		{
			model[i] = TIFF_data[i + offset];
		}
		model[count] = '\0';
		header->camera.model = (char*)model;

	}
}

void readStartOfframe(byte* buf, Header* header)
{
	//std::cout << "Reading SOF Marker\n";
	if (header->numComponents != 0) {
		//std::cout << "Error - Multiple SOFs detected\n";
	}
	unsigned int lenght = sj_get_2byte(&buf);
	byte precision = sj_get_byte(&buf);
	if (precision != 8)
	{
		std::cout << "Error - Invalid precision " << (unsigned int)precision << '\n';
	}
	header->height = sj_get_2byte(&buf);
	header->width = sj_get_2byte(&buf);
	if (header->height == 0 || header->width == 0)
	{
		std::cout << "Error - Invalid dimensions\n";
	}
	header->numComponents = sj_get_byte(&buf);
	//if (header->numComponents == 4) {
	//	std::cout << "Error - CMYK color mode not supported \n";
	//}
	//if (header->numComponents == 0) {
	//	std::cout << "Error - Number of Color Components must not be zero\n";
	//}

	for (unsigned int i = 0; i < header->numComponents; ++i)
	{
		byte componentID = sj_get_byte(&buf);
		//component IDs are usually 1,2,3 but rarely can be seen as 0, 1, 2
		// always force them into 1, 2, 3 for consistency
		if (componentID == 0)
		{
			header->zeroBased = true;
		}
		if (header->zeroBased == true)
		{
			componentID += 1;
		}
		if (componentID == 4 || componentID == 5)
		{
			std::cout << "Error - YIQ color mode not supported\n";
			//header->valid = false;
			//return;
		}
		if (componentID == 0 || componentID > 3)
		{
			std::cout << "Error - Invalid Component ID : " << (unsigned int)componentID << "\n";
			//header->valid = false;
			//return;
		}

		ColorComponent* component = &header->colorComponents[componentID - 1];
		if (component->used) {
			std::cout << "Error- Duplicate color component ID \n";
			//header->valid = false;
			//return;
		}
		component->used = true;
		byte samplingFactor = sj_get_byte(&buf);
		component->horizontalSamplingFactor = samplingFactor >> 4;
		component->verticalSamplingFactor = samplingFactor & 0x0F;


		component->quantizationTableID = sj_get_byte(&buf);

		if (component->quantizationTableID > 3)
		{
			std::cout << "Error - Invalid Quantization Table ID in frame component\n";
		}
	}
	if (lenght - 8 - (3 * header->numComponents) != 0) {
		std::cout << "Error - SOF invalid\n";
	}

}

bool readAPP1Exif(byte** buf, Header* header, int length)
{
	const int No_of_Cameras = 17;
	std::string camera_model[No_of_Cameras] = {
		"Insta360",
		"RICOH THETA V",
		"RICOH THETA S",
		"VIRB 360",
		"GoPro Fusion",
		"Samsung Gear 360",
		"LG-R105",
		"360fly Android",
		"HumanEyes VR Studio",
		"PTGui",
		"DETU",
		"F4_PLUS",
		"Insta360 ONE X",
		"ONEPLUS A6003",
		"iris360",
		"360cam - 04.09",
		"Nexus 5X"
	};
	sj_get_2byte(buf); // two bytes after exif
	byte* buf1 = *(buf);
	int TIFF_size = length - 6;
	byte* TIFF_data = *buf;

	int endianness = sj_get_2byte(buf);
	if (endianness == 0x4949)
	{
		//std::cout << "endiness of exif is : Intelbased " << std::endl;
		int fixd2bytes = sj_get_2IIbyte(buf);
		int ofst2IFD0 = sj_get_4IIbyte(buf);
		int No_of_fields = sj_get_2IIbyte(buf);
		for (int i = 0; i < No_of_fields; i++)
		{
			decodeIFD_IIfield(buf, header, TIFF_data);
		}
	}
	else if (endianness == 0x4D4D)
	{
		//std::cout << "endianess of exif is : Motorolla \n";
		int fixd2bytes = sj_get_2byte(buf);
		int ofst2IFD0 = sj_get_4byte(buf);
		int No_of_fields = sj_get_2byte(buf);
		for (int i = 0; i < No_of_fields; i++)
		{
			decodeIFD_MMfield(buf, header, TIFF_data);
		}
	}
	else
	{
		printf("Error - endianess of Exif not supported\n");
		return false;
	}
	for (int i = 0; i < No_of_Cameras; i++)
	{
		if (header->camera.model.compare(0, camera_model[i].length(), camera_model[i], 0, camera_model[i].length()) == 0) {
			header->Camera360_flag = true;
			break;
		}
	}
	return true;
}

bool manipulateXMP_gpano(unsigned char* xmp_packet, Header* header)
{
	bool Gpano_is_standard = false;
	std::string xmpPacket((char*)xmp_packet);
	//std::string packetstr;
	int packet_s = xmpPacket.find("<x:xmpmeta");
	int packet_e = xmpPacket.find("</x:xmpmeta");
	if (packet_e > packet_s) {
		int length = packet_e - packet_s + 11;
		char* str = new char[length + 1];
		xmpPacket.copy(str, length, packet_s);
		str[length] = '\n';
		std::string packetstr((char*)str);
	}
	//char * prj_line, usePanoramaViewerline;
	bool GPanoUsed = false;
	int found = xmpPacket.find("xmlns:GPano=\"http://ns.google.com/photos/1.0/panorama/\"");
	if (found != std::string::npos)
	{
		GPanoUsed = true;
	}
	if (GPanoUsed) {
		int ab = xmpPacket.find("<GPano:ProjectionType>");
		int cd = xmpPacket.find("GPano:ProjectionType=\"");
		if (ab != std::string::npos)
		{
			Gpano_is_standard = true;
			int val_s = ab + 22;
			int val_e = xmpPacket.find("</GPano:ProjectionType>");
			uint length = val_e - val_s;
			char* prjType = new char[length + 1];
			xmpPacket.copy(prjType, length, val_s);
			prjType[length] = '\0';
			header->projectionTypeGPano = prjType;
			if (header->projectionTypeGPano == "equirectangular" || header->projectionTypeGPano == "Equirectangular")
				header->ERP_flag = true;
		}
		else if (cd != std::string::npos) {
			int PrjType = xmpPacket.find("GPano:ProjectionType=\"");
			if (PrjType != std::string::npos)
			{
				int val_s = PrjType + 22;
				int val_e = xmpPacket.find("\"", val_s + 1);
				uint len = val_e - val_s;
				char* prj_Type = new char[len + 1];
				xmpPacket.copy(prj_Type, len, val_s);
				prj_Type[len] = '\0';
				header->projectionTypeGPano = prj_Type;
				if (header->projectionTypeGPano == "equirectangular" || header->projectionTypeGPano == "Equirectangular")
					header->ERP_flag = true;
				//std::cout << "value_s : " << val_s << "   value end : " << val_e << std::endl;
			}
		}
		else
		{
			//printf("\nXMP Packet format not supported");
			return false;
		}
		std::string tru("true");
		std::string tru1("True");
		std::string fal("false");
		std::string fal1("False");
		int ef = xmpPacket.find("<GPano:UsePanoramaViewer>");
		int gh = xmpPacket.find("GPano:UsePanoramaViewer=\"");
		if (ab != std::string::npos)
		{
			//	Gpano_is_standard = true;
			int val_s = ab + 25;
			int val_e = xmpPacket.find("</GPano:UsePanoramaViewer>");
			int lengthv = val_e - val_s;
			char* P_view = new char[lengthv + 1];
			xmpPacket.copy(P_view, lengthv, val_s);
			P_view[lengthv] = '\0';
			if (P_view == tru || P_view == tru1)
				header->UsePanoramaViewerGPano = true;
			else if (P_view == fal || P_view == fal1)
				header->UsePanoramaViewerGPano = false;
		}
		else if (cd != std::string::npos) {

			int PN_view = int(xmpPacket.find("GPano:UsePanoramaViewer=\""));
			if (PN_view != std::string::npos)
			{
				int val_s = PN_view + 25;
				int val_e = xmpPacket.find("\"", val_s + 1);
				int len = val_e - val_s;
				char* PN_view_v = new char[len + 1];
				xmpPacket.copy(PN_view_v, len, val_s);
				PN_view_v[len] = '\0';
				if (PN_view_v == tru || PN_view_v == tru1)
					header->UsePanoramaViewerGPano = true;
				else if (PN_view_v == fal || PN_view_v == fal1)
					header->UsePanoramaViewerGPano = false;
			}
		}
		else
		{
			//printf("\nXMP Packet format not supported");
			return false;
		}
	}
	else
	{
		//printf("\nGoogle Panaroma metadata not found\n");
		return false;
	}
	return 1;
}


//decodes app1 Marker for GPano and Exif(Camera) metadata. 
bool readAPP1(byte* app1ptr, Header* header)  // App1ptr is pointer to start of APP1 data
{
	int marker = sj_get_2byte(&app1ptr);
	if (marker != 0xFFE1) {
		cout << "Marker Error" << endl;
		return false;
	}
	bool xmp_flag = false;
	const char* xmp_sign = "http://ns.adobe.com/xap/1.0/";
	unsigned char* xmp_packet;
	int length = sj_get_2byte(&app1ptr);
	int signature = sj_get_4byte(&app1ptr);
	if (signature == APP1_EXIF)
	{
		readAPP1Exif(&app1ptr, header, length);
	}
	else if (signature == APP1_XMP)
	{
		for (int i = 4; i < 28; i++)
		{
			byte temp = sj_get_byte(&app1ptr);
			if (temp != xmp_sign[i])
			{
				xmp_flag = false;
				break;
			}
			else {
				xmp_flag = true;
			}
			length--;
		}
		if (xmp_flag)
		{
			byte nullc = sj_get_byte(&app1ptr);
			xmp_packet = app1ptr;
			manipulateXMP_gpano(xmp_packet, header);
		}
	}
	else {
		printf("Error - APP1 format not supported\n");
		return false;
	}
	return true;
}

void manipulateImageMetadataLine1(string this_line, SJ_360ImageMeta_Ptr IMG_data)
{

	int length = this_line.length();
	int val_S, val_E;
	static string PreviosValue1;
	//char *currentValue;

	val_S = this_line.find(">") + 1;
	val_E = this_line.rfind("<");
	int val_siz = val_E - val_S;
	if (val_siz > 0) {
		//cout << this_line << endl;
		//cout << "Value Starts at : " << val_S << "    Ends at : " << val_E << endl;
		char* val = new char[val_siz + 1];
		this_line.copy(val, val_siz, val_S);
		val[val_siz] = '\0';
		string value((char*)val);
		if (value == string("JPEG360Version"))
		{
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			IMG_data->JPEG360Version = atoi(ab);
		}
		else if (value == string("MediaType"))
		{
			int length = PreviosValue1.length();
			char* result = new char[length];
			PreviosValue1.copy(result, length, 0);
			result[length] = '\0';
			IMG_data->MediaType = result;
		}
		else if (value == string("ProjectionType")) {
			int length = PreviosValue1.length();
			char* result = new char[length];
			PreviosValue1.copy(result, length, 0);
			result[length] = '\0';
			IMG_data->ProjectionType = result;
		}
		else if (value == string("PhiMin")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			IMG_data->PhiMin = atoi(ab);
		}
		else if (value == string("PhiMax")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			IMG_data->PhiMax = atoi(ab);
		}
		else if (value == string("ThetaMin")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			IMG_data->ThetaMin = atoi(ab);
		}
		else if (value == string("ThetaMax")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			IMG_data->ThetaMax = atoi(ab);
		}
		else if (value == string("PhiGravity")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			IMG_data->PhiGravity = atoi(ab);
		}
		else if (value == string("ThetaGravity")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			IMG_data->ThetaGravity = atoi(ab);
		}
		else if (value == string("CompassPhi")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			IMG_data->CompassPhi = atoi(ab);
		}
		else if (value == string("BoxReference")) {
			int length = PreviosValue1.length();
			char* result = new char[length];
			PreviosValue1.copy(result, length, 0);
			result[length] = '\0';
			IMG_data->BoxReference = result;
		}
		else {
			PreviosValue1 = value;
		}
	}
}

void manipulateVPLine1(string this_line, SJ_360ViewMeta_ELE_Ptr VP_data)
{

	int length = this_line.length();
	int val_S, val_E;
	static string PreviosValue1;
	char* currentValue;

	val_S = this_line.find(">") + 1;
	val_E = this_line.rfind("<");
	int val_siz = val_E - val_S;
	if (val_siz > 0) {
		char* val = new char[val_siz + 1];
		this_line.copy(val, val_siz, val_S);
		val[val_siz] = '\0';
		string value((char*)val);
		if (value == string("JPEG360ViewportNumber"))
		{
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			VP_data->JPEG360ViewportNumber = atoi(ab);
		}
		else if (value == string("ViewportPhi")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			VP_data->ViewportPhi = atoi(ab);
		}
		else if (value == string("ViewportTheta")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			VP_data->ViewportTheta = atoi(ab);
		}
		else if (value == string("ViewportPhiFOV")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			VP_data->ViewportPhiFOV = atoi(ab);
		}
		else if (value == string("ViewportThetaFOV")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			VP_data->ViewportThetaFOV = atoi(ab);
		}
		else if (value == string("ViewportRoll")) {
			char* ab = new char[PreviosValue1.length()];
			PreviosValue1.copy(ab, PreviosValue1.length(), 0);
			VP_data->ViewportRoll = atoi(ab);
		}
		else {
			PreviosValue1 = value;
		}
	}
}

//this function extracts JPEG360 Metadata from xml XMPpacket.
void decodeJPEG360XML(string xmlpacket, Header* header) {


	int Total_VP = 0;
	if (header->xmlbox.used && (header->xmlbox.boxType == 0x786D6C20))
	{
		bool Img_Metadata_Present = false;
		bool VP_Metadata_Present = false;
		string VP[MAX_VP];
		///******Extracting JPEG360 Metadata Portion*******//////
		int MetadataStartLOC = xmlpacket.find("<umf:metadata>");
		int MetadataEndLOC = xmlpacket.find("</umf:metadata>") + 16;
		int MetadataLen = MetadataEndLOC - MetadataStartLOC;
		char* MetadataXML = new char[MetadataLen + 1];
		xmlpacket.copy(MetadataXML, MetadataLen, MetadataStartLOC);
		MetadataXML[MetadataLen] = '\0';
		string imgMetadata((char*)MetadataXML);
		int yes = imgMetadata.find("<umf:name>JPEG360ImageMetadata</umf:name>");
		int yesVP = imgMetadata.find("<umf:name>JPEG360ViewportMetadata</umf:name>");

		if (yes <= 0) {
			Img_Metadata_Present = false;
		}
		else {
			Img_Metadata_Present = true;
		}
		if (yesVP <= 0)
		{
			VP_Metadata_Present = false;
		}
		else {
			VP_Metadata_Present = true;
		}

		////******Extracting Image Metadata and Viewport Metadata portions from JPEG360 Metadata Portion
		int IMG_MData_Start, IMG_MData_end, VP_MData_start, VP_MData_end, IMG_MData_Len, VP_MData_Len;
		IMG_MData_Start = imgMetadata.find("<umf:fields>");
		IMG_MData_end = imgMetadata.find("</umf:fields>");
		IMG_MData_Len = IMG_MData_end - IMG_MData_Start + 14;
		if (IMG_MData_Len > 14)
		{
			char* Img_MD_XML = new char[IMG_MData_Len + 1];
			int previosEnd = 0;
			if (Img_Metadata_Present) {

				imgMetadata.copy(Img_MD_XML, IMG_MData_Len, IMG_MData_Start);
				Img_MD_XML[IMG_MData_Len] = '\0';
				previosEnd = IMG_MData_end + 20; // 20 is rough value
			}
			else {
				cout << "\nImage Metadata Not found\n";
			}

			if (VP_Metadata_Present) {
				bool stop = false;
				for (int i = 0; !stop; i++)
				{

					VP_MData_start = imgMetadata.find("<umf:fields>", previosEnd + 10);
					if (VP_MData_start == string::npos || VP_MData_start < 0)
					{
						stop = true;
						break;
					}
					else
					{
						VP_MData_end = imgMetadata.find("</umf:fields>", VP_MData_start);
						VP_MData_Len = VP_MData_end - VP_MData_start + 14;
						char* VP_MD_XML = new char[VP_MData_Len + 1];
						imgMetadata.copy(VP_MD_XML, VP_MData_Len, VP_MData_start);
						VP_MD_XML[VP_MData_Len] = '\0';
						VP[i] = VP_MD_XML;
						Total_VP++;
					}
					previosEnd = VP_MData_end;
				}

			}

			///////////////////extraction of portions completed...//////////////////////

			string IMGString((char*)Img_MD_XML);
			//string VPString((char*)VP_MD_XML);

			size_t start = 0;
			size_t end;
			if (Img_Metadata_Present)
			{
				while (1)
				{
					string this_line;
					if ((end = IMGString.find("\n", start)) == string::npos)
					{
						if (!(this_line = IMGString.substr(start)).empty())
						{
							//	printf("%s\n", this_line.c_str());
						}
						break;
					}
					this_line = IMGString.substr(start, end - start);
					start = end + 1;
					manipulateImageMetadataLine1(this_line, &header->IMG_metadata);
				}

			}
			int i = 0;
			while (!(VP[i].empty()))
			{
				start = 0;
				while (1)
				{
					string this_line;
					if ((end = VP[i].find("\n", start)) == string::npos)
					{
						if (!(this_line = VP[i].substr(start)).empty())
						{
							//printf("%s\n", this_line.c_str());
						}
						break;
					}

					this_line = VP[i].substr(start, end - start);
					start = end + 1;
					manipulateVPLine1(this_line, &header->VP_metadata[i]);
				}
				//Print_VP_MetaData(&VP_metadata[i]);
				i++;
			}
			if (VP[0].empty())
				cout << "\nNo veiwport Metadata found\n";
		}
		else {
			std::cout << "\nJPEG360 Metadata fields not found : " << endl;
		}
	}

	else {
		cout << "Error - XML content box not found\n";
	}
}

//this function decode app11 marker for JPEG360 Metadata JUMBF box
bool readAPP11(byte* app11ptr, Header* header)
{
	int marker = sj_get_2byte(&app11ptr);
	if (marker != 0xFFEB) {
		cout << "Marker Error" << endl;
		return false;
	}
	unsigned char type_jpeg360[16] = { 0x78, 0x5F, 0x34, 0xB7, 0x5D, 0x4B, 0x47, 0x4C, 0xB8, 0x9F, 0x1D, 0x99, 0xE0, 0xE3, 0xA8, 0xDD };
	byte* buf = app11ptr;
	header->app11.APP11Flag = true;
	header->app11.Le = sj_get_2byte(&buf);
	header->app11.CI_dec = sj_get_2byte(&buf);
	header->app11.En = sj_get_2byte(&buf);
	header->app11.Z = sj_get_4byte(&buf);
	header->app11.LBox = sj_get_4byte(&buf);
	header->app11.Tbox = sj_get_4byte(&buf);
	if (header->app11.LBox == 1)
	{
		header->app11.XLBox_present = true;
		header->app11.XLBox = (sj_get_4byte(&buf) << 32) + sj_get_4byte(&buf);
	}
	else {
		header->app11.XLBox_present = false;
	}

	if (header->app11.Tbox == BOX_TYPE_JUMB)
	{
		header->superbox.used = true;
		header->superbox.boxType = 0x6A756D62;
		header->superbox.length = header->app11.LBox;
	}
	else {
		//cout << "Error- Super Box Type : " << header->app11.Tbox << " not supported  " << endl;
		header->isJPEG360 = false;
		return false;
	}
	if (header->superbox.boxType == 0x6A756D62) {
		header->jumdbox.used = true;
		header->jumdbox.jumd_lenght = sj_get_4byte(&buf);
		header->jumdbox.type = sj_get_4byte(&buf);
		for (int i = 0; i < 16; i++) {
			header->jumdbox.UUID[i] = sj_get_byte(&buf);
		}
		//jumdbox->UUID = sj_get_16byte(&buf);
		header->jumdbox.toggle = sj_get_byte(&buf);
		header->jumdbox.requestable = isKthBitSet_1(header->jumdbox.toggle, 1) && isKthBitSet_1(header->jumdbox.toggle, 2);
		header->jumdbox.label_present = isKthBitSet_1(header->jumdbox.toggle, 2); //&& (isKthBitSet(jumdbox->toggle, 1));
		header->jumdbox.ID_present = isKthBitSet_1(header->jumdbox.toggle, 3);
		header->jumdbox.sign_present = isKthBitSet_1(header->jumdbox.toggle, 4);
		if (header->jumdbox.label_present)
		{
			for (int i = 0; i < header->jumdbox.jumd_lenght - 25; i++)// 25 = length ofbox(4) + type(4) + UUID(16)+ toggle(1)
			{
				header->jumdbox.lable[i] = sj_get_byte(&buf);
			}
		}
		if (header->jumdbox.ID_present)
		{
			header->jumdbox.ID = sj_get_4byte(&buf);
		}
		if (header->jumdbox.sign_present)
		{
			for (int i = 0; i < 32; i++)
			{
				header->jumdbox.signature[i] = sj_get_byte(&buf);
			}
		}

	}
	else {
		//cout << "Error - box Type other than JUMBF not supported \n";
		header->isJPEG360 = false;
		return 0;
	}
	bool xmlcontentTypeFlag = false;

	for (int i = 0; i < 16; i++) {
		if (type_jpeg360[i] == header->jumdbox.UUID[i]) {
			xmlcontentTypeFlag = true;
		}
		else {
			xmlcontentTypeFlag = false;
			break;
		}
	}
	if (xmlcontentTypeFlag)
	{
		header->isJPEG360 = true;
		header->xmlbox.used = true;
		header->xmlbox.xmlLength = sj_get_4byte(&buf);
		header->xmlbox.boxType = sj_get_4byte(&buf);
		int xml_L = header->xmlbox.xmlLength - 8;
		if (header->xmlbox.boxType == 0x786D6C20) {
			header->xmlbox.data = new unsigned char[xml_L + 1];
			for (int i = 0; i < xml_L; i++)
			{
				byte temp = sj_get_byte(&buf);
				if (temp != 0x00)
					header->xmlbox.data[i] = temp;
				else {
					header->xmlbox.data[i] = ' ';
					//cout << "The Null character present at i = " << i << " in thexml packet is changed to space character \n" << endl;
				}
			}
			header->xmlbox.data[xml_L] = '\0';
			decodeJPEG360XML((char*)header->xmlbox.data, header);
		}
		else {
			//cout << "Error - Type of XML box is different than \"0x786D6C20\"" <<endl;
			header->isJPEG360 = false;
			return true;
		}

	}
	else {
		//cout << "Error - UUID other than XML Content box UUID is not supported " << endl;
		header->isJPEG360 = false;
		return true;
	}
	return true;
}

Header* read_image_metadata(byte* fdata, uint fsize)//fdata = imagebytestream ...fsize = bytestream size
{
	Header* header = new (std::nothrow) Header;
	if (header == nullptr)
	{
		std::cout << "Error - Memory error\n";
		return nullptr;
	}
	byte last = sj_get_byte(&fdata);
	byte current = sj_get_byte(&fdata);
	if (last != 0xFF || current != M_SOI) {
		header->valid = false;;
		cout << "Image is not JPEG" << endl;
		return nullptr;
	}
	last = sj_get_byte(&fdata);
	current = sj_get_byte(&fdata);

	//reading markers;
	while (header->valid)
	{

		if (last != 0xFF)
		{
			std::cout << "Error - Expected a marker\n";
			header->valid = false;;
			return nullptr;
		}
		if (current == M_DQT)
		{
			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
			//std::cout << "Entered DQT Marker Segment\n";

			//readQuantizationTable(&fdata, header);
		}
		else if (current == M_DHT) {
			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
		}
		else if (current == M_SOS) {

			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
			//std::cout << "Reading Start of Scan Marker :\n";
			//readStartOfScan(&fdata, header);
			break;
		}
		else if (current == M_APP11)
		{
			byte* app11data = fdata - 2;
			readAPP11(app11data, header);
			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
		}
		else if (current == M_APP1)
		{
			byte* app1data = fdata - 2;
			readAPP1(app1data, header);
			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
		}
		else if (current >= M_APP0 && current <= M_APP15)
		{
			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
			//std::cout << "\nReading APPN Marker\n";
			//readAPPn(&fdata, header);
		}
		else if (current == M_SOF0)
		{
			header->frameType = M_SOF0;
			byte* sofData = fdata;
			readStartOfframe(sofData, header);
			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
		}
		else if (current == M_DRI)
		{
			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
			//readRestartInterval(&fdata, header);
		}
		else if (current == M_COM) {

			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
			//readComment(&fdata, header);
		}
		//unused markers that can be skipped
		else if ((current >= M_JPG0 && current <= M_JPG13) || current == M_DNL || current == M_DHP || current == M_EXP) {
			uint length = sj_get_2byte(&fdata);
			fdata += (length - 2);
			//readComment(&fdata, header);
		}
		else if (current == M_TEM) {
			//TEM has no size
		}
		//any number of 0xFF in a row is allowed and should be ignored
		else if (current == 0xFF) {
			current = sj_get_byte(&fdata);
			continue;
		}
		else if (current == M_SOI) {
			std::cout << "Error - Embedded JPGs not supported\n";
			header->valid = false;;
			return nullptr;
		}
		else if (current == M_EOI) {
			std::cout << "Error - EOI detected before SOS\n";
			header->valid = false;;
			return nullptr;
		}
		else if (current == M_DAC) {
			std::cout << "Error - Arithmetic Coding Mode not supported\n";
			header->valid = false;;
			return nullptr;
		}
		else if (current >= M_SOF0 && current <= M_SOF15) {
			std::cout << "Error - SOF marker not supported : 0x" << std::hex << (uint)current << std::dec << '\n';
			header->valid = false;;
			return nullptr;
		}
		else if (current >= M_RST0 && current <= M_RST7) {
			std::cout << "Error - RSTN detected before SOS\n";
			header->valid = false;;
			return nullptr;
		}
		else {
			std::cout << "Error - Unknown marker: 0x" << std::hex << (uint)current << std::dec << '\n';
			header->valid = false;;
			return nullptr;
		}

		last = sj_get_byte(&fdata);
		current = sj_get_byte(&fdata);
	}

	return header;
}

void copy_jpeg_header(byte* image_data, uint img_size, byte** header_buf, uint* header_size) {
	byte* fdata = image_data;

	vector<byte> header;
	header.push_back(0xFF);
	header.push_back(0xD8);


	byte last = sj_get_byte(&fdata);
	byte current = sj_get_byte(&fdata);
	if (last != 0xFF || current != M_SOI) {
		cout << "Image is not JPEG" << endl;
		return;
	}
	last = sj_get_byte(&fdata);
	current = sj_get_byte(&fdata);

	//reading markers;
	while (1)
	{
		if (last != 0xFF)
		{
			std::cout << "Error - Expected a marker\n";
			return;
		}
		if (current == M_DQT || current == M_DHT || current == M_SOS || current == M_SOF0)
		{
			int length = sj_get_2byte(&fdata);
			fdata -= 4;
			for (int i = 0; i < length + 2; ++i)
			{
				header.push_back(sj_get_byte(&fdata));
			}
			if (current == M_SOS)
				break;
		}
		//any number of 0xFF in a row is allowed and should be ignored
		else if (current == 0xFF) {
			current = sj_get_byte(&fdata);
			continue;
		}
		else {
			unsigned long int length = sj_get_2byte(&fdata);
			fdata += (length - 2);
		}

		last = sj_get_byte(&fdata);
		current = sj_get_byte(&fdata);
	}
	*header_size = header.size();
	*header_buf = new byte[*header_size];
	memcpy(*header_buf, header.data(), *header_size);

}
