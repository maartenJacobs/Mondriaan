#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <png.h>

using namespace std;

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

    enum DirectionPoint : uint8_t {
        TopLeft = 0,
        TopRight,
        RightTop,
        RightBottom,
        BottomRight,
        BottomLeft,
        LeftBottom,
        LeftTop,
    };

    namespace Parse {
        struct Position {
            uint32_t row;
            uint32_t column;
        };

        class Image {
        public:
            Image(Color ** matrix, uint32_t rows, uint32_t columns) 
                : matrix(matrix), rows(rows), columns(columns), position{} {
                    cellOwners = (uint32_t *)malloc(sizeof(uint32_t) * rows * columns);
                    for (uint32_t i = 0; i < rows * columns; i++) {
                        cellOwners[i] = 0;
                    }
                }

            ~Image();

            void fill(Position *position, Color colour);

            void markOwner(Position *position, uint32_t owner);

            Color at(Position *position);

            uint32_t ownerAt(Position *position);

            bool in(Position *position);

        private:
            Color ** matrix;
            uint32_t * cellOwners;
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

        class Graph;
        class GraphNode;
        class GraphEdge;

        class Parser {
        public:
            Parser(Image *image) : image(image) {}
            Piet::Parse::Graph *parse();
        private:
            Image *image;
        };

        class Graph {
        public:
            Graph(vector<GraphNode *> nodes, GraphNode *initialNode)
                : nodes(nodes), initialNode(initialNode) {}
        private:
            vector<GraphNode *> nodes;
            GraphNode *initialNode;
        };

        class GraphEdge {
        public:
            GraphEdge(DirectionPoint *newDirection)
                : newDirection(newDirection), target(nullptr) {}
            GraphEdge(DirectionPoint *newDirection, GraphNode *target)
                : newDirection(newDirection), target(target) {}

            bool isRedirect();
        private:
            DirectionPoint *newDirection;
            GraphNode *target;
        };

        class GraphNode {
        public:
            GraphNode(Color color, uint32_t size) : color(color), size(size) {}
            void markAsInitial();
            void markAsTerminal();
            void connect(DirectionPoint direction, GraphEdge *edge);
            GraphEdge *edgeForDirection(DirectionPoint direction);
        private:
            Color color;
            uint32_t size;
            unordered_map<DirectionPoint, GraphEdge *> edges;
            bool isTerminal = false;
            bool isInitial = false;
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
