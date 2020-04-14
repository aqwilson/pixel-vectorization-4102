#include "PixelGraph.h"
#include <iostream>
#include <algorithm>
#include <deque>

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
        for (int j = 0; j < graph->at(i)->size(); j++) {
            Node* n = graph->at(i)->at(j);

            firstNeighbourhoodPrunePass(n);
            flatPruning(n);
        }
    }
}

void PixelGraph::runHeuristics() {
    // re-add based on other stuff

    for (int i = 0; i < img->rows - 1; i++) {
        for (int j = 0; j < img->cols - 1; j++) {
            //This will be the top left of our 2 x 2 region, and we should never go out of bounds because of the loop max
            Node* topLeftNode = graph->at(i)->at(j);
            Node* topRightNode = graph->at(i)->at(j + 1.0);
            Node* bottomLeftNode = graph->at(i + 1.0)->at(j);
            Node* bottomRightNode = graph->at(i + 1.0)->at(j + 1.0);

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

void PixelGraph::runCurveHeuristic(cv::Point topLeftPoint, cv::Point2f* weightVals) {
    Node* topLeftNode = graph->at(topLeftPoint.y)->at(topLeftPoint.x);
    Node* topRightNode = graph->at(topLeftPoint.y)->at(topLeftPoint.x + 1.0);
    Node* bottomLeftNode = graph->at(topLeftPoint.y + 1.0)->at(topLeftPoint.x);
    Node* bottomRightNode = graph->at(topLeftPoint.y + 1.0)->at(topLeftPoint.x + 1.0);


    weightVals->x = 0.0f;
    weightVals->y = 0.0f;

    int curveLen1 = GetCurveLength(topLeftNode, bottomRightNode);
    int curveLen2 = GetCurveLength(topRightNode, bottomLeftNode);

    if (curveLen1 > curveLen2) {
        weightVals->x = curveLen1 - curveLen2;
    }

    if (curveLen2 > curveLen1) {
        weightVals->y = curveLen2 - curveLen1;
    }
}

void PixelGraph::runSparseHeuristic(cv::Point topLeft, cv::Point2f* weightVals){
    weightVals->x = 0.0f;
    weightVals->y = 0.0f;

    // if the topLeft and topRight are the same for whatever reason, then weight is 0, because there is no difference
    if (comparePixels(graph->at(topLeft.y)->at(topLeft.x), graph->at(topLeft.y)->at(topLeft.x + 1.0))) {
        return;
    }

    // create 2 component connection graphs for comparison
    std::vector<std::vector<ConnType>> connectedTL;
    std::vector<std::vector<ConnType>> connectedTR;

    // fill with "untested" value
    for (int i = 0; i < 8; i++) {
        std::vector<ConnType> rowTL;
        std::vector<ConnType> rowTR;
        for (int j = 0; j < 8; j++) {
            rowTL.push_back(UNTESTED);
            rowTR.push_back(UNTESTED);
        }
        connectedTL.push_back(rowTL);
        connectedTR.push_back(rowTR);
    }

    // we know of 2 connected nodes already; put them where they belong
    connectedTL[3][3] = CONNECTED;
    connectedTL[4][4] = CONNECTED;

    connectedTR[3][4] = CONNECTED;
    connectedTR[4][3] = CONNECTED;

    // generate connections
    connections(connectedTL, topLeft);
    connections(connectedTR, topLeft);
    
    // count connections
    int tlConnNum = 0;
    int trConnNum = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (connectedTL[y][x] == CONNECTED) {
                tlConnNum++;
            }

            if (connectedTR[y][x] == CONNECTED) {
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

    // fetch other points
    cv::Point topRight = cv::Point(topLeft.x + 1, topLeft.y);
    cv::Point bottomLeft = cv::Point(topLeft.x, topLeft.y + 1);
    cv::Point bottomRight = cv::Point(topLeft.x + 1, topLeft.y + 1);

    // if there is top left/bottom right valence of 1, then there is an island on this diagonal
    if (calculateValence(topLeft) == 1 || calculateValence(bottomRight) == 1) {
        weightVals->x = ISLAND_NUM;
    }

    // if there is a top right/bottom left valence of 1, then there is an island on this diagonal
    if (calculateValence(topRight) == 1 || calculateValence(bottomLeft) == 1) {
        weightVals->y = ISLAND_NUM;
    }
}

int PixelGraph::GetCurveLength(Node* startingNode1, Node* startingNode2) {

    //Calculate length of curve source top left to bottom right
    Node* track1 = startingNode1;
    Node* track2 = startingNode2;

    Node* track1Prev = startingNode2;
    Node* track2Prev = startingNode1;

    Node* next;

    //Add one to one of the lengths to account for the initial connection
    int track1Length = 1;
    int track2Length = 0;

    while ((calculateValence(track1->pos) == 2) || (calculateValence(track2->pos) == 2)) {
        if (calculateValence(track1->pos) == 2) {
            track1Length++;
            next = FindSecondConnection(track1, track1Prev);
            track1Prev = track1;
            track1 = next;

            //We might either hit the other track, or ourself
            if (track1 == track2 || track1 == startingNode1) {
                track1Length--;
                break;
            }
        }
        if (calculateValence(track2->pos) == 2) {
            track2Length++;
            next = FindSecondConnection(track2, track2Prev);
            track2Prev = track2;
            track2 = next;

            //We might either hit the other track or ourself
            if (track2 == track1 || track2 == startingNode2) {
                track2Length--;
                break;
            }
        }
    }

    return (track1Length + track2Length);
}

//Finds the second connection of a valence 2 node
Node* PixelGraph::FindSecondConnection(Node* nodeToCalc, Node* otherConn) {
    if (calculateValence(nodeToCalc->pos) != 2) {
        std::cout << "Find second connection only valid for valence 2 nodes\n";
        return NULL;
    }

    else {
        if (nodeToCalc->topLeft && nodeToCalc->topLeft != otherConn) {
            return nodeToCalc->topLeft;
        }
        else if (nodeToCalc->top && nodeToCalc->top != otherConn) {
            return nodeToCalc->top;
        }
        else if (nodeToCalc->topRight && nodeToCalc->topRight != otherConn) {
            return nodeToCalc->topRight;
        }
        else if (nodeToCalc->right && nodeToCalc->right != otherConn) {
            return nodeToCalc->right;
        }
        else if (nodeToCalc->bottomRight && nodeToCalc->bottomRight != otherConn) {
            return nodeToCalc->bottomRight;
        }
        else if (nodeToCalc->bottom && nodeToCalc->bottom != otherConn) {
            return nodeToCalc->bottom;
        }
        else if (nodeToCalc->bottomLeft && nodeToCalc->bottomLeft != otherConn) {
            return nodeToCalc->bottomLeft;
        }
        else {
            return nodeToCalc->left;
        }
    }
}

// create a component-based connection graph to quickly count number of nodes connected.
void PixelGraph::connections(std::vector<std::vector<ConnType>>& connGraph, cv::Point topLeft) {
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
                if (connGraph[y][x] == CONNECTED) {
                    remainingUntested = true;
                    continue;
                }

                // not yet connected
                if (connGraph[y][x] == DISCONNECTED || connGraph[y][x] == INVALID) {
                    continue;
                }

                // don't go out of bounds
                int realY = topLeft.y + y - 3;
                int realX = topLeft.x + x - 3;

                if (realY < 0 || realY >= graph->size() || realX < 0 || realX >= graph->at(0)->size()) {
                    connGraph[y][x] = INVALID;
                    continue;
                }

                // some connected value
                // are the values around connected?
                // is there an up, is that geq 0, and is the one above 0 or disconnected?
                if (y > 0) {

                    // checking node above
                    // if not already connected and not out of bounds, look for a connection
                    if (connGraph[y - 1.0][x] != CONNECTED && connGraph[y - 1.0][x] != INVALID) {
                        if (realY <= 0) {
                            connGraph[y - 1.0][x] = INVALID;
                        }
                        else if (graph->at(realY)->at(realX)->top != NULL) {
                            connGraph[y - 1.0][x] = CONNECTED;
                        }
                        else {
                            connGraph[y - 1.0][x] = DISCONNECTED;
                        }
                    }

                    // check node at topLeft
                    if (x > 0) {
                        if (connGraph[y - 1.0][x - 1.0] != CONNECTED && connGraph[y - 1.0][x - 1.0] != INVALID) {
                            if (realY <= 0 || realX <= 0) {
                                connGraph[y - 1.0][x - 1.0] = INVALID;
                            }
                            else if (graph->at(realY)->at(realX)->topLeft != NULL) {
                                connGraph[y - 1.0][x - 1.0] = CONNECTED;
                            }
                            else {
                                connGraph[y - 1.0][x - 1.0] = DISCONNECTED;
                            }
                        }
                    }

                    // check topRight
                    if (x < 7) {
                        if (connGraph[y - 1.0][x + 1.0] != CONNECTED && connGraph[y - 1.0][x + 1.0] != INVALID) {
                            if (realY <= 0 || realX >= graph->at(0)->size()) {
                                connGraph[y - 1.0][x + 1.0] = INVALID;
                            }
                            else if (graph->at(realY)->at(realX)->topRight != NULL) {
                                connGraph[y - 1.0][x + 1.0] = CONNECTED;
                            }
                            else {
                                connGraph[y - 1.0][x + 1.0] = DISCONNECTED;
                            }
                        }
                    }
                }

                // check bottom row
                if (y < 7) {
                    // check bottom
                    if (connGraph[y + 1.0][x] != CONNECTED && connGraph[y + 1.0][x] != INVALID) {
                        if (realY >= graph->size()) {
                            connGraph[y + 1.0][x] = INVALID;
                        }
                        else if (graph->at(realY)->at(realX)->bottom != NULL) {
                            connGraph[y + 1.0][x] = CONNECTED;
                        }
                        else {
                            connGraph[y + 1.0][x] = DISCONNECTED;
                        }
                    }

                    // check bottom left
                    if (x > 0) {
                        if (connGraph[y + 1.0][x - 1.0] != CONNECTED && connGraph[y + 1.0][x - 1.0] != INVALID) {
                            if (realY >= graph->size() || realX <= 0) {
                                connGraph[y + 1.0][x - 1.0] = INVALID;
                            }
                            else if (graph->at(realY)->at(realX)->bottomLeft != NULL) {
                                connGraph[y + 1.0][x - 1.0] = CONNECTED;
                            }
                            else {
                                connGraph[y + 1.0][x - 1.0] = DISCONNECTED;
                            }
                        }
                    }

                    // check bottom right
                    if (x < 7) {
                        if (connGraph[y + 1.0][x + 1.0] != CONNECTED && connGraph[y + 1.0][x + 1.0] != INVALID) {
                            if (realY >= graph->size() || realX >= graph->at(0)->size()) {
                                connGraph[y + 1.0][x + 1.0] = INVALID;
                            }
                            else if (graph->at(realY)->at(realX)->bottomRight != NULL) {
                                connGraph[y + 1.0][x + 1.0] = CONNECTED;
                            }
                            else {
                                connGraph[y + 1.0][x + 1.0] = DISCONNECTED;
                            }
                        }
                    }
                }

                // check left
                if (x > 0) {
                    if (connGraph[y][x - 1.0] != CONNECTED && connGraph[y][x - 1.0] != -INVALID) {
                        if (realX <= 0) {
                            connGraph[y][x - 1.0] = INVALID;
                        }
                        else if (graph->at(realY)->at(realX)->left != NULL) {
                            connGraph[y][x - 1.0] = CONNECTED;
                        }
                        else {
                            connGraph[y][x - 1.0] = DISCONNECTED;
                        }
                    }
                }

                // check right
                if (x < 7) {
                    if (connGraph[y][x + 1.0] != CONNECTED && connGraph[y][x + 1.0] != INVALID) {
                        if (realX >= graph->at(0)->size()) {
                            connGraph[y][x + 1.0] = INVALID;
                        }
                        else if (graph->at(realY)->at(realX)->right != NULL) {
                            connGraph[y][x + 1.0] = CONNECTED;
                        }
                        else {
                            connGraph[y][x + 1.0] = DISCONNECTED;
                        }
                    }
                }
            }
        }

        // check for reachable untested values - if there are no reachable values, don't bother continuing
        if (remainingUntested) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (connGraph[y][x] != UNTESTED) {
                        continue;
                    }

                    // uses clamp to quickly check surrounding values.
                    if (connGraph[std::clamp(y - 1, 0, 7)][std::clamp(x - 1, 0, 7)] == CONNECTED 
                        || connGraph[std::clamp(y-1, 0, 7)][x] == CONNECTED
                        || connGraph[std::clamp(y-1, 0, 7)][std::clamp(x+1, 0, 7)] == CONNECTED
                        || connGraph[y][std::clamp(x-1, 0, 7)] == CONNECTED
                        || connGraph[y][std::clamp(x+1, 0, 7)] == CONNECTED
                        || connGraph[std::clamp(y+1, 0, 7)][std::clamp(x-1, 0, 7)] == CONNECTED
                        || connGraph[std::clamp(y+1, 0, 7)][x] == CONNECTED
                        || connGraph[std::clamp(y+1, 0, 7)][std::clamp(x+1, 0, 7)] == CONNECTED)
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

// calculate the valence of a given node
int PixelGraph::calculateValence(cv::Point p) {
    int connections = 0;

    Node* n = graph->at(p.y)->at(p.x);

    // incremement number of connections if one exists
    if (n->bottom != NULL) connections++;
    if (n->bottomLeft != NULL) connections++;
    if (n->left != NULL) connections++;
    if (n->topLeft != NULL) connections++;
    if (n->top != NULL) connections++;
    if (n->topRight != NULL) connections++;
    if (n->right != NULL) connections++;
    if (n->bottomRight != NULL) connections++;

    return connections;
}

void PixelGraph::calculateFillType(std::vector<std::vector<FillType>>& fills) {
    // iterate over the graph. We will be considering 3x3 sections
    for (int y = 0; y < graph->size(); y++) {
        for (int x = 0; x < graph->at(0)->size(); x++) {
            fills[y][x] = FillType::NONE;

            Node* n = graph->at(y)->at(x);
            int valence = calculateValence(cv::Point(x, y));

            if (valence == 0) {
                if (hasExternalDiagonals(n, x, y)) {
                    int numExt = countExternalDiagonals(n, x, y);
                    
                    if (numExt == 1) {
                        fills[y][x] = FillType::NO_CORNER;
                    }
                    else if (numExt == 2) {
                        Node* top = (y - 1.0 < 0 || n->top != NULL) ? NULL : graph->at(y - 1.0)->at(x);
                        Node* bottom = (y + 1.0 >= graph->size() || n->bottom != NULL) ? NULL : graph->at(y + 1.0)->at(x);

                        bool tlCorner = top != NULL && top->bottomLeft != NULL;
                        bool trCorner = top != NULL && top->bottomRight != NULL;
                        bool blCorner = bottom != NULL && bottom->topLeft != NULL;
                        bool brCorner = bottom != NULL && bottom->topRight != NULL;

                        if ((tlCorner && brCorner) || (trCorner && blCorner)) {
                            fills[y][x] = FillType::DIAG_END;
                        }
                        else {
                            fills[y][x] = FillType::CARD_END;
                        }
                    }
                    else if (numExt == 3) {
                        fills[y][x] = FillType::THREE_CORNER;
                    }
                    else {
                        fills[y][x] = FillType::FOUR_CORNER;
                    }
                }

                fills[y][x] = FillType::FULL;
            } else if (valence == 1) {
                // either diagonal end or cardinal end
                if (n->top != NULL || n->left != NULL || n->right != NULL || n->bottom != NULL) {
                    fills[y][x] = FillType::CARD_END;
                }
                else {
                    fills[y][x] = FillType::DIAG_END;
                }
            }
            else if (valence == 2) {
                if ((n->top != NULL && n->bottom != NULL) || (n->left != NULL && n->right != NULL)) {
                    // straight line through cardinals
                    fills[y][x] = FillType::FULL;
                }
                else if ((n->topLeft != NULL && n->bottomRight != NULL) || (n->topRight != NULL && n->bottomLeft != NULL)) {
                    // true diagonal
                    fills[y][x] = FillType::DIAGONAL;
                }
                else {
                    // does n have any diagonal or cardinal connections?
                    bool nHasDiagonals = n->bottomLeft != NULL || n->bottomRight != NULL || n->topLeft != NULL || n->topRight != NULL;
                    bool nHasCardinals = n->top != NULL || n->bottom != NULL || n->left != NULL || n->right != NULL;

                    // are there detached diagonals?
                    bool otherDiagonals = hasExternalDiagonals(n, x, y);
                    
                    // diagonal + cardinal, or just cardinals + other diagonals around you
                    if ((nHasDiagonals && nHasCardinals) || (nHasCardinals && !nHasDiagonals && otherDiagonals)) {
                        fills[y][x] = FillType::NO_CORNER;
                    }
                    else if (nHasDiagonals && !nHasCardinals) {
                        // only diagonals, but not in a line
                        fills[y][x] = FillType::CARD_END; // this seems weird, but it equates to the same pattern
                    }
                    else {
                        // other condition
                        fills[y][x] = FillType::FULL;
                    }
                }
            }
            else {
                // valence 3+ all have very similar rules (2 has some special cases, so we handle it separately)
                if ((n->top != NULL && n->bottom != NULL) || (n->left != NULL && n->right != NULL)) {
                    // straight line through cardinals
                    fills[y][x] = FillType::FULL;
                }
                else if ((n->topLeft != NULL && n->bottomRight != NULL) || (n->topRight != NULL && n->bottomLeft != NULL)) {
                    // true diagonal
                    fills[y][x] = FillType::NO_CORNER;
                }
                else {
                    // now we need to check for diagonals that are NOT source this colour!
                    bool otherDiagonals = hasExternalDiagonals(n, x, y);

                    // we can also have an elbow in a limited set of valence 3 scenarios, which we'll check for here as well
                    // I think it's a little more readable to extract the bool logic here instead of just put it all in a big if statement
                    bool topElbow = n->topLeft != NULL && n->top != NULL && n->topRight != NULL;
                    bool rightElbow = n->topRight != NULL && n->right != NULL && n->bottomRight != NULL;
                    bool bottomElbow = n->bottomRight != NULL && n->bottom != NULL && n->bottomLeft != NULL;
                    bool leftElbow = n->bottomLeft != NULL && n->left != NULL && n->topLeft != NULL;
                    bool hasElbow = topElbow || rightElbow || leftElbow || bottomElbow;

                    if (valence == 3 && hasElbow) {
                        fills[y][x] = FillType::CARD_END;
                    } else if (otherDiagonals) {
                        fills[y][x] = FillType::NO_CORNER;
                    }
                    else {
                        fills[y][x] = FillType::FULL;
                    }
                }
            }
        }
    }
}

void PixelGraph::computeVoronoi(cv::Mat* m) {
    std::vector<std::vector<FillType>> fills;

    for (int y = 0; y < graph->size(); y++) {
        std::vector<FillType> f;
        for (int x = 0; x < graph->at(0)->size(); x++) {
            f.push_back(FillType::NONE);
        }
        fills.push_back(f);
    }

    calculateFillType(fills);

    for (int y = 0; y < graph->size(); y++) {
        for (int x = 0; x < graph->at(0)->size(); x++) {
            cv::Mat temp = cv::Mat(4, 4, img->type());
            cv::Point p = cv::Point(x, y);

            switch (fills[y][x]) {
            case(FillType::CARD_END):
                renderCardEndPixel(temp, p);
                break;
            case (FillType::DIAGONAL):
                renderDiagonalPixel(temp, p);
                break;
            case (FillType::DIAG_END):
                renderDiagEndPixel(temp, p);
                break;
            case (FillType::FULL):
                renderFullPixel(temp, p);
                break;
            case (FillType::NO_CORNER):
                renderNoCornerPixel(temp, p);
                break;
            case (FillType::FOUR_CORNER):
                renderFourDiag(temp, p);
                break;
            case(FillType::THREE_CORNER):
                renderThreeDiag(temp, p);
                break;
            default:
                renderNonePixel(temp, p);
            }

            for (int b = 0; b < 4; b++) {
                for (int a = 0; a < 4; a++) {
                    cv::Point mPt = cv::Point(x * 4 + a, y * 4 + b);
                    cv::Point tPt = cv::Point(a, b);
                    m->at<cv::Vec3b>(mPt) = temp.at<cv::Vec3b>(tPt);
                }
            }
        }
    }
}

void PixelGraph::renderFullPixel(cv::Mat& m, cv::Point p) {
    // filling with colour
    cv::Vec3b color = img->at<cv::Vec3b>(p);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            m.at<cv::Vec3b>(cv::Point(x, y)) = color;
        }
    }
}

void PixelGraph::renderNoCornerPixel(cv::Mat& m, cv::Point p) {
    // flood with colour first, and then figure out which corner to change
    cv::Vec3b color = img->at<cv::Vec3b>(p);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            m.at<cv::Vec3b>(cv::Point(x, y)) = color;
        }
    }

    Node* n = graph->at(p.y)->at(p.x);
    Node* top = (p.y - 1 < 0 || n->top != NULL) ? NULL : graph->at(p.y - 1)->at(p.x);
    Node* bottom = (p.y + 1 > graph->size() || n->bottom != NULL) ? NULL : graph->at(p.y + 1)->at(p.x);

    if (top != NULL && top->bottomRight != NULL) {
        // topRight corner
        cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
        cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
        m.at<cv::Vec3b>(cv::Point(2, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = rightColor;
    }
    else if (top != NULL && top->bottomLeft != NULL) {
        // topleft corner
        cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
        cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));
        m.at<cv::Vec3b>(cv::Point(0, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(1, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 1)) = leftColor;
    }
    else if (bottom != NULL && bottom->topRight != NULL) {
        // bottomRight corner
        cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));
        cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
        m.at<cv::Vec3b>(cv::Point(3, 2)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = rightColor;
    }
    else if (bottom != NULL && bottom->topLeft != NULL) {
        // bottomLeft corner - we're explicitly checking this to avoid unexpected data
        cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));
        cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));
        m.at<cv::Vec3b>(cv::Point(0, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(1, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 2)) = leftColor;
    }
}

