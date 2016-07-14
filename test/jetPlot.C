void jetPlot(std::string runNumber = "276282") {

  std::string fileName = "/data/dasu/l1tCaloSummary-" + runNumber + ".root";

  TFile file1(fileName.c_str());

  TTree *tree = (TTree*) file1.Get("Events");

  TH1F *cJetPT = new TH1F("CentralJetPT", "PT Distribution", 300, 0, 300);
  tree->Project("CentralJetPT", "l1extraL1JetParticles_uct2016EmulatorDigis_Central_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  cJetPT->SetLineColor(kBlue);

  TH1F *fJetPT = new TH1F("ForwardJetPT", "PT Distribution", 300, 0, 300);
  tree->Project("ForwardJetPT", "l1extraL1JetParticles_uct2016EmulatorDigis_Forward_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  fJetPT->SetLineColor(kRed);


  TCanvas *canvas = new TCanvas();
  gPad->SetLogy(1);
  gStyle->SetOptStat(0);
  cJetPT->Draw("HIST,E1");
  fJetPT->Draw("SAME,HIST,E1");

  TLegend *lg=new TLegend(0.55,0.55,0.85,0.85);
  lg->SetFillColor(kWhite);
  lg->AddEntry(cJetPT,"Central Jet PT","lf");
  lg->AddEntry(fJetPT,"Forward Jet PT","lf");
  lg->Draw();

  canvas->SaveAs("JetPlot.png");

}
