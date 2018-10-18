#include <stack>
#include <cassert>
#include "../include/Piet.h"

namespace Piet::Parse {
    Position *move(DirectionPoint inDirection, Image *image, Position *position) {
        Position *next = nullptr;

        switch (inDirection) {
            case RightTop:
                next = new Position{position->row, position->column + 1};
                break;
            case RightBottom:
                next = new Position{position->row, position->column + 1};
                break;
            case BottomLeft:
                next = new Position{position->row + 1, position->column};
                break;
            case BottomRight:
                next = new Position{position->row + 1, position->column};
                break;
            case LeftBottom:
                if (position->column > 0) {
                    next = new Position{position->row, position->column - 1};
                }
                break;
            case LeftTop:
                if (position->column > 0) {
                    next = new Position{position->row, position->column - 1};
                }
                break;
            case TopRight:
                if (position->row > 0) {
                    next = new Position{position->row - 1, position->column};
                }
                break;
            case TopLeft:
                if (position->row > 0) {
                    next = new Position{position->row - 1, position->column};
                }
                break;
        }

        if (next && image->in(next)) {
            return next;
        }

        return nullptr;
    }

    class ExitPosition {
    public:
        ExitPosition(Position *position, DirectionPoint direction, Image *image)
                : position(position), direction(direction), image(image) {
            assert(position != nullptr);
            assert(image != nullptr);
        }

        /**
         * \brief Determine the position of the codel that should be entered from the exit position.
         * \return nullptr if the next position starting from the exit position is an invalid point.
         * Otherwise the next position from the exit position, belonging to the codel block that should
         * be entered.
         */
        Position *next() {
            return move(direction, image, position);
        };

        uint32_t getRow() {
            return position->row;
        }

        uint32_t getColumn() {
            return position->column;
        }

        DirectionPoint getDirection() {
            return direction;
        };

        Position *getPosition() {
            return position;
        };
    private:
        Position *position;
        DirectionPoint direction;
        Image *image;
    };

    struct CodelBlock {
        Color color = Black;
        uint32_t size = 0;
        GraphNode *constructingNode = nullptr;
        ExitPosition *topLeftExit = nullptr,
                *topRightExit = nullptr,
                *rightTopExit = nullptr,
                *rightBottomExit = nullptr,
                *bottomLeftExit = nullptr,
                *bottomRightExit = nullptr,
                *leftBottomExit = nullptr,
                *leftTopExit = nullptr;
        vector<Position *> visitPositions;
    };

    DirectionPoint nextDirection(DirectionPoint current) {
        using namespace Piet;

        switch (current) {
            case RightTop:
                return RightBottom;
            case RightBottom:
                return BottomLeft;
            case BottomLeft:
                return BottomRight;
            case BottomRight:
                return LeftBottom;
            case LeftBottom:
                return LeftTop;
            case LeftTop:
                return TopRight;
            case TopRight:
                return TopLeft;
            case TopLeft:
                return RightTop;
            default:
                assert(false);
                return RightBottom;
        }
    }

    void visitWhiteBlock(uint32_t owner, CodelBlock *whiteBlock, Image *image, Position *initialPosition) {
        stack<Position *> visitPositions;
        visitPositions.push(initialPosition);

        while (!visitPositions.empty()) {
            auto position = visitPositions.top();
            visitPositions.pop();

            // Not a valid point.
            if (!image->in(position)) {
                continue;
            }

            Color positionColor = image->at(position);
            if (positionColor == Control) {
                continue;
            } else if (positionColor == Black) {
                continue;
            } else if (positionColor != White) {
                whiteBlock->visitPositions.push_back(position);
                continue;
            }

            whiteBlock->size++;

            // Mark as visited.
            image->fill(position, Control);

            // Mark the owner of the cell.
            image->markOwner(position, owner);

            // Add positions around current position to stack.
            if (position->row > 0) {
                visitPositions.push(new Position{position->row - 1, position->column});
            }
            visitPositions.push(new Position{position->row + 1, position->column});
            if (position->column > 0) {
                visitPositions.push(new Position{position->row, position->column - 1});
            }
            visitPositions.push(new Position{position->row, position->column + 1});
        }
    }

