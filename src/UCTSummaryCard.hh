#ifndef UCTSummaryCard_hh
#define UCTSummaryCard_hh

#include <vector>
#include <list>

#include "UCTGeometryExtended.hh"

class UCTLayer1;
class UCTObject;
class UCTRegionExtended;

class UCTSummaryCard {
public:

  UCTSummaryCard(const UCTLayer1* in);

  virtual ~UCTSummaryCard();

  // UCTSummaryCard process event

  bool clearEvent();
  bool process();

  // More access functions

  void print();

private:

  // No copy constructor is needed

  UCTSummaryCard(const UCTSummaryCard&);

  // No equality operator is needed

  const UCTSummaryCard& operator=(const UCTSummaryCard&);

  // Helper functions

  bool processRegion(UCTRegionIndex regionIndex);

  // Owned card level data 

  std::list<UCTObject*> emObjs;
  std::list<UCTObject*> isoEMObjs;
  std::list<UCTObject*> tauObjs;
  std::list<UCTObject*> isoTauObjs;
  std::list<UCTObject*> jetObjs;

  UCTObject *ET;
  UCTObject *MET;

  UCTObject *HT;
  UCTObject *MHT;

  uint32_t cardSummary;

  const UCTLayer1 *uctLayer1;

  std::vector<UCTRegionExtended*> extendedRegions;

};

#endif
