//
//  Decompressor.hpp
//  datar
//
//  Created by Vahagn Nurijanyan on 2016-07-04.
//  Copyright © 2016 BABELONi INC. All rights reserved.
//

#ifndef Decompressor_hpp
#define Decompressor_hppl

//If memory is critical, HARDIN: can get encoded patterns directly from the file or HARDOUT: can put decoded patterns directly to the file
//This code is developed for HARDIN / !HARDIN & !HARDOUT, but could be developed also for HARDOUT

#include <fstream>
#include <unordered_map>
#include <vector>
#include "io.hpp"
#ifdef COMPLEX
#ifdef HUFFMAN
#include "Huffman.hpp"
#endif
//class Huffman;
//codesAbs.size()==codeNumber+1
//here tree is mirror of secondTree of the Compressor
#endif

#ifdef STAT
static uint bornJoints1;
static uint diedJoints1;
#endif
template<class P> struct DJoint
{
    DJoint *parent;
    uint nodeNumber=0;
    int level;
    P code;
    std::unordered_map <P, DJoint*> tree;
    DJoint();
    DJoint(P code, DJoint *pre);
    ~DJoint();
};

template <class P> DJoint<P>::DJoint() {
    parent=NULL;
    level=-1;
}

template <class P> DJoint<P>::DJoint(P cod, DJoint *pre) {
    parent=pre;
    level=parent->level+1;
    code=cod;
#ifdef STAT
    bornJoints1++;
#endif
}

template <class P> DJoint<P>::~DJoint() {
    if(parent)
    {
        parent->tree.erase(code);
#ifdef STAT
        diedJoints1++;
#endif
    }
}

template<class P> class Decompressor {
    P *codes;
    P code;
    P *pattern;
#ifdef COMPLEX
    P previousCode, previousRelativeCode;
    P previousTree, previousRelativeTree, currentRelativeTree;
#else
    std::vector<DJoint<P> *> patterns;
    long patternNumber;
#endif
    uint currentNumber;
#ifdef COMPLEX
#ifndef HUFFMAN
    uint currentRelativeNumber;
#endif
    std::vector<P> codesAbs;//absolute codes
    std::unordered_map <P, P> codesAbsRel;//absolute -> relative codes
    std::vector<std::vector<P>> relatives;//arranged relative codes
#ifdef HUFFMAN
//can be char instead of int...
    std::vector<std::vector<int>> hufBases;
    std::vector<std::vector<Node<P>*>> hufTrees;
    Huffman<P> huffman=Huffman<P>();
#endif
    std::vector<std::vector<DJoint<P> *>> patterns;
    long patternNumber;
#endif
    uint8_t numberBase=0;
#ifdef COMPLEX
    P codeNumber=0;
    P maxCodeNumber;
#else
    int numberBaseChanges=0;
#ifdef CODENUMBER
    P codeNumber=0;
    P maxCodeNumber;
#endif
#endif
    DJoint<P> *forest;
    DJoint<P> *previousJoint;
#ifdef COMPLEX
    //STRONG. can use "currentJoint0"instead of "keys", like in Compressor...
    std::vector<P> keys;//STRONG.
#endif
    DJoint<P> *currentJoint;
    std::ifstream* input;
    std::ofstream* output;
    long size;
    uint counter;
    char *source;
    bool canCut;
    int patternSize;
#ifndef COMPLEX
//previousTree & currentIsSecure can be absent
    P previousTree;
#endif
    bool currentIsSecure;
    bool bu;
    void cycle();
    void keepNodes();
    void keepNode(DJoint<P> *joint);
    void cut(DJoint<P> *leaf);
    void getNumberBase();
#ifdef COMPLEX
#ifndef HUFFMAN
    void getCurrentRelatives();
#endif
#ifndef TABLES
    void getNumberBaseAndCurrentRelatives();
#endif
#endif
    uint getNodeNumber();
    clock_t start;
public:
    Decompressor(std::ifstream* infile, std::ofstream* outfile);
    Decompressor(std::ifstream* infile, P *cods, long siz);
    ~Decompressor();
    bool Decompress();
};