void PixelGraph::renderNonePixel(cv::Mat& m, cv::Point p) {
    // flood of red for debugging
    cv::Vec3b color = cv::Vec3b(1.0f, 0.0f, 0.0f);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            m.at<cv::Vec3b>(cv::Point(x, y)) = color;
        }
    }
}

void PixelGraph::renderDiagonalPixel(cv::Mat& m, cv::Point p) {
    // flood with colour first, and then figure out which corners to change
    cv::Vec3b color = img->at<cv::Vec3b>(p);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            m.at<cv::Vec3b>(cv::Point(x, y)) = color;
        }
    }

    Node* n = graph->at(p.y)->at(p.x);

    cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
    cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
    cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));
    cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));

    if (n->topLeft != NULL && n->bottomRight != NULL) {
        // this diagonal goes \  //
        m.at<cv::Vec3b>(cv::Point(2, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(0, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(1, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 2)) = leftColor;
    }
    else if (n->topRight != NULL && n->bottomLeft != NULL) {
        // this diagonal goes /
        m.at<cv::Vec3b>(cv::Point(1, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(0, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 1)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = rightColor;
    }
}

void PixelGraph::renderCardEndPixel(cv::Mat& m, cv::Point p) {
    cv::Vec3b color = img->at<cv::Vec3b>(p);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            m.at<cv::Vec3b>(cv::Point(x, y)) = color;
        }
    }

    Node* n = graph->at(p.y)->at(p.x);

    // we check for a few different cases here since we may have valence 1, 2, or 3 nodes here.
    if (n->top != NULL || (n->topLeft != NULL && n->topRight != NULL)) {
        // connected through the top (and/or elbow through the top)
        cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
        cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));
        cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));

        m.at<cv::Vec3b>(cv::Point(0, 2)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 3)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(1, 3)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 2)) = rightColor;
    }
    else if (n->bottom != NULL || (n->bottomLeft != NULL && n->bottomRight != NULL)) {
        // connected through the bottom (and/or elbow through the bottom)
        cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
        cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
        cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));

        m.at<cv::Vec3b>(cv::Point(0, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 1)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(1, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(2, 0)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = rightColor;

    }
    else if (n->right != NULL || (n->topRight != NULL && n->bottomRight != NULL)) {
        // connected through the right (and/or elbow through the right)
        cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
        cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));
        cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));

        m.at<cv::Vec3b>(cv::Point(1, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(0, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(0, 1)) = topColor;
        m.at<cv::Vec3b>(cv::Point(0, 2)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(1, 3)) = bottomColor;

    }
    else if (n->left != NULL || (n->topLeft != NULL && n->bottomLeft != NULL)) {
        // connected through the left (and/or elbow through the left)
        cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
        cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
        cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));

        m.at<cv::Vec3b>(cv::Point(2, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 2)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = bottomColor;
    }
}

