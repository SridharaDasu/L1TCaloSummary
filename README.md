# L1TCaloSummary

The package L1Trigger/L1TCaloSummary is prepared for monitoring the
CMS Layer-1 Calorimeter Trigger.

It is a playpen for various tests.

Unit test for this directory can be run using:

pushd $CMSSW_RELEASE/src/L1Trigger/L1TCaloSummary/tests;scram b runtests;popd

The CMSSW producer can be excercised using:

cmsRun $CMSSW_RELEASE/src/L1Trigger/L1TCaloSummary/tests/testL1TCaloSummary.py
