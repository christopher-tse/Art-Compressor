
#include "toqutree.h"

toqutree::Node::Node(pair<int,int> ctr, int dim, HSLAPixel a)
	:center(ctr),dimension(dim),avg(a),NW(NULL),NE(NULL),SE(NULL),SW(NULL)
	{}

toqutree::~toqutree(){
	clear(root);

}

toqutree::toqutree(const toqutree & other) {
	root = copy(other.root);
}


toqutree & toqutree::operator=(const toqutree & rhs){
	if (this != &rhs) {
		clear(root);
		root = copy(rhs.root);
	}
	return *this;
}

toqutree::toqutree(PNG & imIn, int k){

/* This constructor grabs the 2^k x 2^k sub-image centered */
/* in imIn and uses it to build a quadtree. It may assume  */
/* that imIn is large enough to contain an image of that size. */

	int width = imIn.width();
	int height = imIn.height();
	int subWidth = pow(2, k);
	int subHeight = pow(2, k);
	int centerX = (width - subWidth) / 2;
	int centerY = (height - subHeight) / 2;
	PNG * subImg = new PNG(subWidth, subHeight);
	int imgDim = width * height;
	int subImgDim = subWidth * subHeight;

	if (imgDim == subImgDim) {
		*subImg = imIn;
		root = buildTree(subImg, k);
	} else {
		for (int x = 0; x < subWidth; x++) {
			for (int y = 0; y < subHeight; y++) {
				*subImg->getPixel(x,y) = *imIn.getPixel(centerX + x, centerY + y);
			}
		}
		root = buildTree(subImg, k);
	}

	delete subImg;
	subImg = NULL;

}

int toqutree::size() {
	if (root == NULL) {
		return 0;
	}
	return getSize(root);
}

int toqutree::getSize(Node* node) {
	if (node == NULL) {
		return 0;
	}
	return 1 + getSize(node->SE) + getSize(node->SW) +
					getSize(node->NE) + getSize(node->NW);
}

toqutree::Node * toqutree::buildTree(PNG * im, int k) {
	stats * sptr = new stats(*im);
	pair<int, int> splitPoint;
	double currLowestEntropy;
	int greenDim = pow(2, k-1);
	int imDim = pow(2, k);
	pair<int, int> origin(0,0);
	pair<int, int> lowerRight(imDim-1, imDim-1);

	if (k == 0) {
		Node * nodePtr = new Node(origin, 0, sptr->getAvg(origin, origin));
		nodePtr->SE = NULL;
		nodePtr->SW = NULL;
		nodePtr->NE = NULL;
		nodePtr->NW = NULL;
		delete sptr;
		sptr = NULL;
		return nodePtr;
	}

	//getting "green region"
	int originToUL = (pow(2, k) / 4);
	int ulToLR = (pow(2, k) / 2) -1;
	pair<int, int> ul(originToUL, originToUL);
	pair<int, int> lr(ul.first + ulToLR, ul.second + ulToLR);

	//iterate through the green region to find which point should be the split
	int countX = 0;
	int countY = 0;
	currLowestEntropy = getAvgEntropy(ul, pow(2, k-1), pow(2, k), countX, sptr, countY);
	splitPoint.first = ul.first;
	splitPoint.second = ul.second;
	for (int x = ul.first; x < lr.second + 1; x++) {
		for (int y = ul.second; y < lr.second + 1; y++) {
			pair<int, int> ul(x, y);
			double thisEntropy = getAvgEntropy(ul, pow(2, k-1), pow(2, k), countX, sptr, countY);
			if (thisEntropy < currLowestEntropy) {
				currLowestEntropy = thisEntropy;
				splitPoint.first = x;
				splitPoint.second = y;
			}
			countY++;
		}
		countY = 0; //reset for next column
		countX++;
	}

	PNG * seImgPtr;
	PNG * swImgPtr;
	PNG * neImgPtr;
	PNG * nwImgPtr;

	//SE
	pair<int, int> ulSE = splitPoint;
	pair<int, int> lrSE;
	lrSE.first = (ulSE.first + (greenDim -1)) % imDim;
	lrSE.second = (ulSE.second + (greenDim - 1)) % imDim;
	seImgPtr = generateChild(im, ulSE, greenDim, imDim);
	//SW
	pair<int, int> ulSW;
	pair<int, int> lrSW;
	ulSW.first = (ulSE.first + greenDim) % imDim;
	ulSW.second = ulSE.second;
	lrSW.first = (ulSW.first + (greenDim - 1)) % imDim;
	lrSW.second = (ulSW.second + (greenDim - 1)) % imDim;
	swImgPtr = generateChild(im, ulSW, greenDim, imDim);
	//NE
	pair<int, int> ulNE;
	pair<int, int> lrNE;
	ulNE.first = ulSE.first;
	ulNE.second = (ulSE.second + greenDim) % imDim;
	lrNE.first = (ulNE.first + (greenDim - 1)) % imDim;
	lrNE.second = (ulNE.second + (greenDim - 1)) % imDim;
	neImgPtr = generateChild(im, ulNE, greenDim, imDim);
	//Nw
	pair<int, int> ulNW;
	pair<int, int> lrNW;
	ulNW.first = (ulSE.first + greenDim) % imDim;
	ulNW.second = (ulSE.second + greenDim) % imDim;
	lrNW.first = (ulNW.first + (greenDim - 1)) % imDim;
	lrNW.second = (ulNW.second + (greenDim - 1)) % imDim;
	nwImgPtr = generateChild(im, ulNW, greenDim, imDim);


	Node * nodePtr = new Node(splitPoint, k, sptr->getAvg(origin, lowerRight));
	nodePtr->SE = buildTree(seImgPtr, k-1);
	nodePtr->SW = buildTree(swImgPtr, k-1);
	nodePtr->NE = buildTree(neImgPtr, k-1);
	nodePtr->NW = buildTree(nwImgPtr, k-1);
	delete sptr;
	delete seImgPtr;
	delete swImgPtr;
	delete neImgPtr;
	delete nwImgPtr;
	sptr = NULL;
	seImgPtr = NULL;
	swImgPtr = NULL;
	neImgPtr = NULL;
	nwImgPtr = NULL;

	return nodePtr;
}

