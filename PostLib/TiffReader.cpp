/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "TiffReader.h"
#include <ImageLib/3DImage.h>
#include <stdexcept>

#ifndef  WORD
#define WORD	uint16_t
#define DWORD	uint32_t
#endif // ! WORD

enum TifCompression {
	TIF_COMPRESSION_NONE = 1,
	TIF_COMPRESSION_CCITTRLE = 2,
	TIF_COMPRESSION_CCITTFAX3 = 3, // = COMPRESSION_CCITT_T4
	TIF_COMPRESSION_CCITTFAX4 = 4, // = COMPRESSION_CCITT_T6
	TIF_COMPRESSION_LZW = 5,
	TIF_COMPRESSION_OJPEG = 6,
	TIF_COMPRESSION_JPEG = 7,
	TIF_COMPRESSION_NEXT = 32766,
	TIF_COMPRESSION_CCITTRLEW = 32771,
	TIF_COMPRESSION_PACKBITS = 32773,
	TIF_COMPRESSION_THUNDERSCAN = 32809,
	TIF_COMPRESSION_IT8CTPAD = 32895,
	TIF_COMPRESSION_IT8LW = 32896,
	TIF_COMPRESSION_IT8MP = 32897,
	TIF_COMPRESSION_IT8BL = 32898,
	TIF_COMPRESSION_PIXARFILM = 32908,
	TIF_COMPRESSION_PIXARLOG = 32909,
	TIF_COMPRESSION_DEFLATE = 32946,
	TIF_COMPRESSION_ADOBE_DEFLATE = 8,
	TIF_COMPRESSION_DCS = 32947,
	TIF_COMPRESSION_JBIG = 34661,
	TIF_COMPRESSION_SGILOG = 34676,
	TIF_COMPRESSION_SGILOG24 = 34677,
	TIF_COMPRESSION_JP2000 = 34712
};

typedef struct _TiffHeader
{
	WORD  Identifier;  /* Byte-order Identifier */
	WORD  Version;     /* TIFF version number (always 2Ah) */
	DWORD IFDOffset;   /* Offset of the first Image File Directory*/
} TIFHEAD;

typedef struct _TifTag
{
	WORD   TagId;       /* The tag identifier  */
	WORD   DataType;    /* The scalar type of the data items  */
	DWORD  DataCount;   /* The number of items in the tag data  */
	DWORD  DataOffset;  /* The byte offset to the data items  */
} TIFTAG;

typedef struct _TifIfd
{
	WORD    NumDirEntries;    /* Number of Tags in IFD  */
	TIFTAG* TagList;			/* Array of Tags  */
	DWORD   NextIFDOffset;    /* Offset to next IFD  */
} TIFIFD;

typedef struct _TifStrip
{
	DWORD	offset;
	DWORD	byteCount;
} TIFSTRIP;

size_t lzw_decompress(Byte* dst, Byte* src, size_t max_dst_size);

CTiffImageSource::CTiffImageSource(Post::CImageModel* imgModel, const std::string& filename) : CImageSource(RAW, imgModel)
{
	m_filename = filename;
}

CTiffImageSource::CTiffImageSource(Post::CImageModel* imgModel) : CImageSource(RAW, imgModel)
{
	m_fp = nullptr;
}

CTiffImageSource::~CTiffImageSource()
{
	if (m_fp) fclose(m_fp);
	if (m_pd.empty() == false)
	{
		for (int i = 0; i < m_pd.size(); ++i) delete[] m_pd[i];
		m_pd.clear();
	}
}

void byteswap(WORD& v)
{
	unsigned char* b = (unsigned char*)(&v);
	b[0] ^= b[1];
	b[1] ^= b[0];
	b[0] ^= b[1];
}

void byteswap(DWORD& v)
{
	unsigned char* b = (unsigned char*)(&v);
	b[0] ^= b[3]; b[3] ^= b[0]; b[0] ^= b[3];
	b[1] ^= b[2]; b[2] ^= b[1]; b[1] ^= b[2];
}

