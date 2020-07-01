//
//  Compressor.hpp
//  datar
//
//  Created by Vahagn Nurijanyan on 2016-07-04.
//  Copyright © 2016 BABELONi INC. All rights reserved.
//

#ifndef Compressor_hpp
#define Compressor_hpp

//If memory is critical, HARDIN: can get encoded patterns directly from the file or HARDOUT: can put decoded patterns directly to the file
//This code is developed for !HARDIN & HARDOUT, but could be developed also for HARDIN & !HARDOUT

#include <fstream>
#include <unordered_map>
#include <vector>
#include "io.hpp"
#ifdef COMPLEX
#ifdef HUFFMAN
#include "Huffman.hpp"
#endif
//codesAbs.size()==codeNumber+1
//STRONG. can be absent, but it is improvement...
#endif
#ifdef STAT
#include <iostream>
#endif
/*
nodeId==0 -> nodeNumber==0 too: it is (becomes) channel
nodeId>0 && !isUsedNode: it is not used (to be used) yet node
nodeId>0 && isUsedNode: it is already used (at least once) node
*/
#ifdef STAT
static uint bornJoints;
static uint diedJoints;
#endif
template<class P> struct CJoint
{
private:
    P code;//can be absent: just makes faster to erase pointer to this in the parent
public:
    uint nodeId=0;
    uint nodeNumber=0;
    CJoint *parent;
    std::unordered_map <P, CJoint*> firstTree;
    std::unordered_map <P, CJoint*> secondTree;
    CJoint();
    CJoint(P cod, CJoint<P> *pre);
    ~CJoint();
};

template <class P> CJoint<P>::CJoint() {
    parent=NULL;
}

template <class P> CJoint<P>::CJoint(P cod, CJoint<P> *pre) {
    parent=pre;
//THIS.    parent->firstTree[cod] = this;
    code=cod;
#ifdef STAT
    bornJoints++;
#endif
}

template <class P> CJoint<P>::~CJoint() {
    if(parent)
    {
        parent->secondTree.erase(code);
#ifdef STAT
        diedJoints++;
#endif
    }
}

template<class P> class Compressor {
    P *codes;
    P *code;
#ifdef COMPLEX
    P previousRelativeCode;
    P previousRelativeTree, currentRelativeTree;
    std::vector<std::vector<uint>> patterns;
    std::vector<P> codesAbs;//absolute codes
    std::unordered_map <P, P> codesAbsRel;//absolute -> relative codes
    std::vector<std::vector<P>> relatives;//arranged relative codes
#ifdef HUFFMAN
    long hufRelativeTree;
    std::vector<std::vector<int>> hufBases;
    std::vector<std::vector<uint32_t>> hufLeafs;
    Huffman<P> huffman=Huffman<P>();
#else
    uint currentNumber;
    uint8_t numberBase;
#endif
    P codeNumber=0;
    P maxCodeNumber=0;
#else
    std::vector<uint> patterns;
    long patternNumber;
    uint8_t numberBase;
    int numberBaseChanges=0;
#ifdef CODENUMBER
    P codeNumber=0;
    P maxCodeNumber=0;
#endif
#endif
    CJoint<P> *forest;
#ifdef COMPLEX
    CJoint<P> *previousJoint0;
#endif
    CJoint<P> *previousJoint;
#ifdef COMPLEX
//STRONG. can use "keys" instead of "currentJoint0", like in Decompressor...
    CJoint<P> *currentJoint0=NULL;//STRONG.
#endif
    CJoint<P> *currentJoint;
    std::ifstream* input;
    std::ofstream* output;
    long size;
    long counter;
//    uint maxNodeId=1;
    uint maxNodeId=0;
#ifdef COMPLEX
    bool canCut0;//STRONG.
#endif
    bool canCut;
#ifdef STAT
    uint filledQuantity=0;
    uint emptyQuantity=0;
    uint fullLeafQuantity=0;
    uint emptyLeafQuantity=0;
    uint leafNodeQuantity=0;
    uint nodeQuantity=0;
    uint channelQuantity=0;
    uint singleNodeNumber=0;
    uint multipleNodeNumber=0;
    uint n0=0;
    uint n1=0;
    uint n2=0;
    uint n3=0;
    uint n4=0;
    uint n5=0;
    uint n6=0;
    uint n7=0;
    uint n8=0;
    uint n9=0;
#endif
    uint patternSize;
//    P *pattern;
    P previousTree, currentTree;
    void firstCycle();
    void secondCycle();
    void keepNodes();
    void keepNode(CJoint<P> *joint);
//    void cut(CJoint<P> *joint);
    void cut(CJoint<P> *joint, uint pattern_size);
#ifndef COMPLEX
    void getNumberBase();
#endif
    void putNodeNumber(CJoint<P> *joint);
#ifdef COMPLEX
#ifdef TABLES
#ifndef HUFFMAN
    void getNumberBase(bool isOld=false);
#endif
#else
    void getNumberBase(int old=0);
#endif
#endif
    clock_t start;
public:
    Compressor(std::ifstream* infile, std::ofstream* outfile);
    Compressor(P *cods, long siz, std::ofstream* outfile);
    ~Compressor();
    bool Compress();
};

