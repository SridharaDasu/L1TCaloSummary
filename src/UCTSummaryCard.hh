#ifndef UCTSummaryCard_hh
#define UCTSummaryCard_hh

#include <vector>
#include <list>

#include "UCTGeometryExtended.hh"

class UCTLayer1;
class UCTObject;

class UCTSummaryCard {
public:

  UCTSummaryCard(const UCTLayer1* in, const std::vector< std::vector< uint32_t > > *l);

  virtual ~UCTSummaryCard();

  // UCTSummaryCard process event

  bool clearEvent();
  bool process();

  // Access to data

  const std::list<UCTObject*>& getEMObjs() {return emObjs;}
  const std::list<UCTObject*>& getIsoEMObjs() {return isoEMObjs;}
  const std::list<UCTObject*>& getTauObjs() {return tauObjs;}
  const std::list<UCTObject*>& getIsoTauObjs() {return isoTauObjs;}
  const std::list<UCTObject*>& getCentralJetObjs() {return centralJetObjs;}
  const std::list<UCTObject*>& getForwardJetObjs() {return forwardJetObjs;}

  const UCTObject* getET() {return ET;}
  const UCTObject* getMET() {return MET;}

  const UCTObject* getHT() {return HT;}
  const UCTObject* getMHT() {return MHT;}

  // More access functions  
  void setTauSeed(uint32_t in){
    tauSeed = in;
  };
  void setTauIsolationFactor(double in){
    tauIsolationFactor = in;
  };

  bool setPUMLUT(const std::vector< std::vector< uint32_t > > *l) {
    pumLUT = l;
    return true;
  }

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
  std::list<UCTObject*> centralJetObjs;
  std::list<UCTObject*> forwardJetObjs;

  UCTObject *ET;
  UCTObject *MET;

  UCTObject *HT;
  UCTObject *MHT;

  uint32_t cardSummary;

  const UCTLayer1 *uctLayer1;

  double sinPhi[73]; // Make one extra so caloPhi : 1-72 can be used as index directly
  double cosPhi[73];

  uint32_t tauSeed;
  float tauIsolationFactor;

  // Pileup subtraction LUT based on multiplicity of regions > threshold (presently == 0)

  uint32_t pumLevel;
  uint32_t pumBin;
  const std::vector< std::vector< uint32_t > > *pumLUT;

};

#endif
