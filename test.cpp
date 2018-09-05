#include <iostream>
#include <cstdio>
#include <cassert>
#include <png.h>

using namespace std;

// struct point?
// (uint, uint) = ParserImage::position()
// (numberOfCells, previousColor, newColor) = ParserImage::connectedCell(directionPointer, codelChooser)

namespace Piet {
    enum Color {
        /**
         * @brief The control colour is used to override the colour of a
         * block of pixels and later reset the colour. This effectively marks
         * each pixel visited, preventing the duplication of the pixel block,
         * e.g. a visited and an unvisited matrix.
         */
        Control = 0x999999,

        White = 0xFFFFFF,
        Black = 0x000000,

        LightRed = 0xFFC0C0,
        Red = 0xFF0000,
        DarkRed = 0xC00000,

        LightYellow = 0xFFFFC0,
        Yellow = 0xFFFF00,
        DarkYellow = 0xC0C000,

        LightGreen = 0xC0FFC0,
        Green = 0x00FF00,
        DarkGreen = 0x00C000,

        LightCyan = 0xC0FFFF,
        Cyan = 0x00FFFF,
        DarkCyan = 0x00C0C0,

        LightBlue = 0xC0C0FF,
        Blue = 0x0000FF,
        DarkBlue = 0x0000C0,

        LightMagenta = 0xFFC0FF,
        Magenta = 0xFF00FF,
        DarkMagenta = 0xC000C0,
    };

    enum DirectionPointerDirection {
        DPUp,
        DPDown,
        DPLeft,
        DPRight,
    };

    enum CodelChooserDirection {
        CCLeft,
        CCRight,
    };

    enum ChooseCodel {
        LeftLowermost,
        LeftUppermost,
        RightLowermost,
        RightUppermost,
        UpLeftmost,
        UpRightmost,
        DownLeftmost,
        DownRightmost,
    };

    namespace Parse {
        struct Position {
            uint32_t row;
            uint32_t column;
        };

        class Image {
        public:
            Image(Color ** matrix, uint32_t rows, uint32_t columns) 
                : matrix(matrix), rows(rows), columns(columns), position{} {}

            ~Image();

            void fill(Position *position, Color colour);

            Color at(Position *position);

        private:
            Color ** matrix;
            uint32_t rows;
            uint32_t columns;
            Position *position;
        };

        class Reader {
        public:
            Image *readFromFile(string filename);
        };
        
        namespace Read {
            class PNG {
            public:
                Image *readFromPNGFile(FILE *png);
            private:
                bool isValidPNGFile(FILE *png, png_structp *png_ptr, png_infop *info_ptr);
            };
        }

        struct Step {
            Color previous;
            Color next;
            uint32_t numberOfCells;
        };

        class Parser {
        public:
            Parser(Image *image) : image(image) {}
            Piet::Parse::Graph *parse();
        private:
            Image *image;
            DirectionPointerDirection dpDirection = DPRight;
            CodelChooserDirection ccDirection = CCLeft;
            // ChooseCodel 
        };

        class Graph {

        };
    }

    namespace Translate {
        class LLVM {
        public:
            LLVM(Piet::Parse::Parser *parser) : parser(parser) {}
            void translateToObjectFile(string outputFilename);
        private:
            Piet::Parse::Parser *parser;
        };
    }
}

Piet::Parse::Image::~Image() {
    for (int row = 0; row < rows; row++) {
        delete [] matrix[row];
    }
    delete [] matrix;
}

void Piet::Parse::Image::fill(Position *position, Color colour) {
    assert(position->row < rows);
    assert(position->column < columns);

    matrix[position->row][position->column] = colour;
}

Piet::Parse::Image *Piet::Parse::Reader::readFromFile(string filename) {
    FILE *png = fopen(filename.c_str(), "rb");
    if (!png) {
        cout << "Unable to open test png file" << endl;
        exit(1);
    }

    // FIXME: add more file formats.

    Piet::Parse::Read::PNG pngReader;
    Image *pietImage = pngReader.readFromPNGFile(png); 

    fclose(png);
    
    return pietImage;
}

