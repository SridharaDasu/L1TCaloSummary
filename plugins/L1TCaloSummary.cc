// -*- C++ -*-
//
// Package:    L1Trigger/L1TCaloSummary
// Class:      L1TCaloSummary
// 
/**\class L1TCaloSummary L1TCaloSummary.cc L1Trigger/L1TCaloSummary/plugins/L1TCaloSummary.cc

   Description: The package L1Trigger/L1TCaloSummary is prepared for monitoring the CMS Layer-1 Calorimeter Trigger.

   Implementation:
   It prepares region objects and puts them in the event
*/
//
// Original Author:  Sridhara Dasu
//         Created:  Sat, 14 Nov 2015 14:18:27 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"

#include "L1Trigger/L1TCaloLayer1/src/UCTLayer1.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTCrate.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTCard.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTRegion.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTTower.hh"
#include "L1Trigger/L1TCaloLayer1/src/UCTGeometry.hh"

#include "L1Trigger/L1TCaloSummary/src/UCTObject.hh"
#include "L1Trigger/L1TCaloSummary/src/UCTSummaryCard.hh"
#include "L1Trigger/L1TCaloSummary/src/UCTGeometryExtended.hh"

#include "DataFormats/L1Trigger/interface/L1EmParticle.h"
#include "DataFormats/L1Trigger/interface/L1EmParticleFwd.h"
#include "DataFormats/L1Trigger/interface/L1JetParticle.h"
#include "DataFormats/L1Trigger/interface/L1JetParticleFwd.h"
#include "DataFormats/L1Trigger/interface/L1EtMissParticle.h"
#include "DataFormats/L1Trigger/interface/L1EtMissParticleFwd.h"

typedef std::vector<uint32_t> L1TCaloRegionCollection;

#include "DataFormats/Math/interface/LorentzVector.h"

#include "L1Trigger/L1TCaloLayer1/src/L1TCaloLayer1FetchLUTs.hh"

using namespace l1extra;
using namespace std;

//
// class declaration
//

class L1TCaloSummary : public edm::EDProducer {
public:
  explicit L1TCaloSummary(const edm::ParameterSet&);
  ~L1TCaloSummary();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginJob() override;
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
      
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  void print();

  // ----------member data ---------------------------

  edm::EDGetTokenT<EcalTrigPrimDigiCollection> ecalTPSource;
  std::string ecalTPSourceLabel;
  edm::EDGetTokenT<HcalTrigPrimDigiCollection> hcalTPSource;
  std::string hcalTPSourceLabel;

  std::vector< std::vector< std::vector < uint32_t > > > ecalLUT;
  std::vector< std::vector< std::vector < uint32_t > > > hcalLUT;
  std::vector< std::vector< uint32_t > > hfLUT;

  std::vector< UCTTower* > twrList;

  bool useLSB;
  bool useCalib;
  bool useECALLUT;
  bool useHCALLUT;
  bool useHFLUT;
  bool verbose;

