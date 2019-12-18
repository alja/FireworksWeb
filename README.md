

![stronghold logo](https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/45e30b390e75381869dce49cc7e2489fae910ce8/logo-fireworks.png)

[![Downloads](http://cmsshow-rels.web.cern.ch/cmsShow-rels/webFireworks/)]

`Fireworks2` is server-client implementation of original Fireworks event display

![GIF demo](https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/45e30b390e75381869dce49cc7e2489fae910ce8/demo.gif)


**Usage**
---

```
Usage: fire [options] <data-file>

  CMS event display.


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

2. Download the `Fireworks2` tarball from Releases tab.


**Running**
---

1. Supported systems

    + cmsShow server is supported on Linix operating systems.
    + event display client is web browser on any operating system. Most test during development are done in Chrome browser

2. Local run

    + If server is running on your desktop. E.g. have Linux OS, the server prints URL in the terminal that can be pasted to your Chrome browser. E.g. this is how it looks an URL when server is running on port 7777:
    http://localhost:7777/win1/
    

3. Running remotely, when firewall is closed

    + If server is on remote desktop, e.g. lxplus, it is recommended to create a SSH tunnel. Below is an example of a tunnel from port 7777 to local desktop on port 1234:
   ` ssh -f -L 1234:localhost:7777 cms-ed-1 sleep 10000`

    + The URL for event display in this case would be:
  ` http://localhost:1234/win1/`


**User Guide**
---

1.  Collection Dialog
The collection dialog can be accessed by clicking an edit button on collection node in the tree-like browser

<img src="https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/c4e2f44aad37b707b8a493ee47a75f708666f275/edit-collection.png" width="300">

2. Item Dialog
Items in the collection can be edited as well. For the moment it is possible to manally set visibilty, and to see filter status that has propagated from its collection


<img src="https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/7ca1b2512e3f7570d8d3ccc2789c8411d5d82ad2/edit-item.png" width="300">

3. Table edit
   In the table view user can choose any collection from the drop box in the table view header. The edit button give access to a new column dialog. The tab complete list public member function for this class. At the moment the list does not contain public functions that are inherited from base classes.  Note, just like in collection controller, the collection object to evaluate expression is accessed by 'i.functionName()' syntax.

<img src="https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/812c53e5cd0f01c5e164f48a6f1103e81a1726ea/edit-table.png" width="400" align="center">



5. Add Collection Dialog
     Like in original Fireworks, it is possible to look at possible EDM collections to analyse in graphical or table view. The dialog contains general search filed on the top. The list can be long. In this case one can click on a table column to sort by visualization purpose, label, process name, or class type.
     
     <img src="https://gist.githubusercontent.com/alja/2b7656a65bf8e78a26f1f7e93cbd5282/raw/4b4f64e23042125141b9524f5c4022427669655c/add-collection.png" align="left">

