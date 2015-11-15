import FWCore.ParameterSet.Config as cms

uct2016EmulatorDigis = cms.EDProducer('L1TCaloSummary',
                                      ecalTPSource = cms.InputTag("l1tCaloLayer1Digis"),
                                      hcalTPSource = cms.InputTag("l1tCaloLayer1Digis"),
                                      verbose = cms.bool(False)
                                      )