//HARDIN / !HARDIN & HARDOUT CONSTRUCTOR
template <class P> Decompressor<P>::Decompressor(std::ifstream* infile, std::ofstream* outfile) {
    start= clock();
    input=infile;
    output=outfile;
    forest=new DJoint<P>();
    forest->nodeNumber=1;
    input->read((char*)&counter, 4);
#ifdef COMPLEX
/*    r_base=sizeof(P)*8;
     read(maxCodeNumber);*/
    input->read((char*)&maxCodeNumber, sizeof(P));
#else
#ifdef CODENUMBER
/*    r_base=sizeof(P)*8;
     read(maxCodeNumber);*/
    input->read((char*)&maxCodeNumber, sizeof(P));
#endif
#endif
    codes=new P[counter];
#ifdef HARDIN
    i_file=input;
#else
    long long offset=input->tellg();
    input->seekg(0, std::ios_base::end);
    size=input->tellg()-offset;
    source=new char[size];
    input->seekg(offset, std::ios_base::beg);
    input->read(source, size);
//    r_buffer=source;
    input->seekg(offset, std::ios_base::beg);
    r_buffer=source+sizeof(P);
#endif
}

//HARDIN / !HARDIN & !HARDOUT CONSTRUCTOR
template <class P> Decompressor<P>::Decompressor(std::ifstream* infile, P *cods, long siz) {
    input=infile;
    forest=new DJoint<P>();
    forest->nodeNumber=2;
    input->read((char*)&counter, 4);
#ifdef COMPLEX
    input->read((char*)&maxCodeNumber, sizeof(P));
#else
#ifdef CODENUMBER
    input->read((char*)&maxCodeNumber, sizeof(P));
#endif
#endif
    siz=counter;
    cods=new P[siz];
    codes=cods;
#ifdef HARDIN
    i_file=input;
#else
    long long offset=input->tellg();
    input->seekg(0, std::ios_base::end);
    size=input->tellg()-offset;
    source=new char[size];
    input->seekg(offset, std::ios_base::beg);
    input->read(source, size);
//    r_buffer=source;
    input->seekg(offset, std::ios_base::beg);
    r_buffer=source+sizeof(P);
#endif
}

template <class P> Decompressor<P>::~Decompressor() {
#ifdef STAT
    std::cout << "Born Joints: " << bornJoints1 << '\n';
    std::cout << "Died Joints: " << diedJoints1 << '\n';
#endif
    if(forest->nodeNumber==1)
        delete[] codes;
    delete forest;
#ifndef HARDIN
    delete[] source;
#endif
}

//HARDIN / !HARDIN & HARDOUT / !HARDOUT
template <class P> bool Decompressor<P>::Decompress() {
    size=counter;
    pattern=codes;
#ifndef COMPLEX
/*    r_base=sizeof(P)*8;
     read(code);*/
    input->read((char*)&code, sizeof(P));
    patternNumber=0;
#else
#ifdef TABLES
    uint8_t items_quantity_base=get_base(maxCodeNumber+1);
    uint8_t items_base=get_base(maxCodeNumber);
    for(int i=0; i<=maxCodeNumber; i++) {
        patterns.push_back(std::vector<DJoint<P> *>());
        relatives.push_back(std::vector<P>());
#ifdef HUFFMAN
        hufBases.push_back(std::vector<int>());
        hufTrees.push_back(std::vector<Node<P>*>());
#endif
        r_base=items_quantity_base;
        uint32_t size;
        read(size);
        if(size) {
#ifdef HUFFMAN
            r_base=items_base;
            if(size == 1) {
                P relative;
                read(relative);
                relatives[i].push_back(relative);
            }
            else if(size == 2) {
                P relative;
                read(relative);
                relatives[i].push_back(relative);
                read(relative);
                relatives[i].push_back(relative);
            }
            else {
                huffman.readHufTable(size, relatives[i], hufBases[i], hufTrees[i]);
            }
#else
            r_base=items_base;
            for(int j=0; j<size; j++) {//size can't be 0
                P relative;
                read(relative);
                relatives[i].push_back(relative);
            }
#endif
        }
    }
#else
    for(int i=0; i<=maxCodeNumber; i++) {
        patterns.push_back(std::vector<DJoint<P> *>());
        relatives.push_back(std::vector<P>());
    }
#endif
    r_base=sizeof(P)*8;
     read(code);
//    input->read((char*)&code, sizeof(P));
    codesAbs.push_back(code);
    codesAbsRel.insert(std::make_pair(code, 0));
    previousRelativeTree=0;
#endif
    keepNode(forest);
    *pattern++=code;
    counter--;
    previousJoint=forest->tree[code];
    previousTree=code;
#ifdef COMPLEX
    previousCode=code;
#endif
    while (counter) {
        cycle();
    };
#ifdef COMPLEX
#ifdef HUFFMAN
    for(int i=0; i<hufTrees.size(); i++) {
        int size=(int)hufTrees[i].size();
        for(int j=size-1 ; j>=0 ; j--)
            delete hufTrees[i][j];
    }
#endif
#endif
    //#ifndef HARDOUT
    output->write((char*)codes, size*sizeof(P));
    //#endif
    double cpu_time=((double)(clock()-start))/CLOCKS_PER_SEC;
    std::cout << "CPU Time: " << cpu_time << '\n';
    return true;
}

