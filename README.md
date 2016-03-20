# L1TCaloSummary

The package L1Trigger/L1TCaloSummary is prepared for monitoring the CMS Layer-1 Calorimeter Trigger.

It is a playpen for various tests.

Unit test for this directory can be run using:

```bash
pushd $CMSSW_RELEASE/src/L1Trigger/L1TCaloSummary/tests;scram b runtests;popd
```

The CMSSW producer can be excercised using:

```bash
pushd $CMSSW_RELEASE/src/L1Trigger/L1TCaloSummary/tests
cmsRun testL1TCaloSummary.py runNumber=260627 dataStream=/JetHT/Run2015D-v1/RAW
popd
```

Take a look at the resulting file /data/$USER/l1tCaloSummary-<runNumber>.root using root
