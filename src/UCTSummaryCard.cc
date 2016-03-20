#include <iostream>
#include <iomanip>
#include <vector>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

using namespace std;

#include "UCTSummaryCard.hh"

#include "UCTObject.hh"

#include "L1Trigger/L1TCaloLayer1/src/UCTLayer1.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTCrate.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTCard.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTRegion.hh"

#include "L1Trigger/L1TCaloLayer1/src/UCTGeometry.hh"

UCTSummaryCard::UCTSummaryCard(const UCTLayer1* in) : uctLayer1(in) {
  // FIXME: phi = 0 is probably not correct
  sinPhi[0] = 0;
  cosPhi[0] = 1;
  for(int iPhi = 1; iPhi <= 72; iPhi++) {
    sinPhi[iPhi] = sin((((double) iPhi / (double) 72) * 2 * 3.1415927) - 0.043633);
    cosPhi[iPhi] = cos((((double) iPhi / (double) 72) * 2 * 3.1415927) - 0.043633);
  }
}

UCTSummaryCard::~UCTSummaryCard() {
}

bool UCTSummaryCard::process() {
  clearEvent();
  uint32_t etValue = 0;
  uint32_t htValue = 0;
  int sumEx = 0;
  int sumEy = 0;
  int sumHx = 0;
  int sumHy = 0;
  for(int iEta = -NRegionsInCard; iEta <= NRegionsInCard; iEta++) {
    if(iEta == 0) break;
    for(uint32_t iPhi = 1; iPhi <= MaxUCTRegionsPhi; iPhi++) {
      UCTRegionIndex regionIndex(iEta, iPhi);
      processRegion(regionIndex);
      const UCTRegion* uctRegion = uctLayer1->getRegion(regionIndex);
      uint32_t et = uctRegion->et();
      int hitCaloPhi = uctRegion->hitCaloPhi();
      if(iEta == -NRegionsInCard) {
	sumEx += ((int) ((double) et) * cosPhi[hitCaloPhi]);
	sumEy += ((int) ((double) et) * sinPhi[hitCaloPhi]);
	etValue += et;
	if(et > 10) {
	  sumHx += ((int) ((double) et) * cosPhi[hitCaloPhi]);
	  sumHy += ((int) ((double) et) * sinPhi[hitCaloPhi]);
	  htValue += et;
	}
      }
    }
  }
  uint32_t metSquare = sumEx * sumEx + sumEy * sumEy;
  uint32_t metValue = sqrt((double) metSquare);
  double metPhi = (atan2(sumEy, sumEx) * 180. / 3.1415927) + 180.; // FIXME - phi=0 may not be correct
  int metIPhi = (int) ( 72. * (metPhi / 360.));
  uint32_t mhtSquare = sumHx * sumHx + sumHy * sumHy;
  uint32_t mhtValue = sqrt((double) mhtSquare);
  double mhtPhi = (atan2(sumHy, sumHx) * 180. / 3.1415927) + 180.; // FIXME - phi=0 may not be correct
  int mhtIPhi = (int) ( 72. * (mhtPhi / 360.));

  ET = new UCTObject(UCTObject::ET, etValue, 0, metIPhi, 0, 0, 0);
  HT = new UCTObject(UCTObject::HT, htValue, 0, mhtIPhi, 0, 0, 0);
  MET = new UCTObject(UCTObject::MET, metValue, 0, metIPhi, 0, 0, 0);
  MHT = new UCTObject(UCTObject::MHT, mhtValue, 0, mhtIPhi, 0, 0, 0); // FIXME - cheating for now - requires more work

  // Then sort the candidates for output usage
  emObjs.sort();
  isoEMObjs.sort();
  tauObjs.sort();
  isoTauObjs.sort();
  centralJetObjs.sort();
  forwardJetObjs.sort();
  // Cool we never fail :)
  return true;
}

