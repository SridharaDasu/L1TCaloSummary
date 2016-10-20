/*
 * \file L1TPatternGenerator.cc
 *
 * \author I. Ojalvo
 * Written for miniAOD
 */

#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "L1Trigger/L1TCaloSummary/interface/L1TPatternGenerator.h"
#include "DataFormats/Provenance/interface/EventAuxiliary.h"
#include "DataFormats/L1Trigger/interface/Tau.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/TauReco/interface/PFTau.h"
#include "DataFormats/Common/interface/RefToPtr.h"

#include "DataFormats/Math/interface/deltaR.h"

#include "L1Trigger/L1TCaloLayer1/src/UCTGeometry.hh"
#include <iostream>
#include <fstream>

using namespace edm;
using std::cout;
using std::endl;
using std::vector;
/*
#Total ET, total pileup, pumBin
#EG nonIso: 8 highest, (et, ieta, iphi)
#EG Iso: 8 highest, (et, ieta, iphi)
#Tau nonIso: 8 highest, (et, ieta, iphi)
#Tau Iso: 8 highest, (et, ieta, iphi)
#Jets: 8 highest, (et, ieta, iphi)
#MET (value, phi)
#ET (value, phi)
#MHT (value, phi)
#HT (value, phi)
 */

L1TPatternGenerator::L1TPatternGenerator( const ParameterSet & cfg ) :
  l1ExtraIsoEGSource_(      consumes<L1EmParticleCollection>(     cfg.getParameter<edm::InputTag>("l1ExtraIsoEGSource"))),
  l1ExtraEGSource_(         consumes<L1EmParticleCollection>(     cfg.getParameter<edm::InputTag>("l1ExtraEGSource"))),
  l1ExtraJetCentralSource_( consumes<L1JetParticleCollection>(    cfg.getParameter<edm::InputTag>("l1ExtraJetCentralSource"))),
  l1ExtraJetForwardSource_( consumes<L1JetParticleCollection>(    cfg.getParameter<edm::InputTag>("l1ExtraJetForwardSource"))),
  l1ExtraIsoTauSource_(     consumes<L1JetParticleCollection>(    cfg.getParameter<edm::InputTag>("l1ExtraIsoTauSource"))),
  l1ExtraTauSource_(        consumes<L1JetParticleCollection>(    cfg.getParameter<edm::InputTag>("l1ExtraTauSource"))),
  l1ExtraMETSource_(        consumes<L1EtMissParticleCollection>( cfg.getParameter<edm::InputTag>("l1ExtraMETSource"))),
  l1ExtraMHTSource_(        consumes<L1EtMissParticleCollection>( cfg.getParameter<edm::InputTag>("l1ExtraMHTSource"))),
  regionSource_(            consumes<vector <L1CaloRegion> >(     cfg.getParameter<edm::InputTag>("UCTRegion")))
  {
    summaryCardOutputFileName_  = cfg.getUntrackedParameter<std::string>("summaryCardOutputFileName");
    summaryCardInputFileName_   = cfg.getUntrackedParameter<std::string>("summaryCardInputFileName");
    
    fout.open(summaryCardOutputFileName_);
    fin.open(summaryCardInputFileName_);
    
    std::cout<<"beginning job"<<std::endl;
    fout<<"#EG nonIso: 8 highest"<<endl;
    fout<<"#EG Iso: 8 highest"<<endl;
    fout<<"#Tau nonIso: 8 highest"<<endl;
    fout<<"#Tau Iso: 8 highest"<<endl;
    fout<<"#Jets: 8 highest"<<endl;
    fout<<"#MET "<<endl;
    fout<<"#ET "<<endl;
    fout<<"#MHT "<<endl;
    fout<<"#HT "<<endl;
    fout<<"#PUMBin, pileup"<<endl;
    fout<<endl;

    patternNumber = 0;
  }

void L1TPatternGenerator::beginJob( const EventSetup & es) {

}

void L1TPatternGenerator::printNZeros(ofstream &file, int nZeros){
  for(int i = nZeros; i < 8; i++){
    //if(i<nZeros)
    //if(i != nZeros) 
    //if(nZeros != 0 || i != nZeros)
    //file<<" ";
    file<<"00000000 ";
  }
}

void L1TPatternGenerator::printEGTau(ofstream &file, uint32_t iso, uint32_t phi, uint32_t etaSign, uint32_t eta, uint32_t et ){
  uint32_t printNumber = 0;
  printNumber += et&0x1FF;
  printNumber += ((eta&0xFF)<<9);
  printNumber += (etaSign<<16);
  printNumber += ((phi&0xFF)<<17);
  printNumber += ((iso&0x1)<<25);
  file<<std::hex<<setfill('0') << setw(8)<<printNumber<<" ";
}

void L1TPatternGenerator::printJet(ofstream &file, uint32_t phi, uint32_t etaSign, uint32_t eta, uint32_t et ){
  uint32_t printNumber = 0;
  printNumber += et&0x3FF;
  printNumber += ((eta&0xFF)<<11);
  printNumber += (etaSign<<18);
  printNumber += ((phi&0xFF)<<19);
  file<<std::hex<<setfill('0') << setw(8)<<printNumber<<" ";
}

