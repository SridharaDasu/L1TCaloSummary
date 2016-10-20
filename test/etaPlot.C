void etaPlot(std::string runNumber = "276282") {

  std::string fileName = "/data/dasu/l1tCaloSummary-" + runNumber + ".root";

  TFile file1(fileName.c_str());

  TTree *tree = (TTree*) file1.Get("Events");

  TH1F *jetEta = new TH1F("JetEta", "Eta Distribution", 103, -5.1, 5.1);
  tree->Project("JetEta", "l1extraL1JetParticles_uct2016EmulatorDigis_Central_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fEta");
  jetEta->SetLineColor(kGreen);

  TH1F *tauEta = new TH1F("TauEta", "Eta Distribution", 103, -5.1, 5.1);
  tree->Project("TauEta", "l1extraL1JetParticles_uct2016EmulatorDigis_Tau_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fEta");
  tauEta->SetLineColor(kBlack);

  TH1F *isoTauEta = new TH1F("IsoTauEta", "Eta Distribution", 103, -5.1, 5.1);
  tree->Project("IsoTauEta", "l1extraL1JetParticles_uct2016EmulatorDigis_IsoTau_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fEta");
  isoTauEta->SetLineColor(kBlue);

  TH1F *eGammaEta = new TH1F("EGammaEta", "Eta Distribution", 103, -5.1, 5.1);
  tree->Project("EGammaEta", "l1extraL1EmParticles_uct2016EmulatorDigis_NonIsolated_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fEta");
  eGammaEta->SetLineColor(kRed);

  TH1F *isoEGammaEta = new TH1F("IsoEGammaEta", "Eta Distribution", 103, -5.1, 5.1);
  tree->Project("IsoEGammaEta", "l1extraL1EmParticles_uct2016EmulatorDigis_Isolated_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fEta");
  isoEGammaEta->SetLineColor(kMagenta);

  TCanvas *canvas = new TCanvas();
  gPad->SetLogy(0);
  gStyle->SetOptStat(0);
  jetEta->Draw("HIST,E1");
  tauEta->Draw("SAME,HIST,E1");
  isoTauEta->Draw("SAME,HIST,E1");
  eGammaEta->Draw("SAME,HIST,E1");
  isoEGammaEta->Draw("SAME,HIST,E1");

  TLegend *lg=new TLegend(0.55,0.55,0.85,0.85);
  lg->SetFillColor(kWhite);
  lg->AddEntry(jetEta,"Jet Eta","lf");
  lg->AddEntry(tauEta,"Tau Eta","lf");
  lg->AddEntry(isoTauEta,"Iso Tau Eta","lf");
  lg->AddEntry(eGammaEta,"eGamma Eta","lf");
  lg->AddEntry(isoEGammaEta,"Iso eGamma Eta","lf");
  lg->Draw();

  fileName = "EtaPlot-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

}
