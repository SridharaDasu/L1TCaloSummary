#ifndef UCTRegionExtended_hh
#define UCTRegionExtended_hh

#include "L1Trigger/L1TCaloLayer1/src/UCTRegion.hh"

#define RegionEGVeto  0x00000400
#define RegionTauVeto 0x00000800
#define RegionLocBits 0x0000F000
#define LocationShift 12

class UCTRegionExtended : public UCTRegion {
public:

  UCTRegionExtended(const UCTRegion* r);

  virtual ~UCTRegionExtended() {;}

  bool processMore(const UCTRegion* r);

  const bool isEGammaLike() const {return !((RegionEGVeto & regionSummary) == RegionEGVeto);}
  const bool isTauLike() const {return !((RegionTauVeto & regionSummary) == RegionTauVeto);}

private:

  // No default constructor is needed

  UCTRegionExtended();

  // No copy constructor is needed

  UCTRegionExtended(const UCTRegionExtended&);

  // No equality operator is needed

  const UCTRegionExtended& operator=(const UCTRegionExtended&);

};

#endif
