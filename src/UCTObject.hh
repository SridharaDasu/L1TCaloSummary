#ifndef UCTObject_hh
#define UCTObject_hh

class UCTObject {
public:

  enum UCTObjectType {
    jet = 0x0000,
    tau = 0x0001,
    eGamma = 0x0010,
    isoTau = 0x0100,
    isoEGamma = 0x1000,
    ET = 0x10000,
    HT = 0x100000,
    MET = 0x1000000,
    MHT = 0x10000000,
    unknown = 0xDEADBEEF
  };

  UCTObject(UCTObjectType type, uint32_t et, int iEta, int iPhi,
	    uint32_t pileup, uint32_t isolation, uint32_t et3x3) :
    myType(type),
    myET(et),
    myEta(iEta), 
    myPhi(iPhi),
    myPileup(pileup),
    myIsolation(isolation),
    myEt3x3(et3x3)
  {
  }
  
  virtual ~UCTObject() {;}
  
  // Equality operator is needed

  const UCTObject& operator=(const UCTObject& i) {
    myET = i.et();
    myEta = i.iEta();
    myPhi = i.iPhi();
    return *this;
  }

  // For sorting

  bool operator < (const UCTObject& other) const {
    return this->et() < other.et();
  }

  // Is this needed?
  bool operator > (const UCTObject& other) const {
    return this->et() > other.et();
  }

  // This is to compare exactly -- including location!
  bool operator == (const UCTObject& other) const {
    if(this->iEta() == other.iEta()) {
      if(this->iPhi() == other.iPhi()) {
	return this->et() == other.et();
      }
    }
    return false;
  }

  bool clearEvent() {
    myET = 0;
    return true;
  }

  // Access functions for convenience

  const uint32_t et() const {return myET;}
  const int iEta() const {return myEta;}
  const int iPhi() const {return myPhi;}

  const uint32_t pileup() const {return myPileup;}
  const uint32_t isolation() const {return myIsolation;}
  const uint32_t et3x3() const {return myEt3x3;}

  void print(bool header = true);

private:

  // No default constructor is needed

  UCTObject();

  // No copy constructor is needed

  UCTObject(const UCTObject&);

  // Object data

  UCTObjectType myType;

  uint32_t myET;

  int myEta;
  int myPhi;

  uint32_t myPileup;
  uint32_t myIsolation;
  uint32_t myEt3x3;

};

#endif
