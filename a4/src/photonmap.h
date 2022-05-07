// photonmap.h


#ifndef PHOTONMAP_H
#define PHOTONMAP_H


#include "headers.h"
#include "seq.h"
#include "linalg.h"
#include "priority.h"


#define PHOTONS_PER_LIGHT 100000

#define MIN_LANDED_PHOTONS 20000  // accumulate at least this many photons on surfaces

class Scene;
class Photon;

// ----------------------------------------------------------------
//
// KD Trees
//
// This should really be implemented as a compact array, since it's
// balanced, but we'll use a standard tree structure with nodes and
// pointers to subtrees, since it's a bit easier to use.  This won't
// work for large number of photons!
//
// ----------------------------------------------------------------


class KDSubtree {
 public:

  int       splitDir;       // direction in which this tree is split (x=0, y=1, z=2)
  Photon    *photon;        // single photon at tree root
  KDSubtree *left, *right;  // subtrees

  KDSubtree( int splitDir, Photon *photon, KDSubtree *left, KDSubtree *right ):
    splitDir(splitDir), photon(photon), left(left), right(right) {}

  ~KDSubtree() {
    if (left != NULL)
      delete left;
    if (right != NULL)
      delete right;
  }

  KDSubtree * buildSubtreeFromPhotons( Photon **photons, int n, vec3 &min, vec3 &max );
  void findSubtreeNearest( vec3 &pos, float maxSqDist, int maxCount, priority_queue<Photon*> &queue );
  void draw( vec3 &min, vec3 &max, int numLevels, mat4 &WCS_to_VCS, mat4 &WCS_to_CCS, vec3 lightDir );
};



class KDTree {
 public:

  KDSubtree *root;

  vec3 min, max;		// bounding box

  KDTree():
    root(NULL) {}

  ~KDTree() {
    if (root != NULL)
      delete root;
  }

  void buildFromPhotons( seq<Photon*> &photons, vec3 &min, vec3 &max );
  seq<Photon*> * findNearest( vec3 &pos, float maxSqDist, int maxCount );
  void draw( vec3 &min, vec3 &max, int numLevels, mat4 &WCS_to_VCS, mat4 &WCS_to_CCS, vec3 lightDir );
};


// ----------------------------------------------------------------
//
//   Photon map
//
// ----------------------------------------------------------------


class Photon {
 public:

  vec3 pos;    // 3D position (which happens to be on a surface)
  vec3 dir;    // incident direction
  vec3 power;  // as RGB

  Photon() {}
  
  Photon( vec3 &pos, vec3 &dir, vec3 &power ):
    pos(pos), dir(dir), power(power) {}
};



class PhotonMap {

 public:

  seq<Photon*> photons;		// list of photons
  KDTree *tree;			// KD tree of photons
  vec3 c000, c111;		// corners of bounding box

  int treeDisplayDepth;		// max depth to display KD partitions

  PhotonMap():
    tree(NULL), treeDisplayDepth(1) {}

  ~PhotonMap() {
    clear();
  }

  void clear() {
    for (int i=0; i<photons.size(); i++)
      delete photons[i];
    photons.clear();
    if (tree != NULL)
      delete tree;
    tree = NULL;
  }

  void addPhoton( Photon *p ) {
    photons.add( p );
  }

  bool isPopulated() {
    return (tree != NULL);
  }

  void populate( Scene *scene );
  void draw( seq<Photon*> &photonsToHighlight, mat4 &WCS_to_VCS, mat4 &WCS_to_CCS, vec3 lightDir );
  vec3 randomDir();
  void forwardTraceRay( vec3 &start, vec3 &dir, vec3 colour, int thisObjIndex, int thisObjPartIndex, Scene *scene, bool specularDone, int depth );

  seq<Photon*> * findNearest( vec3 &pos, float maxSqDist, int maxCount ) {
    if (tree == NULL) {
      seq<Photon*> * emptySeq = new seq<Photon*>(0);
      return emptySeq;
    } else
      return tree->findNearest( pos, maxSqDist, maxCount );
  }
};


#endif


