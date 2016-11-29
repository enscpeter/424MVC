/*********************************************************************************
 *
 * Lossy decoder with 4-point DCT, midtread quantizer, DC prediction, 
 * Golomb-Rice code, and binary arithmetic code.
 *
 * The arithmetic codec was originally written by Dr. Chengjie Tu 
 * at the Johns Hopkins University (now with Microsoft Windows Media Division).
 *
 * Modified by Jie Liang
 * School of Engineering Science
 * Simon Fraser University
 *
 * Feb. 2005
 *
 *********************************************************************************/
#include <iostream>
#include <fstream>
using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "codeclib.h"

void usage() {
    cout << "Video decoder with motion est, Golomb-Rice code, and arithmetic code:" << endl;;
    cout << "Usage: viddec infile outfile" << endl;
    cout << "    infile:  Input compressed file." << endl;
    cout << "    outfile: Output yuv file1." << endl;
	cout << "    outfile: Output yuv file2." << endl;
	cout << "    outfile: Output yuv file3." << endl;
}

int main(int argc,char **argv) {
 
    ifstream ifsInfile;
    ofstream ofsOutfile, ofsOutfile3, ofsOutfile5;
    int nViews = 1, iWidth, iHeight, iTotalBytes, iFrames;
    unsigned char *pcImgBuf, *pcBitstreamBuf;
    float fQstep;

    if(argc < 5) {
        usage();
        return -1;
    }

    //open input file
    ifsInfile.open(argv[1], ios::in|ios::binary);
    if(!ifsInfile)
    {
        cout << "Can't open file " << argv[1] << endl;
        return -2;
    }

    //open output file
    ofsOutfile.open(argv[2], ios::out|ios::binary);
    if(!ofsOutfile)
    {
        cout << "Can't open file " << argv[2] << endl;
        return -3;
    }
	if (std::string(argv[3]) != "0") {
		ofsOutfile3.open(argv[3], ios::out | ios::binary);
		if (!ofsOutfile3)
		{
			cout << "Can't open file " << argv[2] << endl;
			return -3;
		}
		nViews++;
	}
	if (std::string(argv[4]) != "0") {
		ofsOutfile5.open(argv[4], ios::out | ios::binary);
		if (!ofsOutfile5)
		{
			cout << "Can't open file " << argv[2] << endl;
			return -3;
		}
		nViews++;
	}

    //read numViews, width, height, total bytes
	//ifsInfile.read((char *)&nViews, sizeof(int));
	ifsInfile.read((char *)&iWidth, sizeof(int));
	ifsInfile.read((char *)&iHeight, sizeof(int));
	ifsInfile.read((char *)&iFrames, sizeof(int));
	ifsInfile.read((char *)&fQstep, sizeof(float));

    int iImageArea = iWidth * iHeight * 3 / 2;
    pcImgBuf = new unsigned char[iImageArea];
    if (!pcImgBuf) {
        cout << "Fail to create image buffer." << endl;
        return -5;
    }

    pcBitstreamBuf = new unsigned char[iImageArea];
    if (!pcBitstreamBuf) {
        cout << "Fail to create output buffer." << endl;
        return -7;
    }

    IDecoder *pDecoder = new IDecoder(iWidth, iHeight);
	//decoder routine
    for (int i = 0; i < iFrames; i++) {
		cout << "Decoding View 1..." << endl;
        ifsInfile.read((char *)&iTotalBytes, sizeof(int));
        ifsInfile.read((char *)pcBitstreamBuf, iTotalBytes);

        int iUsedBytes = pDecoder->decodeImage(i == 0, pcBitstreamBuf, fQstep);

        pDecoder->GetImage(pcImgBuf);    
        ofsOutfile.write((const char *)pcImgBuf, iImageArea);

		//decode additional views
		if (nViews > 1) {
			cout << "Decoding View 2..." << endl;
			ifsInfile.read((char *)&iTotalBytes, sizeof(int));
			ifsInfile.read((char *)pcBitstreamBuf, iTotalBytes);

			int iUsedBytes = pDecoder->decodeImage(i == 0, pcBitstreamBuf, fQstep);

			pDecoder->GetImage(pcImgBuf);
			ofsOutfile3.write((const char *)pcImgBuf, iImageArea);
			if (nViews > 2) {
				cout << "Decoding View 3..." << endl;
				ifsInfile.read((char *)&iTotalBytes, sizeof(int));
				ifsInfile.read((char *)pcBitstreamBuf, iTotalBytes);

				int iUsedBytes = pDecoder->decodeImage(i == 0, pcBitstreamBuf, fQstep);

				pDecoder->GetImage(pcImgBuf);
				ofsOutfile5.write((const char *)pcImgBuf, iImageArea);
			}
		}
    }
    cout << endl;

    ifsInfile.close();
    ofsOutfile.close();

    delete pDecoder;
    delete pcImgBuf;
    delete pcBitstreamBuf;

    return 0;
}