//there is no need to grow tree having size of the tree of compressor to cut it...
template <class P> void Decompressor<P>::cycle() {
#ifndef COMPLEX
    if(numberBaseChanges)
#endif
        getNumberBase();
    r_base=numberBase;
    read(currentNumber);
    if(currentNumber<patternNumber)
    {
#ifdef COMPLEX
#ifndef TABLES
        bool isFirstTime=false;
        if(currentNumber+1==patternNumber && relatives[previousRelativeCode].size()!=codesAbs.size())
        {
            getNumberBaseAndCurrentRelatives();
            isFirstTime=true;
        }
        else
#endif
#ifndef HUFFMAN
        getCurrentRelatives();
#endif
#endif
#ifdef STAT1
        std::cout << currentNumber << '\t' << counter <<'\n';
#endif
#ifdef COMPLEX
#ifdef HUFFMAN
        currentJoint=patterns[currentRelativeTree][currentNumber];
#else
        currentJoint=patterns[currentRelativeTree][currentRelativeNumber];
#endif
#else
        currentJoint=patterns[currentNumber];
#endif
/*       DJoint<P> *joint=currentJoint;
        while(joint->level)
        joint=joint->parent;
        nextTree=joint->code;*/
        patternSize=currentJoint->level+1;
        const int pattern_size=patternSize;
        DJoint<P> *joint=currentJoint;
        do {
            pattern[joint->level]=joint->code;
            joint=joint->parent;
        } while (joint->level>=0);//while (joint->level!=-1)
        currentIsSecure=true;
        if(currentJoint->nodeNumber==1) {//don't need to cut currentJoint: it either will become previous or it is previousJoint's base...
#ifdef COMPLEX
#ifdef HUFFMAN
            patterns[currentRelativeTree].erase(patterns[currentRelativeTree].begin()+currentNumber);
#else
            patterns[currentRelativeTree].erase(patterns[currentRelativeTree].begin()+currentRelativeNumber);
#endif
#else
            patterns.erase(patterns.begin()+currentNumber);
#endif
#ifdef STAT1
            std::cout << '-' << currentNumber << '\t' << counter << '\n';
#endif
#ifndef COMPLEX
            numberBaseChanges--;
#endif
            if(previousTree==pattern[0]) {
                currentIsSecure=false;
            }
        }
#ifdef COMPLEX
#ifndef TABLES
        else if(isFirstTime) {
            long number=lower_bound(relatives[previousRelativeCode].begin(), relatives[previousRelativeCode].end(), currentRelativeTree)-relatives[previousRelativeCode].begin();
            if(number<relatives[previousRelativeCode].size())
                relatives[previousRelativeCode].insert(relatives[previousRelativeCode].begin()+number, currentRelativeTree);
            else
                relatives[previousRelativeCode].push_back(currentRelativeTree);
        }
#endif
#endif
        currentJoint->nodeNumber--;
        counter-=patternSize;
        if(!counter) {
            cut(previousJoint);
            if(forest->tree.size())
                cut(currentJoint);
            return;
        }
#ifdef COMPLEX
//*STRONG. comment for excluding STRONG.
        keys.clear();
        if(currentJoint->tree.size()) {
            keys.reserve(currentJoint->tree.size());
        
            for(auto kv : currentJoint->tree) {
                keys.push_back(kv.first);
            }
        }
//*/
#endif
        keepNodes();
        if(canCut)
        {
#ifdef COMPLEX
//*STRONG. comment for excluding STRONG.
            if(keys.size()!=currentJoint->tree.size())//if(keys.size()==currentJoint->tree.size()+1)
            {
                for(int i=0; i<keys.size(); i++) {
                    if(currentJoint->tree.count(keys[i])==0) {
                        keys.erase(keys.begin()+i);
                        break;
                    }
                }
            }
//*/
#endif
            previousJoint=currentJoint;
#ifdef COMPLEX
            previousTree=pattern[0];
            previousRelativeTree=currentRelativeTree;
#endif
        }
#ifdef COMPLEX
        previousCode=currentJoint->code;
#else
        previousTree=pattern[0];
#endif
        pattern+=pattern_size;
    }
    else {
#ifdef STAT1
        std::cout << '+' << currentNumber << '\t' << counter << '\n';
#endif
        r_base=sizeof(P)*8;
        read(code);
        currentIsSecure=true;
        if(!--counter) {
#ifdef COMPLEX
            ++codeNumber;
#else
#ifdef CODENUMBER
            ++codeNumber;
#endif
#endif
            cut(previousJoint);
            *pattern=code;
            return;
        }
/*
        keepNode(forest);
        keepNode(previousJoint);
*/
        keepNode(previousJoint);
#ifdef COMPLEX
//        previousRelativeTree=(P)codesAbs.size();
        previousRelativeTree=++codeNumber;
#endif
        keepNode(forest);
#ifdef COMPLEX
        codesAbs.push_back(code);
        codesAbsRel.insert(std::make_pair(code, previousRelativeTree));
#endif
        previousJoint=forest->tree[code];
#ifndef COMPLEX
#ifdef CODENUMBER
        if(++codeNumber==maxCodeNumber) {
            numberBaseChanges--;
        }
#endif
#endif
        previousTree=code;
#ifdef COMPLEX
        previousCode=code;
#endif
        *pattern++=code;
#ifdef COMPLEX
        keys.clear();//STRONG.
#endif
    }
}