void L1TPatternGenerator::printSum(ofstream &file, uint32_t phi, uint32_t et ){
  uint32_t printNumber = 0;
  printNumber += et&0x7FF;
  printNumber += ((phi&0xFF)<<9);
  file<<std::hex<<setfill('0') << setw(8)<<printNumber<<" ";
}

void L1TPatternGenerator::analyze( const Event& evt, const EventSetup& es )
 {

   run = evt.id().run();
   lumi = evt.id().luminosityBlock();
   event = evt.id().event();
   
   fout<<"#Event "<<patternNumber<<endl;
   fin <<"#Event "<<patternNumber<<endl;
   patternNumber++;
   std::cout<<"run: "<<run<<" lumi: "<< lumi <<" event: "<< event<<std::endl;
   
  Handle<L1CaloRegionCollection> regions;
  
  edm::Handle < L1EmParticleCollection > l1ExtraIsoEGs;
  edm::Handle < L1EmParticleCollection > l1ExtraEGs;
  edm::Handle < vector<l1extra::L1JetParticle> > l1ExtraJetsCentral;
  edm::Handle < vector<l1extra::L1JetParticle> > l1ExtraJetsForward;
  edm::Handle < vector<l1extra::L1JetParticle> > l1ExtraIsoTaus;
  edm::Handle < vector<l1extra::L1JetParticle> > l1ExtraTaus;
  edm::Handle < L1EtMissParticleCollection > l1ExtraMET;
  edm::Handle < L1EtMissParticleCollection > l1ExtraMHT;

  UCTGeometry g;
  //isoEG
  //fout<<"EG Iso ";
  int i = 0;
  if(evt.getByToken(l1ExtraIsoEGSource_, l1ExtraIsoEGs)){
    for( L1EmParticleCollection::const_iterator isoEG = l1ExtraIsoEGs->begin(); 
	 isoEG != l1ExtraIsoEGs->end();
	 isoEG++ ) {

      uint32_t pt      = isoEG->pt();
      float    eta     = isoEG->eta();
      float    phi     = isoEG->phi();
      uint32_t ieta    = convertGenEta(eta);
      uint32_t iphi    = convertGenPhi(phi);
      uint8_t  etaSign = 0;

      if(eta<0)etaSign = 1;
      printEGTau(fout, 1, iphi, etaSign, ieta, pt );
      i++; if(i==7)break;
    }
  }
  if(i<8)
    printNZeros(fout,i);
  fout<<std::endl;
  
  //EG
  //fout<<"EG nonIso ";
  i = 0;
  if(evt.getByToken(l1ExtraEGSource_, l1ExtraEGs)){
    for( L1EmParticleCollection::const_iterator eg = l1ExtraEGs->begin(); 
	 eg != l1ExtraEGs->end();
	 eg++ ) {

      uint32_t pt      = eg->pt();
      float    eta     = eg->eta();
      float    phi     = eg->phi();
      uint32_t ieta    = convertGenEta(eta);
      uint32_t iphi    = convertGenPhi(phi);
      uint8_t  etaSign = 0;

      if(eta<0)etaSign = 1;
      printEGTau(fout, 0, iphi, etaSign, ieta, pt );
      //fout<<"("<<pt<<","<<eta<<","<<phi<<")";
      i++; if(i==7)break;
    }
  }
  if(i<8)
    printNZeros(fout,i);
  fout<<endl;

  //taus
  //fout<<"Tau nonIso ";
  i = 0;
  if(evt.getByToken(l1ExtraTauSource_, l1ExtraTaus)){
    for( vector<l1extra::L1JetParticle>::const_iterator rlxTau = l1ExtraTaus->begin(); 
	 rlxTau != l1ExtraTaus->end(); 
	 rlxTau++ ) {
      uint32_t pt      = rlxTau->pt();
      float    eta     = rlxTau->eta();
      float    phi     = rlxTau->phi();
      uint32_t ieta    = convertGenEta(eta);
      uint32_t iphi    = convertGenPhi(phi);
      uint8_t  etaSign = 0;

      if(eta<0)etaSign = 1;
      printEGTau(fout, 1, iphi, etaSign, ieta, pt );
       i++; if(i==7)break;
    }
  }
  if(i<8)
    printNZeros(fout,i);
  fout<<endl;
  
  //isotaus
  //fout<<"Tau Iso ";
  i = 0;
  if(evt.getByToken(l1ExtraIsoTauSource_ , l1ExtraIsoTaus)){
    for( vector<l1extra::L1JetParticle>::const_iterator isoTau = l1ExtraIsoTaus->begin();  
	 isoTau != l1ExtraIsoTaus->end();  
	 ++isoTau ) {

      uint32_t pt      = isoTau->pt();
      float    eta     = isoTau->eta();
      float    phi     = isoTau->phi();
      uint32_t ieta    = convertGenEta(eta);
      uint32_t iphi    = convertGenPhi(phi);
      uint8_t etaSign  = 0;

      if(eta<0)etaSign = 1;
      printEGTau(fout, 1, iphi, etaSign, ieta, pt );
      i++; if(i==7)break;
    }
  }
  if(i<8)
    printNZeros(fout,i);
  fout<<endl;

  //jets
  i = 0;
  //fout<<"Jets Central ";
  if(evt.getByToken(l1ExtraJetCentralSource_, l1ExtraJetsCentral)){
    for( L1JetParticleCollection::const_iterator l1Jet = l1ExtraJetsCentral->begin(); 
	 l1Jet != l1ExtraJetsCentral->end(); 
	 l1Jet++ ) {

      uint32_t pt      = l1Jet->pt();
      float    eta     = l1Jet->eta();
      float    phi     = l1Jet->phi();
      uint32_t ieta    = convertGenEta(eta);
      uint32_t iphi    = convertGenPhi(phi);
      uint8_t  etaSign = 0;

      if(eta<0)etaSign = 1;
      printJet(fout, iphi, etaSign, ieta, pt );

      //if(i>0)
      //fout<<", ";
      //fout<<"("<<pt<<","<<eta<<","<<phi<<")";
      i++; if(i==7)break;
    }
  }
  if(i<8)
    printNZeros(fout,i);
  fout<<endl;
  
  //jets
  //fout<<"Jets Forward ";
  i = 0;
  if(evt.getByToken(l1ExtraJetForwardSource_, l1ExtraJetsForward)){
    for( L1JetParticleCollection::const_iterator l1Jet = l1ExtraJetsForward->begin(); 
	 l1Jet != l1ExtraJetsForward->end(); 
	 l1Jet++ ) {

      uint32_t pt      = l1Jet->pt();
      float    eta     = l1Jet->eta();
      float    phi     = l1Jet->phi();
      uint32_t ieta    = convertGenEta(eta);
      uint32_t iphi    = convertGenPhi(phi);
      uint8_t etaSign  = 0;

      if(eta<0)etaSign = 1;
      printJet(fout, iphi, etaSign, ieta, pt );
      //if(i>0)
      //fout<<", ";
      //fout<<"("<<pt<<","<<eta<<","<<phi<<")";
      i++; if(i==7)break;
    }
  }
  if(i<8)
    printNZeros(fout,i);
  fout<<endl;
  
  //MET
  //fout<<"MET ";
  i = 0;
  if(evt.getByToken(l1ExtraMETSource_ , l1ExtraMET)){
    for( L1EtMissParticleCollection::const_iterator met = l1ExtraMET->begin();  
	 met != l1ExtraMET->end();  
	 ++met ) {

      uint32_t pt     = met->pt();
      uint32_t energy = met->energy();
      float    phi    = met->phi();
      uint32_t iphi   = convertGenPhi(phi);
      //print MET
      printSum(fout, pt, iphi);
      fout<<endl;
      //print ET
      printSum(fout, energy, 0);
      i++; if(i==1)break;
    }
  }
  fout<<endl;

  //MHT
  //fout<<"MHT ";
  i = 0;
  if(evt.getByToken(l1ExtraMHTSource_ , l1ExtraMHT)){
    for( L1EtMissParticleCollection::const_iterator mht = l1ExtraMHT->begin();  
	 mht != l1ExtraMHT->end();  
	 ++mht ) {

      uint32_t pt     = mht->pt();
      uint32_t energy = mht->energy();
      float    phi    = mht->phi();
      uint32_t iphi   = convertGenPhi(phi);

      //print mht
      printSum(fout, pt, iphi);
      fout<<endl;
      //print ET
      printSum(fout, energy, 0);
      i++; if(i==1)break;
    }
  }
  fout<<endl;
  fout<<endl;


  ////Now for regions in input file
  vector<region_p_type> allRegions[18];
  //UCTGeometry g;
  if(!evt.getByToken(regionSource_,regions)){
    std::cout<<"ERROR GETTING THE REGIONS!!!"<<std::endl;}
  else{
    for(vector<L1CaloRegion>::const_iterator region = regions->begin(); region != regions->end(); ++region){
      region_p_type temp;

      temp.pt = (region->et());
      temp.rctEta = region->id().ieta();
      temp.rctPhi = region->id().iphi();
      temp.rctRaw = region->raw();
      int element = region->id().iphi();

      if(element<0||element>17)
	std::cout<<"ERROR FILLING REGIONS Phi element is "<<element<<std::endl;
      else
	allRegions[element].push_back(temp);
    }
  }

  for(int j = 0; j< 18; j++){
    sort(allRegions[j].begin(), allRegions[j].end(), compareByRCTEta);
  }

  for(int j = 17; j> -1; j--){
    fin<<"0000 0000 0000 0000 0000 0000 0000 ";
    for(auto region : allRegions[j]){
      fin<<std::hex<<setfill('0') << setw(4)<<region.rctRaw<<" ";
    }
    fin<<"0000 0000 0000 0000 0000 0000 0000 ";
    fin<<std::endl;
  }
  fin<<std::endl;
 
 }


void L1TPatternGenerator::endJob() {
  fout.close();
  fin.close();
}

L1TPatternGenerator::~L1TPatternGenerator(){
}

DEFINE_FWK_MODULE(L1TPatternGenerator);
