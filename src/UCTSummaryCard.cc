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
  // Then do the summary card processing
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

  UCTGeometryExtended g;
  UCTRegionIndex nIndex = g.getUCTRegionNorth(center);
  UCTRegionIndex sIndex = g.getUCTRegionSouth(center);
  UCTRegionIndex eIndex = g.getUCTRegionEast(center);
  UCTRegionIndex wIndex = g.getUCTRegionWest(center);
  UCTRegionIndex neIndex = g.getUCTRegionNE(center);
  UCTRegionIndex nwIndex = g.getUCTRegionNW(center);
  UCTRegionIndex seIndex = g.getUCTRegionSE(center);
  UCTRegionIndex swIndex = g.getUCTRegionSW(center);

  const UCTRegion* cRegion(uctLayer1->getRegion(center));
  const UCTRegion* nRegion(uctLayer1->getRegion(nIndex));
  const UCTRegion* sRegion(uctLayer1->getRegion(sIndex));
  const UCTRegion* eRegion(uctLayer1->getRegion(eIndex));
  const UCTRegion* wRegion(uctLayer1->getRegion(wIndex));
  const UCTRegion* neRegion(uctLayer1->getRegion(neIndex));
  const UCTRegion* nwRegion(uctLayer1->getRegion(nwIndex));
  const UCTRegion* seRegion(uctLayer1->getRegion(seIndex));
  const UCTRegion* swRegion(uctLayer1->getRegion(swIndex));

  uint32_t cET = cRegion->et();
  uint32_t nET = nRegion->et();
  uint32_t sET = sRegion->et();
  uint32_t eET = eRegion->et();
  uint32_t wET = wRegion->et();
  uint32_t neET = neRegion->et();
  uint32_t seET = seRegion->et();
  uint32_t nwET = nwRegion->et();
  uint32_t swET = swRegion->et();

  uint32_t et3x3 = cET + nET + nwET + wET + swET + sET + seET + eET + neET;

  uint32_t pileup = 0; // FIXME: This should be looked up for the region using the calculated PUM 

  uint32_t JetSeed = 10; // FIXME: This should be a configurable parameter

  // Jet - a 3x3 object with center greater than a seed and all neighbors

  if(cET >= nET && cET >= nwET && cET >= wET && cET >= swET &&
     cET >  sET && cET >  seET && cET >  eET && cET >  neET &&
     cET > JetSeed) {
    uint32_t jetET = et3x3 - pileup;
    centralJetObjs.push_back(new UCTObject(UCTObject::jet, jetET, cRegion->hitCaloEta(), cRegion->hitCaloPhi(), pileup, 0, et3x3));
    std::cout << "Jet(et, eta, phi) = (" << jetET << ", " << cRegion->hitCaloEta() << ", " << cRegion->hitCaloPhi() << ")" << std::endl;
  }

  // tau Object - a single region or a 2-region sum, where the neighbor with lower ET is located using matching hit calo towers

  uint32_t TauSeed = 10; // FIXME: This should be a configurable parameter
  if(cRegion->isTauLike() && cRegion->et() > TauSeed) {
    UCTTowerIndex cHitTower = cRegion->hitTowerIndex();
    uint32_t tauET = cRegion->et();
    uint32_t isolation = et3x3;
    if(!g.isEdgeTower(cHitTower)) {
      UCTTowerIndex nHitTower = nRegion->hitTowerIndex();
      if(g.areNeighbors(cHitTower, nHitTower)) {
	tauET = cRegion->et() + nRegion->et();
      }
      else {
	UCTTowerIndex sHitTower = sRegion->hitTowerIndex();
	if(g.areNeighbors(cHitTower, sHitTower)) {
	  tauET = cRegion->et() + sRegion->et();
	}
	else {
	  UCTTowerIndex wHitTower = wRegion->hitTowerIndex();
	  if(g.areNeighbors(cHitTower, wHitTower)) {
	    tauET = cRegion->et() + wRegion->et();
	  }
	  else {
	    UCTTowerIndex eHitTower = eRegion->hitTowerIndex();
	    if(g.areNeighbors(cHitTower, eHitTower)) {
	      tauET = cRegion->et() + eRegion->et();
	    }
	  }
	}
      }
    }
    double IsolationFactor = 0.3; // FIXME: This should be a configurable parameter
    isolation = et3x3 - tauET - pileup;
    tauObjs.push_back(new UCTObject(UCTObject::tau, tauET, cRegion->hitCaloEta(), cRegion->hitCaloPhi(), pileup, isolation, et3x3));
    std::cout << "Tau(et, eta, phi) = (" << tauET << ", " << cRegion->hitCaloEta() << ", " << cRegion->hitCaloPhi() << ")" << std::endl;
    if(isolation < ((uint32_t) (IsolationFactor * (double) tauET))) {
      isoTauObjs.push_back(new UCTObject(UCTObject::isoTau, tauET, cRegion->hitCaloEta(), cRegion->hitCaloPhi(), pileup, isolation, et3x3));
      std::cout << "isoTau(et, eta, phi) = (" << tauET << ", " << cRegion->hitCaloEta() << ", " << cRegion->hitCaloPhi() << ")" << std::endl;
    }
  }
  
  // eGamma Object - This is a sad story
  // eGamma should be in 2-3 contiguous towers, but we have no bandwidth to get a second cluster from cards
  // so we use essentially the same clustering as for tau, but demand that energy is almost all in the ECAL
  // pileup subtraction is critical to not overshoot - further this should work better for isolated eGamma
  // a single region or a 2-region sum, where the neighbor with lower ET is located using matching hit calo towers

  uint32_t eGammaSeed = 5; // FIXME: This should be a configurable parameter
  if(cRegion->isEGammaLike() && cRegion->et() > eGammaSeed) {
    UCTTowerIndex cHitTower = cRegion->hitTowerIndex();
    uint32_t eGammaET = cRegion->et();
    uint32_t isolation = et3x3;
    if(!g.isEdgeTower(cHitTower)) {
      UCTTowerIndex nHitTower = nRegion->hitTowerIndex();
      if(g.areNeighbors(cHitTower, nHitTower)) {
	eGammaET = cRegion->et() + nRegion->et();
      }
      else {
	UCTTowerIndex sHitTower = sRegion->hitTowerIndex();
	if(g.areNeighbors(cHitTower, sHitTower)) {
	  eGammaET = cRegion->et() + sRegion->et();
	}
	else {
	  UCTTowerIndex wHitTower = wRegion->hitTowerIndex();
	  if(g.areNeighbors(cHitTower, wHitTower)) {
	    eGammaET = cRegion->et() + wRegion->et();
	  }
	  else {
	    UCTTowerIndex eHitTower = eRegion->hitTowerIndex();
	    if(g.areNeighbors(cHitTower, eHitTower)) {
	      eGammaET = cRegion->et() + eRegion->et();
	    }
	  }
	}
      }
    }
    double IsolationFactor = 0.3; // FIXME: This should be a configurable parameter
    isolation = et3x3 - eGammaET - pileup;
    emObjs.push_back(new UCTObject(UCTObject::eGamma, eGammaET, cRegion->hitCaloEta(), cRegion->hitCaloPhi(), pileup, isolation, et3x3));
    std::cout << "emObj(et, eta, phi) = (" << eGammaET << ", " << cRegion->hitCaloEta() << ", " << cRegion->hitCaloPhi() << ")" << std::endl;
    if(isolation < ((uint32_t) (IsolationFactor * (double) eGammaET))) {
      isoEMObjs.push_back(new UCTObject(UCTObject::isoEGamma, eGammaET, cRegion->hitCaloEta(), cRegion->hitCaloPhi(), pileup, isolation, et3x3));
      std::cout << "isoEMObj(et, eta, phi) = (" << eGammaET << ", " << cRegion->hitCaloEta() << ", " << cRegion->hitCaloPhi() << ")" << std::endl;
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
