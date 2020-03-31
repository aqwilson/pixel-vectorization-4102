#include "PixelGraph.h"
#include <iostream>
#include <algorithm>

PixelGraph::PixelGraph() {
    img = NULL;
}

PixelGraph::PixelGraph(cv::Mat* img) {
    this->graph = new std::vector<std::vector<Node*>*>();

    for (int i = 0; i < img->rows; i++) {
        std::vector<Node*>* line = new std::vector<Node*>();

        for (int j = 0; j < img->cols; j++) {
            line->push_back(new Node(cv::Point2f(j,i)));
        }

        graph->push_back(line);
        
    }

    this->img = img;
}

PixelGraph::~PixelGraph() {
    img = NULL;

    for (int i = 0; i < graph->size(); i++) {
        for (int j = 0; j < graph->at(i)->size(); j++) {
            delete graph->at(i)->at(j);
        }

        delete graph->at(i);
    }

    delete graph;
}

Node* PixelGraph::getIfExists(int r, int c) {
    if (r < 0 || r >= graph->size() || c < 0 || c >= graph->at(r)->size()) {
        return nullptr;
    }
    return graph->at(r)->at(c);
}

void PixelGraph::generateGraph() {
    // CV_8UC3
    for (int r = 0; r < graph->size(); r++) {
        for (int c = 0; c < graph->at(r)->size(); c++) {
            Node* n = graph->at(r)->at(c);

            n->topLeft = getIfExists(r - 1, c - 1);
            n->top = getIfExists(r - 1, c);
            n->topRight = getIfExists(r - 1, c + 1);

            n->left = getIfExists(r, c - 1);
            n->right = getIfExists(r, c + 1);

            n->bottomLeft = getIfExists(r + 1, c - 1);
            n->bottom = getIfExists(r + 1, c);
            n->bottomRight = getIfExists(r + 1, c + 1);
        }
    }
}

bool PixelGraph::comparePixels(Node* n1, Node* n2) {
    if (n1 == NULL || n2 == NULL) {
        return false;
    }

    cv::Vec3b n1Vec = img->at<cv::Vec3b>(n1->pos);
    cv::Vec3b n2Vec = img->at<cv::Vec3b>(n2->pos);

    return (n1Vec[0] == n2Vec[0] && n1Vec[1] == n2Vec[1] && n1Vec[2] == n2Vec[2]);
}

void PixelGraph::firstNeighbourhoodPrunePass(Node* n) {
    if (n->top != NULL && !comparePixels(n, n->top)) {
        n->top->bottom = NULL;
        n->top = NULL;
    }

    if (n->bottom != NULL && !comparePixels(n, n->bottom)) {
        n->bottom->top = NULL;
        n->bottom = NULL;
    }

    if (n->left != NULL && !comparePixels(n, n->left)) {
        n->left->right = NULL;
        n->left = NULL;
    }

    if (n->right != NULL && !comparePixels(n, n->right)) {
        n->right->left = NULL;
        n->right = NULL;
    }

    if (n->topLeft != NULL && !comparePixels(n, n->topLeft)) {
        n->topLeft->bottomRight = NULL;
        n->topLeft = NULL;
    }

    if (n->topRight != NULL && !comparePixels(n, n->topRight)) {
        n->topRight->bottomLeft = NULL;
        n->topRight = NULL;
    }

    if (n->bottomLeft != NULL && !comparePixels(n, n->bottomLeft)) {
        n->bottomLeft->topRight = NULL;
        n->bottomLeft = NULL;
    }

    if (n->bottomRight != NULL && !comparePixels(n, n->bottomRight)) {
        n->bottomRight->topLeft = NULL;
        n->bottomRight = NULL;
    }
}

