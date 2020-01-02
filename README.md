

![fireworks logo](https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/45e30b390e75381869dce49cc7e2489fae910ce8/logo-fireworks.png)

[Releases & Downloads](https://github.com/alja/Fireworks2/releases)

`Fireworks2`is server-client implementation of original Fireworks event display.

![GIF demo](https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/16559fd61756bb713c2d60012b56632a878b5464/demo.gif)

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
## Contents

- [Usage](#usage)
- [Installation Options](#installation-options)
- [Running](#running)
- [User Guide](#user-guide)
- [Resources](#resources)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

**Usage**
---

```
Usage: fire [options] <data-file>

  Web based CMS event display.

Options:
General:
  -i [ --input-files ] arg Input root files
  -c [ --config-file ] arg Include configuration file
  --port arg               Listen to port for new data files to open
  --log arg                Set log level starting from 0 to 4 : kDebug, kInfo,
                           kWarning, kError
  --eve                    Eve plain interface
  -h [ --help ]            Display help message
```

**Installation Options**
---

1. Compile module [`Fireworks2`](https://github.com/alja/Fireworks2/) with CMSSW project
    + `$ git clone https://github.com/alja/Fireworks2/`
    + `$ scram b`

2. Download the `Fireworks2` tarball from [Releases](https://github.com/alja/Fireworks2/releases).


**Running**
---

1. Supported Operating Systems

    + cmsShow server is supported on Linux operating systems.
    + A client running the user interface can function in any web browser on any operating system. The most tested browser is Chrome.

2. Launch Server & Open A Client
    + In CMSSW area or in the tarball directory, event display is launched from terminal, e.g.  
     `fire -- port 7777 data.root`

   + After running the server command, an URL is printed, which you can paste in your browser. In this case, the URL will be :
 `http://localhost:7777/win1/`

3. Problem with closed firewall
    + Some computers, like lxplus nodes, have closed ports. In this case an SSH tunnel can be a solution. Below is simple example of a tunnel from local computer to a specific lxplus host that lasts 10000 seconds:    
   ` ssh -f -L 1234:localhost:7777 lxplus736.cern.ch sleep 10000`

    + The URL for event display which is running on port 7777 would be:
  ` http://localhost:1234/win1/`


**User Guide**
---

1. **Collection Controller:** Editing Physics Collection filters and display properties
The collection dialog can be accessed by clicking the *edit* button on the list-tree browser entry corresponding to the desired collection.

<img src="https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/c4e2f44aad37b707b8a493ee47a75f708666f275/edit-collection.png" width="300">

2. Physics Object / Item Dialog
Items in the collection can be edited as well. For the moment it is possible to manually set color, visibility, and examine the collection filter status for this item.

<img src="https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/7ca1b2512e3f7570d8d3ccc2789c8411d5d82ad2/edit-item.png" width="300">

3. Editing Table view
In the table view any loaded physics collection can be chosen from the drop-down menu in the table view header. The edit button gives access to a dialog for adding a new table column. The column expression entry has a tab-complete functionality that lists public member function for the physics item class. At the moment, the list does not contain functions from the base-classfrom baseclasses.  Note, just like in the collection controller, the physics item that is to be evaluated is passed into the expression as variable 'i' and functions can be accessed by the 'i.functionName()' syntax.

<img src="https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/812c53e5cd0f01c5e164f48a6f1103e81a1726ea/edit-table.png" width="400" align="center">

4. **"Add Collection" Dialog:** Selecting and adding new physics collections
Like in the [original Fireworks](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookFireworks) original Fireworks, it is possible to select EDM collections and to add them into graphical or table views. The dialog contains general search filed on the top that matches text anywhere in the table (module label, type, process name, or visualization purpose). Sorting on any column can be activated by clicking on the column heading.<img src="https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/4b4f64e23042125141b9524f5c4022427669655c/add-collection.png" align="left">


**Resources**
---
+ **Presentations**
  + CHEP 2019 [ EVE-7 and FireworksWeb: The next generation event visualization tools for ROOT and CMS](https://indico.cern.ch/event/773049/contributions/3474840/) 
  +  CHEP 2018[Exploring server/web-client event display for CMS](https://indico.cern.ch/event/587955/contributions/2938069/)
  

+ **Contact**  
  + Fireworks developers mailing list:  fireworks@cern.ch
