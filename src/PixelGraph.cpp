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

void PixelGraph::generateGraph() {
    // CV_8UC3
    for (int i = 0; i < graph->size(); i++) {
        for (int j = 0; j < graph->at(i)->size(); j++) {
            Node* n = graph->at(i)->at(j);

            if (i == 0) {
                n->bottom = graph->at(i + 1)->at(j);

                if (j == 0) {
                    n->bottomRight = graph->at(i + 1)->at(j + 1);
                    n->right = graph->at(i)->at(j + 1);
                }
                else if (j == img->cols - 1) {
                    n->bottomLeft = graph->at(i + 1)->at(j - 1);
                    n->left = graph->at(i)->at(j - 1);
                }
                else {
                    n->bottomRight = graph->at(i + 1)->at(j + 1);
                    n->right = graph->at(i)->at(j + 1);
                    n->bottomLeft = graph->at(i + 1)->at(j - 1);
                    n->left = graph->at(i)->at(j - 1);
                }
            }
            else if (i == img->rows - 1) {
                n->top = graph->at(i - 1)->at(j);
                
                if (j == 0) {
                    n->topRight = graph->at(i - 1)->at(j + 1);
                    n->right = graph->at(i)->at(j + 1);
                }
                else if (j == img->cols - 1) {
                    n->topLeft = graph->at(i - 1)->at(j - 1);
                    n->left = graph->at(i)->at(j - 1);
                }
                else {
                    n->topRight = graph->at(i - 1)->at(j + 1);
                    n->right = graph->at(i)->at(j + 1);
                    n->topLeft = graph->at(i - 1)->at(j - 1);
                    n->left = graph->at(i)->at(j - 1);
                }
            }
            else {
                n->top = graph->at(i - 1)->at(j);
                n->bottom = graph->at(i + 1)->at(j);

                if (j == 0) {
                    n->topRight = graph->at(i - 1)->at(j + 1);
                    n->right = graph->at(i)->at(j + 1);
                    n->bottomRight = graph->at(i + 1)->at(j + 1);
                }
                else if (j == img->cols - 1) {
                    n->topLeft = graph->at(i - 1)->at(j - 1);
                    n->left = graph->at(i)->at(j - 1);
                    n->bottomLeft = graph->at(i - 1)->at(j - 1);
                }
                else {
                    n->topRight = graph->at(i - 1)->at(j + 1);
                    n->right = graph->at(i)->at(j + 1);
                    n->bottomRight = graph->at(i + 1)->at(j + 1);
                    n->topLeft = graph->at(i - 1)->at(j - 1);
                    n->left = graph->at(i)->at(j - 1);
                    n->bottomLeft = graph->at(i - 1)->at(j - 1);
                }
            }
        }
    }
}

void PixelGraph::brutePrune() {
    // break up based on colour
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
