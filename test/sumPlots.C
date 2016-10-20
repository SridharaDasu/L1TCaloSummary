void sumPlots(std::string runNumber = "276282") {

  std::string fileName = "/data/dasu/l1tCaloSummary-" + runNumber + ".root";
  TFile file1(fileName.c_str());
  TTree *tree = (TTree*) file1.Get("Events");

  TCanvas *canvas = new TCanvas();

  gPad->SetLogy(1);
  tree->Draw("l1extraL1EtMissParticles_uct2016EmulatorDigis_MET_L1TCaloSummaryTest.obj.etTot_");
  fileName = "tet-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

  gPad->SetLogy(1);
  tree->Draw("l1extraL1EtMissParticles_uct2016EmulatorDigis_MET_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  fileName = "met-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

  gPad->SetLogy(0);
  tree->Draw("l1extraL1EtMissParticles_uct2016EmulatorDigis_MET_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPhi",
	     "l1extraL1EtMissParticles_uct2016EmulatorDigis_MET_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt > 10");
  fileName = "met-phi-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

  gPad->SetLogy(1);
  tree->Draw("l1extraL1EtMissParticles_uct2016EmulatorDigis_MHT_L1TCaloSummaryTest.obj.etTot_");
  fileName = "tht-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

  gPad->SetLogy(1);
  tree->Draw("l1extraL1EtMissParticles_uct2016EmulatorDigis_MHT_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt");
  fileName = "mht-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

  gPad->SetLogy(0);
  tree->Draw("l1extraL1EtMissParticles_uct2016EmulatorDigis_MHT_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPhi",
	     "l1extraL1EtMissParticles_uct2016EmulatorDigis_MHT_L1TCaloSummaryTest.obj.m_state.p4Polar_.fCoordinates.fPt > 10");
  fileName = "mht-phi-" + runNumber + ".png";
  canvas->SaveAs(fileName.c_str());

}
