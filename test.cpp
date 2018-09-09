#include <iostream>
#include <cstdio>
#include <cassert>
#include <vector>
#include <stack>
#include <png.h>
#include "include/Piet.h"

using namespace std;

int main(int argc, char **argv) {
    using namespace std;

    Piet::Parse::Reader reader;

    auto image = reader.readFromFile(argv[1]);
    cout << "read image" << endl;
    auto parser = new Piet::Parse::Parser{image};
    auto graph = parser->parse();

    return 0;
}
