//
//  main.cpp
//  datar
//
//  Created by Vahagn Nurijanyan on 2016-08-13.
//  Copyright © 2016 BABELONi INC. All rights reserved.
//

#include <iostream>
#include "Datar.hpp"

int main(int argc, const char * argv[]) {
    if(argc == 1) {
        std::cout << "Needs source file!\n";
        return 1;
    }
    Datar *datar;
    char defaultType[]="z";
    char defaultBasis[]="8";
    const char *type;
    const char *basis;
    if(argc == 2) {
        type=defaultType;
        basis=defaultBasis;
    }
    else if(argc == 3) {
        type=argv[2];
        basis=defaultBasis;
    }
    else {
        type=argv[2];
        basis=argv[3];
    }
    datar = new Datar(argv[1], type, basis);
    if(datar -> type == -1) {
        std::cout << "Wrong source file!\n";
        delete datar;
        return 3;
    }
    std::cout << argv[1] << '\n';
    delete datar;
    return 0;
}