void PixelGraph::renderDiagEndPixel(cv::Mat& m, cv::Point p) {
    // flood with colour first, and then figure out which corners to change
    cv::Vec3b color = img->at<cv::Vec3b>(p);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            m.at<cv::Vec3b>(cv::Point(x, y)) = color;
        }
    }

    Node* n = graph->at(p.y)->at(p.x);

    cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
    cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
    cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));
    cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));

    if (n->bottomRight != NULL) {
        // this diagonal goes \  //
        m.at<cv::Vec3b>(cv::Point(2, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = topColor;
        m.at<cv::Vec3b>(cv::Point(0, 3)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(1, 3)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 2)) = leftColor;
    }
    else if (n->bottomLeft != NULL) {
        // this diagonal goes /
        m.at<cv::Vec3b>(cv::Point(1, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(0, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(0, 1)) = topColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 2)) = rightColor;
    }
    else if (n->topLeft != NULL) {
        // this one goes \ //
        m.at<cv::Vec3b>(cv::Point(2, 0)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(0, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(1, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 2)) = bottomColor;
    }
    else if (n->topRight != NULL) {
        // this one goes /
        m.at<cv::Vec3b>(cv::Point(1, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 1)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(3, 2)) = bottomColor;
    }
}

void PixelGraph::renderFourDiag(cv::Mat& m, cv::Point p) {
    // flood with colour first, and then figure out which corners to change
    cv::Vec3b color = img->at<cv::Vec3b>(p);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            m.at<cv::Vec3b>(cv::Point(x, y)) = color;
        }
    }

    Node* n = graph->at(p.y)->at(p.x);

    cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
    cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
    cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));
    cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));

    m.at<cv::Vec3b>(cv::Point(0, 0)) = topColor;
    m.at<cv::Vec3b>(cv::Point(3, 0)) = rightColor;
    m.at<cv::Vec3b>(cv::Point(3, 3)) = bottomColor;
    m.at<cv::Vec3b>(cv::Point(0, 3)) = leftColor;
}