PNG * toqutree::generateChild(PNG * parentImg, pair<int, int> ul, int greenDim, int imDim) {
	PNG * img = new PNG(greenDim, greenDim);
	for(int x = 0; x < greenDim; x++) {
		for (int y = 0; y < greenDim; y++) {
			int parentX = (ul.first + x) % imDim;
			int parentY = (ul.second + y) % imDim;
			*img->getPixel(x,y) = *parentImg->getPixel(parentX, parentY);
		}
	}
	return img;
}

double toqutree::getAvgEntropy(pair<int, int> ulSE, int greenDim, int imDim, int adjX, stats * s, int adjY) {
	//Entropy SE
	pair<int, int> lrSE;
	lrSE.first = (ulSE.first + (greenDim - 1)) % imDim;
	lrSE.second = (ulSE.second + (greenDim - 1)) % imDim;
	double seEntropy = getEntropy(ulSE, lrSE, adjX, s, greenDim, adjY, imDim);
	//Entropy SW
	pair<int, int> ulSW;
	pair<int, int> lrSW;
	ulSW.first = (ulSE.first + greenDim) % imDim;
	ulSW.second = ulSE.second;
	lrSW.first = (ulSW.first + (greenDim - 1)) % imDim;
	lrSW.second = (ulSW.second + (greenDim - 1)) % imDim;
	double swEntropy = getEntropy(ulSW, lrSW, adjX, s, greenDim, adjY, imDim);
	//Entropy NE
	pair<int, int> ulNE;
	pair<int, int> lrNE;
	ulNE.first = ulSE.first;
	ulNE.second = (ulSE.second + greenDim) % imDim;
	lrNE.first = (ulNE.first + (greenDim - 1)) % imDim;
	lrNE.second = (ulNE.second + (greenDim - 1)) % imDim;
	double neEntropy = getEntropy(ulNE, lrNE, adjX, s, greenDim, adjY, imDim);
	//Entropy NW
	pair<int, int> ulNW;
	pair<int, int> lrNW;
	ulNW.first = (ulSE.first + greenDim) % imDim;
	ulNW.second = (ulSE.second + greenDim) % imDim;
	lrNW.first = (ulNW.first + (greenDim - 1)) % imDim;
	lrNW.second = (ulNW.second + (greenDim - 1)) % imDim;
	double nwEntropy = getEntropy(ulNW, lrNW, adjX, s, greenDim, adjY, imDim);

	//Sum all entropies then divide by 4 and return average entropy
	return (seEntropy + swEntropy + neEntropy + nwEntropy) / 4;
}

