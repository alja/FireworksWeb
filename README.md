Fireworks 2
========
web based Fireworks
cmssw and fwlite tests. For sparse checkouts use:
```
git clone --no-checkout git@github.com:alja/Fireworks2.git
cd Fireworks2
echo Fireworks2 >> .git/info/sparse-checkout
git checkout
scram b
 fire2.exe data.root
```

## BranchAddr

Reproduce a bug observed in cmsShow filtering that screws up branch addresses in simple FWlite program.