template <class P> void Decompressor<P>::keepNodes() {
    read(bu);
    if(bu) {
        if(patternSize==1) {
            canCut=currentJoint!=previousJoint?true:false;
            code=pattern[0];
            previousJoint->tree.insert(std::make_pair(code, new DJoint<P>(code, previousJoint)));
            previousJoint=previousJoint->tree[code];
            previousJoint->nodeNumber=getNodeNumber();
#ifdef COMPLEX
            patterns[previousRelativeTree].push_back(previousJoint);
#else
            patterns.push_back(previousJoint);
            numberBaseChanges++;
#endif
//            pattern++;
            return;
        }
        DJoint<P> *joint;
        uint number;
        r_base=get_base(patternSize-1);
        read(number);
        if(currentJoint!=previousJoint) {
            canCut=true;
            patternSize-=number+1;
            joint=previousJoint;
            for(int i=0; i<=patternSize; i++) {
                joint->tree.insert(std::make_pair(pattern[i], new DJoint<P>(pattern[i], joint)));
                joint=joint->tree[pattern[i]];
            }
        }
        else {
            canCut=false;
            for(int i=0; i<patternSize; i++) {
                previousJoint->tree.insert(std::make_pair(pattern[i], new DJoint<P>(pattern[i], previousJoint)));
                previousJoint=previousJoint->tree[pattern[i]];
            }
            joint=previousJoint;
            patternSize-=number+1;
            while(number--) {
                joint=joint->parent;
            }
        }
        joint->nodeNumber=getNodeNumber();
#ifdef COMPLEX
        patterns[previousRelativeTree].push_back(joint);
#else
        patterns.push_back(joint);
        numberBaseChanges++;
#endif
//w_base can't be 0: it is at least 1...
        while (patternSize) {
            r_base=get_base(patternSize);
            read(number);
            patternSize-=number+1;
            if(patternSize==-1)
                break;
            do {
                joint=joint->parent;
            } while(number--);
            joint->nodeNumber=getNodeNumber();
#ifdef COMPLEX
            patterns[previousRelativeTree].push_back(joint);
#else
            patterns.push_back(joint);
            numberBaseChanges++;
#endif
        };
    }
    else {
        if(previousJoint!=currentJoint) {
            canCut=true;
            if(!previousJoint->nodeNumber && !previousJoint->tree.size())
            {
                cut(previousJoint);
            }
        }
        else {
            canCut=false;
            for(int i=0; i<patternSize; i++) {
                previousJoint->tree.insert(std::make_pair(pattern[i], new DJoint<P>(pattern[i], previousJoint)));
                previousJoint=previousJoint->tree[pattern[i]];
            }
        }
    }
//    pattern+=patternSize;
}

