void ptPlot(std::string runNumber = "276282") {

  std::string fileName = "/data/dasu/l1tCaloSummary-" + runNumber + ".root";

  TFile file1(fileName.c_str());
  TTree *tree = (TTree*) file1.Get("Events");

  TH1F *jetPT = new TH1F("JetPT", "PT Distribution", 300, 0, 300);
  tree->Project("JetPT", "l1extraL1JetParticles_uct2016EmulatorDigis_Central_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  jetPT->SetLineColor(kGreen);

  TH1F *tauPT = new TH1F("TauPT", "PT Distribution", 300, 0, 300);
  tree->Project("TauPT", "l1extraL1JetParticles_uct2016EmulatorDigis_Tau_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  tauPT->SetLineColor(kBlack);

  TH1F *isoTauPT = new TH1F("IsoTauPT", "PT Distribution", 300, 0, 300);
  tree->Project("IsoTauPT", "l1extraL1JetParticles_uct2016EmulatorDigis_IsoTau_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  isoTauPT->SetLineColor(kBlue);

  TH1F *eGammaPT = new TH1F("EGammaPT", "PT Distribution", 300, 0, 300);
  tree->Project("EGammaPT", "l1extraL1EmParticles_uct2016EmulatorDigis_NonIsolated_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  eGammaPT->SetLineColor(kRed);

  TH1F *isoEGammaPT = new TH1F("IsoEGammaPT", "PT Distribution", 300, 0, 300);
  tree->Project("IsoEGammaPT", "l1extraL1EmParticles_uct2016EmulatorDigis_Isolated_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  isoEGammaPT->SetLineColor(kMagenta);

  TCanvas *canvas = new TCanvas();
  gPad->SetLogy(1);
  gStyle->SetOptStat(0);
  jetPT->Draw("HIST,E1");
  tauPT->Draw("SAME,HIST,E1");
  isoTauPT->Draw("SAME,HIST,E1");
  eGammaPT->Draw("SAME,HIST,E1");
  isoEGammaPT->Draw("SAME,HIST,E1");

  TLegend *lg=new TLegend(0.55,0.55,0.85,0.85);
  lg->SetFillColor(kWhite);
  lg->AddEntry(jetPT,"Jet PT","lf");
  lg->AddEntry(tauPT,"Tau PT","lf");
  lg->AddEntry(isoTauPT,"Iso Tau PT","lf");
  lg->AddEntry(eGammaPT,"eGamma PT","lf");
  lg->AddEntry(isoEGammaPT,"Iso eGamma PT","lf");
  lg->Draw();

  fileName = "PTPlot-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

}
