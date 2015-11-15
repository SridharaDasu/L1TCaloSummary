#include <iostream>
#include <iomanip>
#include <vector>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

#include "UCTSummaryCard.hh"

#include "UCTRegionExtended.hh"
#include "UCTObject.hh"

#include "L1Trigger/L1TCaloLayer1/src/UCTLayer1.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTCrate.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTCard.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTRegion.hh"

#include "L1Trigger/L1TCaloLayer1/src/UCTGeometry.hh"

UCTSummaryCard::UCTSummaryCard(const UCTLayer1* in) : uctLayer1(in) {
  extendedRegions.clear();
  for(int iEta = -NRegionsInCard; iEta <= NRegionsInCard; iEta++) {
    if(iEta == 0) continue;
    for(uint32_t iPhi = 1; iPhi <= MaxUCTRegionsPhi; iPhi++) {
      UCTRegionIndex regionIndex(iEta, iPhi);
      UCTRegionExtended* extendedRegion = new UCTRegionExtended(uctLayer1->getRegion(regionIndex));
      extendedRegions.push_back(extendedRegion);
    }
  }
}

UCTSummaryCard::~UCTSummaryCard() {
}

bool UCTSummaryCard::process() {
  // First initiate extended region processing
  uint32_t iReg = 0;
  for(int iEta = -NRegionsInCard; iEta <= NRegionsInCard; iEta++) {
    if(iEta == 0) break;
    for(uint32_t iPhi = 1; iPhi <= MaxUCTRegionsPhi; iPhi++) {
      UCTRegionExtended* extendedRegion = extendedRegions[iReg];
      // Set the normal region information and the process more
      UCTRegionIndex regionIndex(iEta, iPhi);
      extendedRegion->processMore(uctLayer1->getRegion(regionIndex));
      // Next region in list
      iReg++;
    }
  }
  // Then do the summary card processing
  for(int iEta = -NRegionsInCard; iEta <= NRegionsInCard; iEta++) {
    if(iEta == 0) break;
    for(uint32_t iPhi = 1; iPhi <= MaxUCTRegionsPhi; iPhi++) {
      UCTRegionIndex regionIndex(iEta, iPhi);
      processRegion(regionIndex);
    }
  }
  // Then sort the candidates for output usage
  emObjs.sort();
  isoEMObjs.sort();
  tauObjs.sort();
  isoTauObjs.sort();
  jetObjs.sort();
  // Cool we never fail :)
  return true;
}

bool UCTSummaryCard::processRegion(UCTRegionIndex center) {

  UCTGeometryExtended g;
  UCTRegionIndex nIndex = g.getUCTRegionNorth(center);
  UCTRegionIndex sIndex = g.getUCTRegionSouth(center);
  UCTRegionIndex eIndex = g.getUCTRegionEast(center);
  UCTRegionIndex wIndex = g.getUCTRegionWest(center);
  UCTRegionIndex neIndex = g.getUCTRegionNE(center);
  UCTRegionIndex nwIndex = g.getUCTRegionNW(center);
  UCTRegionIndex seIndex = g.getUCTRegionSE(center);
  UCTRegionIndex swIndex = g.getUCTRegionSW(center);

  UCTRegionExtended cRegion(uctLayer1->getRegion(center));
  UCTRegionExtended nRegion(uctLayer1->getRegion(nIndex));
  UCTRegionExtended sRegion(uctLayer1->getRegion(sIndex));
  UCTRegionExtended eRegion(uctLayer1->getRegion(eIndex));
  UCTRegionExtended wRegion(uctLayer1->getRegion(wIndex));
  UCTRegionExtended neRegion(uctLayer1->getRegion(neIndex));
  UCTRegionExtended nwRegion(uctLayer1->getRegion(nwIndex));
  UCTRegionExtended seRegion(uctLayer1->getRegion(seIndex));
  UCTRegionExtended swRegion(uctLayer1->getRegion(swIndex));

  uint32_t cET = cRegion.et();
  uint32_t nET = nRegion.et();
  uint32_t sET = sRegion.et();
  uint32_t eET = eRegion.et();
  uint32_t wET = wRegion.et();
  uint32_t neET = neRegion.et();
  uint32_t seET = seRegion.et();
  uint32_t nwET = nwRegion.et();
  uint32_t swET = swRegion.et();

  uint32_t et3x3 = cET + nET + nwET + wET + swET + sET + seET + eET + neET;

  uint32_t pileup = 0; // FIXME: This should be looked up for the region using the calculated PUM 

  uint32_t JetSeed = 10; // FIXME: This should be a configurable parameter

  // Jet - a 3x3 object with center greater than a seed and all neighbors

  if(cET >= nET && cET >= nwET && cET >= wET && cET >= swET &&
     cET >  sET && cET >  seET && cET >  eET && cET >  neET &&
     cET > JetSeed) {
    uint32_t jetET = et3x3 - pileup;
    jetObjs.push_back(new UCTObject(UCTObject::jet, jetET, cRegion.hitCaloEta(), cRegion.hitCaloPhi(), pileup, 0, et3x3));
  }

  // tau Object - a single region or a 2-region sum, where the neighbor with lower ET is located using matching hit calo towers

  if(cRegion.isTauLike()) {
    UCTTowerIndex cHitTower = cRegion.hitTowerIndex();
    uint32_t tauET = 0;
    uint32_t isolation = et3x3;
    if(g.isEdgeTower(cHitTower)) {
      tauET = cRegion.et();
    }
    else {
      UCTTowerIndex nHitTower = nRegion.hitTowerIndex();
      if(g.areNeighbors(cHitTower, nHitTower)) {
	tauET = cRegion.et() + nRegion.et();
      }
      else {
	UCTTowerIndex sHitTower = sRegion.hitTowerIndex();
	if(g.areNeighbors(cHitTower, sHitTower)) {
	  tauET = cRegion.et() + sRegion.et();
	}
	else {
	  UCTTowerIndex wHitTower = wRegion.hitTowerIndex();
	  if(g.areNeighbors(cHitTower, wHitTower)) {
	    tauET = cRegion.et() + wRegion.et();
	  }
	  else {
	    UCTTowerIndex eHitTower = eRegion.hitTowerIndex();
	    if(g.areNeighbors(cHitTower, eHitTower)) {
	      tauET = cRegion.et() + eRegion.et();
	    }
	  }
	}
      }
    }
    double IsolationFactor = 0.3; // FIXME: This should be a configurable parameter
    isolation = et3x3 - tauET - pileup;
    tauObjs.push_back(new UCTObject(UCTObject::tau, tauET, cRegion.hitCaloEta(), cRegion.hitCaloPhi(), pileup, isolation, et3x3));
    if(isolation < ((uint32_t) (IsolationFactor * (double) tauET))) {
      isoTauObjs.push_back(new UCTObject(UCTObject::isoTau, tauET, cRegion.hitCaloEta(), cRegion.hitCaloPhi(), pileup, isolation, et3x3));
    }
  }
  
  return true;
}

bool UCTSummaryCard::clearEvent() {
  return true;
}

void UCTSummaryCard::print() {
  if(cardSummary > 0)
    std::cout << "UCTSummaryCard: result = " << cardSummary << std::endl;
}
