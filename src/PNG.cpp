#include "../include/Piet.h"
#include <iostream>
#include <png.h>

namespace Piet::Parse::Read {
Image *PNG::readFromPNGFile(FILE *png, uint32_t codelSize) {
  // Check if the PNG file is valid.
  png_structp png_ptr;
  png_infop info_ptr;
  if (!isValidPNGFile(png, &png_ptr, &info_ptr)) {
    fclose(png);
    exit(1);
  }

  // Do actual things with the file.

  png_init_io(png_ptr, png);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  png_uint_32 imageWidth, imageHeight;
  int depth, colorType; // depth 8, color type 2
  png_get_IHDR(png_ptr, info_ptr, &imageWidth, &imageHeight, &depth, &colorType,
               nullptr, nullptr, nullptr);
  if (depth != 8 || colorType != 2) {
    cout << "Expected bit depth 8 and color type 2, but received bit depth "
         << depth << " and color type " << colorType;
    cout << endl;
    exit(1);
  }
  if ((imageWidth % codelSize) != 0 || (imageHeight % codelSize) != 0) {
    throw CodelMismatchException{};
  }

  // Scale the dimensions down based on the codel size.
  auto matrixWidth = (uint32_t)(imageWidth / codelSize);
  auto matrixHeight = (uint32_t)(imageHeight / codelSize);

  // Read in the entire file.
  png_read_update_info(png_ptr, info_ptr);

  png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  unsigned char *image_data;
  if ((image_data = (unsigned char *)malloc(rowbytes * imageHeight)) ==
      nullptr) {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(png);
    exit(1);
  }

  png_bytep row_pointers[imageHeight];
  for (png_uint_32 row = 0; row < imageHeight; row++) {
    row_pointers[row] = image_data + row * rowbytes;
  }
  png_read_image(png_ptr, row_pointers);
  png_read_end(png_ptr, nullptr);

  // Translate the PNG data into a color matrix.
  auto matrix = new Color *[matrixHeight];
  for (int row = 0; row < imageHeight; row += codelSize) {
    // Remember to downscale based on codel size. The image is using codels of
    // size codelSize by codelSize, so we must downscale before creating a row.
    matrix[row / codelSize] = new Color[matrixWidth];

    for (int column = 0; column < imageWidth; column += codelSize) {
      png_bytep pixelPointer = &row_pointers[row][column * 3];
      uint32_t pixelRgb =
          (pixelPointer[0] << 16) + (pixelPointer[1] << 8) + pixelPointer[2];

      // Ensure that the pixel is one of the valid colours.
      // We are not checking for the control colour because that is not a valid
      // Piet colour.
      if (pixelRgb != White && pixelRgb != Black && pixelRgb != LightRed &&
          pixelRgb != Red && pixelRgb != DarkRed && pixelRgb != LightGreen &&
          pixelRgb != Green && pixelRgb != DarkGreen && pixelRgb != LightBlue &&
          pixelRgb != Blue && pixelRgb != DarkBlue &&
          pixelRgb != LightMagenta && pixelRgb != Magenta &&
          pixelRgb != DarkMagenta && pixelRgb != LightCyan &&
          pixelRgb != Cyan && pixelRgb != DarkCyan && pixelRgb != LightYellow &&
          pixelRgb != Yellow && pixelRgb != DarkYellow) {
        cout << "Invalid pixel detected at (" << row << ", " << column
             << "). Color value: " << hex << pixelRgb << dec << endl;
        exit(1);
      }

      // The image can be larger than the matrix, so we need to downscale when
      // storing the codel.
      matrix[row / codelSize][column / codelSize] = (Color)pixelRgb;
    }
  }

  free(image_data);

  // Cleanup: close file, structs and matrix.
  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  auto pietImage = new Image(matrix, matrixHeight, matrixWidth);
  return pietImage;
}

bool PNG::isValidPNGFile(FILE *png, png_structp *png_ptr, png_infop *info_ptr) {
  // Check for the PNG header.
  u_char header[8];
  fread(header, 1, 8, png);
  if (!png_check_sig(header, 8)) {
    return false;
  }

  // Read in the PNG structs.
  *png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!(*png_ptr)) {
    return false;
  }

  *info_ptr = png_create_info_struct(*png_ptr);
  if (!(*info_ptr)) {
    png_destroy_read_struct(&(*png_ptr), nullptr, nullptr);
    return false;
  }

  return true;
}
} // namespace Piet::Parse::Read
