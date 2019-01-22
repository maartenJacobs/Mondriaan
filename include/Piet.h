#ifndef PIET_H
#define PIET_H

#include <array>
#include <cassert>
#include <cstdio>
#include <exception>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <png.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace Piet {
enum DirectionPoint : uint8_t {
  TopLeft = 0, // DP = top ; CC = left
  TopRight,    // DP = top ; CC = right
  RightTop,    // DP = right ; CC = left
  RightBottom, // DP = right ; CC = right
  BottomRight, // DP = down ; CC = left
  BottomLeft,  // DP = down ; CC = right
  LeftBottom,  // DP = left ; CC = left
  LeftTop,     // DP = left ; CC = right
};

DirectionPoint incrementDirectionPointer(DirectionPoint direction);
} // namespace Piet

namespace std {
template <> struct hash<Piet::DirectionPoint> {
  size_t operator()(const Piet::DirectionPoint &x) const {
    return hash<int>()(x);
  }
};
} // namespace std

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

const uint8_t MIN_DIRECTION_POINT = TopLeft;
const uint8_t MAX_DIRECTION_POINT = LeftTop;

enum HueChange {
  HueChange0,
  HueChange1,
  HueChange2,
  HueChange3,
  HueChange4,
  HueChange5,
};

enum LightnessChange {
  LightnessChange0,
  LightnessChange1,
  LightnessChange2,
};

namespace Parse {
struct Position {
  uint32_t row;
  uint32_t column;
};

class Image {
public:
  Image(Color **matrix, uint32_t rows, uint32_t columns)
      : matrix(matrix), rows(rows), columns(columns) {
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
  Color **matrix;
  uint32_t *cellOwners;
  uint32_t rows;
  uint32_t columns;
};

class Reader {
public:
  Image *readFromFile(string filename, uint32_t codelSize = 1);
};

namespace Read {
struct CodelMismatchException : public std::exception {
  const char *what() const noexcept override {
    return "Image size does not seem to match specified codel size";
  }
};

class PNG {
public:
  Image *readFromPNGFile(FILE *png, uint32_t codelSize = 1);

private:
  bool isValidPNGFile(FILE *png, png_structp *png_ptr, png_infop *info_ptr);
};
} // namespace Read

class Graph;
class GraphNode;
class GraphEdge;

/**
 * @brief Piet::Parser parses a loaded image into a graph representing the flow
 * of the Piet program.
 */
class Parser {
public:
  explicit Parser(Image *image) : image(image) {}
  Piet::Parse::Graph *parse();

private:
  Image *image;
};

struct GraphStep {
  GraphNode *previous;
  GraphNode *current;
  bool skipTransition = false;
};

class Graph {
public:
  Graph(vector<GraphNode *> nodes, GraphNode *initialNode)
      : nodes(nodes), initialNode(initialNode) {}
  GraphStep *walk();
  void restartWalk(GraphNode *fromNode, DirectionPoint inDirection);
  DirectionPoint getCurrentDirection();
  GraphNode *getInitialNode();
  GraphNode *getCurrentNode();

private:
  vector<GraphNode *> nodes;
  GraphNode *initialNode = nullptr;
  GraphNode *currentNode = nullptr;
  DirectionPoint currentDirection = RightTop;
};

class GraphEdge {
public:
  explicit GraphEdge(DirectionPoint *newDirection)
      : newDirection(newDirection), target(nullptr) {
    assert(*newDirection >= MIN_DIRECTION_POINT &&
           *newDirection <= MAX_DIRECTION_POINT);
  }
  GraphEdge(DirectionPoint *newDirection, GraphNode *target, bool noop)
      : newDirection(newDirection), target(target), noop(noop) {
    assert(*newDirection >= MIN_DIRECTION_POINT &&
           *newDirection <= MAX_DIRECTION_POINT);
  }

  bool isRedirect();

  DirectionPoint *getNewDirection();
  GraphNode *getTarget();
  bool isNoop();

private:
  DirectionPoint *newDirection;
  GraphNode *target;
  bool noop = false;
};

class GraphNode {
public:
  GraphNode(Color color, uint32_t size, string identifier)
      : color(color), size(size), identifier(identifier) {}
  void markAsInitial();
  void markAsTerminal();
  bool isTerminal();
  bool isInitial();
  void connect(DirectionPoint direction, GraphEdge *edge);
  GraphEdge *edgeForDirection(DirectionPoint direction);
  Color getColor();
  uint32_t getSize();
  string getIdentifier();

private:
  Color color;
  uint32_t size;
  string identifier;
  unordered_map<DirectionPoint, GraphEdge *> edges;
  bool terminal = false;
  bool initial = false;
};
} // namespace Parse

class ColorTransition {
public:
  ColorTransition(HueChange hueChange, LightnessChange lightnessChange)
      : hueChange(hueChange), lightnessChange(lightnessChange) {}

  /**
   * @brief Determine the transition that occurs between 2 graph nodes.
   * @param previous
   * @param current
   * @return NULL if there is no transition. An instance of ColorTransition
   * otherwise.
   */
  static ColorTransition *determineTransition(Parse::GraphNode *previous,
                                              Parse::GraphNode *current);
  HueChange getHueChange();
  LightnessChange getLightnessChange();

private:
  HueChange hueChange;
  LightnessChange lightnessChange;
};

// FIXME: what can you do with this externally?
typedef string OpKeyType;
const string OP_NOOP = "noop", OP_PUSH = "push", OP_POP = "pop", OP_ADD = "add",
             OP_SUBTRACT = "subtract", OP_MULTIPLY = "multiply",
             OP_DUPLICATE = "duplicate", OP_OUT_CHAR = "out(char)",
             OP_OUT_NUMBER = "out(number)", OP_POINTER = "pointer",
             OP_SWITCH = "switch", OP_IN_NUMBER = "in(number)",
             OP_DIVIDE = "divide", OP_ROLL = "roll";

class Translator {
public:
  explicit Translator(Parse::Graph *graph)
      : builder(context), module(llvm::Module("piet", context)), graph(graph) {}

  /*!
   * @brief Translate the graph to an executable file or LLVM IR code.
   * @param filename The output filename.
   * @param onlyIR Limit output to LLVM IR code.
   */
  void translateToExecutable(string filename, bool onlyIR);

private:
  void translateIRToExecutable(string objectFilename);
  llvm::Function *translateBranch(Parse::GraphNode *node, DirectionPoint dp);
  void registerPietGlobals();

  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  llvm::Module module;
  Parse::Graph *graph;
  unordered_map<string, llvm::Function *> translatedBranches;
  const array<array<OpKeyType, 3>, 6> operationTable = {
      array<OpKeyType, 3>{OP_NOOP, OP_PUSH, OP_POP},
      {OP_ADD, OP_SUBTRACT, OP_MULTIPLY},
      {OP_DIVIDE, "mod", "not"},
      {"greater", OP_POINTER, OP_SWITCH},
      {OP_DUPLICATE, OP_ROLL, OP_IN_NUMBER},
      {"in(char)", OP_OUT_NUMBER, OP_OUT_CHAR}};
};
} // namespace Piet

#endif