bool CTiffImageSource::Load()
{
	if (m_filename.empty()) return false;
	const char* szfile = m_filename.c_str();
	m_fp = fopen(szfile, "rb");
	if (m_fp == 0) return false;

	// read the header
	TIFHEAD hdr;
	fread(&hdr, sizeof(TIFHEAD), 1, m_fp);

	// see if this is a tiff (and determine endianess)
	m_bigE = false;
	if (hdr.Identifier == 0x4D4D) m_bigE = true;
	else if (hdr.Identifier == 0x4949) m_bigE = false;

	if (m_bigE) byteswap(hdr.Version);
	if (hdr.Version != 0x2A) return false;

	// jump to the first IFD
	if (m_bigE) byteswap(hdr.IFDOffset);
	fseek(m_fp, hdr.IFDOffset, SEEK_SET);

	// read the image
	try {
		bool bdone = false;
		while (bdone == false)
		{
			bdone = (readImage() != true);
		}
	}
	catch (std::exception e)
	{
		return false;
	}
	catch (...)
	{
		return false;
	}
	fclose(m_fp);

	// see if we read any image data
	if (m_pd.size() == 0) return false;
	int nz = m_pd.size();

	// build the 3D image
	C3DImage* im = new C3DImage;
	im->Create(m_cx, m_cy, nz);
	Byte* buf = im->GetBytes();
	DWORD imSize = m_cx * m_cy;
	for (int i = 0; i < nz; ++i)
	{
		if (m_bits == 8) memcpy(buf, m_pd[i], imSize);
		else
		{
			Byte* b = m_pd[i];
			for (int j = 0; j < imSize; ++j, b += 2) buf[j] = b[0];
		}
		buf += imSize;
	}

	// clean up
	for (int i = 0; i < m_pd.size(); ++i) delete[] m_pd[i];
	m_pd.clear();

	BOX box(0, 0, 0, m_cx, m_cy, nz);
	im->SetBoundingBox(box);
	AssignImage(im);

	return true;
}

