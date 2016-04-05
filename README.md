# L1TCaloSummary

The package L1Trigger/L1TCaloSummary is prepared for monitoring the CMS Layer-1 Calorimeter Trigger.

It is a playpen for various tests.

Installation:

```bash
cmsrel CMSSW_8_0_2
cd CMSSW_8_0_2/src
cmsenv
git cms-init
git remote add cms-l1t-offline git@github.com:cms-l1t-offline/cmssw.git
git fetch cms-l1t-offline
git cms-merge-topic cms-l1t-offline:layer1-dev-$CMSSW_VERSION
git cms-addpkg L1Trigger/L1TCommon
cd L1Trigger
git clone git@github.com:SridharaDasu/L1TCaloSummary.git
cd ..
scram b -j 8
```

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