//!HARDIN  & HARDOUT CONSTRUCTOR
template <class P> Compressor<P>::Compressor(std::ifstream* infile, std::ofstream* outfile) {
    start= clock();
    input=infile;
    output=outfile;
    /*#ifdef HARDIN
        i_file=input;
    #else
        r_buffer=codes;
    #endif*/
    #ifdef HARDOUT
        o_file=output;
    #endif
    forest=new CJoint<P>();
    forest->nodeNumber=1;
    input->seekg(0, std::ios_base::end);
    size=(uint32_t)input->tellg()/sizeof(P);
    input->seekg(0, std::ios_base::beg);
    codes=new P[size+1];
    input->read((char *)codes, size*sizeof(P));
}

//!HARDIN & HARDOUT  CONSTRUCTOR
template <class P> Compressor<P>::Compressor(P *cods, long siz, std::ofstream* outfile) {
    output=outfile;
    #ifdef HARDOUT
        o_file=output;
    #endif
    forest=new CJoint<P>();
    forest->nodeNumber=2;
    size=siz;
    codes=cods;
}

template <class P> Compressor<P>::~Compressor() {
#ifdef STAT
    std::cout << "Born Joints: " << bornJoints << '\n';
    std::cout << "Died Joints: " << diedJoints << '\n';
#endif
    if(forest->nodeNumber==1)
        delete[] codes;
    delete forest;
}

//!HARDIN & HARDOUT / !HARDOUT
template <class P> bool Compressor<P>::Compress() {
    if(size<sizeof(P)) {
        std::cout << "Contains less than a code!\n";
        return false;
    }
    output->put(sizeof(P)*8-1);
/*    w_base=32;
    write((uint32_t)size);*/
    output->write((char*)&size, 4);
//    pattern=codes;
    counter=size-1;
    code=codes;
    forest->firstTree.insert(std::make_pair(*code, new CJoint<P>(*code, forest)));//THIS.
#ifdef COMPLEX
    codesAbs.push_back(*code);
    codesAbsRel.insert(std::make_pair(*code, 0));
    relatives.push_back(std::vector<P>());
#ifdef HUFFMAN
    hufBases.push_back(std::vector<int>());
    hufLeafs.push_back(std::vector<uint32_t>());
#endif
    patterns.push_back(std::vector<uint>());
    previousRelativeCode=0;
#endif
    currentTree=*code;
    previousJoint=forest->firstTree[*code++];
    while (counter) {
        firstCycle();
    };
#ifdef COMPLEX
/*    w_base=sizeof(P)*8;
     write(maxCodeNumber);*/
    output->write((char*)&maxCodeNumber, sizeof(P));
#ifdef TABLES
    uint8_t items_quantity_base=get_base(maxCodeNumber+1);
    uint8_t items_base=get_base(maxCodeNumber);
    for(int i=0; i<=maxCodeNumber; i++) {
        w_base=items_quantity_base;
        write((uint32_t)relatives[i].size());
        if(relatives[i].size()) {
#ifdef HUFFMAN
            w_base=items_base;
            if(relatives[i].size() == 1) {
                write(relatives[i][0]);
                hufBases[i][0] = 0;
                hufLeafs[i].push_back(0);
            }
            else if(relatives[i].size() == 2) {
                write(relatives[i][0]);
                write(relatives[i][1]);
                hufBases[i][0] = 1;
                hufBases[i][1] = 1;
                hufLeafs[i].push_back(0);
                hufLeafs[i].push_back(1);
            }
            else {
                huffman.writeHufTable(relatives[i], hufBases[i], hufLeafs[i]);
            }
#else
            w_base=items_base;
            for(int j=0; j<relatives[i].size(); j++) {//size can't be 0
                write(relatives[i][j]);
            }
#endif
        }
    }
#endif
#else
#ifdef CODENUMBER
    /*    w_base=sizeof(P)*8;
     write(maxCodeNumber);*/
    output->write((char*)&maxCodeNumber, sizeof(P));
#endif
#endif
    counter=size-1;
    code=codes;
    forest->secondTree.insert(std::make_pair(*code, forest->firstTree[*code]));
    forest->firstTree.erase(*code);//it slows but it is correct
#ifdef COMPLEX
    w_base=8*sizeof(P);
    write(*code);
#else
    output->write((char*)code, sizeof(P));
#endif
    currentTree=*code;
#ifdef COMPLEX
    previousRelativeTree=0;
    previousRelativeCode=0;
#endif
    previousJoint=forest->secondTree[*code++];
    keepNode(previousJoint);
/*
     do {
        ;
     } while(forest->secondTree.size() || forest->firstTree.size());
*/
    while (counter) {
        secondCycle();
    };
    if(w_pos!=8)
#ifdef HARDOUT
        o_file->put(w_byte);
#endif
    w_byte=0;
    double cpu_time=((double)(clock()-start))/CLOCKS_PER_SEC;
    std::cout << "CPU Time: " << cpu_time << '\n';
#ifdef STAT
//    std::cout << codeNumber << '\n';
//    std::cout << "Max Code Number: " << maxCodeNumber << '\n';
    std::cout << "Filled Quantity: " << filledQuantity << '\n';
    std::cout << "Empty Quantity: " << emptyQuantity << '\n';
    std::cout << "Full Leaf Quantity: " << fullLeafQuantity << '\n';
    std::cout << "Empty Leaf Quantity: " << emptyLeafQuantity << '\n';
    std::cout << "Node Quantity: " << leafNodeQuantity <<'/' << nodeQuantity << '\n';
    std::cout << "Channel Quantity: " << channelQuantity << '\n';
    if(filledQuantity)
        std::cout << "Branch Size: " << (nodeQuantity + channelQuantity) / filledQuantity << '\n';
    std::cout << "Single Node Number: " << singleNodeNumber << '\n';
    std::cout << "Multiple Node Number: " << multipleNodeNumber << '\n';
    std::cout << "N0: " << n0 << "\nN1: " << n1 << "\nN2: " << n2 << "\nN3: " << n3 << "\nN4: " << n4 << "\nN5: " << n5 << "\nN6: " << n6 << "\nN7: " << n7 << "\nN8: " << n8 << "\nN9: " << n9 << '\n';
#endif
    return true;
}