void PixelGraph::renderThreeDiag(cv::Mat& m, cv::Point p) {
    // flood with colour first, and then figure out which corners to change
    cv::Vec3b color = img->at<cv::Vec3b>(p);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            m.at<cv::Vec3b>(cv::Point(x, y)) = color;
        }
    }

    Node* n = graph->at(p.y)->at(p.x);
    Node* top = (p.y - 1.0 < 0 || n->top != NULL) ? NULL : graph->at(p.y - 1.0)->at(p.x);
    Node* bottom = (p.y + 1.0 >= graph->size() || n->bottom != NULL) ? NULL : graph->at(p.y + 1.0)->at(p.x);

    cv::Vec3b topColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y - 1));
    cv::Vec3b rightColor = img->at<cv::Vec3b>(cv::Point(p.x + 1, p.y));
    cv::Vec3b bottomColor = img->at<cv::Vec3b>(cv::Point(p.x, p.y + 1));
    cv::Vec3b leftColor = img->at<cv::Vec3b>(cv::Point(p.x - 1, p.y));

    if (top != NULL && top->bottomLeft == NULL) {
        // topLeft corner filled
        m.at<cv::Vec3b>(cv::Point(2, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 2)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(1, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 2)) = bottomColor;
    }
    else if (top != NULL && top->bottomRight == NULL) {
        // topRight corner filled
        m.at<cv::Vec3b>(cv::Point(3, 2)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(1, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 2)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 1)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(1, 0)) = leftColor;
    }
    else if (bottom != NULL && bottom->topRight == NULL) {
        // bottomRight corner filled
        m.at<cv::Vec3b>(cv::Point(1, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 3)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 2)) = bottomColor;
        m.at<cv::Vec3b>(cv::Point(0, 1)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(1, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(2, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = topColor;
    }
    else if (bottom != NULL && bottom->topLeft == NULL) {
        // bottomLeft corner filled
        m.at<cv::Vec3b>(cv::Point(0, 1)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(0, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(1, 0)) = leftColor;
        m.at<cv::Vec3b>(cv::Point(2, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 0)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 1)) = topColor;
        m.at<cv::Vec3b>(cv::Point(3, 2)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(3, 3)) = rightColor;
        m.at<cv::Vec3b>(cv::Point(2, 3)) = rightColor;
    }
}

bool PixelGraph::hasExternalDiagonals(Node* n, int x, int y) {
    // now we need to check for diagonals not attached to n!
    Node* top = (y - 1.0 < 0 || n->top != NULL) ? NULL : graph->at(y - 1.0)->at(x);
    Node* bottom = (y + 1.0 >= graph->size() || n->bottom != NULL) ? NULL : graph->at(y + 1.0)->at(x);

    // are there detached diagonals?
    bool topDiagonals = top != NULL && (top->bottomLeft != NULL || top->bottomRight != NULL);
    bool bottomDiagonals = bottom != NULL && (bottom->topLeft != NULL || bottom->topRight != NULL);
    bool otherDiagonals = topDiagonals || bottomDiagonals;
    
    return otherDiagonals;
}

int PixelGraph::countExternalDiagonals(Node* n, int x, int y) {
    Node* top = (y - 1.0 < 0 || n->top != NULL) ? NULL : graph->at(y - 1.0)->at(x);
    Node* bottom = (y + 1.0 >= graph->size() || n->bottom != NULL) ? NULL : graph->at(y + 1.0)->at(x);

    int diagonals = 0;

    if (top != NULL && top->bottomLeft != NULL) diagonals++;
    if (top != NULL && top->bottomRight != NULL) diagonals++;
    if (bottom != NULL && bottom->topLeft != NULL) diagonals++;
    if (bottom != NULL && bottom->topRight != NULL) diagonals++;

    return diagonals;
}

void PixelGraph::computeAllPolygons(std::vector<Polygon>& polygons) {
    int numRows = graph->size();
    int numCols = graph->at(0)->size();

    polygons.clear();

    // Create all intersections
    grid.resize(numRows + 1.0);
    for (int r = 0; r < numRows + 1; r++) {
        grid[r].resize(numCols + 1.0);
    }
    // Initialize intersection properties
    for (int r = 0; r < numRows + 1; r++) {
        for (int c = 0; c < numCols + 1; c++) {
            initGridIntersection(r, c, grid[r][c]);
        }
    }

    // Begin checking each intersection for remaining contours to walk
    for (int r = 0; r < numRows + 1; r++) {
        for (int c = 0; c < numCols + 1; c++) {
            GridIntersection* start = &grid[r][c];
            // Exhaust all contours that include this intersection
            while (!start->contourWalkMap.empty()) {
                // Create a new polygon
                Polygon& poly = polygons.emplace_back();

                // Set the source node to be any node CCW along a contour
                GridIntersection* source = start->contourWalkMap.begin()->first;
                GridIntersection* through = start;

                // Determine the color of the polygon
                if (source->pos.x < through->pos.x - 0.5f) {
                    // Coming from left, so use bottom left pixel
                    poly.color = img->at<cv::Vec3b>(through->pos.y, through->pos.x - 1);
                }
                else if (source->pos.y < through->pos.y - 0.5f) {
                    // Coming from top, so use top left pixel
                    poly.color = img->at<cv::Vec3b>(through->pos.y - 1, through->pos.x - 1);
                }
                else if (source->pos.x > through->pos.x + 0.5f) {
                    // Coming from right, so use top right pixel
                    poly.color = img->at<cv::Vec3b>(through->pos.y - 1, through->pos.x);
                }
                else if (source->pos.y > through->pos.y + 0.5f) {
                    // Coming from bottom, so use bottom right pixel
                    poly.color = img->at<cv::Vec3b>(through->pos.y, through->pos.x);
                }

                int turnCount = 0;
                do {
                    // Walk to the next neighbor and erase the connection behind us
                    WalkInfo info = through->contourWalkMap[source];
                    through->contourWalkMap.erase(source);

                    // Add the new corner
                    poly.contour.push_back(info.cornerPosition);

                    if (source->pos.x < through->pos.x - 0.5f) {
                        // Coming from left

                        if (info.destination->pos.y < through->pos.y - 0.5f) {
                            // Going to top
                            turnCount--;
                        } else if (info.destination->pos.y > through->pos.y + 0.5f) {
                            // Going to bottom
                            turnCount++;
                        }
                    } else if (source->pos.y < through->pos.y - 0.5f) {
                        // Coming from top

                        if (info.destination->pos.x < through->pos.x - 0.5f) {
                            // Going to left
                            turnCount++;
                        } else if (info.destination->pos.x > through->pos.x + 0.5f) {
                            // Going to right
                            turnCount--;
                        }
                    } else if (source->pos.x > through->pos.x + 0.5f) {
                        // Coming from right

                        if (info.destination->pos.y < through->pos.y - 0.5f) {
                            // Going to top
                            turnCount++;
                        } else if (info.destination->pos.y > through->pos.y + 0.5f) {
                            // Going to bottom
                            turnCount--;
                        }
                    } else if (source->pos.y > through->pos.y + 0.5f) {
                        // Coming from bottom

                        if (info.destination->pos.x < through->pos.x - 0.5f) {
                            // Going to left
                            turnCount--;
                        } else if (info.destination->pos.x > through->pos.x + 0.5f) {
                            // Going to right
                            turnCount++;
                        }
                    }

                    // Update the source and through intersections
                    source = through;
                    through = info.destination;
                } while (through != start);

                // Discard the polygon if the cumulative number of CW - CCW turns was not 4 (we only want CW contours)
                if (turnCount != 4) {
                    polygons.pop_back();
                }
            }
        }
    }

    /* I don't think we need this since we discover the polygons from top-left to bottom-right
    // If any polygons are contained inside another polygon, add their contour as a hole in the outer polygon
    for (int i = 0; i < polygons.size(); i++) {
        for (int j = 0; j < polygons.size(); j++) {
            if (i == j) {
                continue;
            }
            // If the point p is contained inside an outer polygon, increase the depth of the inner polygon
            cv::Point2f p = polygons[j].contour[0];
            if (cv::pointPolygonTest(polygons[i].contour, p, false)) {
                polygons[j].depth++;
            }
        }
    }

    // Sort polygons by depth
    std::sort(polygons.begin(), polygons.end(), [](Polygon& a, Polygon& b) {
        return a.depth > b.depth;
    });
    */
}

PixelGraph::WalkInfo::WalkInfo() {
    this->destination = nullptr;
    this->cornerPosition = cv::Point2f();
}

PixelGraph::WalkInfo::WalkInfo(GridIntersection* destination, cv::Point2f cornerPosition) {
    this->destination = destination;
    this->cornerPosition = cornerPosition;
}

PixelGraph::GridIntersection* PixelGraph::getIntersectionIfExists(int r, int c) {
    if (r < 0 || r >= grid.size() || c < 0 || c >= grid[r].size()) {
        return nullptr;
    }
    return &grid[r][c];
}

void PixelGraph::initGridIntersection(int row, int col, PixelGraph::GridIntersection& intersection) {
    intersection.pos = cv::Point2i(col, row);

    // Get the 4 surrounding nodes that meet at this intersection
    Node* topLeft = getIfExists(row - 1, col - 1);
    Node* topRight = getIfExists(row - 1, col);
    Node* bottomLeft = getIfExists(row, col - 1);
    Node* bottomRight = getIfExists(row, col);

    // Get the 4 adjacent intersections
    GridIntersection* left = getIntersectionIfExists(row, col - 1);
    GridIntersection* top = getIntersectionIfExists(row - 1, col);
    GridIntersection* right = getIntersectionIfExists(row, col + 1);
    GridIntersection* bottom = getIntersectionIfExists(row + 1, col);

    auto& map = intersection.contourWalkMap;
    auto p = intersection.pos;

    // Top left must exist and not be connected to other nodes
    if (topLeft && !topLeft->right && !topLeft->bottom && !topLeft->bottomRight) {
        // "/" diagonal of a different color, add the CCW direction and also shift the corner
        if (topRight && topRight->bottomLeft) {
            cv::Point2f cornerPoint(p.x - 0.25f, p.y - 0.25f);
            map[top] = WalkInfo(left, cornerPoint);
            map[left] = WalkInfo(top, cornerPoint);
        } else {
            map[top] = WalkInfo(left, p);
        }
    }

    // Top right must exist and not be connected to other nodes
    if (topRight && !topRight->left && !topRight->bottom && !topRight->bottomLeft) {
        // "\" diagonal of a different color, add the CCW direction and also shift the corner
        if (topLeft && topLeft->bottomRight) {
            cv::Point2f cornerPoint(p.x + 0.25f, p.y - 0.25f);
            map[right] = WalkInfo(top, cornerPoint);
            map[top] = WalkInfo(right, cornerPoint);
        } else {
            map[right] = WalkInfo(top, p);
        }
    }

    // Bottom left must exist and not be connected to other nodes
    if (bottomLeft && !bottomLeft->right && !bottomLeft->top && !bottomLeft->topRight) {
        // "\" diagonal of a different color, add the CCW direction and also shift the corner
        if (topLeft && topLeft->bottomRight) {
            cv::Point2f cornerPoint(p.x - 0.25f, p.y + 0.25f);
            map[left] = WalkInfo(bottom, cornerPoint);
            map[bottom] = WalkInfo(left, cornerPoint);
        } else {
            map[left] = WalkInfo(bottom, p);
        }
    }

    // Bottom right must exist and not be connected to other nodes
    if (bottomRight && !bottomRight->left && !bottomRight->top && !bottomRight->topLeft) {
        // "/" diagonal of a different color, add the CCW direction and also shift the corner
        if (topRight && topRight->bottomLeft) {
            cv::Point2f cornerPoint(p.x + 0.25f, p.y + 0.25f);
            map[bottom] = WalkInfo(right, cornerPoint);
            map[right] = WalkInfo(bottom, cornerPoint);
        } else {
            map[bottom] = WalkInfo(right, p);
        }
    }

    // The top nodes exist and are connected to each other but not the rest
    if (topLeft && topLeft->right && !topLeft->bottom && !topLeft->bottomRight) {
        map[right] = WalkInfo(left, p);
    }

    // The bottom nodes exist and are connected to each other but not the rest
    if (bottomLeft && bottomLeft->right && !bottomLeft->top && !bottomLeft->topRight) {
        map[left] = WalkInfo(right, p);
    }

    // The left nodes exist and are connected to each other but not the rest
    if (topLeft && topLeft->bottom && !topLeft->right && !topLeft->bottomRight) {
        map[top] = WalkInfo(bottom, p);
    }

    // The right nodes exist and are connected to each other but not the rest
    if (topRight && topRight->bottom && !topRight->left && !topRight->bottomLeft) {
        map[bottom] = WalkInfo(top, p);
    }
}