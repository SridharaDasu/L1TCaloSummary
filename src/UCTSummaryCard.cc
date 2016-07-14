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

UCTSummaryCard::UCTSummaryCard(const UCTLayer1* in, 
			       const std::vector< std::vector< std::vector < uint32_t > > > *l,
			       uint32_t jetSeedIn,
			       uint32_t tauSeedIn,
			       double tauIsolationFactorIn,
			       uint32_t eGammaSeedIn,
			       double eGammaIsolationFactorIn
			       ) : 
  uctLayer1(in), pumLUT(l), jetSeed(jetSeedIn), 
  tauSeed(tauSeedIn), tauIsolationFactor(tauIsolationFactorIn),
  eGammaSeed(tauSeedIn), eGammaIsolationFactor(tauIsolationFactorIn)
{
  UCTGeometry g;
  sinPhi[0] = 0;
  cosPhi[0] = 1;
  for(int iPhi = 1; iPhi <= 72; iPhi++) {
    sinPhi[iPhi] = sin(g.getUCTTowerPhi(iPhi));
    cosPhi[iPhi] = cos(g.getUCTTowerPhi(iPhi));
  }
}

UCTSummaryCard::~UCTSummaryCard() {
}

bool UCTSummaryCard::process() {
  clearEvent();
  UCTGeometry g;
  uint32_t etValue = 0;
  uint32_t htValue = 0;
  int sumEx = 0;
  int sumEy = 0;
  int sumHx = 0;
  int sumHy = 0;
  int etaMin = -NRegionsInCard;
  int etaMax = NRegionsInCard;
  // Determine pumLevel using only the central regions
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
  pumBin = floor(pumLevel/14); // Using only barrel+endcap to estimate pumBin
  if(pumBin > 17) pumBin = 17; // Max PUM value
  // We walk the eta-phi plane looping over all regions.
  // to make global objects like TotalET, HT, MET, MHT
  // subtracting pileup along the way
  // For compact objects we use processRegion(regionIndex)
  uint32_t pileup = 0;
  uint32_t et = 0;
  etaMin = -g.getNRegions(); // Process all regions
  etaMax = g.getNRegions();
  for(uint32_t side = 0; side < 2; side++) {
    bool negativeSide = true;
    if(side == 1) negativeSide = false;
    for(uint32_t crate = 0; crate < g.getNCrates(); crate++) {
      for(uint32_t card = 0; card < g.getNCards(); card++) {
	for(uint32_t region = 0; region < g.getNRegions(); region++) {
	  int iEta = g.getUCTRegionEtaIndex(negativeSide, region);
	  uint32_t iPhi = g.getUCTRegionPhiIndex(crate, card);
	  UCTRegionIndex regionIndex(iEta, iPhi);
	  processRegion(regionIndex);
	  const UCTRegion* uctRegion = uctLayer1->getRegion(regionIndex);
	  if(uctRegion == 0) {
	    std::cerr << "getRegion() returned 0 for (iEta, iPhi) = (" << iEta << ", " << iPhi << ")" << std::endl;
	    continue;
	  }
	  et = uctRegion->et();
	  uint32_t pileupInRegion = (*pumLUT)[pumBin][side][region];
	  pileup += pileupInRegion;
	  if(pileupInRegion < et) et -= pileupInRegion;
	  else et = 0;
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
    }
  }
  uint32_t metSquare = sumEx * sumEx + sumEy * sumEy;
  uint32_t metValue = sqrt((double) metSquare);
  double metPhi = (atan2(sumEy, sumEx) * 180. / 3.1415927) + 180.;
  int metIPhi = (int) ( 72. * (metPhi / 360.));
  uint32_t mhtSquare = sumHx * sumHx + sumHy * sumHy;
  uint32_t mhtValue = sqrt((double) mhtSquare);
  double mhtPhi = (atan2(sumHy, sumHx) * 180. / 3.1415927) + 180.;
  int mhtIPhi = (int) ( 72. * (mhtPhi / 360.));

  ET = new UCTObject(UCTObject::ET, etValue, 0, metIPhi, pileup, 0, 0);
  HT = new UCTObject(UCTObject::HT, htValue, 0, mhtIPhi, pileup, 0, 0);
  MET = new UCTObject(UCTObject::MET, metValue, 0, metIPhi, pileup, 0, 0);
  MHT = new UCTObject(UCTObject::MHT, mhtValue, 0, mhtIPhi, pileup, 0, 0);

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
  if(cRegion == nullptr) {
    std::cerr << "getRegion() returned 0 for (rEta, rPhi) = (" << center.first << ", " << center.second << ")" << std::endl;
    return false;
  }

  // Central jets do not use HF - note border exclusion
  bool centralRegion = false;
  if(cRegion->getRegion() < (NRegionsInCard - 1)) centralRegion = true;

  uint32_t centralET = cRegion->et();
  uint32_t centralPU = std::min(centralET, (*pumLUT)[pumBin][0][cRegion->getRegion()]);
  if(!cRegion->isNegativeEta())
    centralPU = std::min(centralET, (*pumLUT)[pumBin][1][cRegion->getRegion()]);
  centralET -= centralPU;
  UCTTowerIndex centralHitTower = cRegion->hitTowerIndex();
  bool centralIsTauLike = cRegion->isTauLike();
  bool centralIsEGammaLike = cRegion->isEGammaLike();
  int hitCaloEta = cRegion->hitCaloEta();
  int hitCaloPhi = cRegion->hitCaloPhi();

  UCTRegionIndex northIndex = g.getUCTRegionNorth(center);
  const UCTRegion* northRegion(uctLayer1->getRegion(northIndex));
  uint32_t northET = 0;
  uint32_t northPU = 0;
  UCTTowerIndex northHitTower;
  bool northIsTauLike = false;
  bool northIsEGammaLike = false;
  if(northRegion != nullptr) {
    northET = northRegion->et();
    northPU = std::min(northET, (*pumLUT)[pumBin][0][northRegion->getRegion()]);
    if(!northRegion->isNegativeEta())
      northPU = std::min(northET, (*pumLUT)[pumBin][1][northRegion->getRegion()]);
    northET -= northPU;
    northHitTower = northRegion->hitTowerIndex();
    northIsTauLike = northRegion->isTauLike();
    northIsEGammaLike = northRegion->isEGammaLike();
  }

  UCTRegionIndex southIndex = g.getUCTRegionSouth(center);
  const UCTRegion* southRegion(uctLayer1->getRegion(southIndex));
  uint32_t southET = 0;
  uint32_t southPU = 0;
  UCTTowerIndex southHitTower;
  bool southIsTauLike = false;
  bool southIsEGammaLike = false;
  if(southRegion != nullptr) {
    southET = southRegion->et();
    southPU = std::min(southET, (*pumLUT)[pumBin][0][southRegion->getRegion()]);
    if(!southRegion->isNegativeEta())
      southPU = std::min(southET, (*pumLUT)[pumBin][1][southRegion->getRegion()]);
    southET -= southPU;
    southHitTower = southRegion->hitTowerIndex();
    southIsTauLike = southRegion->isTauLike();
    southIsEGammaLike = southRegion->isEGammaLike();
  }

  UCTRegionIndex westIndex = g.getUCTRegionWest(center);
  const UCTRegion* westRegion(uctLayer1->getRegion(westIndex));
  uint32_t westET = 0;
  uint32_t westPU = 0;
  UCTTowerIndex westHitTower;
  bool westIsTauLike = false;
  bool westIsEGammaLike = false;
  if(westRegion != nullptr) {
    westET = westRegion->et();
    westPU = std::min(westET, (*pumLUT)[pumBin][0][westRegion->getRegion()]);
    if(!westRegion->isNegativeEta())
      westPU = std::min(westET, (*pumLUT)[pumBin][1][westRegion->getRegion()]);
    westET -= westPU;
    westHitTower = westRegion->hitTowerIndex();
    westIsTauLike = westRegion->isTauLike();
    westIsEGammaLike = westRegion->isEGammaLike();
  }

  UCTRegionIndex eastIndex = g.getUCTRegionEast(center);
  const UCTRegion* eastRegion(uctLayer1->getRegion(eastIndex));
  uint32_t eastET = 0;
  uint32_t eastPU = 0;
  UCTTowerIndex eastHitTower;
  bool eastIsTauLike = false;
  bool eastIsEGammaLike = false;
  if(eastRegion != nullptr) {
    eastET = eastRegion->et();
    eastPU = std::min(eastET, (*pumLUT)[pumBin][0][eastRegion->getRegion()]);
    if(!eastRegion->isNegativeEta())
      eastPU = std::min(eastET, (*pumLUT)[pumBin][1][eastRegion->getRegion()]);
    eastET -= eastPU;
    eastHitTower = eastRegion->hitTowerIndex();
    eastIsTauLike = eastRegion->isTauLike();
    eastIsEGammaLike = eastRegion->isEGammaLike();
  }

  UCTRegionIndex nwIndex = g.getUCTRegionNW(center);
  const UCTRegion* nwRegion(uctLayer1->getRegion(nwIndex));
  uint32_t nwET = 0;
  uint32_t nwPU = 0;
  UCTTowerIndex nwHitTower;
  if(nwRegion != nullptr) {
    nwET = nwRegion->et();
    nwPU = std::min(nwET, (*pumLUT)[pumBin][0][nwRegion->getRegion()]);
    if(!nwRegion->isNegativeEta())
      nwPU = std::min(nwET, (*pumLUT)[pumBin][1][nwRegion->getRegion()]);
    nwET -= nwPU;
    nwHitTower = nwRegion->hitTowerIndex();
  }

  UCTRegionIndex neIndex = g.getUCTRegionNE(center);
  const UCTRegion* neRegion(uctLayer1->getRegion(neIndex));
  uint32_t neET = 0;
  uint32_t nePU = 0;
  UCTTowerIndex neHitTower;
  if(neRegion != nullptr) {
    neET = neRegion->et();
    nePU = std::min(neET, (*pumLUT)[pumBin][0][neRegion->getRegion()]);
    if(!neRegion->isNegativeEta())
      nePU = std::min(neET, (*pumLUT)[pumBin][1][neRegion->getRegion()]);
    neET -= nePU;
    neHitTower = neRegion->hitTowerIndex();
  }

  UCTRegionIndex swIndex = g.getUCTRegionSW(center);
  const UCTRegion* swRegion(uctLayer1->getRegion(swIndex));
  uint32_t swET = 0;
  uint32_t swPU = 0;
  UCTTowerIndex swHitTower;
  if(swRegion != nullptr) {
    swET = swRegion->et();
    swPU = std::min(swET, (*pumLUT)[pumBin][0][swRegion->getRegion()]);
    if(!swRegion->isNegativeEta())
      swPU = std::min(swET, (*pumLUT)[pumBin][1][swRegion->getRegion()]);
    swET -= swPU;
    swHitTower = swRegion->hitTowerIndex();
  }

  UCTRegionIndex seIndex = g.getUCTRegionSE(center);
  const UCTRegion* seRegion(uctLayer1->getRegion(seIndex));
  uint32_t seET = 0;
  uint32_t sePU = 0;
  UCTTowerIndex seHitTower;
  if(seRegion != nullptr) {
    seET = seRegion->et();
    sePU = std::min(seET, (*pumLUT)[pumBin][0][seRegion->getRegion()]);
    if(!seRegion->isNegativeEta())
      sePU = std::min(seET, (*pumLUT)[pumBin][1][seRegion->getRegion()]);
    seET -= sePU;
    seHitTower = seRegion->hitTowerIndex();
  }

  uint32_t et3x3 = centralET + northET + nwET + westET + swET + southET + seET + eastET + neET;
  if(et3x3 > 0x3FF) et3x3 = 0x3FF; // Peg at 10-bits

  uint32_t pu3x3 = centralPU + northPU + nwPU + westPU + swPU + southPU + sePU + eastPU + nePU;

  // Jet - a 3x3 object with center greater than a seed and all neighbors

  if(centralET >= northET && centralET >= nwET && centralET >= westET && centralET >= swET &&
     centralET >  southET && centralET >  seET && centralET >  eastET && centralET >  neET &&
     centralET > jetSeed) {
    uint32_t jetET = et3x3;
    if(centralRegion) 
      centralJetObjs.push_back(new UCTObject(UCTObject::jet, jetET, hitCaloEta, hitCaloPhi, pu3x3, 0, et3x3));
    else
      forwardJetObjs.push_back(new UCTObject(UCTObject::jet, jetET, hitCaloEta, hitCaloPhi, pu3x3, 0, et3x3));
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
    
  if(centralRegion && centralIsTauLike && centralET > tauSeed) {
    uint32_t tauET = centralET;
    uint32_t tauPU = centralPU;
    int neighborMatchCount = 0;
    //check to see if we are on the edge of the calorimeter
    if(!g.isEdgeTower(centralHitTower)) {
      //Check to see if the neighbor regions are tau like and if central ET is greater
      //if region is tau like and a neighbor AND with less energy, set it to 0.
      if(g.areNeighbors(centralHitTower, northHitTower) && northIsTauLike && centralET >= northET) {
	tauET += northET;
	tauPU += northPU;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, northHitTower) && northIsTauLike && centralET < northET){
	tauET = 0;
      }
      if(g.areNeighbors(centralHitTower, southHitTower) && southIsTauLike && centralET > southET) {
	tauET += southET;
	tauPU += southPU;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, southHitTower) && southIsTauLike && centralET <= southET){
	tauET = 0;
      }
      if(g.areNeighbors(centralHitTower, westHitTower) && westIsTauLike && centralET >= westET) {
	tauET += westET;
	tauPU += westPU;
	neighborMatchCount++;
      }      
      else if(g.areNeighbors(centralHitTower, westHitTower) && westIsTauLike && centralET < westET){
	tauET = 0;
      }
      if(g.areNeighbors(centralHitTower, eastHitTower) && eastIsTauLike && centralET > eastET) {
	tauET += eastET;
	tauPU += eastPU;
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
      tauObjs.push_back(new UCTObject(UCTObject::tau, tauET, hitCaloEta, hitCaloPhi, tauPU, 0xDEADBEEF, et3x3));
      // Subtract footprint
      uint32_t isolation = 0;
      if(et3x3 > tauET) isolation = et3x3 - tauET;
      if(isolation < ((uint32_t) (tauIsolationFactor * (double) tauET))) {
	isoTauObjs.push_back(new UCTObject(UCTObject::isoTau, tauET, hitCaloEta, hitCaloPhi, pu3x3, isolation, et3x3));
      }
    }
  }
  
  // eGamma Object - This is a sad story
  // eGamma should be in 2-3 contiguous towers, but we have no bandwidth to get a second cluster from cards
  // so we use essentially the same clustering as for tau, but demand that energy is almost all in the ECAL
  // pileup subtraction is critical to not overshoot - further this should work better for isolated eGamma
  // a single region or a 2-region sum, where the neighbor with lower ET is located using matching hit calo towers

  if(centralRegion && centralIsEGammaLike && centralET > eGammaSeed) {
    uint32_t eGammaET = centralET;
    uint32_t eGammaPU = centralPU;
    int neighborMatchCount = 0;
    if(!g.isEdgeTower(centralHitTower)) {
      if(g.areNeighbors(centralHitTower, northHitTower) && northIsEGammaLike && centralET >= northET) {
	eGammaET += northET;
	eGammaPU += northPU;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, northHitTower) && northIsEGammaLike && centralET < northET){
        eGammaET = 0;
      }
      if(g.areNeighbors(centralHitTower, southHitTower) && southIsEGammaLike && centralET > southET) {
	eGammaET += southET;
	eGammaPU += southPU;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, southHitTower) && southIsEGammaLike && centralET <= southET){
        eGammaET = 0;
      }
      if(g.areNeighbors(centralHitTower, westHitTower) && westIsEGammaLike && centralET >= westET) {
	eGammaET += westET;
	eGammaPU += westPU;
	neighborMatchCount++;
      }
      else if(g.areNeighbors(centralHitTower, westHitTower) && westIsEGammaLike && centralET < westET){
        eGammaET = 0;
      }
      if(g.areNeighbors(centralHitTower, eastHitTower) && eastIsEGammaLike && centralET > eastET) {
	eGammaET += eastET;
	eGammaPU += eastPU;
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
      emObjs.push_back(new UCTObject(UCTObject::eGamma, eGammaET, hitCaloEta, hitCaloPhi, eGammaPU, 0xDEADBEEF, et3x3));
      uint32_t isolation = 0;
      if(et3x3 > eGammaET) isolation = et3x3 - eGammaET;
      if(isolation < ((uint32_t) (eGammaIsolationFactor * (double) eGammaET))) {
	isoEMObjs.push_back(new UCTObject(UCTObject::isoEGamma, eGammaET, hitCaloEta, hitCaloPhi, pu3x3, isolation, et3x3));
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