template <class P> void Compressor<P>::firstCycle() {
//    if(!forest->firstTree[*code]) {
//    if(forest->firstTree.find(*code)==currentJoint->firstTree.end()) {
    if(forest->firstTree.count(*code)==1) {
#ifdef COMPLEX
#ifdef TABLES
        currentRelativeTree=codesAbsRel[*code];
        long number=lower_bound(relatives[previousRelativeCode].begin(), relatives[previousRelativeCode].end(), currentRelativeTree)-relatives[previousRelativeCode].begin();
        if(number!=relatives[previousRelativeCode].size()) {//if(number<relatives[previousRelativeCode].size())
            if(relatives[previousRelativeCode][number]!=currentRelativeTree) {
                relatives[previousRelativeCode].insert(relatives[previousRelativeCode].begin()+number, currentRelativeTree);
#ifdef HUFFMAN
                hufBases[previousRelativeCode].insert(hufBases[previousRelativeCode].begin()+number, 1);
#endif
            }
#ifdef HUFFMAN
            else {
                hufBases[previousRelativeCode][number]++;
            }
#endif
        }
        else {
            relatives[previousRelativeCode].push_back(currentRelativeTree);
#ifdef HUFFMAN
            hufBases[previousRelativeCode].push_back(1);
#endif
        }
#endif
#endif
        currentJoint=forest;
        previousTree=currentTree;
        currentTree=*code;
        if(previousTree!=currentTree) {
            do {
                currentJoint=currentJoint->firstTree[*code];
                previousJoint->firstTree.insert(std::make_pair(*code, new CJoint<P>(*code, previousJoint)));
                previousJoint=previousJoint->firstTree[*code];
            } while (--counter && currentJoint->firstTree.count(*++code)==1);
            currentJoint->nodeNumber++;
            previousJoint=currentJoint;
        }
        else {
#ifdef COMPLEX
            previousJoint0=previousJoint;
#else
            CJoint<P> *previousJoint0=previousJoint;
#endif
            do {
                currentJoint=currentJoint->firstTree[*code];
                previousJoint->firstTree.insert(std::make_pair(*code, new CJoint<P>(*code, previousJoint)));
                previousJoint=previousJoint->firstTree[*code];
/*                if(currentJoint==previousJoint0) {
                    counter--;
                    code++;
                    break;
                }*/
                if(currentJoint==previousJoint0  && currentTree==*(code+1)) {
                    counter--;
                    code++;
                    break;
                }
        //    } while (currentJoint->firstTree[*++code]);
        //    } while (currentJoint->firstTree.find(*++code)!=currentJoint->firstTree.end());
            } while (--counter && currentJoint->firstTree.count(*++code)==1);
            currentJoint->nodeNumber++;
//*NEW.
            if(currentJoint!=previousJoint0) {
                previousJoint=currentJoint;
            }
//*/
        }
#ifdef COMPLEX
        previousRelativeCode=codesAbsRel[*(code-1)];
#endif
    }
    else {
#ifndef COMPLEX
        currentTree=*code;
#endif
        previousJoint->firstTree.insert(std::make_pair(*code, new CJoint<P>(*code, previousJoint)));
        forest->firstTree.insert(std::make_pair(*code, new CJoint<P>(*code, forest)));//THIS.
#ifdef COMPLEX
        maxCodeNumber++;
#else
#ifdef CODENUMBER
        maxCodeNumber++;
#endif
#endif
        if(--counter) {
            currentTree=*code;
#ifdef COMPLEX
            currentRelativeTree=(P)codesAbs.size();
            codesAbs.push_back(currentTree);//this can be done in secondCycle() too
            codesAbsRel.insert(std::make_pair(currentTree, currentRelativeTree));
#ifdef TABLES
//SHORTER. will slow Decompressor code: it will need to check relatives[previousRelativeCode] has currentRelativeTree or not...
#ifdef HUFFMAN
            relatives[previousRelativeCode].push_back(currentRelativeTree);//SHORTER.
            hufBases[previousRelativeCode].push_back(1);//SHORTER.
#else
//SHORTER.            relatives[previousRelativeCode].push_back(currentRelativeTree);//previousCode != *code fore sure
#endif
#endif
//            relatives.push_back(std::vector<P>());
            patterns.push_back(std::vector<uint>());
            previousRelativeCode=currentRelativeTree;
#endif
            previousJoint=forest->firstTree[*code++];
        }
#ifdef COMPLEX
        relatives.push_back(std::vector<P>());
#ifdef HUFFMAN
        hufBases.push_back(std::vector<int>());
        hufLeafs.push_back(std::vector<uint32_t>());
#endif
#endif
    }
}