double toqutree::getEntropy(pair<int, int> ul, pair<int, int> lr, int adjX, stats * s, int greenDim, int adjY, int imDim) {
	vector< int> dist(36);

	if ((ul.first > lr.first) && (ul.second > lr.second)) {
		//NW
		pair<int, int> ulLRNW;
		pair<int, int> lrULNW;
		pair<int, int> urULNW;
		pair<int, int> urLRNW;
		pair<int, int> llULNW;
		pair<int, int> llLRNW;

		ulLRNW.first = (ul.first + ((greenDim / 2) - 1) - adjX) % imDim;
		ulLRNW.second = (ul.second +((greenDim / 2) - 1) - adjY) % imDim;
		lrULNW.first = (ul.first + (greenDim / 2) - adjX) % imDim;
		lrULNW.second = (ul.second + (greenDim / 2) - adjY) % imDim;
		urULNW.first = ul.first;
		urULNW.second = (ul.second + (greenDim / 2) - adjY) % imDim;
		urLRNW.first = (ul.first + ((greenDim / 2)- 1) - adjX) % imDim;
		urLRNW.second = lr.second;
		llULNW.first = (ul.first + (greenDim / 2) - adjX) % imDim;
		llULNW.second = ul.second;
		llLRNW.first = lr.first;
		llLRNW.second = (ul.first + ((greenDim / 2) - 1) - adjY) % imDim;

		vector< int> topLeft(36);
		vector< int> topRight(36);
		vector< int> bottomLeft(36);
		vector< int> bottomRight(36);
		topLeft = s->buildHist(lrULNW, lr);
		topRight = s->buildHist(urULNW, urLRNW);
		bottomLeft = s->buildHist(llULNW, llLRNW);
		bottomRight = s->buildHist(ul, ulLRNW);

		for (int i = 0; i < 36; i++) {
			dist[i] = topLeft[i] + topRight[i] + bottomLeft[i] + bottomRight[i];
		}
	} else if (ul.first > lr.first) {
		//SW
		pair<int, int> ulLRSW;
		pair<int, int> lrULSW;
		ulLRSW.first = (ul.first + ((greenDim / 2) - 1) - adjX) % imDim;
		ulLRSW.second = lr.second;
		lrULSW.first = (ul.first + (greenDim / 2) - adjX) % imDim;
		lrULSW.second = ul.second;

		vector< int> rightDist(36);
		vector< int> leftDist(36);
		rightDist = s->buildHist(ul, ulLRSW);
		leftDist = s->buildHist(lrULSW, lr);

		for(int i = 0; i < 36; i++) {
			dist[i] = rightDist[i] + leftDist[i];
		}
	} else if (ul.second > lr.second) {
		//NE
		pair<int, int> ulLRNE;
		pair<int, int> lrULNE;
		ulLRNE.first = lr.first;
		ulLRNE.second = (ul.second + ((greenDim / 2) -1) - adjY) % imDim;
		lrULNE.first = ul.first;
		lrULNE.second = (ul.second + (greenDim / 2) - adjY) % imDim;

		vector< int> bottomDist(36);
		vector< int> topDist(36);
		bottomDist = s->buildHist(ul, ulLRNE);
		topDist = s->buildHist(lrULNE, lr);

		for (int i = 0; i < 36; i++) {
			dist[i] = bottomDist[i] + topDist[i];
		}
	} else {
		//SE
		dist = s->buildHist(ul, lr);
	}

	return s->entropy(dist, pow(greenDim, 2));
}

PNG toqutree::render(){
	return renderTree(root);
}

