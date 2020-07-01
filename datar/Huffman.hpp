//
//  Huffman.hpp
//  datar
//
//  Created by Vahagn Nurijanyan on 2017-01-04.
//  Copyright © 2017 BABELONi INC. All rights reserved.
//

#ifndef Huffman_hpp
#define Huffman_hpp

#include <stdio.h>
#include <vector>
#include "io.hpp"

/*
1
01
000
0010
00110
001110
0011110
0011111
*/

template <typename P> struct HuffmanSort {
    int count;
    P index;
    P code;
};

template <typename P> bool operator<(const HuffmanSort<P> &s1, const HuffmanSort<P> &s2)
{
    return s1.count<s2.count;
}

template<class P> class Node
{
public:
    bool Side;
    /*
     false - left
     true - right
     */
    bool Leaf;
    /*
     false - node
     true - leaf
     */
    unsigned int Gen;
    Node<P>* N0;
    Node<P>* N1;
    P Value;
    //
    Node(bool side,unsigned int gen=1);
    void Divide(Node<P>** node0,Node<P>** node1);
};
template <class P> Node<P>::Node(bool side,unsigned int gen)
{
    Leaf=false;
    Side=side;
    Gen=gen;
    N0=N1=NULL;
}
template <class P> void Node<P>::Divide(Node<P>** node0,Node<P>** node1)
{
    N0=new Node(false,Gen+1);
    N1=new Node(true,Gen+1);
    *node0=N0;
    *node1=N1;
}

template<class P> class Huffman {
    std::vector<HuffmanSort<P>> hufSort;
    void huf(int A[], int n);
    void hufBase(std::vector<P> &relative, std::vector<int> &huf_base);
    void hufLeaf(std::vector<uint32_t> &huf_leaf);
    void writeBits(int number);
    void hufTree(std::vector<Node<P>*>& huf_tree);
    int readBits();
public:
    Huffman<P>();
    long size;
    void writeHufTable(std::vector<P> &relative, std::vector<int> &huf_base, std::vector<uint32_t> &huf_leaf);
    void writeCode(int base, uint32_t leaf);
    void readHufTable(uint32_t size, std::vector<P> &relative, std::vector<int> &huf_base, std::vector<Node<P>*>& huf_tree);
    void ReadCode(P& table_number);
    void ReadCode(P& table_number, std::vector<Node<P>*>& tree);
};

template <class P> Huffman<P>::Huffman()
{
    
}

#pragma MARK - Compress

template <class P> void Huffman<P>::huf(int A[], int n)
{
    int root;                  /* next root node to be used */
    int leaf;                  /* next leaf to be used */
    int next;                  /* next value to be assigned */
    int avbl;                  /* number of available nodes */
    int used;                  /* number of internal nodes */
    int dpth;                  /* current depth of leaves */
    /* check for pathological cases */
    if(n==0)
        return;
    if(n==1)
    {
        A[0]=0;
        return;
    }
    /* first pass, left to right, setting parent pointers */
    A[0]+=A[1];
    root=0;
    leaf=2;
    for(next=1; next<n-1; next++)
    {
        /* select first item for a pairing */
        if(leaf>=n || A[root]<A[leaf])
        {
            A[next]=A[root];
            A[root++]= next;
        }
        else
            A[next]=A[leaf++];
        /* add on the second item */
        if(leaf>=n || (root<next && A[root]<A[leaf]))
        {
            A[next]+=A[root];
            A[root++]=next;
        }
        else
            A[next]+=A[leaf++];
    }
    /* second pass, right to left, setting internal depths */
    A[n-2]=0;
    for(next=n-3; next>=0; next--)
        A[next]=A[A[next]]+1;
    /* third pass, right to left, setting leaf depths */
    avbl=1;
    used=dpth=0;
    root=n-2;
    next=n-1;
    while(avbl>0)
    {
        while(root>=0 && A[root]==(unsigned long)dpth)
        {
            used++;
            root--;
        }
        while(avbl>used)
        {
            A[next--]=dpth;
            avbl--;
        }
        avbl=used+used;
        dpth++;
        used=0;
    }
}

template <class P> void Huffman<P>::hufBase(std::vector<P> &relative, std::vector<int> &huf_base)
{
    size=relative.size();
    //size > 2
    /*        if(size==1) {
     huf_base[0]=0;
     return;
     }
     else if(size==2)
     {
     huf_base[0]=1;
     huf_base[1]=1;
     return;
     }*/
    HuffmanSort<P> temp;
    for(int i=0; i<size; i++)
    {
        temp.count=huf_base[i];
        temp.index=i;
        temp.code=relative[i];
        hufSort.push_back(temp);
    }
    sort(hufSort.begin(), hufSort.end());
    int huf_b[size];
    for(int i=0; i<size; i++)
        huf_b[i]=hufSort[i].count;
    huf(huf_b, (int)size);
    for(int i=0; i<size; i++) {
        huf_base[hufSort[i].index]=huf_b[i];
        hufSort[i].count=huf_b[i];
    }
    reverse(hufSort.begin(), hufSort.end());
}

