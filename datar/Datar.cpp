//
//  Datar.cpp
//  datarz8
//
//  Created by Vahagn Nurijanyan on 2015-09-19.
//  Copyright (c) 2015 BABELONi INC. All rights reserved.
//
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "Datar.hpp"
#include "Compressor.hpp"
#include "Decompressor.hpp"

Datar::Datar(const char* src, const char* tp, const char* bs) {
    const char types[5][3] = {"z"};
    const char exts[5][5] = {".dtz"};
    if(strstr(tp, types[0])) {
        type = 0;
    }
    infile=new std::ifstream(src, std::ios_base::binary);
    if(!infile)
    {
        std::cout << src << " infile can't be opened!\n";
        return;
    }
    infile->seekg(0,std::ios_base::end);
    if(!infile->tellg())
    {
        std::cout << src << " infile is empty!\n";
        return;
    }
    if(type == 0) {
        if(strstr(src, exts[0])) {
            destName = new char[strlen(src)+1];
            strcpy(destName, src);
            destName[strlen(src)-4] = 0;
            isCompressor = false;
        }
        else {
            destName = new char[strlen(src)+5];
            strcpy(destName, src);
            strcat(destName, exts[0]);
            isCompressor = true;
        }
    }
    outfile=new std::ofstream(destName, std::ios_base::binary);
    if(!outfile)
    {
        std::cout<<src<<" outfile can't be opened!\n";
        return;
    }
    uint basis;
    if(type==0) {
        if(isCompressor) {
            basis=atoi(bs);
//            if(basis<1 || basis>64)
            if(basis!=8 && basis!=16 && basis!=32 && basis!=64)
            {
                std::cout<<"Abnormal basis! This application compresses only data with 1-64 code lengths!\n";
                return;
            }
//            if(basis<=8)
            if(basis==8)
            {
                Compressor<uint8_t> *compressor=new Compressor<uint8_t>(infile, outfile);
                if(!compressor->Compress())
                {
                    std::cout<<"Compression error!\n";
                    delete compressor;
                    return;
                }
                delete compressor;
            }
//            else if(basis<=16)
            else if(basis==16)
            {
                Compressor<uint16_t> *compressor=new Compressor<uint16_t>(infile, outfile);
                if(!compressor->Compress())
                {
                    std::cout<<"Compression error!\n";
                    delete compressor;
                    return;
                }
                delete compressor;
            }
//            else if(basis<=32)
            else if(basis==32)
            {
                Compressor<uint32_t> *compressor=new Compressor<uint32_t>(infile, outfile);
                if(!compressor->Compress())
                {
                    std::cout<<"Compression error!\n";
                    delete compressor;
                    return;
                }
                delete compressor;
            }
/*
//            else if(basis<=64)
            else if(basis==64)
            {
                Compressor<uint64_t> *compressor=new Compressor<uint64_t>(infile, outfile);
                if(!compressor->Compress())
                {
                    std::cout<<"Compression error!";
                    delete compressor;
                    return;
                }
                delete compressor;
            }*/
            else
            {
                std::cout<<"Abnormal basis! This application compresses only whole data codes!\n";
                return;
            }
        }
        else
        {
            infile->seekg(0,std::ios_base::beg);
            basis=infile->get();
            basis++;
//            if(basis<1 && basis>64)
            if(basis!=8 && basis!=16 && basis!=32 && basis!=64)
            {
                std::cout<<src<<" file's header is corrupted 0!\n";
                return;
            }
//            if(basis<=8)
            if(basis==8)
            {
                Decompressor<uint8_t> *decompressor=new Decompressor<uint8_t>(infile, outfile);
                if(!decompressor->Decompress())
                {
                    std::cout<<"Decompression error!\n";
                    delete decompressor;
                    return;
                }
                delete decompressor;
            }
//            else if(basis<=16)
            else if(basis==16)
            {
                Decompressor<uint16_t> *decompressor=new Decompressor<uint16_t>(infile, outfile);
                if(!decompressor->Decompress())
                {
                    std::cout<<"Decompression error!\n";
                    delete decompressor;
                    return;
                }
                delete decompressor;
            }
//            else if(basis<=32)
            else if(basis==32)
            {
                Decompressor<uint32_t> *decompressor=new Decompressor<uint32_t>(infile, outfile);
                if(!decompressor->Decompress())
                {
                    std::cout<<"Decompression error!\n";
                    delete decompressor;
                    return;
                }
                delete decompressor;
            }
/*
//            else// if(basis<=64)
            else// if(basis==64)
            {
                Decompressor<uint64_t> *decompressor=new Decompressor<uint64_t>(infile, outfile);
                if(!decompressor->Decompress())
                {
                    std::cout<<"Decompression error!";
                    delete decompressor;
                    return;
                }
                delete decompressor;
            }
*/
        }
    }
}

Datar::~Datar() {
    if(infile) {
        infile->close();
        delete infile;
    }
    if(outfile) {
        outfile->close();
        delete outfile;
    }
    if(destName)
        delete[] destName;
}
