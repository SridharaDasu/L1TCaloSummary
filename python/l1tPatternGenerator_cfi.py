import FWCore.ParameterSet.Config as cms


l1tPatternGenerator = cms.EDAnalyzer("L1TPatternGenerator",
                                     l1ExtraIsoEGSource        = cms.InputTag("uct2016EmulatorDigis","Isolated"),
                                     l1ExtraEGSource           = cms.InputTag("uct2016EmulatorDigis","NonIsolated"),
                                     l1ExtraJetCentralSource   = cms.InputTag("uct2016EmulatorDigis","Central"),
                                     l1ExtraJetForwardSource   = cms.InputTag("uct2016EmulatorDigis","Forward"),
                                     l1ExtraIsoTauSource       = cms.InputTag("uct2016EmulatorDigis","IsoTau"),
                                     l1ExtraTauSource          = cms.InputTag("uct2016EmulatorDigis","Tau"),
                                     l1ExtraMETSource          = cms.InputTag("uct2016EmulatorDigis","MET"),
                                     l1ExtraMHTSource          = cms.InputTag("uct2016EmulatorDigis","MHT"),
                                     UCTRegion                 = cms.InputTag('uct2016EmulatorDigis'),
                                     summaryCardInputFileName  = cms.untracked.string("patternsSCInput.txt"),
                                     summaryCardOutputFileName = cms.untracked.string("patternsSCOutput.txt")
)
