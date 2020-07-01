//
//  Datar.h
//  datarz8
//
//  Created by Vahagn Nurijanyan on 2015-09-19.
//  Copyright (c) 2015 BABELONi INC. All rights reserved.
//

#ifndef Datar_h
#define Datar_h

class Datar {
    bool isCompressor;
public:
    std::ifstream* infile;
    std::ofstream* outfile=NULL;
    char *destName=NULL;
    int type;
    Datar(const char* src, const char* tp, const char* bs);
    ~Datar();
};
#endif
