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

#include "codeclib.h"

int iTotalBytes1=0;
int iTotalBytes3=0;

void usage() {
    cout << "Video decoder with motion est, Golomb-Rice code, and arithmetic code:" << endl;;
    cout << "Usage: viddec infile outfile" << endl;
    cout << "    infile:  Input compressed file." << endl;
    cout << "    outfile: Output yuv file." << endl;
}

int openout(ofstream & opfile, char **arguv, int index){ //Function to open output files
	//open output file
    opfile.open(arguv[index], ios::out|ios::binary);
    if(!opfile)
    {
        cout << "Can't open file " << arguv[index] << endl;
        return -3;
    }
}

void decode_out(ofstream & opfile, ifstream & ipfile, unsigned char *ImgBuf, IDecoder *pDec, unsigned char *pcBSB, float Qstep, int ImageArea, int TotalBytes, int Frames){

    for (int i = 0; i < Frames; i++) {
        cout << ".";

        ipfile.read((char *)&TotalBytes, sizeof(int)); // Read size of input file to TotalBytes

        ipfile.read((char *)pcBSB, TotalBytes); //read input file into Bitstream buffer with size of TotalBytes

        int iUsedBytes = pDec->decodeImage(i == 0, pcBSB, Qstep);

        pDec->GetImage(ImgBuf);
    
        opfile.write((const char *)ImgBuf, ImageArea);
    }
    cout << TotalBytes << endl;

    cout << endl;	
	opfile.close();
}

int main(int argc,char **argv) {
 
    ifstream ifsInfile;
    ofstream ofsOutfile;

    int iWidth, iHeight, iFrames;
    unsigned char *pcImgBuf, *pcBitstreamBuf, *pcBitstreamBuf3;
    float fQstep;

    if(argc < 3) {
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

	openout(ofsOutfile, argv, 2);

    //read width, height, total bytes
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

	decode_out(ofsOutfile, ifsInfile, pcImgBuf, pDecoder, pcBitstreamBuf, fQstep, iImageArea, iTotalBytes1, iFrames); //decode first view

	pcBitstreamBuf3 = new unsigned char[iImageArea];
    if (!pcBitstreamBuf3) {
        cout << "Fail to create output buffer." << endl;
        return -7;
    }

	IDecoder *pDecoder3 = new IDecoder(iWidth, iHeight);

	ofstream ofsOutfile3;

	openout(ofsOutfile3, argv, 3);

	decode_out(ofsOutfile3, ifsInfile, pcImgBuf, pDecoder, pcBitstreamBuf, fQstep, iImageArea, iTotalBytes3, iFrames);	//decode 2nd view   

    ifsInfile.close();

    delete pDecoder;
	delete pDecoder3;
    delete pcImgBuf;
    delete pcBitstreamBuf;
	delete pcBitstreamBuf3;

    return 0;
}