    CodelBlock *visitBlock(uint32_t owner, Image *image, Position *initialPosition) {
        stack<Position *> visitPositions;
        visitPositions.push(initialPosition);

        auto block = new CodelBlock;
        block->color = image->at(initialPosition);
        block->size = 0;
        if (block->color == White) {
            visitWhiteBlock(owner, block, image, initialPosition);
            return block;
        }

        while (!visitPositions.empty()) {
            auto position = visitPositions.top();
            visitPositions.pop();

            // Not a valid point.
            if (!image->in(position)) {
                continue;
            }

            // Visited or different color? Skip.
            Color positionColor = image->at(position);
            if (positionColor != block->color) {
                continue;
            }

            // Mark as visited.
            image->fill(position, Control);

            // Increment size.
            block->size++;

            // Mark the owner of the cell.
            image->markOwner(position, owner);

            // Is one of the exit points? Set exit points.
            if (block->rightTopExit == nullptr
                || (position->column >= block->rightTopExit->getColumn()
                    && (position->column > block->rightTopExit->getColumn() || position->row < block->rightTopExit->getRow()))) {
                block->rightTopExit = new ExitPosition(new Position(*position), RightTop, image);
            }
            if (block->rightBottomExit == nullptr
                || (position->column >= block->rightBottomExit->getColumn()
                    && (position->column > block->rightBottomExit->getColumn() || position->row > block->rightBottomExit->getRow()))) {
                block->rightBottomExit = new ExitPosition(new Position(*position), RightBottom, image);
            }
            if (block->bottomRightExit == nullptr
                || (position->row >= block->bottomRightExit->getRow()
                    && (position->row > block->bottomRightExit->getRow() || position->column > block->bottomRightExit->getColumn()))) {
                block->bottomRightExit = new ExitPosition(new Position(*position), BottomRight, image);
            }
            if (block->bottomLeftExit == nullptr
                || (position->row >= block->bottomLeftExit->getRow()
                    && (position->row > block->bottomLeftExit->getRow() || position->column < block->bottomLeftExit->getColumn()))) {
                block->bottomLeftExit = new ExitPosition(new Position(*position), BottomLeft, image);
            }
            if (block->leftBottomExit == nullptr
                || (position->column <= block->leftBottomExit->getColumn()
                    && (position->column < block->leftBottomExit->getColumn() || position->row > block->leftBottomExit->getRow()))) {
                block->leftBottomExit = new ExitPosition(new Position(*position), LeftBottom, image);
            }
            if (block->leftTopExit == nullptr
                || (position->column <= block->leftTopExit->getColumn()
                    && (position->column < block->leftTopExit->getColumn() || position->row < block->leftTopExit->getRow()))) {
                block->leftTopExit = new ExitPosition(new Position(*position), LeftTop, image);
            }
            if (block->topLeftExit == nullptr
                || (position->row <= block->topLeftExit->getRow()
                    && (position->row < block->topLeftExit->getRow() || position->column < block->topLeftExit->getColumn()))) {
                block->topLeftExit = new ExitPosition(new Position(*position), TopLeft, image);
            }
            if (block->topRightExit == nullptr
                || (position->row <= block->topRightExit->getRow()
                    && (position->row < block->topRightExit->getRow() || position->column > block->topRightExit->getColumn()))) {
                block->topRightExit = new ExitPosition(new Position(*position), TopRight, image);
            }

            // Add positions around current position to stack.
            if (position->row > 0) {
                visitPositions.push(new Position{position->row - 1, position->column});
            }
            visitPositions.push(new Position{position->row + 1, position->column});
            if (position->column > 0) {
                visitPositions.push(new Position{position->row, position->column - 1});
            }
            visitPositions.push(new Position{position->row, position->column + 1});
        }

        return block;
    }

    void incrementIdentifierParts(vector<char> &identifierParts) {
        char last = identifierParts.back();
        if (last == 'Z') {
            identifierParts.push_back('A');
            return;
        }

        identifierParts.pop_back();
        identifierParts.push_back((char)(last + 1));
    }

    string identifierFromParts(vector<char> identifierParts) {
        string id;
        for (const char c : identifierParts) {
            id += c;
        }

        return id;
    }

