#include "../include/Piet.h"

namespace Piet::Parse {
    Image::~Image() {
        for (int row = 0; row < rows; row++) {
            delete [] matrix[row];
        }
        delete [] matrix;
    }

    void Image::fill(Position *position, Color colour) {
        assert(position->row < rows);
        assert(position->column < columns);

        matrix[position->row][position->column] = colour;
    }

    void Image::markOwner(Position *position, uint32_t owner) {
        assert(position->row < rows);
        assert(position->column < columns);

        cellOwners[position->row * position->column + position->column] = owner;
    }

    Color Image::at(Position *position) {
        return matrix[position->row][position->column];
    }

    uint32_t Image::ownerAt(Position *position) {
        return cellOwners[position->row * position->column + position->column];
    }

    bool Image::in(Position *position) {
        return position->row < rows && position->column < columns;
    }
}