bool CTiffImageSource::readImage()
{
	// read the IFD
	TIFIFD ifd;
	fread(&ifd.NumDirEntries, sizeof(WORD), 1, m_fp);
	if (m_bigE) byteswap(ifd.NumDirEntries);
	if ((ifd.NumDirEntries <= 0) || (ifd.NumDirEntries >= 65536)) return false;

	// read the tags
	ifd.TagList = new _TifTag[ifd.NumDirEntries];
	int nread = fread(ifd.TagList, sizeof(_TifTag), ifd.NumDirEntries, m_fp);
	if (nread != ifd.NumDirEntries) { delete[] ifd.TagList; return false; }
	if (m_bigE)
	{
		for (int i = 0; i < ifd.NumDirEntries; ++i)
		{
			TIFTAG& t = ifd.TagList[i];
			byteswap(t.TagId);
			byteswap(t.DataType);
			byteswap(t.DataCount);

			if (t.DataType == 3)
			{
				WORD tmp = t.DataOffset;
				byteswap(tmp);
				t.DataOffset = tmp;
			}
			else if (t.DataType == 1) byteswap(t.DataOffset);
			else if (t.DataType == 2) byteswap(t.DataOffset);
			else if (t.DataType == 4) byteswap(t.DataOffset);
		}
	}

	// read the next IDF offset
	DWORD nextIFD = 0;
	fread(&nextIFD, sizeof(DWORD), 1, m_fp);
	if (m_bigE) byteswap(nextIFD);

	// process tags
	DWORD imWidth = 0, imLength = 0;
	DWORD rowsPerStrip = 0, stripOffsets = 0, stripByteCounts, bitsPerSample = 0, compression = 0;
	int numberOfStrips = 1;
	for (int i = 0; i < ifd.NumDirEntries; ++i)
	{
		TIFTAG& t = ifd.TagList[i];
		switch (t.TagId)
		{
		case 256: imWidth = t.DataOffset; break;
		case 257: imLength = t.DataOffset; break;
		case 258: bitsPerSample = t.DataOffset; break;
		case 259: compression = t.DataOffset; break;
		case 278: rowsPerStrip = t.DataOffset; break;
		case 273: { stripOffsets = t.DataOffset; numberOfStrips = t.DataCount; break; }
		case 279: stripByteCounts = t.DataOffset; break;
		}
	}
	m_cx = imWidth;
	m_cy = imLength;
	m_bits = bitsPerSample;

	if ((bitsPerSample != 8) && (bitsPerSample != 16))
	{
		delete[] ifd.TagList;
		throw std::domain_error("Only 8 and 16 bit tif supported.");
	}

	if ((compression != TIF_COMPRESSION_NONE) && (compression != TIF_COMPRESSION_LZW))
	{
		delete[] ifd.TagList;
		throw std::domain_error("Only uncompressed and LZW compressed tiff are supported.");
	}

	// find the strips
	if (numberOfStrips == 0) throw std::invalid_argument("no strips");
	std::vector<TIFSTRIP> strips(numberOfStrips);
	if (numberOfStrips == 1)
	{
		strips[0].offset = stripOffsets;
		strips[0].byteCount = stripByteCounts;
	}
	else
	{
		fseek(m_fp, stripOffsets, SEEK_SET);
		DWORD* tmp = new DWORD[numberOfStrips];
		fread(tmp, sizeof(DWORD), numberOfStrips, m_fp);
		for (int i = 0; i < numberOfStrips; ++i)
		{
			if (m_bigE) byteswap(tmp[i]);
			strips[i].offset = tmp[i];
		}

		fseek(m_fp, stripByteCounts, SEEK_SET);
		fread(tmp, sizeof(DWORD), numberOfStrips, m_fp);
		for (int i = 0; i < numberOfStrips; ++i)
		{
			if (m_bigE) byteswap(tmp[i]);
			strips[i].byteCount = tmp[i];
		}
		delete[] tmp;
	}

	// allocate buffer for image
	DWORD imSize = imWidth * imLength * (bitsPerSample == 16 ? 2 : 1);
	Byte* buf = new Byte[imSize];
	Byte* tmp = buf;
	m_pd.push_back(buf);

	// This assumes only one strip per image!!
	DWORD bytesRead = 0;
	int nstrip = 0;
	while (bytesRead < imSize)
	{
		TIFSTRIP& strip = strips[nstrip++];
		fseek(m_fp, strip.offset, SEEK_SET);

		if (compression == TIF_COMPRESSION_NONE)
		{
			fread(tmp, strip.byteCount, 1, m_fp);
			bytesRead += strip.byteCount;
/*
			FILE* fp = fopen("C:\\Users\\Steve\\Documents\\Tiff\\plane1.txt", "wt");
			Byte* d = tmp;
			for (int j = 0; j < 724; ++j)
			{
				for (int i = 0; i < 724 * 2; ++i)
				{
					int c = *d++;
					fprintf(fp, "%d ", c);
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
*/
			tmp += stripByteCounts;
		}
		else if (compression == TIF_COMPRESSION_LZW)
		{
			// read the compressed stream
			Byte* stream = new Byte[strip.byteCount];
			fread(stream, strip.byteCount, 1, m_fp);

			// decompress the zream
			int decompressedSize = lzw_decompress(tmp, stream, imSize);
			bytesRead += decompressedSize;
			tmp += decompressedSize;
		}
	}

	// cleanup
	delete[] ifd.TagList;

	// jump to the next IFD
	if (nextIFD != 0) fseek(m_fp, nextIFD, SEEK_SET);
	else return false;

	return true;
}

void CTiffImageSource::Save(OArchive& ar)
{

}

void CTiffImageSource::Load(IArchive& ar)
{

}

