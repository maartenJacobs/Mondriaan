#include "../include/Piet.h"

namespace Piet::Parse {
bool GraphEdge::isRedirect() { return target == nullptr; }

void GraphNode::markAsInitial() { initial = true; }

void GraphNode::markAsTerminal() { terminal = true; }

string GraphNode::getIdentifier() { return identifier; }

GraphEdge *GraphNode::edgeForDirection(Piet::DirectionPoint direction) {
  auto edgeIterator = edges.find(direction);
  if (edgeIterator == edges.end()) {
    return nullptr;
  }

  return edgeIterator->second;
}

void GraphNode::connect(DirectionPoint direction, GraphEdge *edge) {
  edges[direction] = edge;
}

DirectionPoint *GraphEdge::getNewDirection() { return newDirection; }

GraphNode *GraphEdge::getTarget() { return target; }

bool GraphEdge::isNoop() { return noop; }

bool GraphNode::isTerminal() { return terminal; }

bool GraphNode::isInitial() { return initial; }

DirectionPoint Graph::getCurrentDirection() { return currentDirection; }

GraphNode *Graph::getInitialNode() { return initialNode; }

GraphNode *Graph::getCurrentNode() { return currentNode; }

void Graph::restartWalk(GraphNode *fromNode, DirectionPoint inDirection) {
  currentNode = fromNode;
  currentDirection = inDirection;
}

GraphStep *Graph::walk() {
  if (currentNode == nullptr) {
    currentNode = initialNode;
  }
  if (currentNode->isTerminal()) {
    return nullptr;
  }

  GraphEdge *edge = nullptr;
  for (uint8_t i = 0; i < 8; i++) {
    edge = currentNode->edgeForDirection(currentDirection);
    if (!edge->isRedirect()) {
      break;
    }

    currentDirection = *(edge->getNewDirection());
  }

  // If the node is not a terminal node, then it should have
  // a non-redirect node.
  assert(edge != nullptr);

  auto step = new GraphStep();
  step->previous = currentNode;
  step->current = edge->getTarget();
  step->skipTransition = edge->isNoop();

  currentNode = step->current;

  return step;
}

Color GraphNode::getColor() { return color; }

uint32_t GraphNode::getSize() { return size; }
} // namespace Piet::Parse
