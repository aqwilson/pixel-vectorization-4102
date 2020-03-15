#include "PixelGraph.h"

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

                runCurveHeuristic(cv::Point2f(i, j), &curveWeights);
                runSparseHeuristic(cv::Point2f(i, j), &sparseWeights);
                runIslandHeuristic(cv::Point2f(i, j), &islandWeights);

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
}

void PixelGraph::runIslandHeuristic(cv::Point topLeft, cv::Point2f* weightVals){
    weightVals->x = 0.0f;
    weightVals->y = 0.0f;
}
