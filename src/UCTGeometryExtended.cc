#include <iostream>
#include <stdlib.h>
#include <stdint.h>

#include "UCTGeometryExtended.hh"

UCTRegionIndex UCTGeometryExtended::getUCTRegionNorth(UCTRegionIndex center) {
  int eta = center.first;
  uint32_t phi = center.second;
  phi += 1;
  if(phi > MaxUCTRegionsPhi) phi -= MaxUCTRegionsPhi;
  return UCTRegionIndex(eta, phi);
}

UCTRegionIndex UCTGeometryExtended::getUCTRegionSouth(UCTRegionIndex center) {
  int eta = center.first;
  uint32_t phi = center.second;
  phi -= 1;
  if(phi == 0) phi = MaxUCTRegionsPhi;
  return UCTRegionIndex(eta, phi);
}

UCTRegionIndex UCTGeometryExtended::getUCTRegionEast(UCTRegionIndex center) {
  int eta = center.first;
  uint32_t phi = center.second;
  eta += 1;
  if(eta == 0) eta = 1; // eta = 0 is illegal, go one above
  if(eta > MaxUCTRegionsEta) eta = 0; // beyond high Eta edge - should not be used
  return UCTRegionIndex(eta, phi);
}

UCTRegionIndex UCTGeometryExtended::getUCTRegionWest(UCTRegionIndex center) {
  int eta = center.first;
  uint32_t phi = center.second;
  eta -= 1;
  if(eta == 0) eta = -1; // eta = 0 is illegal, go one below
  if(eta < -MaxUCTRegionsEta) eta = 0; // beyond high Eta edge - should not be used
  return UCTRegionIndex(eta, phi);
}

UCTRegionIndex UCTGeometryExtended::getUCTRegionNE(UCTRegionIndex center) {
  int eta = center.first;
  uint32_t phi = center.second;
  phi += 1;
  if(phi > MaxUCTRegionsPhi) phi -= MaxUCTRegionsPhi;
  eta += 1;
  if(eta == 0) eta = 1; // eta = 0 is illegal, go one above
  if(eta > MaxUCTRegionsEta) eta = 0; // beyond high Eta edge - should not be used
  return UCTRegionIndex(eta, phi);
}

UCTRegionIndex UCTGeometryExtended::getUCTRegionNW(UCTRegionIndex center) {
  int eta = center.first;
  uint32_t phi = center.second;
  phi += 1;
  if(phi > MaxUCTRegionsPhi) phi -= MaxUCTRegionsPhi;
  eta -= 1;
  if(eta == 0) eta = -1; // eta = 0 is illegal, go one below
  if(eta < -MaxUCTRegionsEta) eta = 0; // beyond high Eta edge - should not be used
  return UCTRegionIndex(eta, phi);
}

UCTRegionIndex UCTGeometryExtended::getUCTRegionSE(UCTRegionIndex center) {
  int eta = center.first;
  uint32_t phi = center.second;
  phi -= 1;
  if(phi == 0) phi = MaxUCTRegionsPhi;
  eta += 1;
  if(eta == 0) eta = 1; // eta = 0 is illegal, go one above
  if(eta > MaxUCTRegionsEta) eta = 0; // beyond high Eta edge - should not be used
  return UCTRegionIndex(eta, phi);
}

UCTRegionIndex UCTGeometryExtended::getUCTRegionSW(UCTRegionIndex center) {
  int eta = center.first;
  uint32_t phi = center.second;
  phi -= 1;
  if(phi == 0) phi = MaxUCTRegionsPhi;
  eta -= 1;
  if(eta == 0) eta = -1; // eta = 0 is illegal, go one below
  if(eta < -MaxUCTRegionsEta) eta = 0; // beyond high Eta edge - should not be used
  return UCTRegionIndex(eta, phi);
}

