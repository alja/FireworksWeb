# User Guide

## Table of Contents
1. [Collections](#collections)
    1. [Collection Controller](#collection-controller)
    2. [Object Controller](#example3)
    2. [Add Collection Dialgo](#example3)
3. [Table View](#table-view)
4. [Event Filter](#fourth-examplehttpwwwfourthexamplecom)

---

## Collections
### Collection Controller:

Editing Physics Collection filters and display properties
The collection dialog can be accessed by clicking the *edit* button on the list-tree browser entry corresponding to the desired collection.

<img src="edit-collection.png" height="120" style="align: left;  margin-left: 10px; " >

### Physics Object / Item Dialog

Items in the collection can be edited as well. For the moment it is possible to manually set color, visibility, and examine the collection filter status for this item.

<img src="edit-item.png" height="120" style="align: left;  margin-left: 10px;">



### Add Collection Dialog 
Selecting and adding new physics collections
Like in the [original Fireworks](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookFireworks) original Fireworks, it is possible to select EDM collections and to add them into graphical or table views. The dialog contains general search filed on the top that matches text anywhere in the table (module label, type, process name, or visualization purpose). Sorting on any column can be activated by clicking on the column heading.

<img src="add-collection.png" style="align: center;">


## Table view

In the table view any loaded physics collection can be chosen from the drop-down menu in the table view header. The edit button gives access to a dialog for adding a new table column. The column expression entry has a tab-complete functionality that lists public member function for the physics item class. At the moment, the list does not contain functions from the base-classfrom baseclasses.  Note, just like in the collection controller, the physics item that is to be evaluated is passed into the expression as variable 'i' and functions can be accessed by the 'i.functionName()' syntax.

<img src="edit-table.png" style="align: center;">

## Event Filter
