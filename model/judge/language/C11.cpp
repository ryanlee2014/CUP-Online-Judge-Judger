//
// Created by Haoyuan Li on 2019/12/24.
//

#include "C11.h"
#include <unistd.h>

void C11::run(int memory) {
    execl("./Main", "./Main", (char *) nullptr);
}

extern "C" Language* createInstancec11() {
    return new C11;
}

extern "C" void destroyInstancec11(Language* language) {
    delete language;
}
