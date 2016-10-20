import os
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras

process = cms.Process("L1TCaloSummaryTest")

import EventFilter.L1TXRawToDigi.util as util

from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing()
options.register('runNumber', 0, VarParsing.multiplicity.singleton, VarParsing.varType.int, 'Run to analyze')
options.register('lumis', '1-max', VarParsing.multiplicity.singleton, VarParsing.varType.string, 'Lumis')
options.register('dataStream', '/ExpressPhysics/Run2015D-Express-v4/FEVT', VarParsing.multiplicity.singleton, VarParsing.varType.string, 'Dataset to look for run in')
#options.register('inputFiles', '/store/mc/RunIIFall15DR76/GluGluHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/25nsFlat10to25TSG_76X_mcRun2_asymptotic_v12-v1/50000/04CBAD42-759D-E511-943C-0025905A60F8.root', VarParsing.multiplicity.list, VarParsing.varType.string, 'Manual file list input, will query DAS if empty')
options.register('inputFiles', 'file:highPtTaus-RAW-MINI.root', VarParsing.multiplicity.list, VarParsing.varType.string, 'Manual file list input, will query DAS if empty')
options.register('inputFileList', '', VarParsing.multiplicity.singleton, VarParsing.varType.string, 'Manual file list input, will query DAS if empty')
options.register('useORCON', False, VarParsing.multiplicity.singleton, VarParsing.varType.bool, 'Use ORCON for conditions.  This is necessary for very recent runs where conditions have not propogated to Frontier')
options.register('data',True, VarParsing.multiplicity.singleton, VarParsing.varType.bool, 'option to switch between data and mc')
options.register('rates',False, VarParsing.multiplicity.singleton, VarParsing.varType.bool, 'option to switch between rates and eff (no secondary file map)')
options.register('jets',False, VarParsing.multiplicity.singleton, VarParsing.varType.bool, 'option to switch between jets and taus')
options.register('tauIsolationFactor', 0.3, VarParsing.multiplicity.singleton, VarParsing.varType.float, 'tau relative isolation factor')
options.parseArguments()

from L1Trigger.Stage3Ntuplizer.ggH_TSG_SecondaryFiles_cfi import *
#from L1Trigger.Stage3Ntuplizer.QCD_PT_15to3000_cfi import *

def formatLumis(lumistring, run) :
    lumis = (lrange.split('-') for lrange in lumistring.split(','))
    runlumis = (['%d:%s' % (run,lumi) for lumi in lrange] for lrange in lumis)
    return ['-'.join(l) for l in runlumis]

print 'Getting files for run %d...' % options.runNumber
if len(options.inputFiles) is 0 and options.inputFileList is '' :
    inputFiles = util.getFilesForRun(options.runNumber, options.dataStream)
elif len(options.inputFileList) > 0 :
    with open(options.inputFileList) as f :
        inputFiles = list((line.strip() for line in f))
else :
    inputFiles = cms.untracked.vstring(options.inputFiles)
    if len(inputFiles) is 0 :
        raise Exception('No files found for dataset %s run %d' % (options.dataStream, options.runNumber))
    
print 'Ok, time to analyze'

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.Geometry.GeometryExtended2016Reco_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.RawToDigi_Data_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')


from Configuration.AlCa.GlobalTag import GlobalTag
if options.data is True :
    process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')
else :
    process.GlobalTag = GlobalTag(process.GlobalTag, '80X_mcRun2_asymptotic_v6', '')


# To get L1 CaloParams
process.load('L1Trigger.L1TCalorimeter.caloStage2Params_2016_v2_2_cfi')
# To get CaloTPGTranscoder
process.load('SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff')
process.HcalTPGCoderULUT.LUTGenerationMode = cms.bool(False)


from L1Trigger.Configuration.customiseReEmul import L1TEventSetupForHF1x1TPs,L1TReEmulFromRAW 
process = L1TEventSetupForHF1x1TPs(process)

process.load('L1Trigger.Configuration.SimL1Emulator_cff')
process.load('L1Trigger.Configuration.CaloTriggerPrimitives_cff')
process.simEcalTriggerPrimitiveDigis.Label = 'ecalDigis'
process.simHcalTriggerPrimitiveDigis.inputLabel = cms.VInputTag(
    cms.InputTag('hcalDigis'),
    cms.InputTag('hcalDigis')
    )
process.L1TReEmul = cms.Sequence(process.simHcalTriggerPrimitiveDigis * process.SimL1Emulator)

process.load('L1Trigger.Configuration.SimL1Emulator_cff')
process.load('L1Trigger.Configuration.CaloTriggerPrimitives_cff')
process.load('EventFilter.L1TXRawToDigi.caloLayer1Stage2Digis_cfi')

process.load('L1Trigger.L1TCaloSummary.uct2016EmulatorDigis_cfi')

process.load("L1Trigger.L1TCaloSummary.l1tPatternGenerator_cfi")
#process.l1CaloSummaryPatternGenerator.ecalDigis = cms.InputTag("ecalDigis","EcalTriggerPrimitives")
#process.l1CaloSummaryPatternGenerator.hcalDigis = cms.InputTag("simHcalTriggerPrimitiveDigis")
process.l1tPatternGenerator.ecalDigis = cms.InputTag("ecalDigis","EcalTriggerPrimitives")
process.l1tPatternGenerator.hcalDigis = cms.InputTag("hcalDigis")
process.l1tPatternGenerator.stage2TauSource = cms.InputTag("garbage")
process.l1tPatternGenerator.stage1TauSource = cms.InputTag("garbage")
process.l1tPatternGenerator.stage1IsoTauSource = cms.InputTag("garbage")

process.uct2016EmulatorDigis.useECALLUT = cms.bool(True)
process.uct2016EmulatorDigis.useHCALLUT = cms.bool(True)
process.uct2016EmulatorDigis.useHFLUT = cms.bool(False)
process.uct2016EmulatorDigis.useLSB = cms.bool(True)
process.uct2016EmulatorDigis.verbose = cms.bool(False)
process.uct2016EmulatorDigis.tauSeed = cms.uint32(10)
process.uct2016EmulatorDigis.tauIsolationFactor = cms.double(options.tauIsolationFactor)
###
#process.uct2016EmulatorDigis.pumLUT00 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT01 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT02 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT03 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT04 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT05 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT06 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT07 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT08 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT09 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT10 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT11 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT12 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT13 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT14 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT15 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT16 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#process.uct2016EmulatorDigis.pumLUT17 = cms.vdouble(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
###
print "tau Isolation Factor: " 
print options.tauIsolationFactor

process.uct2016EmulatorDigis.ecalToken = cms.InputTag("ecalDigis","EcalTriggerPrimitives")
process.uct2016EmulatorDigis.hcalToken = cms.InputTag("hcalDigis")
    
process.l1tPatternGenerator.summaryCardInputFileName = cms.untracked.string("patternsSCInput.txt")
process.l1tPatternGenerator.summaryCardOutputFileName = cms.untracked.string("patternsSCOutput.txt")


process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10) )

process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring(inputFiles)#,
                            #secondaryFileNames = cms.untracked.vstring(secondaryMap[inputFiles[0]])
                            )
#Output
#process.TFileService = cms.Service(
#    "TFileService",
#    fileName = cms.string("l1TNtuple2.root")
#    )

process.p = cms.Path(process.RawToDigi*process.L1TReEmul*process.uct2016EmulatorDigis*process.l1tPatternGenerator)


#dump_file = open('dump.py','w')
#dump_file.write(process.dumpPython())

#process.e = cms.EndPath(process.out)