//size>2
template <class P> void Huffman<P>::hufLeaf(std::vector<uint32_t> &huf_leaf)
{
//    int tree[size]={0};
    int *tree=new int[size]();
    int h=1;
    int code;
    int len;
    huf_leaf.resize(size);
//    reverse(hufSort.begin(), hufSort.end());
    for(unsigned long i=0; i<size; i++)
    {
        len=hufSort[i].count;
        if(len!=0)
        {
            if(len!=h)
                for(; h<len; h++)
                    tree[h]=0;
            code=tree[0]<<(h-1);
            for(int j=1; j<h; j++)
                code+=tree[j]<<(h-j-1);
            huf_leaf[hufSort[i].index]=code;
            do
            {
                if(tree[h-1]==0)
                {
                    tree[h-1]=1;
                    break;
                }
                else
                    h--;
            }while(h!=0);
        }
    }
    hufSort.clear();
    delete[] tree;
}

template <class P> void Huffman<P>::writeHufTable(std::vector<P> &relative, std::vector<int> &huf_base, std::vector<uint32_t> &huf_leaf) {
    hufBase(relative, huf_base);
    //LS. writes 1 bit more...        int number=0;
    int number=1;//LS.
    int i=0;
    do {
        //number > 0 always
        number=hufSort[i].count-number;
        writeBits(number);
        do {
            write(false);
            write(hufSort[i].code);
//if HUFFMAN is defined, compressor's tables' order is different than decompressor's one: if HUFFMAN isn't defined, relatives are arranged according their appearance in compressor & decompressor, else relatives are arranged according their appearance in compressor but in decompressor they are arran according their quantity(papularity).appearance
        } while(++i<size && hufSort[i-1].count==hufSort[i].count);
        number = hufSort[i-1].count;
    } while(i<size);
    hufLeaf(huf_leaf);
}

template <class P> void Huffman<P>::writeCode(int base, uint32_t leaf) {
    w_base=base;
    write(leaf);
}
template <class P> void Huffman<P>::writeBits(int number) {
    for (int i=0; i<number; i++) {
        write(true);
    }
    //    write(false);
}

#pragma MARK - Decompress

//size>2
template <class P> void Huffman<P>::hufTree(std::vector<Node<P>*>& huf_tree)
{
    Node<P>* temp0=new Node<P>(false);
    huf_tree.push_back(temp0);
    int i=0;
    Node<P>* temp1;
//    if(i!=size-1)
    {
        temp1=new Node<P>(true);
        huf_tree.push_back(temp1);
    }
    Node<P>* temp;
    unsigned long size;
    unsigned long j=0;
    while(1)
    {
        size=huf_tree.size();
        for(;j<size;j++)
        {
            temp=huf_tree[j];
            if(temp->Gen==hufSort[i].count)
            {
                temp->Leaf=true;
//HIND.                temp->Value=hufSort[i++].index;
                temp->Value=hufSort[i++].code;//HIND.
            }
            else
                break;
        }
        if(j==size)
            break;
        for(unsigned long k=j;k<size;k++)
        {
            huf_tree[k]->Divide(&temp0,&temp1);
            huf_tree.push_back(temp0);
            huf_tree.push_back(temp1);
        }
        j=size;
    }
    hufSort.clear();
}

template <class P> void Huffman<P>::readHufTable(uint32_t size, std::vector<P> &relative, std::vector<int> &huf_base, std::vector<Node<P>*>& huf_tree) {
    this->size=size;
    //LS.        int length=0;
    int length=1;//LS.
    int d_length=readBits();
    int i=0;
    do {
        P code;
        HuffmanSort<P> temp;
        length+=d_length;
        huf_base.push_back(length);
        read(code);
        relative.push_back(code);
        temp.count=length;
//HIND.        temp.index=i;
        temp.code=code;
        hufSort.push_back(temp);
        while(++i<size && !(d_length=readBits())) {
            huf_base.push_back(length);
            read(code);
            relative.push_back(code);
            temp.count=length;
//HIND.            temp.index=i;
            temp.code=code;
            hufSort.push_back(temp);
        };
    } while(i<size);
    //hufSort is already sorted...        sort(hufSort.begin(), hufSort.end());
    hufTree(huf_tree);
}

//size==1 or size==2
template <class P> void Huffman<P>::ReadCode(P& table_number)
{
    if(size==1) {
        table_number=0;
        return;
    }
    else// if(size==2)
    {
        bool bit;
        read(bit);
        if(bit)
            table_number=1;
        else
            table_number=0;
        return;
    }
}

//size>2
template <class P> void Huffman<P>::ReadCode(P& table_number, std::vector<Node<P>*>& huf_tree)
{
    bool bit;
/*
    if(size==1) {
        table_number=0;
        return;
    }
    else if(size==2) {
        read(bit);
        if(bit)
            table_number=1;
        else
            table_number=0;
        return;
    }
*/
    Node<P>* node;
    read(bit);
    if(bit)
        node=huf_tree[1];
    else
        node=huf_tree[0];
    while(!node->Leaf)
    {
        read(bit);
        if(bit)
            node=node->N1;
        else
            node=node->N0;
    }
    table_number=node->Value;
}

template <class P> int Huffman<P>::readBits() {
    int number=0;
    bool bit;
    read(bit);
    while(bit) {
        number++;
        read(bit);
    };
    return number;
}
#endif /* Huffman_hpp */
