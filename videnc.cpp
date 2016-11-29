/*********************************************************************************
 *
 * A simple video codec:
 * Motion estimation, DCT, midtread quantizer, 
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
 * To do:
 * 1. Intra MB in P frame: criterion?
 * 2. Better entropy coding: zigzag scanning, early termination.
 * 3. CBP
 *
 *********************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip> 
using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "codeclib.h"

void usage() {
    cout << "A simple video codec:" << endl;;
	cout << "Usage: videnc infile1 infile2 infile3 outfile width height frames qstep" << endl;
    cout << "    infile: Input video sequence1." << endl;
	cout << "    infile: Input video sequence2." << endl;
	cout << "    infile: Input video sequence3." << endl;
    cout << "    outfile:    Output compressed file." << endl;
    cout << "    width: Frame width" << endl;
    cout << "    height: Frame height" << endl;
    cout << "    frames: Number of frames to be encoded (from the first frame)" << endl;
    cout << "    qstep:      Quantization step size (can be floating-point)" << endl;
}

//--------------------------------------------------------
//
// main routine
//
//--------------------------------------------------------
int main(int argc,char **argv) {
 
    ifstream ifsInfile, ifsInfile3, ifsInfile5;
    ofstream ofsOutfile;

    int iWidth, iHeight, iFrames, iOutBytes;
    int iGRPara = 0, iAllBytes = 0;
	int nViews = 1;
    unsigned char *pcImgBuf, *pcImgBuf3, *pcImgBuf5, *pcBitstreamBuf;
    float fQstep;

	iWidth = atoi(argv[5]);
	iHeight = atoi(argv[6]);
	iFrames = atoi(argv[7]);
	fQstep = (float)atof(argv[8]);
    if(argc < 9) {
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
	//create imgBuf
	int iFrameSize = iWidth * iHeight * 3 / 2; //for YUV
	pcImgBuf = new unsigned char[iFrameSize];
	if (!pcImgBuf) {
		cout << "Fail to create image buffer." << endl;
		return -5;
	}

	if (std::string(argv[2]) != "0") {
		ifsInfile3.open(argv[2], ios::in | ios::binary);
		if (!ifsInfile)
		{
			cout << "Can't open file " << argv[1] << endl;
			return -4;
		}
		//create imgBuf3
		pcImgBuf3 = new unsigned char[iFrameSize];
		if (!pcImgBuf3) {
			cout << "Fail to create image buffer3." << endl;
			return -5;
		}

		nViews = 2;
	}
	if (std::string(argv[3]) != "0") {
		ifsInfile5.open(argv[3], ios::in | ios::binary);
		if (!ifsInfile)
		{
			cout << "Can't open file " << argv[1] << endl;
			return -2;
		}
		//create imgBuf3
		pcImgBuf5 = new unsigned char[iFrameSize];
		if (!pcImgBuf5) {
			cout << "Fail to create image buffer5." << endl;
			return -5;
		}
		nViews = 3;
	}


    //open output file
    ofsOutfile.open(argv[4], ios::out|ios::binary);
    if(!ofsOutfile)
    {
        cout << "Can't open file " << argv[2] << endl;
        return -3;
    }

    pcBitstreamBuf = new unsigned char[iFrameSize];
    if (!pcBitstreamBuf) {
        cout << "Fail to create output buffer." << endl;
        return -6;
    }

    IEncoder *pEncoder = new IEncoder(iWidth, iHeight);

#ifdef DUMPMV
    ofstream ofsDump;
    ofsDump.open("mv.dat", ios::out|ios::binary);
#endif

    //write file header
	//ofsOutfile.write((char *)&nViews, sizeof(int));
    ofsOutfile.write((const char *) &iWidth, sizeof(int));
    ofsOutfile.write((const char *) &iHeight, sizeof(int));
    ofsOutfile.write((const char *) &iFrames, sizeof(int));
    ofsOutfile.write((const char *) &fQstep, sizeof(float));

    for (int i = 0; i < iFrames; i++) {
        //read one frame
        ifsInfile.read((char *)pcImgBuf, iFrameSize);
        pEncoder->SetImage(pcImgBuf);

        //main routine to encode the frame
        iOutBytes = pEncoder->codeImage(i == 0, pcBitstreamBuf, fQstep);
        iAllBytes += iOutBytes;

        cout << "Encoding View 1:" << iOutBytes << endl;
 
        //write one frame data to output file
        ofsOutfile.write((const char *) &iOutBytes, sizeof(int));
        ofsOutfile.write((const char *) pcBitstreamBuf, iOutBytes);

		//encode additional views
		if (nViews > 1) {
			ifsInfile3.read((char *)pcImgBuf3, iFrameSize);
			pEncoder->SetImage(pcImgBuf3);
			//main routine to encode the frame
			iOutBytes = pEncoder->codeImage(i == 0, pcBitstreamBuf, fQstep);
			iAllBytes += iOutBytes;

			cout << "Encoding View 2:" << iOutBytes << endl;

			//write one frame data to output file
			ofsOutfile.write((const char *)&iOutBytes, sizeof(int));
			ofsOutfile.write((const char *)pcBitstreamBuf, iOutBytes);

			if (nViews > 2) {
				ifsInfile5.read((char *)pcImgBuf5, iFrameSize);
				pEncoder->SetImage(pcImgBuf5);
				//main routine to encode the frame
				iOutBytes = pEncoder->codeImage(i == 0, pcBitstreamBuf, fQstep);
				iAllBytes += iOutBytes;

				cout << "Encoding View 3:" << iOutBytes << endl;

				//write one frame data to output file
				ofsOutfile.write((const char *)&iOutBytes, sizeof(int));
				ofsOutfile.write((const char *)pcBitstreamBuf, iOutBytes);
			}
		}


#ifdef DUMPMV
        if (i > 0) {
            pEncoder->DumpMV(ofsDump);
        }
#endif

    }

    cout << "Bits/pixel: " << std::setprecision(5) << iAllBytes * 8.0f / (iWidth * iHeight * iFrames) << endl;

    ifsInfile.close();
	ifsInfile3.close();
	ifsInfile5.close();
    ofsOutfile.close();

#ifdef DUMPMV
    ofsDump.close();
#endif

    delete pEncoder;
    delete pcImgBuf, pcImgBuf3, pcImgBuf5;
    delete pcBitstreamBuf;

    return 0;
}