template <class P> void Compressor<P>::secondCycle() {
#ifdef COMPLEX
    if(codeNumber==maxCodeNumber || forest->secondTree.count(*code)==1)//if codeNumber==maxCodeNumber, forest->secondTree.count(*code)==1
#else
#ifdef CODENUMBER
        if(codeNumber==maxCodeNumber || forest->secondTree.count(*code)==1)//if codeNumber==maxCodeNumber, forest->secondTree.count(*code)==1
#else
            if(forest->secondTree.count(*code)==1)
#endif
#endif
    {
        currentJoint=forest;
        patternSize=0;
        previousTree=currentTree;
        currentTree=*code;
#ifdef COMPLEX
        currentRelativeTree=codesAbsRel[currentTree];
#endif
        canCut=true;
//currentJoint can't be forest: its' parent can't be null
//previousJoint is always higher than currentJoint
        if(previousTree!=currentTree)
        {
            do {
                currentJoint=currentJoint->secondTree[*code];
                previousJoint->secondTree.insert(std::make_pair(*code, previousJoint->firstTree[*code]));
                previousJoint->firstTree.erase(*code);//it slows but it is correct
                previousJoint=previousJoint->secondTree[*code];
                patternSize++;
            } while (--counter && currentJoint->secondTree.count(*++code)==1);
        }
        else {
//currentJoint can't crash previous joint if they are in different forests
#ifdef COMPLEX
            previousJoint0=previousJoint;
#else
            CJoint<P> *previousJoint0=previousJoint;
#endif
            do {
                currentJoint=currentJoint->secondTree[*code];
                previousJoint->secondTree.insert(std::make_pair(*code, previousJoint->firstTree[*code]));
                previousJoint->firstTree.erase(*code);//it slows but it is correct
                previousJoint=previousJoint->secondTree[*code];
                patternSize++;
/*                if(patternSize && currentJoint==previousJoint0) {
                    canCut=false;
                    counter--;
                    code++;
                    break;
                 }*/
                if(currentJoint==previousJoint0 && currentTree==*(code+1)) {
//NEW.                    canCut=false;
                    counter--;
                    code++;
                    break;
                }
        //    } while (currentJoint->secondTree[*++code]);
        //    } while (currentJoint->secondTree.find(*++code)!=currentJoint->secondTree.end());
            } while (--counter && currentJoint->secondTree.count(*++code)==1);
//*NEW.
            if(currentJoint==previousJoint0) {
                canCut=false;
            }
//*/
        }
//patterns isn't empty
#ifdef COMPLEX
//ITE.        long localNumber=lower_bound(patterns.begin(), patterns.end(), currentJoint->nodeId)-patterns.begin();
        std::vector<uint>::iterator item=lower_bound(patterns[currentRelativeTree].begin(), patterns[currentRelativeTree].end(), currentJoint->nodeId);//ITE.
//ITE.        long localNumber=lower_bound(patterns.begin(), patterns.end(), currentJoint->nodeId)-patterns.begin();
        long localNumber=item-patterns[currentRelativeTree].begin();//ITE.
#ifdef TABLES
#ifdef HUFFMAN
        hufRelativeTree=lower_bound(relatives[previousRelativeCode].begin(), relatives[previousRelativeCode].end(), currentRelativeTree)-relatives[previousRelativeCode].begin();
        huffman.writeCode(hufBases[previousRelativeCode][hufRelativeTree], hufLeafs[previousRelativeCode][hufRelativeTree]);
//patterns[currentRelativeTree].size()>0
        if(codeNumber==maxCodeNumber)
            w_base=get_base((uint32_t)patterns[currentRelativeTree].size()-1);
        else
            w_base=get_base((uint32_t)patterns[currentRelativeTree].size());
        write((uint32_t)localNumber);
#else
        getNumberBase(true);
#endif
#else
        long number=lower_bound(relatives[previousRelativeCode].begin(), relatives[previousRelativeCode].end(), currentRelativeTree)-relatives[previousRelativeCode].begin();
        if(number<relatives[previousRelativeCode].size() && relatives[previousRelativeCode][number]==currentRelativeTree) {
            getNumberBase(2);
        }
        else {//first time usage
            getNumberBase(-1);
            w_base=numberBase;
            //if currentNumber == 0, w_base == 0...
            write(currentNumber-1);
            getNumberBase(1);
            if(currentJoint->nodeNumber!=1) {//if(currentJoint->nodeNumber>1)
                if(number!=relatives[previousRelativeCode].size())
                    relatives[previousRelativeCode].insert(relatives[previousRelativeCode].begin()+number, currentRelativeTree);
                else
                    relatives[previousRelativeCode].push_back(currentRelativeTree);
            }
        }
#endif
#ifndef HUFFMAN
        currentNumber += localNumber;
        //w_base can be 0
        w_base=numberBase;
        write(currentNumber);
#endif
#else
        //ITE.        long currentNumber=lower_bound(patterns.begin(), patterns.end(), currentJoint->nodeId)-patterns.begin();
        std::vector<uint>::iterator item=lower_bound(patterns.begin(), patterns.end(), currentJoint->nodeId);//ITE.
        //ITE.        long currentNumber=lower_bound(patterns.begin(), patterns.end(), currentJoint->nodeId)-patterns.begin();
        long currentNumber=(item-patterns.begin());//ITE.
        if(numberBaseChanges)
            getNumberBase();
        //w_base can be 0
        w_base=numberBase;
        write((uint)currentNumber);
#endif
#ifdef STAT
        if(currentJoint->nodeNumber==1)
            singleNodeNumber++;
        else
            multipleNodeNumber++;
        double n=((double)currentNumber)/patternNumber;
        if(n>0.9) {
            n0++;
        } else if(n>0.8) {
            n1++;
        } else if(n>0.7) {
            n2++;
        } else if(n>0.6) {
            n3++;
        }
        else if(n>0.5) {
           n4++;
       }
        else if(n>0.4) {
           n5++;
       }
        else if(n>0.3) {
           n6++;
       }
        else if(n>0.2) {
           n7++;
       }
        else if(n>0.1) {
           n8++;
       }
        else {
            n9++;
        }
#endif
#ifdef STAT1
        std::cout << currentNumber << '\t' << counter << '\n';
#endif
        if(currentJoint->nodeNumber==1) {
#ifdef COMPLEX
//ITE.            patterns.erase(patterns.begin()+localNumber);
            patterns[currentRelativeTree].erase(item);//ITE.
#else
//ITE.            patterns.erase(patterns.begin()+currentNumber);
            patterns.erase(item);//ITE.
#endif
#ifdef STAT1
            std::cout << '-' << currentNumber << '\t' << counter << '\n';
#endif
#ifndef COMPLEX
            numberBaseChanges--;
#endif
        }
        currentJoint->nodeNumber--;
        if(!counter) {
            cut(previousJoint, patternSize);
            if(canCut && currentTree != previousTree)//if(forest->secondTree.size())
                cut(currentJoint, patternSize);
            return;
        }
        keepNodes();
        if(canCut)
#ifdef COMPLEX
        {
#endif
            previousJoint=currentJoint;
#ifdef COMPLEX
            currentJoint0=currentJoint->secondTree.size()?currentJoint:NULL;//STRONG. comment for excluding STRONG.
        }
//*STRONG.
        else
            currentJoint0=currentJoint->secondTree.size()>1?currentJoint:NULL;//STRONG. comment for excluding STRONG.
//*/
        previousRelativeTree=currentRelativeTree;
        previousRelativeCode=codesAbsRel[*(code-1)];
        canCut0=canCut;//STRONG.
#endif
    }
    else {
        currentTree=*code;
#ifdef COMPLEX
        previousJoint->secondTree.insert(std::make_pair(currentTree, previousJoint->firstTree[currentTree]));
        previousJoint->firstTree.erase(currentTree);//it slows but it is correct
        previousJoint=previousJoint->secondTree[currentTree];
        forest->secondTree.insert(std::make_pair(currentTree, forest->firstTree[currentTree]));
        forest->firstTree.erase(currentTree);//it slows but it is correct
#ifdef HUFFMAN
        hufRelativeTree=lower_bound(relatives[previousRelativeCode].begin(), relatives[previousRelativeCode].end(), codesAbsRel[currentTree])-relatives[previousRelativeCode].begin();
        huffman.writeCode(hufBases[previousRelativeCode][hufRelativeTree], hufLeafs[previousRelativeCode][hufRelativeTree]);//SHORTER. patterns[currentRelativeTree].size()==0
#else
        getNumberBase();
        w_base=numberBase;
        write(currentNumber);
#endif
#else
        previousJoint->secondTree.insert(std::make_pair(*code, previousJoint->firstTree[*code]));
        previousJoint->firstTree.erase(*code);//it slows but it is correct
        previousJoint=previousJoint->secondTree[*code];
        forest->secondTree.insert(std::make_pair(*code, forest->firstTree[*code]));
        forest->firstTree.erase(*code);//it slows but it is correct
        if(numberBaseChanges)
            getNumberBase();
        w_base=numberBase;
        write((uint32_t)patternNumber);
#endif
        w_base=8*sizeof(P);
#ifdef COMPLEX
        write(currentTree);
#else
        write(*code);
#endif
        if(!--counter) {
            cut(previousJoint, 1);
#ifdef COMPLEX
            cut(forest->secondTree[currentTree], 1);
#else
            cut(forest->secondTree[*code], 1);
#ifdef CODENUMBER
            ++codeNumber;
#endif
#ifndef COMPLEX
#endif
#endif
            return;
        }
#ifdef STAT1
#ifdef COMPLEX
        std::cout << '+' << currentNumber << '\t' << counter << '\n';
#else
        std::cout << '+' << patternNumber << '\t' << counter << '\n';
#endif
#endif
/*        patternSize=1;
        keepNodes();*/
#ifdef COMPLEX
/*
        keepNode(forest->secondTree[currentTree]);
        keepNode(previousJoint);
*/
        keepNode(previousJoint);
        previousRelativeTree=codesAbsRel[currentTree];
        //Each code's pattern's first item is the code itself
        keepNode(forest->secondTree[currentTree]);
#else
/*
         keepNode(forest->secondTree[*code]);
         keepNode(previousJoint);
*/
        keepNode(previousJoint);
        keepNode(forest->secondTree[*code]);
        currentTree=*code;
#endif
        previousJoint=forest->secondTree[*code++];
#ifdef COMPLEX
        if(++codeNumber==maxCodeNumber) {
#ifdef STAT
            std::cout << "Counter/Size: " << counter << "/" << size << '\n';
#endif
        }
        previousRelativeCode=previousRelativeTree;
        currentJoint0=NULL;//STRONG.
#else
#ifdef CODENUMBER
        if(++codeNumber==maxCodeNumber) {
            numberBaseChanges--;
#ifdef STAT
            std::cout << "Counter/Size: " << counter << "/" << size << '\n';
#endif
        }
#endif
#endif
    }
}

