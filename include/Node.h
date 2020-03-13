#ifndef NODE_H
#define NODE_H
#endif

#include <opencv2/core.hpp>

class Node {

	// connections to other nodes, wrt directionality
public:
	Node* top;
	Node* bottom;
	Node* left;
	Node* right;
	Node* topLeft;
	Node* topRight;
	Node* bottomLeft;
	Node* bottomRight;

	cv::Point2f pos;

	Node();
	Node(cv::Point2f);
	~Node();

};