bool UCTSummaryCard::processRegion(UCTRegionIndex center) {

  const UCTRegion* cRegion(uctLayer1->getRegion(center));
  uint32_t centralET = cRegion->et();
  UCTTowerIndex centralHitTower = cRegion->hitTowerIndex();
  bool centralIsTauLike = cRegion->isTauLike();
  bool centralIsEGammaLike = cRegion->isEGammaLike();
  int hitCaloEta = cRegion->hitCaloEta();
  int hitCaloPhi = cRegion->hitCaloPhi();

  UCTGeometryExtended g;

  UCTRegionIndex northIndex = g.getUCTRegionNorth(center);
  const UCTRegion* northRegion(uctLayer1->getRegion(northIndex));
  uint32_t northET = 0;
  UCTTowerIndex northHitTower;
  bool northIsTauLike = false;
  bool northIsEGammaLike = false;
  if(northRegion != NULL) {
    northET = northRegion->et();
    northHitTower = northRegion->hitTowerIndex();
    northIsTauLike = northRegion->isTauLike();
    northIsEGammaLike = northRegion->isEGammaLike();
  }

  UCTRegionIndex southIndex = g.getUCTRegionSouth(center);
  const UCTRegion* southRegion(uctLayer1->getRegion(southIndex));
  uint32_t southET = 0;
  UCTTowerIndex southHitTower;
  bool southIsTauLike = false;
  bool southIsEGammaLike = false;
  if(southRegion != NULL) {
    southET = southRegion->et();
    southHitTower = southRegion->hitTowerIndex();
    southIsTauLike = southRegion->isTauLike();
    southIsEGammaLike = southRegion->isEGammaLike();
  }

  UCTRegionIndex eastIndex = g.getUCTRegionEast(center);
  const UCTRegion* eastRegion(uctLayer1->getRegion(eastIndex));
  uint32_t eastET = 0;
  UCTTowerIndex eastHitTower;
  bool eastIsTauLike = false;
  bool eastIsEGammaLike = false;
  if(eastRegion != NULL) {
    eastET = eastRegion->et();
    eastHitTower = eastRegion->hitTowerIndex();
    eastIsTauLike = eastRegion->isTauLike();
    eastIsEGammaLike = eastRegion->isEGammaLike();
  }

  UCTRegionIndex westIndex = g.getUCTRegionWest(center);
  const UCTRegion* westRegion(uctLayer1->getRegion(westIndex));
  uint32_t westET = 0;
  UCTTowerIndex westHitTower;
  bool westIsTauLike = false;
  bool westIsEGammaLike = false;
  if(westRegion != NULL) {
    westET = westRegion->et();
    westHitTower = westRegion->hitTowerIndex();
    westIsTauLike = westRegion->isTauLike();
    westIsEGammaLike = westRegion->isEGammaLike();
  }

  UCTRegionIndex neIndex = g.getUCTRegionNE(center);
  const UCTRegion* neRegion(uctLayer1->getRegion(neIndex));
  uint32_t neET = 0;
  if(neRegion != NULL) {
    neET = neRegion->et();
  }

  UCTRegionIndex nwIndex = g.getUCTRegionNW(center);
  const UCTRegion* nwRegion(uctLayer1->getRegion(nwIndex));
  uint32_t nwET = 0;
  if(nwRegion != NULL) {
    nwET = nwRegion->et();
  }

  UCTRegionIndex seIndex = g.getUCTRegionSE(center);
  const UCTRegion* seRegion(uctLayer1->getRegion(seIndex));
  uint32_t seET = 0;
  if(seRegion != NULL) {
    seET = seRegion->et();
  }

  UCTRegionIndex swIndex = g.getUCTRegionSW(center);
  const UCTRegion* swRegion(uctLayer1->getRegion(swIndex));
  uint32_t swET = 0;
  if(swRegion != NULL) {
    swET = swRegion->et();
  }

  uint32_t et3x3 = centralET + northET + nwET + westET + swET + southET + seET + eastET + neET;

  uint32_t pileup = 0; // FIXME: This should be looked up for the region using the calculated PUM 

  uint32_t JetSeed = 10; // FIXME: This should be a configurable parameter

  // Jet - a 3x3 object with center greater than a seed and all neighbors

  if(centralET >= northET && centralET >= nwET && centralET >= westET && centralET >= swET &&
     centralET >  southET && centralET >  seET && centralET >  eastET && centralET >  neET &&
     centralET > JetSeed) {
    uint32_t jetET = et3x3 - pileup;
    centralJetObjs.push_back(new UCTObject(UCTObject::jet, jetET, hitCaloEta, hitCaloPhi, pileup, 0, et3x3));
  }

  // tau Object - a single region or a 2-region sum, where the neighbor with lower ET is located using matching hit calo towers

  uint32_t TauSeed = 10; // FIXME: This should be a configurable parameter
  if(centralIsTauLike && centralET > TauSeed) {
    uint32_t tauET = centralET;
    uint32_t isolation = et3x3;
    int neighborMatchCount = 0;
    if(!g.isEdgeTower(centralHitTower)) {
      if(g.areNeighbors(centralHitTower, northHitTower) && northIsTauLike && centralET >= northET) {
	tauET += northET;
	neighborMatchCount++;
      }
      if(g.areNeighbors(centralHitTower, southHitTower) && southIsTauLike && centralET > southET) {
	tauET += southET;
	neighborMatchCount++;
      }
      if(g.areNeighbors(centralHitTower, westHitTower) && westIsTauLike && centralET >= westET) {
	tauET += westET;
	neighborMatchCount++;
      }
      if(g.areNeighbors(centralHitTower, eastHitTower) && eastIsTauLike && centralET > eastET) {
	tauET += eastET;
	neighborMatchCount++;
      }
      if(neighborMatchCount == 2) {
	std::cerr << "Triple-region Tau - yuck :(" << std::endl;
      }
      else if(neighborMatchCount > 2) {
	std::cerr << "Too many neighbor matches :( ; Not Tau" << std::endl;
	tauET = 0;
      }
    }
    if(tauET != 0) {
      tauObjs.push_back(new UCTObject(UCTObject::tau, tauET, hitCaloEta, hitCaloPhi, pileup, isolation, et3x3));
      double IsolationFactor = 0.3; // FIXME: This should be a configurable parameter
      isolation = et3x3 - tauET - pileup;
      if(isolation < ((uint32_t) (IsolationFactor * (double) tauET))) {
	isoTauObjs.push_back(new UCTObject(UCTObject::isoTau, tauET, hitCaloEta, hitCaloPhi, pileup, isolation, et3x3));
      }
    }
  }
  
  // eGamma Object - This is a sad story
  // eGamma should be in 2-3 contiguous towers, but we have no bandwidth to get a second cluster from cards
  // so we use essentially the same clustering as for tau, but demand that energy is almost all in the ECAL
  // pileup subtraction is critical to not overshoot - further this should work better for isolated eGamma
  // a single region or a 2-region sum, where the neighbor with lower ET is located using matching hit calo towers

  uint32_t eGammaSeed = 5; // FIXME: This should be a configurable parameter
  if(centralIsEGammaLike && centralET > eGammaSeed) {
    uint32_t eGammaET = centralET;
    uint32_t isolation = et3x3;
    int neighborMatchCount = 0;
    if(!g.isEdgeTower(centralHitTower)) {
      if(g.areNeighbors(centralHitTower, northHitTower) && northIsEGammaLike && centralET >= northET) {
	eGammaET += northET;
	neighborMatchCount++;
      }
      if(g.areNeighbors(centralHitTower, southHitTower) && southIsEGammaLike && centralET > southET) {
	eGammaET += southET;
	neighborMatchCount++;
      }
      if(g.areNeighbors(centralHitTower, westHitTower) && westIsEGammaLike && centralET >= westET) {
	eGammaET += westET;
	neighborMatchCount++;
      }
      if(g.areNeighbors(centralHitTower, eastHitTower) && eastIsEGammaLike && centralET > eastET) {
	eGammaET += eastET;
	neighborMatchCount++;
      }
      if(neighborMatchCount == 2) {
	std::cerr << "Triple-region eGamma - yuck :(" << std::endl;
      }
      else if(neighborMatchCount > 2) {
	std::cerr << "Too many neighbor matches :( ; Not eGamma" << std::endl;
	eGammaET = 0;
      }
    }
    if(eGammaET != 0) {
      emObjs.push_back(new UCTObject(UCTObject::eGamma, eGammaET, hitCaloEta, hitCaloPhi, pileup, isolation, et3x3));
      double IsolationFactor = 0.3; // FIXME: This should be a configurable parameter
      isolation = et3x3 - eGammaET - pileup;
      if(isolation < ((uint32_t) (IsolationFactor * (double) eGammaET))) {
	isoEMObjs.push_back(new UCTObject(UCTObject::isoEGamma, eGammaET, hitCaloEta, hitCaloPhi, pileup, isolation, et3x3));
      }
    }
  }

  return true;

}

bool UCTSummaryCard::clearEvent() {
  emObjs.clear();
  isoEMObjs.clear();
  tauObjs.clear();
  isoTauObjs.clear();
  centralJetObjs.clear();
  forwardJetObjs.clear();
  return true;
}

void UCTSummaryCard::print() {
  if(cardSummary > 0)
    std::cout << "UCTSummaryCard: result = " << cardSummary << std::endl;
}
