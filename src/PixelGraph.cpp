#include "PixelGraph.h"

//using namespace std;
//using namespace cv;

PixelGraph::PixelGraph() {
    img = NULL;
}

PixelGraph::PixelGraph(cv::Mat* img) {
    this->graph = new std::vector<std::vector<Node*>*>();

    for (int i = 0; i < img->rows; i++) {
        std::vector<Node*>* line = new std::vector<Node*>();

        for (int j = 0; j < img->cols; j+=3) {
            line->push_back(new Node(cv::Point2f(j,i)));
        }

        graph->push_back(new std::vector<Node*>());
        
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
}