#include "../include/Piet.h"

namespace Piet::Parse {
void Image::fill(Position position, Color colour) {
  matrix.at(position.row).at(position.column) = colour;
}

void Image::markOwner(Position position, uint32_t owner) {
  size_t offset = position.row * columns + position.column;
  assert(cellOwners.at(offset) == 0);

  cellOwners.at(offset) = owner;
}

Color Image::at(Position position) {
  return matrix.at(position.row).at(position.column);
}

uint32_t Image::ownerAt(Position position) {
  return cellOwners.at(position.row * columns + position.column);
}

bool Image::in(Position position) {
  return position.row < rows && position.column < columns;
}
} // namespace Piet::Parse
