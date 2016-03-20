import FWCore.ParameterSet.Config as cms

uct2016EmulatorDigis = cms.EDProducer('L1TCaloSummary',
                                      ecalToken = cms.InputTag("simEcalTriggerPrimitiveDigis"),
                                      hcalToken = cms.InputTag("simHcalTriggerPrimitiveDigis"),
                                      useLSB = cms.bool(True),
                                      useCalib = cms.bool(True),
                                      useECALLUT = cms.bool(True),
                                      useHCALLUT = cms.bool(True),
                                      useHFLUT = cms.bool(True),
                                      verbose = cms.bool(False)
                                      )
