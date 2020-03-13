#include "Node.h"

Node::Node() {
    top = NULL;
    bottom = NULL;
    right = NULL;
    left = NULL;
    topLeft = NULL;
    topRight = NULL;
    bottomLeft = NULL;
    bottomRight = NULL;
}

Node::Node(cv::Point2f p) {
    top = NULL;
    bottom = NULL;
    right = NULL;
    left = NULL;
    topLeft = NULL;
    topRight = NULL;
    bottomLeft = NULL;
    bottomRight = NULL;
    pos = p;
}

Node::~Node() {}