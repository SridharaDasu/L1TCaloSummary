#ifndef UCTGeometryExtended_hh
#define UCTGeometryExtended_hh

#include "L1Trigger/L1TCaloLayer1/src/UCTGeometry.hh"

class UCTGeometryExtended : UCTGeometry {

public:

  UCTGeometryExtended() {;}
  ~UCTGeometryExtended() {;}

  UCTRegionIndex getUCTRegionNorth(UCTRegionIndex center);
  UCTRegionIndex getUCTRegionSouth(UCTRegionIndex center);
  UCTRegionIndex getUCTRegionEast(UCTRegionIndex center);
  UCTRegionIndex getUCTRegionWest(UCTRegionIndex center);
  UCTRegionIndex getUCTRegionNE(UCTRegionIndex center);
  UCTRegionIndex getUCTRegionNW(UCTRegionIndex center);
  UCTRegionIndex getUCTRegionSE(UCTRegionIndex center);
  UCTRegionIndex getUCTRegionSW(UCTRegionIndex center);

  bool areNeighbors(UCTTowerIndex a, UCTTowerIndex b) {return false;}
  bool isEdgeTower(UCTTowerIndex a) {return false;}

};

#endif