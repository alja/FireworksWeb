

[Releases & Downloads](https://github.com/alja/FireworksWeb/releases)

`FireworksWeb`is server-client implementation of original Fireworks event display.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
## Contents

- [Usage](#usage)
- [Installation Options](#installation-options)
- [Running](#running)
- [User Guide](doc/UserGuide.md)
- [Resources](#resources)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

**Usage**
---

```
Usage: cmsShowWeb [options] <data-file>

  Web based CMS event display.

Options:
General:
  -i [ --input-files ] arg   Input root files
  -c [ --config-file ] arg   Include configuration file
  -n [ --noconfig ]          Empty configuration
  -g [ --geom-file ] arg     Reco geometry file. Default is autodetected
  --no-version-check         No file version check
  --port arg                 Http server port
  --log arg                  Set log level starting from 0 to 4 : kDebug,
                             kInfo, kWarning, kError
  --eve                      Eve plain interface
  -r [ --root-interactive ]  Enable root prompt
  --chain arg                Chain up to a given number of recently open files.
                             Default is 1 - no chain
  -h [ --help ]              Display help message

```

**Installation Options**

1. Compile module [`FireworksWeb`](https://github.com/alja/FireworksWeb/) with CMSSW project
    + `$ git clone https://github.com/alja/FireworksWeb/`
    + `$ scram b`

2. Download the `FireworksWeb` tarball from [Releases](https://github.com/alja/FireworksWeb/releases).


**Running**
---

1. Supported Operating Systems

    + cmsShow server is supported on Linux operating systems.
    + A client running the user interface can function in any web browser on any operating system. The most tested browser is Chrome.

2. Launch Server & Open A Client
    + In CMSSW area or in the tarball directory, event display is launched from terminal, e.g.  
     `cmsShowWeb -- port 7777 data.root`

   + After running the server command, an URL is printed, which you can paste in your browser. In this case, the URL will be :
 `http://localhost:7777/win1/`

**Closed firewall & Tunnels**

   + Some computers, like lxplus nodes, have closed ports. In this case an SSH tunnel can be a solution. Below is simple example of a tunnel from local computer to a specific lxplus host that lasts 10000 seconds:
   
   ```
   ssh -f -L 1234:localhost:7777 lxplus736.cern.ch sleep 10000
   ```

   + The URL for event display which is running on port 7777 would be:
  ```
  http://localhost:1234/win1/
  ```




**Resources**
---
+ **Demo**
  A walkthrough the features is available in a 6 min [recording](https://cmsshow-rels.web.cern.ch/cmsShow-rels/fww-demo.mov) 

+ **Presentations**
  + CHEP 2019 [ EVE-7 and FireworksWeb: The next generation event visualization tools for ROOT and CMS](https://indico.cern.ch/event/773049/contributions/3474840/) 
  +  CHEP 2018[Exploring server/web-client event display for CMS](https://indico.cern.ch/event/587955/contributions/2938069/)
  

+ **Contact**  
  + Fireworks developers mailing list:  fireworks@cern.ch