template <class P> void Decompressor<P>::keepNode(DJoint<P> *joint) {
    read(bu);
    if(bu) {
        joint->tree.insert(std::make_pair(code, new DJoint<P>(code, joint)));
        joint=joint->tree[code];
        joint->nodeNumber=getNodeNumber();
#ifdef COMPLEX
        patterns[previousRelativeTree].push_back(joint);
#else
        patterns.push_back(joint);
        numberBaseChanges++;
#endif
    }
    else {
        if(joint==forest) {
            joint->tree.insert(std::make_pair(code, new DJoint<P>(code, joint)));
        }
/*        else if(canCut && !joint->nodeNumber && !joint->tree.size()) {
            cut(joint);
         }
         else if(!canCut && !joint->nodeNumber) {
            cut(joint);
         }*/
        else if(!joint->nodeNumber && !joint->tree.size()) {
            cut(joint);
        }
    }
//    pattern++;
}

template <class P> void Decompressor<P>::cut(DJoint<P> *leaf) {
    DJoint<P> *parent;
//    currentIsSecure=true;
    if(currentIsSecure) {
        do {
            parent=leaf->parent;
            delete leaf;
            leaf=parent;
        } while(!leaf->nodeNumber && !leaf->tree.size());
    }
    else {
        do {
            parent=leaf->parent;
            delete leaf;
            leaf=parent;
        } while(!leaf->nodeNumber && !leaf->tree.size() && leaf!=currentJoint);
    }
}
#ifndef COMPLEX
template <class P> void Decompressor<P>::getNumberBase() {
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
//calculates numberBase & patternNumber
template <class P> void Decompressor<P>::getNumberBase() {
    previousRelativeCode=codesAbsRel[previousCode];
#ifdef HUFFMAN
    huffman.size=relatives[previousRelativeCode].size();
/*HIND.
    P hufRelativeTree;
    if(huffman.size>2)
        huffman.ReadCode(hufRelativeTree, hufTrees[previousRelativeCode]);
    else
        huffman.ReadCode(hufRelativeTree);
    currentRelativeTree=relatives[previousRelativeCode][hufRelativeTree];
*/
//*HIND.
    if(huffman.size>2)
        huffman.ReadCode(currentRelativeTree, hufTrees[previousRelativeCode]);
    else {
        P hufRelativeTree;
        huffman.ReadCode(hufRelativeTree);
        currentRelativeTree=relatives[previousRelativeCode][hufRelativeTree];
    }
//*/
    
    patternNumber=patterns[currentRelativeTree].size();
#else
    long size=relatives[previousRelativeCode].size();
    patternNumber=0;
    if(keys.empty())//STRONG.
    {
        for(int i=0; i<size; i++) {
/*
            if(codeNumber<relatives[previousRelativeCode][i])
                break;
*/
            patternNumber+=patterns[relatives[previousRelativeCode][i]].size();
        }
    }
//*STRONG.
    else {
        for(int i=0; i<size; i++) {
            P iCode=relatives[previousRelativeCode][i];
//             if(codeNumber<iCode)
//             break;
            if(std::find(keys.begin(), keys.end(), codesAbs[iCode]) == keys.end())
            {
                patternNumber+=patterns[iCode].size();
            }
        }
    }
//*/
#ifndef TABLES
//BGT!    if(size!=codesAbs.size())//this happens nearly always
        patternNumber++;
#endif
#endif
    if(codeNumber==maxCodeNumber) {
        numberBase = get_base((uint32_t)(patternNumber?patternNumber-1:0));
    }
    else
        numberBase = get_base((uint32_t)patternNumber);
}
#ifndef HUFFMAN
//calculates currentRelativeTree & currentRelativeNumber
template <class P> void Decompressor<P>::getCurrentRelatives() {
    currentRelativeNumber=0;
    int i=0;
//this loop will terminate for sure
    if(keys.empty())//STRONG.
    {
        while (currentRelativeNumber+patterns[relatives[previousRelativeCode][i]].size() <= currentNumber)
        {
//            if(codeNumber<relatives[previousRelativeCode][i])
//                break;
                currentRelativeNumber+=patterns[relatives[previousRelativeCode][i++]].size();
        }
        
    }
//*STRONG.
    else {
        for(; i<size; i++) {
            P iCode=relatives[previousRelativeCode][i];
//            if(codeNumber<iCode)
//                break;
            if(std::find(keys.begin(), keys.end(), codesAbs[iCode]) == keys.end())
            {
                if(currentRelativeNumber+patterns[iCode].size() > currentNumber)
                    break;
                currentRelativeNumber+=patterns[iCode].size();
            }
        }
    }
//*/
    currentRelativeNumber=currentNumber-currentRelativeNumber;
    currentRelativeTree = relatives[previousRelativeCode][i];
}
#endif
#ifndef TABLES
//relatives[previousRelativeCode].size()!=codesAbs.size()
template <class P> void Decompressor<P>::getNumberBaseAndCurrentRelatives() {
    std::vector<P> relatives1;
    long size=relatives[previousRelativeCode].size();
//    P i=0;
    int i=0;
    int j=0;
//    P relativeCode=-1;
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
    patternNumber=0;
    if(keys.empty())//STRONG.
    {
        for(i=0; i<size; i++) {
            patternNumber+=patterns[relatives1[i]].size();
        }
    }
//*STRONG.
    else {
        for(i=0; i<size; i++) {
            if(std::find(keys.begin(), keys.end(), codesAbs[relatives1[i]]) == keys.end())
                patternNumber+=patterns[relatives1[i]].size();
        }
    }
//*/
    //patternNumber > 0
    numberBase = get_base((uint32_t)--patternNumber);
    r_base=numberBase;
    read(currentNumber);
    currentRelativeNumber=0;
    i=0;
//this loop will terminate for sure
    if(keys.empty())//STRONG.
    {
        while (currentRelativeNumber+patterns[relatives1[i]].size() <= currentNumber)
        {
            currentRelativeNumber+=patterns[relatives1[i++]].size();
        }
    }
//*STRONG.
    else {
        for(; i<size; i++) {
            if(std::find(keys.begin(), keys.end(), codesAbs[relatives1[i]]) == keys.end())
            {
                if(currentRelativeNumber+patterns[relatives1[i]].size() > currentNumber)
                    break;
                currentRelativeNumber+=patterns[relatives1[i]].size();
            }
        }
    }
//*/
    currentRelativeNumber=currentNumber-currentRelativeNumber;
    currentRelativeTree = relatives1[i];
}
#endif
#endif
template <class P> uint Decompressor<P>::getNodeNumber() {
    uint number=0;
    do {
        read(bu);
        number++;
    } while(bu);
    return number;
}

#endif