    /**
     * @brief Piet::WhiteBlockParser parses the transition of codels across a white block.
     * @paragraph Starting from the exit point's direction, we'll "slide" across the white block to the next
     * non-black codel. The rules for this transition are different than the regular transition rules of the Piet
     * language. For white codels we move straight onwards instead of finding the furthest point in the direction of
     * the Direction Pointer and the Codel Chooser. Additionally, the Codel Chooser and the Direction Pointer are
     * both flipped when a black codel or an edge is reached (the CC is flipped twice to flip the DP). Similar to
     * the regular rules, we flip the DP 4 times (similar to the 8 tries of the regular rules) before giving up.
     * @paragraph This algorithm gives us either the next codel (non-black and non-white) or the white codel, in
     * which case the white codel should be considered terminal.
     * @paragraph Note that the Piet language spec mentions "If [the interpreter] retraces its route entirely within
     * a white block, there is no way out of the white block and execution should terminate." The npiet interpreter
     * does not implement this rule; instead programs consisting of 1 coloured codel in the top-left corner with a
     * white codel around and nothing else result in an infinite loop. The Mondriaan compiler will follow the
     * specification instead.
     */
    class WhiteBlockParser {
    public:
        explicit WhiteBlockParser(Image *image) : image(image) {}

        GraphEdge *parse(Position *previousCodelPosition, Position *startPosition, DirectionPoint startDirection, const vector<CodelBlock *> &blocks) {
            DirectionPoint currentDirection = startDirection;
            auto currentPosition = new Position(*startPosition);
            for (uint8_t attempts = 0; attempts < 4; attempts++) {
                Position *nextPosition = nullptr;
                while ((nextPosition = move(currentDirection, image, currentPosition)) != nullptr) {
                    auto nextOwner = blocks.at(image->ownerAt(nextPosition) - 1);
                    auto currentColor = nextOwner->color;
                    if (currentColor == Black) {
                        break;
                    } else if (currentColor == White) {
                        currentPosition = nextPosition;
                        continue;
                    } else {
                        // We've found a coloured codel.
                        currentPosition = nextPosition;
                        auto colouredBlockOwner = blocks.at(image->ownerAt(currentPosition) - 1);
                        return new GraphEdge{new DirectionPoint{currentDirection}, colouredBlockOwner->constructingNode, true};
                    }
                }

                currentDirection = nextDirection(nextDirection(currentDirection));
            }

            // We were unable to find another coloured codel. The white block will serve as a terminal block then.
            auto whiteBlockOwner = blocks.at(image->ownerAt(startPosition) - 1);
            return new GraphEdge{new DirectionPoint{currentDirection}, whiteBlockOwner->constructingNode, true};
        }
    private:
        Image *image;
    };

    GraphEdge *edgeFromExitPosition(Image *image, WhiteBlockParser *whiteBlockParser, const vector<CodelBlock *> &blocks, ExitPosition *exit) {
        auto ownerPosition = exit->next();
        if (ownerPosition == nullptr) {
            return new GraphEdge(new DirectionPoint{nextDirection(exit->getDirection())});
        }

        auto owner = blocks.at(image->ownerAt(ownerPosition) - 1);
        if (owner->color == Black) {
            return new GraphEdge(new DirectionPoint{nextDirection(exit->getDirection())});
        } else if (owner->color == White) {
            return whiteBlockParser->parse(exit->getPosition(), exit->next(), exit->getDirection(), blocks);
        } else {
            return new GraphEdge(new DirectionPoint{exit->getDirection()}, owner->constructingNode, false);
        }
    }

