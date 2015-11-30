import FWCore.ParameterSet.Config as cms

process = cms.Process("L1TCaloSummaryTest")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('EventFilter.L1TCaloLayer1RawToDigi.l1tCaloLayer1Digis_cfi')
process.load('L1Trigger.L1TCaloSummary.uct2016EmulatorDigis_cfi')
process.uct2016EmulatorDigis.verbose = cms.bool(True)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:/data/dasu/0EFD41DE-866B-E511-9644-02163E0143CE.root')
)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('/data/dasu/l1tCaloSummary.root'),
    outputCommands = cms.untracked.vstring('drop *', 'keep *_*_*_L1TCaloSummaryTest')
)

process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_condDBv2_cff')
process.GlobalTag.globaltag = '74X_dataRun2_Express_v1'

process.p = cms.Path(process.l1tCaloLayer1Digis*process.uct2016EmulatorDigis)

process.e = cms.EndPath(process.out)
