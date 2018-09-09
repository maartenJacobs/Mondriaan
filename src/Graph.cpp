#include "../include/Piet.h"

namespace Piet::Parse {
    bool GraphEdge::isRedirect() {
        return target == nullptr;
    }

    void GraphNode::markAsInitial() {
        isInitial = true;
    }

    void GraphNode::markAsTerminal() {
        isTerminal = true;
    }

    GraphEdge *GraphNode::edgeForDirection(Piet::DirectionPoint direction) {
        auto edgeIterator = edges.find(direction);
        if (edgeIterator == edges.end()) {
            return nullptr;
        }

        return edgeIterator->second;
    }

    void GraphNode::connect(DirectionPoint direction, GraphEdge* edge) {
        edges[direction] = edge;
    }
}
