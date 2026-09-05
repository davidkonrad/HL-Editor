# An attempt to collaborate with some improvements

This is a playground for a fork of the excellent HL-Editor. The goal is to 

1. Make a full-featured version that works natively in Linux. 

2. Try to implement some of the "whishes" that perhaps could make the editor even better.

## Done so far

#### Toolbar

* Have added a toolbar with the most obvious actions, including new navigation features

* Added zoom-buttons with 0.5 granularity

* Added a 'deselect' button to reset tilelist, unitlist selection

#### Child windows

* Tile and unit windows are now scaled more properly

* You can now set 'Lock Window Tile size', and the child window tile sizes remain the same when You scale 

* Added 'Restore window positions', if checked the mainwindow, tilelist and unitlist opens up in same position and size

* Tile window is now always updated according to summer / winter

#### Other

* Now autoset zoom as well as Scale_factor upon upstart

* Added 'Autoload recent map', if checked the Editor load recent map at start 

* You can now overwrite a level when adding it (instead of going through 'Remove map from game' first

* Added 'Hide native maps', which on 'Remove map from game' hides the native maps (instead of scrolling trough them each time)

* You can now add mountains with a single click! Selecting one of the 8 mountain tiles and right click on the map.

#### Code specific

* Sunset the use of `fopen_s` (as mentioned by Knippert in `main.cpp`). That was obvious, I actually needed to find `msvcp140_1.dll` and put it in the HL-Editor main directory, in order to run the EXE with Wine. That should have been fixed.

* Refactored QMessageboxes to standalone functions. Avoid redundancy and now all messages are on top of the windows. They are still not aligned to the mainscreen

* Introduce `utils.h` as container for code redundancy cleanup (for now dialogs)

* Introduce `resources.qrc` to include graphics and else in the program file

* Replaced multiple IO calls to .TMP file with a single TMP_Rec struct. Less code and easier to expand.

* Refactored the 'CONFIG.CFG' setup, now using QSettings and INI file format (less code, easy expandable)



### Whish list, to be implemented if possible and if I am able to figure it out

* Overview of all units of both sides

* Make a "move unit" feature (by dragging)

* select square of map tiles and move,- or cut, copy, paste
	
* A "paint" mode, so you dont have to click n times

* A tile window with minature map 

* Set COM-type in menu instead of last minute option (with reminding popover titles)

* Possible to choose default side? HL seems always to assume you are allied, except if you override one of the existing level codes.

* Undo (just last action / click), reload 

* Detect "add to map" level code before hitting enter, disallow more or less than 5 chars, upcase

* Fix weight in transporters

* Fix reverse HQ ressources bug

* Disallow override of building areas, except if it is an entrance

* Map annotations: Who did it, story, strategy, preferred level code 

* Calculate power deficit (by strength) in "map info"

* A "clear all customs maps" function, i.e restore original /MAP

* A more useful overview of all registered maps, also when browsing/loading (I think preload a miniature

 
