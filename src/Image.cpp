#include "../include/Piet.h"

namespace Piet::Parse {
Image::~Image() {
  for (int row = 0; row < rows; row++) {
    delete[] matrix[row];
  }
  delete[] matrix;
}

void Image::fill(Position *position, Color colour) {
  assert(position->row < rows);
  assert(position->column < columns);

  matrix[position->row][position->column] = colour;
}

void Image::markOwner(Position *position, uint32_t owner) {
  assert(position->row < rows);
  assert(position->column < columns);

  size_t offset = position->row * columns + position->column;
  assert(cellOwners[offset] == 0);

  cellOwners[offset] = owner;
}

Color Image::at(Position *position) {
  return matrix[position->row][position->column];
}

uint32_t Image::ownerAt(Position *position) {
  return cellOwners[position->row * columns + position->column];
}

bool Image::in(Position *position) {
  return position->row < rows && position->column < columns;
}
} // namespace Piet::Parse
