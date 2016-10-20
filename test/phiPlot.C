void phiPlot(std::string runNumber = "276282") {

  std::string fileName = "/data/dasu/l1tCaloSummary-" + runNumber + ".root";

  TFile file1(fileName.c_str());
  TTree *tree = (TTree*) file1.Get("Events");

  TH1F *jetPhi = new TH1F("JetPhi", "Phi Distribution", 103, -5.1, 5.1);
  tree->Project("JetPhi", "l1extraL1JetParticles_uct2016EmulatorDigis_Central_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPhi");
  jetPhi->SetLineColor(kGreen);

  TH1F *tauPhi = new TH1F("TauPhi", "Phi Distribution", 103, -5.1, 5.1);
  tree->Project("TauPhi", "l1extraL1JetParticles_uct2016EmulatorDigis_Tau_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPhi");
  tauPhi->SetLineColor(kBlack);

  TH1F *isoTauPhi = new TH1F("IsoTauPhi", "Phi Distribution", 103, -5.1, 5.1);
  tree->Project("IsoTauPhi", "l1extraL1JetParticles_uct2016EmulatorDigis_IsoTau_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPhi");
  isoTauPhi->SetLineColor(kBlue);

  TH1F *eGammaPhi = new TH1F("EGammaPhi", "Phi Distribution", 103, -5.1, 5.1);
  tree->Project("EGammaPhi", "l1extraL1EmParticles_uct2016EmulatorDigis_NonIsolated_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPhi");
  eGammaPhi->SetLineColor(kRed);

  TH1F *isoEGammaPhi = new TH1F("IsoEGammaPhi", "Phi Distribution", 103, -5.1, 5.1);
  tree->Project("IsoEGammaPhi", "l1extraL1EmParticles_uct2016EmulatorDigis_Isolated_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPhi");
  isoEGammaPhi->SetLineColor(kMagenta);

  TCanvas *canvas = new TCanvas();
  gPad->SetLogy(0);
  gStyle->SetOptStat(0);
  jetPhi->Draw("HIST,E1");
  tauPhi->Draw("SAME,HIST,E1");
  isoTauPhi->Draw("SAME,HIST,E1");
  eGammaPhi->Draw("SAME,HIST,E1");
  isoEGammaPhi->Draw("SAME,HIST,E1");

  TLegend *lg=new TLegend(0.55,0.55,0.85,0.85);
  lg->SetFillColor(kWhite);
  lg->AddEntry(jetPhi,"Jet Phi","lf");
  lg->AddEntry(tauPhi,"Tau Phi","lf");
  lg->AddEntry(isoTauPhi,"Iso Tau Phi","lf");
  lg->AddEntry(eGammaPhi,"eGamma Phi","lf");
  lg->AddEntry(isoEGammaPhi,"Iso eGamma Phi","lf");
  lg->Draw();

  fileName = "PhiPlot-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

}
