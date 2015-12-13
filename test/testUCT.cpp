#include <iostream>
#include <iomanip>
#include <vector>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

#include "L1Trigger/L1TCaloSummary/src/UCTObject.hh"
#include "L1Trigger/L1TCaloSummary/src/UCTSummaryCard.hh"
#include "L1Trigger/L1TCaloSummary/src/UCTGeometryExtended.hh"

#include "L1Trigger/L1TCaloLayer1/src/UCTLayer1.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTCrate.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTCard.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTRegion.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTTower.hh"

#include "L1Trigger/L1TCaloLayer1/src/UCTGeometry.hh"

double flatRandom(double min, double max) {
  static double rMax = (double) 0x7FFFFFFF;
  uint32_t r = random();
  double d = (double) r;
  double f = min + ((max - min) * d / rMax);
  if(f < min) f = min;
  if(f > max) f = max;
  return f;
}

void print(UCTLayer1& uct) {
  vector<UCTCrate*> crates = uct.getCrates();
  for(uint32_t crt = 0; crt < crates.size(); crt++) {
    vector<UCTCard*> cards = crates[crt]->getCards();
    for(uint32_t crd = 0; crd < cards.size(); crd++) {
      vector<UCTRegion*> regions = cards[crd]->getRegions();
      for(uint32_t rgn = 0; rgn < regions.size(); rgn++) {
	if(regions[rgn]->et() > 0) {
	  int hitEta = regions[rgn]->hitCaloEta();
	  int hitPhi = regions[rgn]->hitCaloPhi();
	  vector<UCTTower*> towers = regions[rgn]->getTowers();
	  bool header = true;
	  for(uint32_t twr = 0; twr < towers.size(); twr++) {
	    if(towers[twr]->caloPhi() == hitPhi && towers[twr]->caloEta() == hitEta) {
	      std::cout << "*";
	    }
	    towers[twr]->print(header);
	    if(header) header = false;
	  }
	  regions[rgn]->print();
	}
      }
      cards[crd]->print();
    }
    crates[crt]->print();
  }
  uct.print();
}

int main(int argc, char** argv) {

  int nEvents = 10000;
  if(argc == 1) std::cout << "Running on " << nEvents << std::endl;
  else if(argc == 2) nEvents = atoi(argv[1]);
  else {std::cout << "Command syntax: testUCTLayer1 [nEvents]" << std::endl; return 1;}

  UCTLayer1 uctLayer1;

  UCTSummaryCard uctLayer2(&uctLayer1);

  // Event loop - use 1M events to test
  for(int event = 0; event < nEvents; event++) {

    if(!uctLayer1.clearEvent()) {
      std::cerr << "UCT: Failed to clear event" << std::endl;
      exit(1);
    }
    
    // Make an electron and put it in a random location within UCT
    
    uint32_t eleET = (random() & 0xFF); // Random energy up to the maximum allowed
    bool eleFG = ((random() % 100) < 95); // 5% of the time eleFG Veto should "kill" electron
    uint32_t hcalE = (random() & 0x00000003); // Assume HCAL is noise in bottom two bits :(
    uint32_t hcalF = 0; // Ignored
    int caloEta = ((random()+1) % 28); // Distribute uniformly in +/- eta within acceptance
    while(caloEta < 1 || caloEta > 28) caloEta = ((random()+1) % 28);
    if((random() & 0x1) != 0) caloEta = -caloEta;
    int caloPhi = ((random()+1) % 72); // Distribute uniformly in all phi
    while(caloPhi < 1 || caloPhi > 72) caloPhi = ((random()+1) % 72);
    UCTTowerIndex t = UCTTowerIndex(caloEta, caloPhi);
    if(!uctLayer1.setEventData(t, eleFG, eleET, hcalF, hcalE)) {
      std::cerr << "UCT: Failed loading an eGamma candidate" << std::endl;
      exit(1);
    }
    uint32_t expectedTotalET = eleET + hcalE;
    // For 30% of the cases add a non-zero neighbor tower
    if((random() % 100) > 70) {
      uint32_t neighborEleET = (random() & (0xFF - eleET));
      int neighborCaloEta = caloEta;
      int neighborCaloPhi = caloPhi;
      // Half the time the neighbor is in eta direction
      bool etaNeighbor = true;
      if((random() & 0x1) != 0) etaNeighbor = false;
      if(etaNeighbor) {
	neighborCaloEta++; 
	if(neighborCaloEta == 0) neighborCaloEta = 1;
      }
      else {
	neighborCaloPhi++; 
	if(neighborCaloPhi == 73) neighborCaloPhi = 1;
      }
      uint32_t neighborHCalE = (random() & 0x00000003); // Assume HCAL is noise in bottom two bits :(
      // Make sure the neighbor is within "acceptance"
      if(neighborCaloEta < 29) {
	UCTTowerIndex n = UCTTowerIndex(neighborCaloEta, neighborCaloPhi);
	if(!uctLayer1.setEventData(n, eleFG, neighborEleET, hcalF, neighborHCalE)) {
	  std::cerr << "UCT: Failed loading an eGamma candidate" << std::endl;
	  exit(1);
	}
	// Set eleET to be the total
	expectedTotalET += (neighborEleET + neighborHCalE);
	// Make sure that position is fixed to the highest, it could be the neighbor!
	if(neighborEleET > eleET) {
	  caloEta = neighborCaloEta;
	  caloPhi = neighborCaloPhi;
	}
      }
    }
  
    if(!uctLayer1.process()) {
      std::cerr << "UCT: Failed to process layer 1" << std::endl;
      exit(1);
    }

    // Crude check if total ET is approximately OK!
    // We can't expect exact match as there is region level saturation to 10-bits
    // 10% is good enough
    int diff = uctLayer1.et() - expectedTotalET;
    if(diff < 0) diff = diff * -1;
    if(diff > (0.10 * expectedTotalET) ) {
      print(uctLayer1);
      std::cout << "Expected " 
		<< std::showbase << std::internal << std::setfill('0') << std::setw(10) << std::hex
		<< expectedTotalET << std::endl;
    }

    if(!uctLayer2.process()) {
      std::cerr << "UCT: Failed to process layer 2" << std::endl;
      exit(1);      
    }

  }

  return 0;

}
