OssTests
========

cmssw and fwlite tests. For spars checkouts use:
```
git clone --no-checkout git@github.com:osschar/OssTests.git
cd OssTests
echo BranchAddr >> .git/info/sparse-checkout
git checkout
```

## BranchAddr

Reproduce a bug observed in cmsShow filtering that screws up branch addresses in simple FWlite program.
