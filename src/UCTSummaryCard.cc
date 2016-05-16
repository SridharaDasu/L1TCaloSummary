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

using namespace l1tcalo;

UCTSummaryCard::UCTSummaryCard(const UCTLayer1* in, const std::vector< std::vector< uint32_t > > *l) : 
  uctLayer1(in), pumLUT(l) 
{
  //initial thresholds (should be set by plugin, putting initial values for sanity)
  tauSeed = 10;
  tauIsolationFactor = 0.3;

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
  int etaMin = -NRegionsInCard;
  int etaMax = NRegionsInCard;
  // Determine pumLevel
  pumLevel = 0;
  for(int iEta = etaMin; iEta <= etaMax; iEta++) { // Note that region eta ranges from -7 to +7
    if(iEta == 0) continue;                        // and, eta == 0 is illegal
    for(uint32_t iPhi = 0; iPhi < MaxUCTRegionsPhi; iPhi++) { // Note that phi ranges from 0 to 17
      UCTRegionIndex regionIndex(iEta, iPhi);
      const UCTRegion* uctRegion = uctLayer1->getRegion(regionIndex);
      uint32_t et = uctRegion->et();
      if(et > 0) pumLevel++;
    }
  }
  pumBin = floor(pumLevel/22);
  if(pumLevel > 17) pumBin = 17; // Max PUM value
  // We walk the eta-phi plane looping over all regions.
  // to make global objects like TotalET, HT, MET, MHT
  // For compact objects we use processRegion(regionIndex)
  for(int iEta = etaMin; iEta <= etaMax; iEta++) {
    if(iEta == 0) continue;
    for(uint32_t iPhi = 0; iPhi < MaxUCTRegionsPhi; iPhi++) {
      UCTRegionIndex regionIndex(iEta, iPhi);
      processRegion(regionIndex);
      const UCTRegion* uctRegion = uctLayer1->getRegion(regionIndex);
      uint32_t et = uctRegion->et();
      int hitCaloPhi = uctRegion->hitCaloPhi();
      sumEx += ((int) (((double) et) * cosPhi[hitCaloPhi]));
      sumEy += ((int) (((double) et) * sinPhi[hitCaloPhi]));
      etValue += et;
      if(et > 10) {
	sumHx += ((int) (((double) et) * cosPhi[hitCaloPhi]));
	sumHy += ((int) (((double) et) * sinPhi[hitCaloPhi]));
	htValue += et;
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

  // We process the region looking at nearest neighbor data
  // We should never need beyond nearest-neighbor for most
  // objects - eGamma, tau or jet

  UCTGeometryExtended g;

  const UCTRegion* cRegion(uctLayer1->getRegion(center));
  uint32_t centralET = cRegion->et();
  UCTTowerIndex centralHitTower = cRegion->hitTowerIndex();
  bool centralIsTauLike = cRegion->isTauLike();
  bool centralIsEGammaLike = cRegion->isEGammaLike();
  int hitCaloEta = cRegion->hitCaloEta();
  int hitCaloPhi = cRegion->hitCaloPhi();
  uint32_t pileup = (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(center)];
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
    pileup += (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(northIndex)];
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
    pileup += (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(southIndex)];
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
    pileup += (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(eastIndex)];
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
    pileup += (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(westIndex)];
  }

  UCTRegionIndex neIndex = g.getUCTRegionNE(center);
  const UCTRegion* neRegion(uctLayer1->getRegion(neIndex));
  uint32_t neET = 0;
  if(neRegion != NULL) {
    neET = neRegion->et();
    pileup += (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(neIndex)]; 
  }

  UCTRegionIndex nwIndex = g.getUCTRegionNW(center);
  const UCTRegion* nwRegion(uctLayer1->getRegion(nwIndex));
  uint32_t nwET = 0;
  if(nwRegion != NULL) {
    nwET = nwRegion->et();
    pileup += (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(nwIndex)];
  }

  UCTRegionIndex seIndex = g.getUCTRegionSE(center);
  const UCTRegion* seRegion(uctLayer1->getRegion(seIndex));
  uint32_t seET = 0;
  if(seRegion != NULL) {
    seET = seRegion->et();
    pileup += (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(seIndex)];
  }

  UCTRegionIndex swIndex = g.getUCTRegionSW(center);
  const UCTRegion* swRegion(uctLayer1->getRegion(swIndex));
  uint32_t swET = 0;
  if(swRegion != NULL) {
    swET = swRegion->et();
    pileup += (*pumLUT)[pumBin][g.getGCTRegionEtaIndex(swIndex)];
  }

  uint32_t et3x3 = centralET + northET + nwET + westET + swET + southET + seET + eastET + neET;
  if(et3x3 > 0x3FF) et3x3 = 0x3FF; // Peg at 10-bits

  uint32_t JetSeed = 10; // FIXME: This should be a configurable parameter

  // Jet - a 3x3 object with center greater than a seed and all neighbors

  if(centralET >= northET && centralET >= nwET && centralET >= westET && centralET >= swET &&
     centralET >  southET && centralET >  seET && centralET >  eastET && centralET >  neET &&
     centralET > JetSeed) {
    uint32_t jetET = et3x3 - pileup;
    if(et3x3 < pileup) jetET = 0;
    centralJetObjs.push_back(new UCTObject(UCTObject::jet, jetET, hitCaloEta, hitCaloPhi, pileup, 0, et3x3));
    if(jetET > 150) {
      std::cout << "Jet (ET, eta, phi) = (" << std::dec << jetET << ", " << hitCaloEta << ", " << hitCaloPhi << ")" << std::endl;
      std::cout << "Center " << *cRegion;
      if(northRegion != nullptr) std::cout << "North " << *northRegion;
      if(southRegion != nullptr) std::cout << "South " << *southRegion;
      if(westRegion != nullptr) std::cout << "West " << *westRegion;
      if(eastRegion != nullptr) std::cout << "East " << *eastRegion;
      if(neRegion != nullptr) std::cout << "NE " << *neRegion;
      if(nwRegion != nullptr) std::cout << "NE " << *nwRegion;
      if(seRegion != nullptr) std::cout << "SE " << *seRegion;
      if(swRegion != nullptr) std::cout << "SW " << *swRegion;
    }
  }

  // tau Object - a single region or a 2-region sum, where the neighbor with lower ET is located using matching hit calo towers

  //uint32_t TauSeed = 10; // FIXME: This should be a configurable parameter
  if(centralIsTauLike && centralET > tauSeed) {
    uint32_t tauET = centralET;
    uint32_t isolation = et3x3;
    int neighborMatchCount = 0;
    //check to see if we are on the edge of the calorimeter
    if(!g.isEdgeTower(centralHitTower)) {
      //Check to see if the neighbor regions are tau like and if central ET is greater
      //if region is tau like and a neighbor AND with less energy, set it to 0.
      if(g.areNeighbors(centralHitTower, northHitTower) && northIsTauLike && centralET >= northET) {
	tauET += northET;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, northHitTower) && northIsTauLike && centralET < northET){
	tauET = 0;
      }
      if(g.areNeighbors(centralHitTower, southHitTower) && southIsTauLike && centralET > southET) {
	tauET += southET;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, southHitTower) && southIsTauLike && centralET <= southET){
	tauET = 0;
      }
      if(g.areNeighbors(centralHitTower, westHitTower) && westIsTauLike && centralET >= westET) {
	tauET += westET;
	neighborMatchCount++;
      }      
      else if(g.areNeighbors(centralHitTower, westHitTower) && westIsTauLike && centralET < westET){
	tauET = 0;
      }
      if(g.areNeighbors(centralHitTower, eastHitTower) && eastIsTauLike && centralET > eastET) {
	tauET += eastET;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, eastHitTower) && eastIsTauLike && centralET <= eastET){
	tauET = 0;
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

      //round pileup
      if(et3x3 > (tauET + pileup) )
	 isolation = et3x3 - tauET - pileup;
      else
	isolation = 0;

      if(isolation < ((uint32_t) (tauIsolationFactor * (double) tauET))) {
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
      else if(g.areNeighbors(centralHitTower, northHitTower) && northIsEGammaLike && centralET < northET){
        eGammaET = 0;
      }
      if(g.areNeighbors(centralHitTower, southHitTower) && southIsEGammaLike && centralET > southET) {
	eGammaET += southET;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, southHitTower) && southIsEGammaLike && centralET <= southET){
        eGammaET = 0;
      }
      if(g.areNeighbors(centralHitTower, westHitTower) && westIsEGammaLike && centralET >= westET) {
	eGammaET += westET;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, westHitTower) && westIsEGammaLike && centralET < westET){
        eGammaET = 0;
      }
      if(g.areNeighbors(centralHitTower, eastHitTower) && eastIsEGammaLike && centralET > eastET) {
	eGammaET += eastET;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, eastHitTower) && eastIsEGammaLike && centralET <= eastET){
        eGammaET = 0;
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
      if(et3x3 > (eGammaET + pileup) )
	 isolation = et3x3 - eGammaET - pileup;
      else 
	isolation = 0;

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