void PixelGraph::flatPruning(Node* n) {
    // top left
    bool leftCompare = comparePixels(n, n->left);
    bool topCompare = comparePixels(n, n->top);
    bool topLeftCompare = comparePixels(n, n->topLeft);
    bool rightCompare = comparePixels(n, n->right);
    bool bottomCompare = comparePixels(n, n->bottom);
    bool topRightCompare = comparePixels(n, n->topRight);
    bool bottomRightCompare = comparePixels(n, n->bottomRight);
    bool bottomLeftCompare = comparePixels(n, n->bottomLeft);

    if (leftCompare && topCompare && topLeftCompare) {
        if (n->topLeft != NULL) {
            n->topLeft->bottomRight = NULL;
            n->topLeft = NULL;
        }
    }

    if (topCompare && topRightCompare && rightCompare) {
        if (n->topRight != NULL) {
            n->topRight->bottomLeft = NULL;
            n->topRight = NULL;
        }
    }

    if (rightCompare && bottomRightCompare && bottomCompare) {
        if (n->bottomRight != NULL) {
            n->bottomRight->topLeft = NULL;
            n->bottomRight = NULL;
        }
    }

    if (bottomCompare && bottomLeftCompare && leftCompare) {
        if (n->bottomLeft != NULL) {
            n->bottomLeft->topRight = NULL;
            n->bottomLeft = NULL;
        }
    }

}

void PixelGraph::brutePrune() {
    // break up based on colour
    for (int i = 0; i < graph->size(); i++) {
        for (int j = 0; j < graph->at(i)->size(); j ++) {
            Node* n = graph->at(i)->at(j);

            firstNeighbourhoodPrunePass(n);
            flatPruning(n);
        }
    }
}

void PixelGraph::runHeuristics() {
    // re-add based on other stuff
    
    for (int i = 0; i < img->rows - 1; i++){
        for (int j = 0; j < img->cols - 1; j++){
            //This will be the top left of our 2 x 2 region, and we should never go out of bounds because of the loop max
            Node* topLeftNode = graph->at(i)->at(j);
            Node* topRightNode = graph->at(i + 1)->at(j);
            Node* bottomLeftNode = graph->at(i)->at(j + 1);
            Node* bottomRightNode = graph->at(i + 1)->at(j + 1);

            // If the following is true, we have two diagonal connections, so one must be removed
            if (topLeftNode->bottomRight != NULL && topRightNode->bottomLeft != NULL) {
                cv::Point2f curveWeights, sparseWeights, islandWeights;

                runCurveHeuristic(cv::Point2f(j, i), &curveWeights);
                runSparseHeuristic(cv::Point2f(j, i), &sparseWeights);
                runIslandHeuristic(cv::Point2f(j, i), &islandWeights);

                float topLeftBottomRightWeight = curveWeights.x + sparseWeights.x + islandWeights.x;
                float topRightBottomLeftWeight = curveWeights.y + sparseWeights.y + islandWeights.y;

                //Remove one of the diagonal connections based on the heuristic weights
                if (topLeftBottomRightWeight > topRightBottomLeftWeight) {
                    topRightNode->bottomLeft = NULL;
                    bottomLeftNode->topRight = NULL;
                }
                else {
                    topLeftNode->bottomRight = NULL;
                    bottomRightNode->topLeft = NULL;
                }
            }
		}
	}

}

void PixelGraph::runCurveHeuristic(cv::Point topLeft, cv::Point2f* weightVals){
    weightVals->x = 0.0f;
    weightVals->y = 0.0f;
}

