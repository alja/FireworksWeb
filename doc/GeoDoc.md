# Geometry Browser

## Development Status

### Existing Functionality
* Able to move up and down the hierarchy
* Able to set the visibility of the geo node itself and its children.
* Cross table and GLview node selection


### Work In Progress
* Reduce streaming and server processing time when the node hierarchy changes by streaming all shape info at the start time. At the moment new shapes are streamed each time the top node is set with visibility level 2.
* The outlines are not cleared in some cases.

### Upcoming Features
* Clipping plane control
* Implementation of overlap/extrusion view

## FireworksWeb gateway
The path to simulation file is specified in the optional input entry.'

Some of the available examples are :
```
/eos/user/a/amraktad/geo/cmsSimGeo2021.root
/eos/user/a/amraktad/geo/cmsSimGeo2026.root
/eos/user/a/amraktad/geo/cmsSimGeom-14.root
/eos/user/a/amraktad/geo/cmsSimGeomRun1.root
/eos/user/a/amraktad/geo/cmsSimGeomRun2.root
/eos/user/a/amraktad/geo/cmsSimGeomSLHC.root
```
<img src="gatewaygeo.png" >

## Moving down in the hierarchy
The geometry sets the tracker geometry as the top node by default.
Use ‘Set As Top’ pop-up menu item to move down in the hierarchy.


<img width="1063" height="751" alt="Startup view" src="trackergeo.png" />

Right mouse click makes access to the pop-up menu in the geometry table:

<img width="391" height="538"  src="tablegeo.png" />

### New top node path
<img width="1064" height="546" src="cddowngeo.png" />


## Move up in the hierarchy
Use 'cdTop' or 'cdUp' button in the geometry table to move up.

 Screenshot when geo browser top node  is set to TGeoManager top node:
<img width="1063" alt="Top geo node in given geometry" src="topnodegeo.png"/>
In cases where the bounding box span changes dramatically, one needs to reset the camera to have a proper camera view and lighting

<img width="1225" height="799" alt="Reset camera" src="resetcamerageo.png"/>