template <class P> void Compressor<P>::keepNodes() {
    uint i=0;
    const uint pattern_size=patternSize;
    CJoint<P> *joint=previousJoint;
    do {
        if(joint->nodeNumber) {
            break;
        }
        joint=joint->parent;
    } while (++i<patternSize);
    if(i==patternSize) {
#ifdef STAT
        if(patternSize>1)
            emptyQuantity++;
#endif
        write(false);
    }
    else {
        write(true);
        if(patternSize==1) {
            putNodeNumber(joint);//joint==previousJoint
#ifdef COMPLEX
            patterns[previousRelativeTree].push_back(++maxNodeId);
#else
            patterns.push_back(++maxNodeId);
            numberBaseChanges++;
#endif
            joint->nodeId=maxNodeId;
            return;
        }
#ifdef STAT
        filledQuantity++;
#endif
//FST.        joint=previousJoint;
//FST.        i=0;
//w_base can't be 0: it is at least 1...
        w_base=get_base(patternSize-1);
        do {
            if(joint->nodeNumber)//FST. it is always true
                //this code runs at least once
            {
#ifdef STAT
                if(joint==previousJoint) {
                    leafNodeQuantity++;
                }
                nodeQuantity++;
#endif
                write(i);
                putNodeNumber(joint);
#ifdef COMPLEX
                patterns[previousRelativeTree].push_back(++maxNodeId);
#else
                patterns.push_back(++maxNodeId);
                numberBaseChanges++;
#endif
                joint->nodeId=maxNodeId;
                patternSize-=i+1;
                if(patternSize) {
                    w_base=get_base(patternSize);
                    i=0;
                }
                else
                    break;
            }
            else {
#ifdef STAT
                channelQuantity++;
#endif
                if(++i==patternSize) {
                    write(i);
                    break;
                }
            }
            joint=joint->parent;
        } while (1);
    }
    if(!previousJoint->nodeNumber && canCut)//if(!previousJoint->firstTree.size() && canCut)
    {
        cut(previousJoint, pattern_size);
#ifdef STAT
        emptyLeafQuantity++;
#endif
    }
#ifdef STAT
    else// if(previousJoint->nodeNumber)
        fullLeafQuantity++;
#endif
}