  UCTLayer1 *layer1;
  UCTSummaryCard *summaryCard;
  
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
L1TCaloSummary::L1TCaloSummary(const edm::ParameterSet& iConfig) :
  ecalTPSource(consumes<EcalTrigPrimDigiCollection>(iConfig.getParameter<edm::InputTag>("ecalToken"))),
  ecalTPSourceLabel(iConfig.getParameter<edm::InputTag>("ecalToken").label()),
  hcalTPSource(consumes<HcalTrigPrimDigiCollection>(iConfig.getParameter<edm::InputTag>("hcalToken"))),
  hcalTPSourceLabel(iConfig.getParameter<edm::InputTag>("hcalToken").label()),
  ecalLUT(28, std::vector< std::vector<uint32_t> >(2, std::vector<uint32_t>(256))),
  hcalLUT(28, std::vector< std::vector<uint32_t> >(2, std::vector<uint32_t>(256))),
  hfLUT(12, std::vector < uint32_t >(256)),
  useLSB(iConfig.getParameter<bool>("useLSB")),
  useCalib(iConfig.getParameter<bool>("useCalib")),
  useECALLUT(iConfig.getParameter<bool>("useECALLUT")),
  useHCALLUT(iConfig.getParameter<bool>("useHCALLUT")),
  useHFLUT(iConfig.getParameter<bool>("useHFLUT")),
  verbose(iConfig.getParameter<bool>("verbose")) 
{
  produces< L1TCaloRegionCollection >( "Regions" );
  produces< L1EmParticleCollection >( "Isolated" ) ;
  produces< L1EmParticleCollection >( "NonIsolated" ) ;
  produces< L1JetParticleCollection >( "Central" ) ;
  produces< L1JetParticleCollection >( "Forward" ) ;
  produces< L1JetParticleCollection >( "Tau" ) ;
  produces< L1JetParticleCollection >( "IsoTau" ) ;
  produces< L1EtMissParticleCollection >( "MET" ) ;
  produces< L1EtMissParticleCollection >( "MHT" ) ;
  layer1 = new UCTLayer1;
  summaryCard = new UCTSummaryCard(layer1);
  vector<UCTCrate*> crates = layer1->getCrates();
  for(uint32_t crt = 0; crt < crates.size(); crt++) {
    vector<UCTCard*> cards = crates[crt]->getCards();
    for(uint32_t crd = 0; crd < cards.size(); crd++) {
      vector<UCTRegion*> regions = cards[crd]->getRegions();
      for(uint32_t rgn = 0; rgn < regions.size(); rgn++) {
	vector<UCTTower*> towers = regions[rgn]->getTowers();
	for(uint32_t twr = 0; twr < towers.size(); twr++) {
	  twrList.push_back(towers[twr]);
	}
      }
    }
  }
}


L1TCaloSummary::~L1TCaloSummary() {
  if(layer1 != 0) delete layer1;
  if(summaryCard != 0) delete summaryCard;
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void
L1TCaloSummary::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  edm::Handle<EcalTrigPrimDigiCollection> ecalTPs;
  iEvent.getByToken(ecalTPSource, ecalTPs);
  edm::Handle<HcalTrigPrimDigiCollection> hcalTPs;
  iEvent.getByToken(hcalTPSource, hcalTPs);

  std::auto_ptr<L1TCaloRegionCollection> rgnCollection(new L1TCaloRegionCollection);
  std::auto_ptr<L1EmParticleCollection> iEGCands(new L1EmParticleCollection);
  std::auto_ptr<L1EmParticleCollection> nEGCands(new L1EmParticleCollection);
  std::auto_ptr<L1JetParticleCollection> iTauCands(new L1JetParticleCollection);
  std::auto_ptr<L1JetParticleCollection> nTauCands(new L1JetParticleCollection);
  std::auto_ptr<L1JetParticleCollection> cJetCands(new L1JetParticleCollection);
  std::auto_ptr<L1JetParticleCollection> fJetCands(new L1JetParticleCollection);
  std::auto_ptr<L1EtMissParticleCollection> metCands(new L1EtMissParticleCollection);
  std::auto_ptr<L1EtMissParticleCollection> mhtCands(new L1EtMissParticleCollection);

  uint32_t expectedTotalET = 0;

  if(!layer1->clearEvent()) {
    std::cerr << "UCT: Failed to clear event" << std::endl;
    exit(1);
  }

  for ( const auto& ecalTp : *ecalTPs ) {
    int caloEta = ecalTp.id().ieta();
    int caloPhi = ecalTp.id().iphi();
    int et = ecalTp.compressedEt();
    bool fgVeto = ecalTp.fineGrain();
    if(et != 0) {
      UCTTowerIndex t = UCTTowerIndex(caloEta, caloPhi);
      if(!layer1->setECALData(t,fgVeto,et)) {
	std::cerr << "UCT: Failed loading an ECAL tower" << std::endl;
	return;
      }
      expectedTotalET += et;
    }
  }

  for ( const auto& hcalTp : *hcalTPs ) {
    int caloEta = hcalTp.id().ieta();
    uint32_t absCaloEta = abs(caloEta);
    // Tower 29 is not used by Layer-1
    if(absCaloEta == 29) {
      continue;
    }
    // Prevent usage of HF TPs with Layer-1 emulator if HCAL TPs are old style
    else if(hcalTp.id().version() == 0 && absCaloEta > 29) {
      continue;
    }
    else if(absCaloEta <= 41) {
      int caloPhi = hcalTp.id().iphi();
      if(caloPhi <= 72) {
	int et = hcalTp.SOI_compressedEt();
	bool fg = hcalTp.SOI_fineGrain();
	if(et != 0) {
	  UCTTowerIndex t = UCTTowerIndex(caloEta, caloPhi);
	  uint32_t featureBits = 0;
	  if(fg) featureBits = 0x1F; // Set all five feature bits for the moment - they are not defined in HW / FW yet!
	  if(!layer1->setHCALData(t, featureBits, et)) {
	    std::cerr << "caloEta = " << caloEta << "; caloPhi =" << caloPhi << std::endl;
	    std::cerr << "UCT: Failed loading an HCAL tower" << std::endl;
	    return;
	    
	  }
	  expectedTotalET += et;
	}
      }
      else {
	std::cerr << "Illegal Tower: caloEta = " << caloEta << "; caloPhi =" << caloPhi << std::endl;	
      }
    }
    else {
      std::cerr << "Illegal Tower: caloEta = " << caloEta << std::endl;
    }
  }

  if(!layer1->process()) {
    std::cerr << "UCT: Failed to process layer 1" << std::endl;
    exit(1);
  }

  // Crude check if total ET is approximately OK!
  // We can't expect exact match as there is region level saturation to 10-bits
  // 1% is good enough
  int diff = abs(layer1->et() - expectedTotalET);
  if(verbose && diff > 0.01 * expectedTotalET ) {
    print();
    std::cout << "Expected " 
	      << std::showbase << std::internal << std::setfill('0') << std::setw(10) << std::hex
	      << expectedTotalET << std::dec << std::endl;
  }
 
  vector<UCTCrate*> crates = layer1->getCrates();
  for(uint32_t crt = 0; crt < crates.size(); crt++) {
    vector<UCTCard*> cards = crates[crt]->getCards();
    for(uint32_t crd = 0; crd < cards.size(); crd++) {
      vector<UCTRegion*> regions = cards[crd]->getRegions();
      for(uint32_t rgn = 0; rgn < regions.size(); rgn++) {
	rgnCollection->push_back(regions[rgn]->rawData());
      }
    }
  }  
  iEvent.put(rgnCollection, "Regions");

  if(!summaryCard->process()) {
    std::cerr << "UCT: Failed to process summary card" << std::endl;
    exit(1);      
  }

  UCTGeometry g;
  double pt = 0;
  double eta = -999.;
  double phi = -999.;
  double mass = 0;
  double caloScaleFactor = 0.5;
  
  std::list<UCTObject*> emObjs = summaryCard->getEMObjs();
  for(std::list<UCTObject*>::const_iterator i = emObjs.begin(); i != emObjs.end(); i++) {
    const UCTObject* object = *i;
    pt = ((double) object->et()) * caloScaleFactor;
    eta = g.getUCTTowerEta(object->iEta());
    phi = g.getUCTTowerPhi(object->iPhi(), object->iEta());
    nEGCands->push_back(L1EmParticle(math::PtEtaPhiMLorentzVector(pt, eta, phi, mass), L1EmParticle::kNonIsolated));
  }
  std::list<UCTObject*> isoEMObjs = summaryCard->getIsoEMObjs();
  for(std::list<UCTObject*>::const_iterator i = isoEMObjs.begin(); i != isoEMObjs.end(); i++) {
    const UCTObject* object = *i;
    pt = ((double) object->et()) * caloScaleFactor;
    eta = g.getUCTTowerEta(object->iEta());
    phi = g.getUCTTowerPhi(object->iPhi(), object->iEta());
    iEGCands->push_back(L1EmParticle(math::PtEtaPhiMLorentzVector(pt, eta, phi, mass), L1EmParticle::kIsolated));
  }
  std::list<UCTObject*> tauObjs = summaryCard->getTauObjs();
  for(std::list<UCTObject*>::const_iterator i = tauObjs.begin(); i != tauObjs.end(); i++) {
    const UCTObject* object = *i;
    pt = ((double) object->et()) * caloScaleFactor;
    eta = g.getUCTTowerEta(object->iEta());
    phi = g.getUCTTowerPhi(object->iPhi(), object->iEta());
    nTauCands->push_back(L1JetParticle(math::PtEtaPhiMLorentzVector(pt, eta, phi, mass), L1JetParticle::kTau));
  }
  std::list<UCTObject*> isoTauObjs = summaryCard->getIsoTauObjs();
  for(std::list<UCTObject*>::const_iterator i = isoTauObjs.begin(); i != isoTauObjs.end(); i++) {
    const UCTObject* object = *i;
    pt = ((double) object->et()) * caloScaleFactor;
    eta = g.getUCTTowerEta(object->iEta());
    phi = g.getUCTTowerPhi(object->iPhi(), object->iEta());
    iTauCands->push_back(L1JetParticle(math::PtEtaPhiMLorentzVector(pt, eta, phi, mass), L1JetParticle::kTau));
  }
  std::list<UCTObject*> centralJetObjs = summaryCard->getCentralJetObjs();
  for(std::list<UCTObject*>::const_iterator i = centralJetObjs.begin(); i != centralJetObjs.end(); i++) {
    const UCTObject* object = *i;
    pt = ((double) object->et()) * caloScaleFactor;
    eta = g.getUCTTowerEta(object->iEta());
    phi = g.getUCTTowerPhi(object->iPhi(), object->iEta());
    cJetCands->push_back(L1JetParticle(math::PtEtaPhiMLorentzVector(pt, eta, phi, mass), L1JetParticle::kCentral));
  }
  std::list<UCTObject*> forwardJetObjs = summaryCard->getForwardJetObjs();
  for(std::list<UCTObject*>::const_iterator i = forwardJetObjs.begin(); i != forwardJetObjs.end(); i++) {
    const UCTObject* object = *i;
    pt = ((double) object->et()) * caloScaleFactor;
    eta = g.getUCTTowerEta(object->iEta());
    phi = g.getUCTTowerPhi(object->iPhi(), object->iEta());
    fJetCands->push_back(L1JetParticle(math::PtEtaPhiMLorentzVector(pt, eta, phi, mass), L1JetParticle::kForward));
  }

  const UCTObject* et = summaryCard->getET();
  pt = ((double) et->et()) * caloScaleFactor;
  double totET = pt;
  const UCTObject* met = summaryCard->getMET();
  pt = ((double) met->et()) * caloScaleFactor;
  eta = g.getUCTTowerEta(met->iEta());
  phi = g.getUCTTowerPhi(met->iPhi(), met->iEta());
  metCands->push_back(L1EtMissParticle(math::PtEtaPhiMLorentzVector(pt, eta, phi, mass), L1EtMissParticle::kMET, totET));

  const UCTObject* ht = summaryCard->getHT();
  pt = ((double) ht->et()) * caloScaleFactor;
  double totHT = pt;
  const UCTObject* mht = summaryCard->getMHT();
  pt = ((double) mht->et()) * caloScaleFactor;
  eta = g.getUCTTowerEta(mht->iEta());
  phi = g.getUCTTowerPhi(mht->iPhi(), mht->iEta());
  mhtCands->push_back(L1EtMissParticle(math::PtEtaPhiMLorentzVector(pt, eta, phi, mass), L1EtMissParticle::kMHT, totHT));
  
  iEvent.put(iEGCands, "Isolated");
  iEvent.put(nEGCands, "NonIsolated");
  iEvent.put(iTauCands, "IsoTau");
  iEvent.put(nTauCands, "Tau");
  iEvent.put(cJetCands, "Central");
  iEvent.put(fJetCands, "Forward");
  iEvent.put(metCands, "MET");
  iEvent.put(mhtCands, "MHT");

}

void L1TCaloSummary::print() {
  vector<UCTCrate*> crates = layer1->getCrates();
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
	    std::cout << *towers[twr];
	    if(header) header = false;
	  }
	  std::cout << *regions[rgn];
	}
      }
      std::cout << *cards[crd];
    }
    std::cout << *crates[crt];
  }
  std::cout << *layer1;
}

// ------------ method called once each job just before starting event loop  ------------
void 
L1TCaloSummary::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
L1TCaloSummary::endJob() {
}

// ------------ method called when starting to processes a run  ------------
void
L1TCaloSummary::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
  if(!L1TCaloLayer1FetchLUTs(iSetup, ecalLUT, hcalLUT, hfLUT, useLSB, useCalib, useECALLUT, useHCALLUT, useHFLUT)) {
    std::cerr << "L1TCaloLayer1::beginRun: failed to fetch LUTS - using unity" << std::endl;
  }
  for(uint32_t twr = 0; twr < twrList.size(); twr++) {
    twrList[twr]->setECALLUT(&ecalLUT);
    twrList[twr]->setHCALLUT(&hcalLUT);
    twrList[twr]->setHFLUT(&hfLUT);
    }
}
 
// ------------ method called when ending the processing of a run  ------------
/*
  void
  L1TCaloSummary::endRun(edm::Run const&, edm::EventSetup const&)
  {
  }
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
  void
  L1TCaloSummary::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
  {
  }
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
  void
  L1TCaloSummary::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
  {
  }
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
L1TCaloSummary::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(L1TCaloSummary);
