#include <stack>
#include "../include/Piet.h"

namespace Piet::Parse {
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
            Position *next = nullptr;

            switch (direction) {
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

    CodelBlock *visitBlock(uint32_t owner, Image *image, Position *initialPosition) {
        stack<Position *> visitPositions;
        visitPositions.push(initialPosition);

        auto block = new CodelBlock;
        block->color = image->at(initialPosition);
        block->size = 0;

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

    GraphEdge *edgeFromExitPosition(vector<CodelBlock *> blocks, Image *image, ExitPosition *exit) {
        auto ownerPosition = exit->next();
        if (ownerPosition == nullptr) {
            return new GraphEdge(new DirectionPoint{nextDirection(exit->getDirection())});
        }

        auto owner = blocks.at(image->ownerAt(ownerPosition) - 1);
        if (owner->color == Black) {
            return new GraphEdge(new DirectionPoint{nextDirection(exit->getDirection())});
        } else {
            return new GraphEdge(new DirectionPoint{exit->getDirection()}, owner->constructingNode);
        }
    }

    Graph *Parser::parse() {
        stack<Position *> visitPositions;
        visitPositions.push(new Position{0, 0});
        uint32_t cellOwner = 0;
        vector<CodelBlock *> blocks;
        vector<GraphNode *> nodes;

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
            block->constructingNode = new GraphNode(block->color, block->size);
            blocks.push_back(block);
            assert(blocks.at(cellOwner - 1) == block);

            // For each 8 exit points that are unvisited, create a graph node starting from that point.
            if (block->topLeftExit->next()) {
                visitPositions.push(block->topLeftExit->next());
            }
            if (block->topRightExit->next()) {
                visitPositions.push(block->topRightExit->next());
            }
            if (block->bottomLeftExit->next()) {
                visitPositions.push(block->bottomLeftExit->next());
            }
            if (block->bottomRightExit->next()) {
                visitPositions.push(block->bottomRightExit->next());
            }
            if (block->leftBottomExit->next()) {
                visitPositions.push(block->leftBottomExit->next());
            }
            if (block->leftTopExit->next()) {
                visitPositions.push(block->leftTopExit->next());
            }
            if (block->rightTopExit->next()) {
                visitPositions.push(block->rightTopExit->next());
            }
            if (block->rightBottomExit->next()) {
                visitPositions.push(block->rightBottomExit->next());
            }
        }

        // Connect the nodes together.
        for (auto block : blocks) {
            // Add the edges to the node.
            block->constructingNode->connect(block->rightTopExit->getDirection(),       edgeFromExitPosition(blocks, image, block->rightTopExit));
            block->constructingNode->connect(block->rightBottomExit->getDirection(),    edgeFromExitPosition(blocks, image, block->rightBottomExit));
            block->constructingNode->connect(block->bottomRightExit->getDirection(),    edgeFromExitPosition(blocks, image, block->bottomRightExit));
            block->constructingNode->connect(block->bottomLeftExit->getDirection(),     edgeFromExitPosition(blocks, image, block->bottomLeftExit));
            block->constructingNode->connect(block->leftBottomExit->getDirection(),     edgeFromExitPosition(blocks, image, block->leftBottomExit));
            block->constructingNode->connect(block->leftTopExit->getDirection(),        edgeFromExitPosition(blocks, image, block->leftTopExit));
            block->constructingNode->connect(block->topLeftExit->getDirection(),        edgeFromExitPosition(blocks, image, block->topLeftExit));
            block->constructingNode->connect(block->topRightExit->getDirection(),       edgeFromExitPosition(blocks, image, block->topRightExit));

            // Check if the node is a terminal node.
            bool isTerminalNode = block->constructingNode->edgeForDirection(RightTop)->isRedirect()
                                  && block->constructingNode->edgeForDirection(RightBottom)->isRedirect()
                                  && block->constructingNode->edgeForDirection(BottomRight)->isRedirect()
                                  && block->constructingNode->edgeForDirection(BottomLeft)->isRedirect()
                                  && block->constructingNode->edgeForDirection(LeftBottom)->isRedirect()
                                  && block->constructingNode->edgeForDirection(LeftTop)->isRedirect()
                                  && block->constructingNode->edgeForDirection(TopLeft)->isRedirect()
                                  && block->constructingNode->edgeForDirection(TopRight)->isRedirect();
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
