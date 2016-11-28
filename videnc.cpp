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
 * Modified AGAIN by Francis Tran, Peter Le
 * School of Engineering Science
 * Simon Fraser University
 *
 *Nov. 2016
 *********************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip> 
using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "codeclib.h"

	int iOutBytes;
    int iGRPara = 0, iAllBytes = 0;

void usage() {
    cout << "A simple video codec:" << endl;;
    cout << "Usage: videnc infile outfile width height frames qstep" << endl;
    cout << "    infile: Input video sequence." << endl;
    cout << "    outfile:    Output compressed file." << endl;
    cout << "    width: Frame width" << endl;
    cout << "    height: Frame height" << endl;
    cout << "    frames: Number of frames to be encoded (from the first frame)" << endl;
    cout << "    qstep:      Quantization step size (can be floating-point)" << endl;
}


int openin(ifstream & ipfile, char **arguv, int index){ //Function to open input files
ipfile.open(arguv[index], ios::in|ios::binary);
    if(!ipfile)
    {
        cout << "Can't open file " << arguv[index] << endl;
        return -2;
    }
}
 
int openout(ofstream & opfile, char **arguv, int index){ //Function to open output files
    opfile.open(arguv[index], ios::out|ios::binary);
    if(!opfile)
    {
        cout << "Can't open file " << arguv[index] << endl;
        return -3;
    }
}

void encode_out(ofstream & opfile, ifstream & ipfile, unsigned char *ImgBuf, IEncoder *pEnc, unsigned char *pcBSB, float Qstep, int frmsz, int Width, int Height, int Frames, ofstream & ofsD){
   
	//write file header
    opfile.write((const char *) &Width, sizeof(int));
    opfile.write((const char *) &Height, sizeof(int));
    opfile.write((const char *) &Frames, sizeof(int));
    opfile.write((const char *) &Qstep, sizeof(float));

    for (int i = 0; i < Frames; i++) {

        //read one frame
		ipfile.read((char *)ImgBuf, frmsz);
        pEnc->SetImage(ImgBuf);

        //main routine to encode the frame
        iOutBytes = pEnc->codeImage(i == 0, pcBSB, Qstep);
        iAllBytes += iOutBytes;

        cout << iOutBytes << endl;
 
        //write one frame data to output file
        opfile.write((const char *) &iOutBytes, sizeof(int));
        opfile.write((const char *) pcBSB, iOutBytes);

		#ifdef DUMPMV
        if (i > 0) {
            pEnc->DumpMV(ofsD);
        }
	#endif
	}
    cout << "Bits/pixel: " << std::setprecision(5) << iAllBytes * 8.0f / (Width * Height * Frames) << endl;

    ipfile.close(); //close input

}

//--------------------------------------------------------
//
// main routine
//
//--------------------------------------------------------
int main(int argc,char **argv) {

    ifstream ifsInfile;
    ofstream ofsOutfile;

    unsigned char *pcImgBuf, *pcImgBuf3, *pcBitstreamBuf, *pcBitstreamBuf3;
    float fQstep;

	int iWidth, iHeight, iFrames;

	iWidth = atoi(argv[5]); //1024 for YUV
    iHeight = atoi(argv[6]); //768 for YUV
    iFrames = atoi(argv[7]); 

	IEncoder *pEncoder = new IEncoder(iWidth, iHeight);
	IEncoder *pEncoder3 = new IEncoder(iWidth, iHeight);

    if(argc < 9) {
        usage();
        return -1;
    }
	cout << argv[1] << " " << argv[2] << " " << argv[3] << " " <<  argv[4] << " " << endl;
     
	fQstep = (float) atof(argv[8]); //quantization step

    int iFrameSize = iWidth * iHeight * 3 / 2; //for YUV

	//open input and output files for balloons1.yuv
	openin(ifsInfile, argv, 1);
	openout(ofsOutfile, argv, 2);

	pcImgBuf = new unsigned char[iFrameSize];

	if (!pcImgBuf) {
        cout << "Failed to create image buffer." << endl;
        return -5;
    }

	pcBitstreamBuf = new unsigned char[iFrameSize]; //not necessary for one z.out
    if (!pcBitstreamBuf) {
        cout << "Failed to create output buffer." << endl;
        return -6;
    }

#ifdef DUMPMV
    ofstream ofsDump;
    ofsDump.open("mv.dat", ios::out|ios::binary);
#endif

encode_out(ofsOutfile, ifsInfile, pcImgBuf, pEncoder, pcBitstreamBuf, fQstep, iFrameSize, iWidth, iHeight, iFrames, ofsDump);

	ifstream ifsInfile3;
	ofstream ofsOutfile3; //omit for big ass z.out

	//open input file for balloons3.yuv
	openin(ifsInfile3, argv, 3);

	pcImgBuf3 = new unsigned char[iFrameSize];

	if (!pcImgBuf3) {
		cout << "Failed to create image buffer." << endl;
		return -5;
	}

encode_out(ofsOutfile, ifsInfile3, pcImgBuf3, pEncoder3, pcBitstreamBuf, fQstep, iFrameSize, iWidth, iHeight, iFrames, ofsDump);

	ofsOutfile.close(); 
	
#ifdef DUMPMV
    ofsDump.close();
#endif

    delete pEncoder;
	delete pEncoder3;
    delete pcImgBuf;
	delete pcImgBuf3;
    delete pcBitstreamBuf;

    return 0;
}