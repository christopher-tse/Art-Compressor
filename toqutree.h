

#ifndef _TOQUTREE_H_
#define _TOQUTREE_H_

#include <utility>
#include <cmath>
#include "util/PNG.h"
#include "util/HSLAPixel.h"
#include "stats.h"

using namespace std;
using namespace util;

/**
 * toqutree: This is a structure used in decomposing an image
 * into rectangles and squares of similarly colored pixels.
 */

class toqutree {
private:

   /**
    * The Node class is private to the tree class via the principle of
    * encapsulation---end user does not need to know our node-based
    * implementation details.
    *
    * Assumes the current node is a 2^dim x 2^dim square whose upper left
    * corner is 0,0, and whose bottom right corner (inclusive) is
    * 2^dim-1,2^dim-1.
    */
   class Node {
   public:

      Node(pair<int,int> ctr, int dim, HSLAPixel a);

        pair<int,int> center; // optimal splitting position
        int dimension; // node represents a square, 2^dim x 2^dim in size
        HSLAPixel avg; // average color over square

      Node * NW; // left top child
      Node * NE; // right top child
      Node * SE; // right bottom child
      Node * SW; // left bottom child

   };


public:

   /**
    * toqutree destructor.
    * Destroys all of the memory associated with the
    * current toqutree. Ensures that memory does not
    * leak on destruction of a toqutree.
    */
   ~toqutree();

   /**
    * Copy constructor for a toqutree.
    * Since toqutrees allocate dynamic memory (i.e., they use "new", we
    * must define the Big Three).
    * @param other The toqutree we are copying.
    */
   toqutree(const toqutree & other);

   /**
    * Overloaded assignment operator for toqutrees.
    * Part of the Big Three that we must define because the class
    * allocates dynamic memory.
    * @param rhs The right hand side of the assignment statement.
    */
   toqutree & operator=(const toqutree & rhs);

   /**
    * Constructor that builds a toqutree out of the given PNG.
    * Every leaf in the tree corresponds to a pixel in the PNG.
    * Every non-leaf node corresponds to a square of pixels
    * which may not be contiguous in the original PNG, due to .
    * the splittingg procedure we've chosen. In addition, the Node
    * stores a pixel representing the average color over the
    * portion of the image it represents.
    *
    * Every node's children correspond to a partition
    * of the node's square into four smaller squares where a square
    * is defined under the assumption that the image's horizontal
    * and vertical borders "wrap" so that pixels at the top are
    * considered to be adjacent to pixels at the bottom, and similarly
    * for the right and left edges of the image. The node's
    * square is partitioned by choosing a splitting point that
    * results in the four smaller squares whose Information Gain
    * is as large as possible.
    *
    * The children of a node are named NW, NE, SE, and SW corresponding
    * to the compass directions, and assuming N is up.
    */

   toqutree(PNG & imIn,int k);

   /**
    * Render returns a PNG image consisting of the pixels
    * stored in the tree. may be used on pruned trees. Draws
    * every pixel onto a PNG canvas using the
    * average color stored in the node.
    */
   PNG render();

   //render helper
   PNG renderTree(Node * node);

   /*
    *  Prune function trims subtrees as high as possible in the tree.
    *  A subtree is pruned (cleared) if ALL of the subtree's leaves are within
    *  tol of the average color stored in the root of the subtree.
    *  Pruning criteria is evaluated on the original tree, not
    *  on any pruned subtree. (we only expect that trees would be pruned once.)
    */
   void prune(double tol);

   //prune helpers
   void prune(Node * node, double tol);
   bool checkLeaves(Node * parentNode, Node * node, double tol);
   bool isWithinTol(Node * parentNode, Node * node, double tol);

    /* returns the number of nodes in the current toqutree. used
     * for debugging and testing.
     */
    int size();

    //size helper
    int getSize(Node* node);

private:
   /*
    * Private member variables.
    */

   Node* root; // ptr to the root of the toqutree

   /**
    * Destroys all dynamically allocated memory associated with the
    * current toqutree class.
    */
   void clear(Node * &croot);

   //clear helper
   void clear(Node* nw, Node* ne, Node* se, Node* sw);

   /**
   * Copies the parameter other toqutree into the current toqutree.
   * Does not free any memory. Called by copy constructor and op=.
   * @param other The toqutree to be copied.
   */
   Node * copy(const Node * croot);

   //copy helper
   Node * copy(Node* thisNode, Node * otherNode);

   /**
   * Private helper function for the constructor. Recursively builds
   * the tree according to the specification of the constructor.
   * @param s Contains the data used to split the rectangles
   * @param ul upper left point of current node's rectangle.
   * @param lr lower right point of current node's rectangle.
   */
   Node * buildTree(PNG * im, int k);

   //buildTree helpers
   PNG * generateChild(PNG * parentImg, pair<int, int> ul, int greenDim, int imDim);
   double getAvgEntropy(pair<int, int> ulSE, int greenDim, int imDim, int adjX, stats * s, int adjY);
   double getEntropy(pair<int, int> ul, pair<int, int> lr, int adjX, stats * s, int greenDim, int adjY, int imDim);

};

#endif