class LZWDecompress
{
	enum {
		CLEAR_CODE = 256,
		EOI_CODE = 257
	};

public:
	LZWDecompress(Byte* src) : m_src(src) 
	{ 
		m_s = m_src; m_startBit = 0; m_bps = 9; 
		buildMask(m_bps);
	}

	void writeString(const std::vector<Byte>& entry)
	{
		for (const Byte& b : entry) { (*m_d++) = b; m_dsize++; }
		int a = 0;
	}

	size_t decompress(Byte* dst, size_t max_buf_size)
	{
		m_d = dst;
		m_dsize = 0;
		DWORD code = 0, oldcode = 0;
		while ((code = nextCode()) != EOI_CODE)
		{
			if (code == CLEAR_CODE)
			{
				initDictionary();
				code = nextCode();
				if (code == EOI_CODE) break;
				writeString(m_dic[code]);
				oldcode = code;
			}
			else
			{
				if (code < m_dic.size())
				{
					writeString(m_dic[code]);
					std::vector<Byte> sc = m_dic[oldcode];
					sc.push_back(m_dic[code][0]);
					addToDictionary(sc);
					oldcode = code;
				}
				else
				{
					std::vector<Byte> OutString = m_dic[oldcode];
					OutString.push_back(m_dic[oldcode][0]);
					writeString(OutString);
					addToDictionary(OutString);
					oldcode = code;
				}
			}
			assert(m_dsize <= max_buf_size);
		}
		return m_dsize;
	}

	void addToDictionary(const std::vector<Byte>& s)
	{
		m_dic.push_back(s);
		if (m_dic.size() == 511)
		{
			m_bps = 10;
			buildMask(m_bps);
		}
		else if (m_dic.size() == 1023)
		{
			m_bps = 11;
			buildMask(m_bps);
		}
		else if (m_dic.size() == 2047)
		{
			m_bps = 12;
			buildMask(m_bps);
		}
		assert(m_dic.size() < 4096);
	}

	void buildMask(int n)
	{
		DWORD m = 0;
		for (int i = 0; i < n; ++i) m |= (1 << (15-i));
		m_mask = m;
	}

	DWORD nextCode()
	{
		DWORD code = 0;
		int bitsread = 0;
		WORD buf = m_s[0];
		while (bitsread < m_bps)
		{
			code |= ((buf >> (7 - m_startBit)) & 0x01) << (m_bps - bitsread - 1);
			bitsread++;
			m_startBit++;
			if (m_startBit == 8)
			{
				m_s++;
				buf = m_s[0];
				m_startBit = 0;
			}
		}
		return code;
	}

	void initDictionary()
	{
		m_dic.clear();
		for (Byte i = 0; i < 255; ++i) m_dic.push_back({ i });
		m_dic.push_back({ 255 });
		m_dic.push_back({ 0 });
		m_dic.push_back({ 0 });
		m_bps = 9;
	}

	const std::vector<Byte>& operator [] (size_t n) { return m_dic[n]; }

private:
	Byte* m_src;
	Byte* m_s;
	Byte* m_d;
	size_t	m_dsize;
	int	  m_startBit;
	int	  m_bps;
	DWORD	m_mask;

	std::vector< std::vector<Byte> >	m_dic;
};

// this function decompresses a LZW compressed strip
// the src is the compressed data, and dst is used to store the decoded strip
size_t lzw_decompress(Byte* dst, Byte* src, size_t max_dst_size)
{
	LZWDecompress lzw(src);
	size_t n = lzw.decompress(dst, max_dst_size);

/*	FILE* fp = fopen("C:\\Users\\Steve\\Documents\\Tiff\\plane1_decompressed.txt", "wt");
	Byte* d = dst;
	for (int j = 0; j < 724; ++j)
	{
		for (int i = 0; i < 724 * 2; ++i)
		{
			int c = *d++;
			fprintf(fp, "%d ", c);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
*/
	return n;
}
