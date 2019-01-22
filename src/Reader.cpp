#include "../include/Piet.h"
#include <iostream>

namespace Piet::Parse {
Image *Reader::readFromFile(string filename, uint32_t codelSize) {
  FILE *png = fopen(filename.c_str(), "rb");
  if (!png) {
    cout << "Unable to open test png file" << endl;
    exit(1);
  }

  // FIXME: add more file formats.

  Read::PNG pngReader;
  Image *pietImage = pngReader.readFromPNGFile(png, codelSize);

  fclose(png);

  return pietImage;
}
} // namespace Piet::Parse