PNG toqutree::renderTree(Node * node) {
	if (node == NULL) {
		PNG mt;
		return mt;
	}
	if (node->SE == NULL || node->SW == NULL || node->NE == NULL || node->NW == NULL) {
		int leafDim = pow(2, node->dimension);
		PNG leaf(leafDim, leafDim);
		for (int x = 0; x < leafDim; x++) {
			for (int y = 0; y < leafDim; y++) {
				*leaf.getPixel(x,y) = node->avg;
			}
		}
		return leaf;
	}

	int imDim = pow(2, node->dimension);
	int greenDim = pow(2, node->dimension-1);
	PNG img(imDim, imDim);

	PNG se = renderTree(node->SE);
	PNG sw = renderTree(node->SW);
	PNG ne = renderTree(node->NE);
	PNG nw = renderTree(node->NW);

	//SE
	pair<int, int> ulSE = node->center;
	//SW
	pair<int, int> ulSW;
	ulSW.first = (ulSE.first + greenDim) % imDim;
	ulSW.second = ulSE.second;
	//NE
	pair<int, int> ulNE;
	ulNE.first = ulSE.first;
	ulNE.second = (ulSE.second + greenDim) % imDim;
	//NW
	pair<int, int> ulNW;
	ulNW.first = (ulSE.first + greenDim) % imDim;
	ulNW.second = (ulSE.second + greenDim) % imDim;

	for(int x = 0; x < greenDim; x++) {
		for (int y = 0; y < greenDim; y++) {
			int newXSE = (x + ulSE.first) % imDim;
			int newYSE = (y + ulSE.second) % imDim;
			*img.getPixel(newXSE, newYSE) = *se.getPixel(x,y);
		}
	}

	for(int x = 0; x < greenDim; x++) {
		for (int y = 0; y < greenDim; y++) {
			int newX = (x + ulSW.first) % imDim;
			int newY = (y + ulSW.second) % imDim;
			*img.getPixel(newX, newY) = *sw.getPixel(x,y);
		}
	}

	for(int x = 0; x < greenDim; x++) {
		for (int y = 0; y < greenDim; y++) {
			int newX = (x + ulNE.first) % imDim;
			int newY = (y + ulNE.second) % imDim;
			*img.getPixel(newX, newY) = *ne.getPixel(x,y);
		}
	}

	for(int x = 0; x < greenDim; x++) {
		for (int y = 0; y < greenDim; y++) {
			int newX = (x + ulNW.first) % imDim;
			int newY = (y + ulNW.second) % imDim;
			*img.getPixel(newX, newY) = *nw.getPixel(x,y);
		}
	}
	return img;
}

void toqutree::prune(double tol) {
	prune(root,tol);
}

void toqutree::prune(Node * node, double tol) {
	if (node != NULL) {
		if (checkLeaves(node, node, tol)) {
			clear(node->SE);
			clear(node->SW);
			clear(node->NE);
			clear(node->NW);
		} else {
			prune(node->SE, tol);
			prune(node->SW, tol);
			prune(node->NE, tol);
			prune(node->NW, tol);
		}
	}
}

bool toqutree::checkLeaves(Node * parentNode, Node * node, double tol) {
	if (node->dimension == 0 || (node->SE == NULL && node->SW == NULL &&
	node->NE == NULL && node->NW == NULL)) {
		return isWithinTol(parentNode, node, tol);
	}
	return checkLeaves(parentNode, node->SE, tol) && checkLeaves(parentNode, node->SW, tol) &&
					checkLeaves(parentNode, node->NE, tol) && checkLeaves(parentNode, node->NW, tol);
}

bool toqutree::isWithinTol(Node * parentNode, Node * node, double tol) {
	double thisAvg = node->avg.dist(parentNode->avg);
	return thisAvg <= tol;
}

/* called by destructor and assignment operator*/
void toqutree::clear(Node * & curr){
	if (curr != NULL) {
		clear(curr->NW, curr->NE, curr->SE, curr->SW);
		delete curr;
		curr = NULL;
	}
}

void toqutree::clear(Node* nw, Node* ne, Node* se, Node* sw) {
	if (nw != NULL) {
		clear(nw->NW, nw->NE, nw->SE, nw->SW);
		delete nw;
		nw = NULL;
	}
	if (ne != NULL) {
		clear(ne->NW, ne->NE, ne->SE, ne->SW);
		delete ne;
		ne = NULL;
	}
	if (se != NULL) {
		clear(se->NW, se->NE, se->SE, se->SW);
		delete se;
		se = NULL;
	}
	if (sw != NULL) {
		clear(sw->NW, sw->NE, sw->SE, sw->SW);
		delete sw;
		sw = NULL;
	}
}

/* done */
/* called by assignment operator and copy constructor */
toqutree::Node * toqutree::copy(const Node * other) {
	if (other != NULL) {
		Node * n = new Node(other->center, other->dimension, other->avg);
		root = n;
		root->SE = copy(root->SE, other->SE);
		root->SW = copy(root->SW, other->SW);
		root->NE = copy(root->NE, other->NE);
		root->NW = copy(root->NW, other->NW);
	}
	return root;
}

toqutree::Node * toqutree::copy(Node* thisNode, Node * otherNode) {
	if (otherNode != NULL) {
		Node * n = new Node(otherNode->center, otherNode->dimension, otherNode->avg);
		thisNode = n;
		thisNode->SE = copy(thisNode->SE, otherNode->SE);
		thisNode->SW = copy(thisNode->SW, otherNode->SW);
		thisNode->NE = copy(thisNode->NE, otherNode->NE);
		thisNode->NW = copy(thisNode->NW, otherNode->NW);
	}
	return thisNode;
}