void PixelGraph::runSparseHeuristic(cv::Point topLeft, cv::Point2f* weightVals){
    weightVals->x = 0.0f;
    weightVals->y = 0.0f;

    // if the pixels are the same for whatever reason, then weight is 0, because there is no difference
    if (comparePixels(graph->at(topLeft.y)->at(topLeft.x), graph->at(topLeft.y)->at(topLeft.x + 1.0))) {
        return;
    }

    // create 2 component connection graphs for comparison
    std::vector<std::vector<int>> connectedTL;
    std::vector<std::vector<int>> connectedTR;

    // fill with "untested" value
    for (int i = 0; i < 8; i++) {
        std::vector<int> rowTL;
        std::vector<int> rowTR;
        for (int j = 0; j < 8; j++) {
            rowTL.push_back(0);
            rowTR.push_back(0);
        }
        connectedTL.push_back(rowTL);
        connectedTR.push_back(rowTR);
    }

    // we know of 2 connected nodes already; put them where they belong
    connectedTL[3][3] = 1;
    connectedTL[4][4] = 1;

    connectedTR[3][4] = 1;
    connectedTR[4][3] = 1;

    // generate connections
    connections(connectedTL, topLeft);
    connections(connectedTR, topLeft);
    
    // count connections
    int tlConnNum = 0;
    int trConnNum = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (connectedTL[y][x] == 1) {
                tlConnNum++;
            }

            if (connectedTR[y][x] == 1) {
                trConnNum++;
            }
        }
    }

    // give weight to the SPARSER connections, as sparser values are foreground
    if (tlConnNum < trConnNum) {
        weightVals->x = trConnNum - tlConnNum;
        return;
    }

    if (trConnNum < tlConnNum) {
        weightVals->y = tlConnNum - trConnNum;
    }
}

void PixelGraph::runIslandHeuristic(cv::Point topLeft, cv::Point2f* weightVals){
    weightVals->x = 0.0f;
    weightVals->y = 0.0f;
}


