## Get list of sockets
ss -n -p | grep 8877

## list of redirectors
* cmsxrootd.fnal.gov
* cms-xrd-global.cern.ch 

example of a file path
```
 fire -r    --port 9092 root://cmsxrootd.fnal.gov//store/relval/CMSSW_11_2_0_pre3/RelValTTbar_14TeV/GEN-SIM-RECO/112X_mcRun3_2021_realistic_v5-v1/20000/FCCAE628-661E-B14A-82C7-68B70BC6320A.root
```

## gdb env in cmssw
. /cvmfs/cms.cern.ch/slc7_amd64_gcc820/external/gdb/8.1/etc/profile.d/init.sh

## http example
slow due to axes to CERN
fire https://cmsshow-rels.web.cern.ch/cmsShow-rels/samples/11_2/RelValZTTminiaod.root