template <class P> void Compressor<P>::keepNode(CJoint<P> *joint) {
    if(joint->nodeNumber) {
        write(true);
        putNodeNumber(joint);
#ifdef COMPLEX
        patterns[previousRelativeTree].push_back(++maxNodeId);
#else
        patterns.push_back(++maxNodeId);
        numberBaseChanges++;
#endif
        joint->nodeId=maxNodeId;
    }
    else {
        write(false);
        if(joint==previousJoint && !joint->firstTree.size()) {//joint->parent!=forest
            cut(joint, 1);
        }
    }
}

/*
template <class P> void Compressor<P>::cut(CJoint<P> *leaf) {
    CJoint<P> *joint=leaf;
    CJoint<P> *parent;
    do {
        parent=joint->parent;
        delete joint;
        joint=parent;
    } while(!joint->nodeNumber && !joint->firstTree.size() && !joint->secondTree.size());//!joint->nodeNumber/joint->parent is for being sure parent!=forest//joint is always leaf of second tree: has always !joint->secondTree.size()
}
*/
//cuts empty / dried extreme paths
//!joint->nodeNumber means joint newer will grow if it is not previousJoint: only previousJoint leaf can grow with !joint->nodeNumber...
//if(joint isn't previousJoint & !joint->nodeNumber -> !joint->firstTree.size() && joint->firstTree.size()) -> it is / stays leaf (it is leaf & will not grow)...
//all currentJoint's joints have their newly created copy joints grown on previousJoint -> previousJoint's last patternSize joints originally aren't branched & previousJoint's last joint originally is always leaf (otherwise previousJoint, which was previous currentJoint, would be longer): If it would exist, it will continue!...
template <class P> void Compressor<P>::cut(CJoint<P> *leaf, uint pattern_size) {
    CJoint<P> *joint=leaf;
    CJoint<P> *parent;
    do {
        parent=joint->parent;
        delete joint;
        joint=parent;
    } while(--pattern_size && !joint->nodeNumber);
    if(pattern_size)
        return;
    while(!joint->nodeNumber && !joint->firstTree.size() && !joint->secondTree.size())
    {
        parent=joint->parent;
        delete joint;
        joint=parent;
    }
}

