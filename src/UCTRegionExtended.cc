#include <iostream>
#include <stdlib.h>
#include <stdint.h>

#include <bitset>
using std::bitset;
#include <string>
using std::string;

#include "UCTRegionExtended.hh"
#include "UCTGeometryExtended.hh"

// Activity fraction to determine how active a tower compared to a region is
// To avoid ratio calculation, one can use comparison to bit-shifted RegionET
// (activityLevelShift, %) = (1, 50%), (2, 25%), (3, 12.5%), (4, 6.125%), (5, 3.0625%)
// Cutting any tighter is rather dangerous
// For the moment we use floating point arithmetic 

const float activityFraction = 0.1;
const float ecalActivityFraction = 0.1;
const float miscActivityFraction = 0.1;

bool vetoBit(bitset<4> etaPattern, bitset<4> phiPattern);

UCTRegionExtended::UCTRegionExtended(const UCTRegion* r) :
  UCTRegion(0, 0, 0, false)
{
  if(r != 0) {
    UCTRegion(r->getCrate(), r->getCard(), r->getRegion(), r->isNegativeEta());
    regionSummary = r->compressedData();
  }
}

bool UCTRegionExtended::processMore(const UCTRegion* r) {

  // Copy processed information from original region

  regionSummary = r->compressedData();

  // Do additional processing

  uint32_t regionET = et();

  // Calculate regionEcalET 

  uint32_t regionEcalET = 0;
  const std::vector<UCTTower*> towerList = getTowers();
  for(uint32_t twr = 0; twr < towerList.size(); twr++) {
    regionEcalET += towerList[twr]->getEcalET();
  }
  if(regionEcalET > RegionETMask) regionEcalET = RegionETMask;

  // Determine region dimension

  UCTGeometry g;
  uint32_t nEta = g.getNEta(region);
  uint32_t nPhi = g.getNPhi(region);

  // For central regions determine extra bits

  if(region < NRegionsInCard) {
    // Identify active towers
    // Tower ET must be a decent fraction of RegionET
    bool activeTower[nEta][nPhi];
    uint32_t activityLevel = ((uint32_t) ((float) regionET) * activityFraction);
    uint32_t nActiveTowers = 0;
    uint32_t activeTowerET = 0;
    uint32_t highestTowerET = 0;
    uint32_t highestTowerLocation = 0;
    for(uint32_t iPhi = 0; iPhi < nPhi; iPhi++) {
      for(uint32_t iEta = 0; iEta < nEta; iEta++) {
	uint32_t towerET = towers[iEta*nEta+iPhi]->et();
	if(towerET > activityLevel) {
	  activeTower[iEta][iPhi] = true;
	  nActiveTowers++;
	  activeTowerET += towers[iEta*nEta+iPhi]->et();
	}
	else
	  activeTower[iEta][iPhi] = false;
	if(highestTowerET < towerET ) {
	  highestTowerET = towerET;
	  highestTowerLocation = iEta*nEta+iPhi;
	}
      }
    }
    if(activeTowerET > RegionETMask) activeTowerET = RegionETMask;
    // Calculate (energy deposition) active tower pattern
    bitset<4> activeTowerEtaPattern = 0;
    for(uint32_t iEta = 0; iEta < nEta; iEta++) {
      bool activeStrip = false;
      for(uint32_t iPhi = 0; iPhi < nPhi; iPhi++) {
	if(activeTower[iEta][iPhi]) activeStrip = true;
      }
      if(activeStrip) activeTowerEtaPattern |= (0x1 >> iEta);
    }
    bitset<4> activeTowerPhiPattern = 0;
    for(uint32_t iPhi = 0; iPhi < nPhi; iPhi++) {
      bool activeStrip = false;
      for(uint32_t iEta = 0; iEta < nEta; iEta++) {
	if(activeTower[iEta][iPhi]) activeStrip = true;
      }
      if(activeStrip) activeTowerPhiPattern |= (0x1 >> iPhi);
    }
    // Calculate veto bits for eg and tau patterns
    bool veto = vetoBit(activeTowerEtaPattern, activeTowerPhiPattern);
    bool egVeto = veto;
    bool tauVeto = veto;
    uint32_t maxMiscActivityLevelForEG = ((uint32_t) ((float) regionET) * ecalActivityFraction);
    uint32_t maxMiscActivityLevelForTau = ((uint32_t) ((float) regionET) * miscActivityFraction);
    if((regionET - regionEcalET) > maxMiscActivityLevelForEG) egVeto = true;
    if((regionET - activeTowerET) > maxMiscActivityLevelForTau) tauVeto = true;
        
    if(egVeto) regionSummary |= RegionEGVeto;
    if(tauVeto) regionSummary |= RegionTauVeto;

    regionSummary |= (highestTowerLocation << LocationShift);

  }
  
  return true;

}

bool vetoBit(bitset<4> etaPattern, bitset<4> phiPattern) {

  bitset<4> badPattern5(string("0101"));
  bitset<4> badPattern7(string("0111"));
  bitset<4> badPattern9(string("1001"));
  bitset<4> badPattern10(string("1010"));
  bitset<4> badPattern11(string("1011"));
  bitset<4> badPattern13(string("1101"));
  bitset<4> badPattern14(string("1110"));
  bitset<4> badPattern15(string("1111"));

  bool answer = true;
  
  if(etaPattern != badPattern5 && etaPattern != badPattern7 && 
     etaPattern != badPattern10 && etaPattern != badPattern11 &&
     etaPattern != badPattern13 && etaPattern != badPattern14 &&
     etaPattern != badPattern15 && phiPattern != badPattern5 && 
     phiPattern != badPattern7 && phiPattern != badPattern10 && 
     phiPattern != badPattern11 && phiPattern != badPattern13 && 
     phiPattern != badPattern14 && phiPattern != badPattern15 &&
     etaPattern != badPattern9 && phiPattern != badPattern9){
    answer = false;
  }
  return answer;

}