Piet::Parse::Image *Piet::Parse::Read::PNG::readFromPNGFile(FILE *png) {
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

    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, nullptr, nullptr, nullptr, nullptr, nullptr);

    // Read in the entire file.
    png_read_update_info(png_ptr, info_ptr);
  
    png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  
    auto image_data = new unsigned char;
    if ((image_data = (unsigned char *)malloc(rowbytes * height)) == nullptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(png);
        exit(1);
    }

    png_bytep row_pointers[height];
    for (png_uint_32 row = 0; row < height; row++) {
        row_pointers[row] = image_data + row * rowbytes;
    }
    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, nullptr);

    // Translate the PNG data into a color matrix.
    Color ** matrix = new Color * [height];
    for (int row = 0; row < height; row++) {
        matrix[row] = new Color [width];

        for (int column = 0; column < width; column++) {
            png_bytep pixelPointer = &row_pointers[row][column * 3];
            uint32_t pixelRgb = (pixelPointer[0] << 16) + (pixelPointer[1] << 8) + pixelPointer[2];

            // Ensure that the pixel is one of the valid colours.
            // We are not checking for the control colour because that is not a valid Piet colour. 
            if (pixelRgb != White && pixelRgb != Black 
                && pixelRgb != LightRed && pixelRgb != Red && pixelRgb != DarkRed
                && pixelRgb != LightGreen && pixelRgb != Green && pixelRgb != DarkGreen
                && pixelRgb != LightBlue && pixelRgb != Blue && pixelRgb != DarkBlue
                && pixelRgb != LightMagenta && pixelRgb != Magenta && pixelRgb != DarkMagenta
                && pixelRgb != LightCyan && pixelRgb != Cyan && pixelRgb != DarkCyan
                && pixelRgb != LightYellow && pixelRgb != Yellow && pixelRgb != DarkYellow)
            {
                cout << "Invalid pixel detected" << endl;
                exit(1);
            }

            matrix[row][column] = (Color)pixelRgb;
        }
    }

    free(image_data);

    // Cleanup: close file, structs and matrix.
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    
    Piet::Parse::Image *pietImage = new Image(matrix, height, width);
    return pietImage;
}

bool Piet::Parse::Read::PNG::isValidPNGFile(FILE *png, png_structp *png_ptr, png_infop *info_ptr) {
    // Check for the PNG header.
    u_char header[8];
    fread(header, 1, 8, png);
    if (!png_check_sig(header, 8)) {
        return false;
    }

    // Read in the PNG structs.
    *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
      NULL);
    if (!png_ptr) {
        return false;
    }
  
    *info_ptr = png_create_info_struct(*png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&(*png_ptr), NULL, NULL);
        return false;
    }

    return true;
}

Piet::Parse::Graph *Piet::Parse::Parser::parse() {
    // Create a new graph node.
    //
    // Fill the codel with the control color.
    // Whilst doing so:
    //  - collect the color of the codel => node.color
    //  - count the number of pixels => node.size
    //  - mark the position (x, y) as belonging the current codel block
    //  - collect the 8 exit points of the codel block, which lie outside the codel block
    //      Each exit point must be marked with the Direction Pointer and Codel Chooser
    //      combination.


    // For each 8 exit points that are unvisited, create the graph node starting from that point.


    // Result: an array of unconnected graph nodes.
    // For every unconnected graph node:
    //      For every exit point:
    //          If the exit point is within the graph,
    //          connect the edge of the unconnected node to the node of the exit point
    //          using the DP + CC pair of the exit point.
    //          Add no change directive to the edge.
    //
    //          If the exit point is not within the graph, get the next
    //          exit point (in DP + CC order of rotation) that is within the graph
    //          and connect the edge of the unconnected node to the node of the
    //          exit point. Add a change directive to the edge, matching the DP + CC
    //          pair of the exit point within the graph.
    //
    //          If there is no exit point within the graph, mark the node as a terminal
    //          node and mark it connected.
}

int main() {
    using namespace std;

    Piet::Parse::Reader reader;
    auto image = reader.readFromFile("PietHello.png");

    return 0;
}