template <class P> void Compressor<P>::putNodeNumber(CJoint<P> *joint) {
/*
    for (int i=1; i<joint->nodeNumber; i++) {
        write(true);
    }
    write(false);
*/
    if(joint->nodeNumber==1) {
        write(false);
    }
    else {
        for (int i=1; i<joint->nodeNumber; i++) {
            write(true);
        }
        write(false);
    }
}

#ifndef COMPLEX
template <class P> void Compressor<P>::getNumberBase() {
    patternNumber=patterns.size();
    //    patternNumber+=numberBaseChanges;
#ifdef CODENUMBER
    if(codeNumber==maxCodeNumber) {
        numberBase=get_base((uint32_t)(patternNumber?patternNumber-1:0));
    }
    else
#endif
        numberBase=get_base((uint32_t)patternNumber);
    numberBaseChanges=0;
}
#else
/*
template <class P> void Compressor<P>::getNumberBase() {
    currentNumber=patterns.size();
#ifdef CODENUMBER
    if(codeNumber==maxCodeNumber) {
        numberBase=get_base((uint32_t)(currentNumber?currentNumber-1:0));
    }
    else
#endif
        numberBase=get_base(currentNumber);
}
*/
#ifdef TABLES
#ifndef HUFFMAN
template <class P> void Compressor<P>::getNumberBase(bool isOld) {
    long size=relatives[previousRelativeCode].size();
    currentNumber=0;
    if(isOld) {
        long maxPatternNumber;
        int i=0;
//patterns[currentRelativeTree].size()>0
//"maxPatternNumber+=patterns[currentRelativeTree].size();i++;" statements can be excluded...
//while loop below will terminate for sure
//*STRONG.
        if(currentJoint0) {
            while (relatives[previousRelativeCode][i]!=currentRelativeTree)
            {
                if(currentJoint0->secondTree.count(codesAbs[relatives[previousRelativeCode][i]])==0 || (!canCut0 && relatives[previousRelativeCode][i]==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    currentNumber+=patterns[relatives[previousRelativeCode][i]].size();
                }
                i++;
            }
            maxPatternNumber=currentNumber;
            maxPatternNumber+=patterns[currentRelativeTree].size();
            i++;
            for(; i<size; i++) {
                P iCode=relatives[previousRelativeCode][i];
//                if(codeNumber<iCode)
//                    break;
                if(currentJoint0->secondTree.count(codesAbs[iCode])==0 || (!canCut0 && iCode==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    maxPatternNumber+=patterns[iCode].size();
                }
            };
        }
        else
//*/
        {
            while (relatives[previousRelativeCode][i]!=currentRelativeTree)
            {
                currentNumber+=patterns[relatives[previousRelativeCode][i++]].size();
            }
            maxPatternNumber=currentNumber;
            maxPatternNumber+=patterns[currentRelativeTree].size();
            i++;
            for(; i<size; i++) {
//                if(codeNumber<relatives[previousRelativeCode][i])
//                    break;
                maxPatternNumber+=patterns[relatives[previousRelativeCode][i]].size();
            };
        }
        if(codeNumber==maxCodeNumber)
            numberBase = get_base((uint32_t)(maxPatternNumber?maxPatternNumber-1:0));
        else
            numberBase = get_base((uint32_t)maxPatternNumber);
    }
    else {//codeNumber<maxCodeNumber
//*STRONG.
        if(currentJoint0) {
            for(int i=0; i<size; i++) {
                P iCode=relatives[previousRelativeCode][i];
                if(codeNumber<iCode)
                    break;
                if(currentJoint0->secondTree.count(codesAbs[iCode])==0 || (!canCut0 && iCode==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    currentNumber+=patterns[iCode].size();
                }
            }
        }
        else
//*/
        {
            for(int i=0; i<size; i++) {
                if(codeNumber<relatives[previousRelativeCode][i])
                    break;
                currentNumber+=patterns[relatives[previousRelativeCode][i]].size();
            }
        }
//size==0 & currentNumber==0 if after previousRelativeCode are coming only lonely codes...
        numberBase = get_base(currentNumber);
    }
}
#endif
#else
//2, 0 & -1 must compute the same numberBase
template <class P> void Compressor<P>::getNumberBase(int old) {
    long size=relatives[previousRelativeCode].size();
    currentNumber=0;
    if(old == 2) {
        long maxPatternNumber;
        int i=0;
//patterns[currentRelativeTree].size()>0
//"maxPatternNumber+=patterns[currentRelativeTree].size();i++;" statements can be excluded...
//while loop below will terminate for sure
//*STRONG.
        if(currentJoint0) {
            while (relatives[previousRelativeCode][i]!=currentRelativeTree)
            {
                if(currentJoint0->secondTree.count(codesAbs[relatives[previousRelativeCode][i]])==0 || (!canCut0 && relatives[previousRelativeCode][i]==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    currentNumber+=patterns[relatives[previousRelativeCode][i]].size();
                }
                i++;
            }
            maxPatternNumber=currentNumber;
            maxPatternNumber+=patterns[currentRelativeTree].size();
            i++;
            for(; i<size; i++) {
                P iCode=relatives[previousRelativeCode][i];
                if(currentJoint0->secondTree.count(codesAbs[iCode])==0 || (!canCut0 && iCode==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    maxPatternNumber+=patterns[iCode].size();
                }
            };
        }
        else
//*/
        {
            while (relatives[previousRelativeCode][i]!=currentRelativeTree)
            {
                currentNumber+=patterns[relatives[previousRelativeCode][i++]].size();
            }
            maxPatternNumber=currentNumber;
            maxPatternNumber+=patterns[currentRelativeTree].size();
            i++;
            for(; i<size; i++) {
                maxPatternNumber+=patterns[relatives[previousRelativeCode][i]].size();
            };
        }
//BGT!        if(size!=codesAbs.size())//this happens nearly always
            maxPatternNumber++;
        if(codeNumber==maxCodeNumber)
            numberBase = get_base((uint32_t)(maxPatternNumber?maxPatternNumber-1:0));
        else
            numberBase = get_base((uint32_t)maxPatternNumber);
    } else if(old==1) {//relatives[previousRelativeCode].size()!=codesAbs.size()
        std::vector<P> relatives1;
        long maxPatternNumber;
//        P i=0;
        int i=0;
        int j=0;
//        P relativeCode=-1;
        int relativeCode=-1;
        while (j<size) {
            relativeCode=relatives[previousRelativeCode][j++];
            while (i<relativeCode) {
                relatives1.push_back(i++);
            }
            i++;
        };
        size=codesAbs.size();
        while(++relativeCode<size)
            relatives1.push_back(relativeCode);
        size=relatives1.size();
        i=0;
//while loop below will terminate for sure
//*STRONG.
        if(currentJoint0) {
            while (relatives1[i]!=currentRelativeTree)
            {
                if(currentJoint0->secondTree.count(codesAbs[relatives1[i]])==0 || (!canCut0 && relatives1[i]==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    currentNumber+=patterns[relatives1[i]].size();
                }
                i++;
            }
            maxPatternNumber=currentNumber;
            maxPatternNumber+=patterns[currentRelativeTree].size();
            i++;
            for(; i<size; i++) {
                if(currentJoint0->secondTree.count(codesAbs[relatives1[i]])==0 || (!canCut0 && relatives1[i]==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    maxPatternNumber+=patterns[relatives1[i]].size();
                }
            }
        }
        else
//*/
        {
            while (relatives1[i]!=currentRelativeTree)
            {
                currentNumber+=patterns[relatives1[i++]].size();
            }
            maxPatternNumber=currentNumber;
            maxPatternNumber+=patterns[currentRelativeTree].size();
            i++;
            for(; i<size; i++) {
                maxPatternNumber+=patterns[relatives1[i]].size();
            }
        }
        //maxPatternNumber > 0
        numberBase = get_base((uint32_t)--maxPatternNumber);
    }
    else if(old==0) {//codeNumber<maxCodeNumber
//*STRONG.
        if(currentJoint0) {
            for(int i=0; i<size; i++) {
                P iCode=relatives[previousRelativeCode][i];
                if(currentJoint0->secondTree.count(codesAbs[iCode])==0 || (!canCut0 && iCode==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    currentNumber+=patterns[iCode].size();
                }
            }
        }
        else
//*/
        {
            for(int i=0; i<size; i++) {
                currentNumber+=patterns[relatives[previousRelativeCode][i]].size();
            }
        }
//BGT!        if(size!=codesAbs.size())//this happens nearly always
            currentNumber++;
        numberBase = get_base(currentNumber);
    }
    else//relatives[previousRelativeCode].size()!=codesAbs.size()
    {
//*STRONG.
        if(currentJoint0) {
            for(int i=0; i<size; i++) {
                P iCode=relatives[previousRelativeCode][i];
                if(currentJoint0->secondTree.count(codesAbs[iCode])==0 || (!canCut0 && iCode==previousRelativeTree))//currentJoint->secondTree.count(codesAbs[iCode])==1 also means it could be currentJoint==previousJoint0 (!canCut)
                {
                    currentNumber+=patterns[iCode].size();
                }
            }
        }
        else
//*/
        {
            for(int i=0; i<size; i++) {
                currentNumber+=patterns[relatives[previousRelativeCode][i]].size();
            }
        }
/*        currentNumber++;
        if(codeNumber==maxCodeNumber)
            currentNumber--;*/
/*        if(codeNumber!=maxCodeNumber)
            currentNumber++;
        numberBase = get_base(currentNumber);*/
        currentNumber++;
        if(codeNumber==maxCodeNumber)
            numberBase = get_base((uint32_t)(currentNumber?currentNumber-1:0));
        else
            numberBase = get_base((uint32_t)currentNumber);
    }
}
#endif
#endif
#endif