    Graph *Parser::parse() {
        stack<Position *> visitPositions;
        visitPositions.push(new Position{0, 0});
        uint32_t cellOwner = 0;
        vector<CodelBlock *> blocks;
        vector<GraphNode *> nodes;
        vector<char> identifierParts = {'A'};
        auto whiteBlockParser = new WhiteBlockParser{image};

        while (!visitPositions.empty()) {
            auto position = visitPositions.top();
            visitPositions.pop();

            // Not a valid point.
            if (!image->in(position)) {
                continue;
            }

            // Visited? Skip.
            Color positionColor = image->at(position);
            if (positionColor == Control) {
                continue;
            }

            cellOwner++;
            CodelBlock *block = visitBlock(cellOwner, image, position);
            block->constructingNode = new GraphNode(block->color, block->size, identifierFromParts(identifierParts));
            incrementIdentifierParts(identifierParts);
            blocks.push_back(block);
            assert(blocks.at(cellOwner - 1) == block);

            // For each 8 exit points that are unvisited, create a graph node starting from that point.
            if (block->topLeftExit && block->topLeftExit->next()) {
                visitPositions.push(block->topLeftExit->next());
            }
            if (block->topRightExit && block->topRightExit->next()) {
                visitPositions.push(block->topRightExit->next());
            }
            if (block->bottomLeftExit && block->bottomLeftExit->next()) {
                visitPositions.push(block->bottomLeftExit->next());
            }
            if (block->bottomRightExit && block->bottomRightExit->next()) {
                visitPositions.push(block->bottomRightExit->next());
            }
            if (block->leftBottomExit && block->leftBottomExit->next()) {
                visitPositions.push(block->leftBottomExit->next());
            }
            if (block->leftTopExit && block->leftTopExit->next()) {
                visitPositions.push(block->leftTopExit->next());
            }
            if (block->rightTopExit && block->rightTopExit->next()) {
                visitPositions.push(block->rightTopExit->next());
            }
            if (block->rightBottomExit && block->rightBottomExit->next()) {
                visitPositions.push(block->rightBottomExit->next());
            }

            // Push the marked visit positions.
            for (auto visitPosition: block->visitPositions) {
                visitPositions.push(visitPosition);
            }
        }

        // Connect the nodes together.
        for (auto block : blocks) {
            // Add the edges to the node.
            if (block->rightTopExit) {
                block->constructingNode->connect(
                        block->rightTopExit->getDirection(),
                        edgeFromExitPosition(image, whiteBlockParser, blocks, block->rightTopExit));
            }
            if (block->rightBottomExit) {
                block->constructingNode->connect(
                        block->rightBottomExit->getDirection(),
                        edgeFromExitPosition(image, whiteBlockParser, blocks, block->rightBottomExit));
            }
            if (block->bottomRightExit) {
                block->constructingNode->connect(
                        block->bottomRightExit->getDirection(),
                        edgeFromExitPosition(image, whiteBlockParser, blocks, block->bottomRightExit));
            }
            if (block->bottomLeftExit) {
                block->constructingNode->connect(
                        block->bottomLeftExit->getDirection(),
                        edgeFromExitPosition(image, whiteBlockParser, blocks, block->bottomLeftExit));
            }
            if (block->leftBottomExit) {
                block->constructingNode->connect(
                        block->leftBottomExit->getDirection(),
                        edgeFromExitPosition(image, whiteBlockParser, blocks, block->leftBottomExit));
            }
            if (block->leftTopExit) {
                block->constructingNode->connect(
                        block->leftTopExit->getDirection(),
                        edgeFromExitPosition(image, whiteBlockParser, blocks, block->leftTopExit));
            }
            if (block->topLeftExit) {
                block->constructingNode->connect(
                        block->topLeftExit->getDirection(),
                        edgeFromExitPosition(image, whiteBlockParser, blocks, block->topLeftExit));
            }
            if (block->topRightExit) {
                block->constructingNode->connect(
                        block->topRightExit->getDirection(),
                        edgeFromExitPosition(image, whiteBlockParser, blocks, block->topRightExit));
            }

            // Check if the node is a terminal node.
            bool isTerminalNode = block->constructingNode->getColor() == White
                                  || (block->constructingNode->edgeForDirection(RightTop)->isRedirect()
                                  && block->constructingNode->edgeForDirection(RightBottom)->isRedirect()
                                  && block->constructingNode->edgeForDirection(BottomRight)->isRedirect()
                                  && block->constructingNode->edgeForDirection(BottomLeft)->isRedirect()
                                  && block->constructingNode->edgeForDirection(LeftBottom)->isRedirect()
                                  && block->constructingNode->edgeForDirection(LeftTop)->isRedirect()
                                  && block->constructingNode->edgeForDirection(TopLeft)->isRedirect()
                                  && block->constructingNode->edgeForDirection(TopRight)->isRedirect());
            if (isTerminalNode) {
                block->constructingNode->markAsTerminal();
            }

            // Add the node to the nodes for the graph.
            nodes.push_back(block->constructingNode);
        }

        nodes.at(0)->markAsInitial();
        return new Graph(nodes, nodes.at(0));
    }
}