// create a component-based connection graph to quickly count number of nodes connected.
void PixelGraph::connections(std::vector<std::vector<int>>& connGraph, cv::Point topLeft) {
    bool remainingUntested = true;
    bool reachableUntested = true;
    int iteration = 0;

    // we want to make sure that we don't iterate more than necessary
    while (remainingUntested && reachableUntested && iteration < 8) {
        iteration++;
        remainingUntested = false;
        reachableUntested = false;

        // for each pixel, are we connected to neighbours?
        // 0 = untested, 1 = connected, 2 = not connected, -1 = OOB
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {

                // untested
                if (connGraph[y][x] == 0) {
                    remainingUntested = true;
                    continue;
                }

                // not yet connected
                if (connGraph[y][x] == 2 || connGraph[y][x] == -1) {
                    continue;
                }

                // don't go out of bounds
                int realY = topLeft.y + y - 3;
                int realX = topLeft.x + x - 3;

                if (realY < 0 || realY >= graph->size() || realX < 0 || realX >= graph->at(0)->size()) {
                    connGraph[y][x] = -2;
                    continue;
                }

                // some connected value
                // are the values around connected?
                // is there an up, is that geq 0, and is the one above 0 or disconnected?
                if (y > 0) {

                    // checking node above
                    // if not already connected and not out of bounds, look for a connection
                    if (connGraph[y - 1.0][x] != 1 && connGraph[y - 1.0][x] != -1) {
                        if (realY <= 0) {
                            connGraph[y - 1.0][x] = -1;
                        }
                        else if (graph->at(realY)->at(realX)->top != NULL) {
                            connGraph[y - 1.0][x] = 1;
                        }
                        else {
                            connGraph[y - 1.0][x] = 2;
                        }
                    }

                    // check node at topLeft
                    if (x > 0) {
                        if (connGraph[y - 1.0][x - 1.0] != 1 && connGraph[y - 1.0][x - 1.0] != -1) {
                            if (realY <= 0 || realX <= 0) {
                                connGraph[y - 1.0][x - 1.0] = -1;
                            }
                            else if (graph->at(realY)->at(realX)->topLeft != NULL) {
                                connGraph[y - 1.0][x - 1.0] = 1;
                            }
                            else {
                                connGraph[y - 1.0][x - 1.0] = 2;
                            }
                        }
                    }

                    // check topRight
                    if (x < 7) {
                        if (connGraph[y - 1.0][x + 1.0] != 1 && connGraph[y - 1.0][x + 1.0] != -1) {
                            if (realY <= 0 || realX >= graph->at(0)->size()) {
                                connGraph[y - 1.0][x + 1.0] = -1;
                            }
                            else if (graph->at(realY)->at(realX)->topRight != NULL) {
                                connGraph[y - 1.0][x + 1.0] = 1;
                            }
                            else {
                                connGraph[y - 1.0][x + 1.0] = 2;
                            }
                        }
                    }
                }

                // check bottom row
                if (y < 7) {
                    // check bottom
                    if (connGraph[y + 1.0][x] != 1 && connGraph[y + 1.0][x] != -1) {
                        if (realY >= graph->size()) {
                            connGraph[y + 1.0][x] = -1;
                        }
                        else if (graph->at(realY)->at(realX)->bottom != NULL) {
                            connGraph[y + 1.0][x] = 1;
                        }
                        else {
                            connGraph[y + 1.0][x] = 2;
                        }
                    }

                    // check bottom left
                    if (x > 0) {
                        if (connGraph[y + 1.0][x - 1.0] != 1 && connGraph[y + 1.0][x - 1.0] != -1) {
                            if (realY >= graph->size() || realX <= 0) {
                                connGraph[y + 1.0][x - 1.0] = -1;
                            }
                            else if (graph->at(realY)->at(realX)->bottomLeft != NULL) {
                                connGraph[y + 1.0][x - 1.0] = 1;
                            }
                            else {
                                connGraph[y + 1.0][x - 1.0] = 2;
                            }
                        }
                    }

                    // check bottom right
                    if (x < 7) {
                        if (connGraph[y + 1.0][x + 1.0] != 1 && connGraph[y + 1.0][x + 1.0] != -1) {
                            if (realY >= graph->size() || realX >= graph->at(0)->size()) {
                                connGraph[y + 1.0][x + 1.0] = -1;
                            }
                            else if (graph->at(realY)->at(realX)->bottomRight != NULL) {
                                connGraph[y + 1.0][x + 1.0] = 1;
                            }
                            else {
                                connGraph[y + 1.0][x + 1.0] = 2;
                            }
                        }
                    }
                }

                // check left
                if (x > 0) {
                    if (connGraph[y][x - 1.0] != 1 && connGraph[y][x - 1.0] != -1) {
                        if (realX <= 0) {
                            connGraph[y][x - 1.0] = -1;
                        }
                        else if (graph->at(realY)->at(realX)->left != NULL) {
                            connGraph[y][x - 1.0] = 1;
                        }
                        else {
                            connGraph[y][x - 1.0] = 2;
                        }
                    }
                }

                // check right
                if (x < 7) {
                    if (connGraph[y][x + 1.0] != 1 && connGraph[y][x + 1.0] != -1) {
                        if (realX >= graph->at(0)->size()) {
                            connGraph[y][x + 1.0] = -1;
                        }
                        else if (graph->at(realY)->at(realX)->right != NULL) {
                            connGraph[y][x + 1.0] = 1;
                        }
                        else {
                            connGraph[y][x + 1.0] = 2;
                        }
                    }
                }
            }
        }

        // check for reachable untested values - if there are no reachable values, don't bother continuing
        if (remainingUntested) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (connGraph[y][x] != 0) {
                        continue;
                    }

                    // uses clamp to quickly check surrounding values.
                    if (connGraph[std::clamp(y - 1, 0, 7)][std::clamp(x - 1, 0, 7)] == 1 
                        || connGraph[std::clamp(y-1, 0, 7)][x] == 1
                        || connGraph[std::clamp(y-1, 0, 7)][std::clamp(x+1, 0, 7)] == 1
                        || connGraph[y][std::clamp(x-1, 0, 7)] == 1
                        || connGraph[y][std::clamp(x+1, 0, 7)] == 1
                        || connGraph[std::clamp(y+1, 0, 7)][std::clamp(x-1, 0, 7)] == 1
                        || connGraph[std::clamp(y+1, 0, 7)][x] == 1
                        || connGraph[std::clamp(y+1, 0, 7)][std::clamp(x+1, 0, 7)] == 1)
                    {
                        reachableUntested = true;
                        break;
                    }
                }

                if (reachableUntested) {
                    break;
                }
            }
        }
    }
}