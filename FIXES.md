# An attempt to collaborate with some improvements

This is a playground for a fork of the excellent HL-Editor. The goal is to 

1. Make a full-featured version that works natively in Linux. 

2. Try to implement some of the "whishes" that perhaps could make the editor even better.

## Done so far

* Sunset the use of `fopen_s` (at mentioned by Knippert in `main.cpp`). That was obvious, I needed to find `msvcp140_1.dll` and put it in the HL-Editor main directory, in order to run it with wine. That should have been solved. 

* Make paths unix-style, a new flag `HL_TARGET` determines if the paths should be C-like or unix-like. 

* Introduce `utils.h` as container for code redundancy cleanup.


### Whishlist, to be implemented if possible and if I am able to figure it out

* Battleships and other large ships only come out as "half" from depots or factories

* Detect "add to map" level code before hitting enter, disallow more or less than 5 chars, upcase

* Insertion of HQ's, factories and depots in one single click (you can do it now by right-clicking on an entrance)

* Fix weight in transporters

* Toolbar with most common actions, like scale, save and so on

* Disallow overrride of building areas, except if it is an entrance

* Tile-window "resets". Annoying that you must right click on the tile windows in order to not set a new terrain or unit while clicking

* Map annotations, who did it, story, strategy and so on

* A map generator (like we have in CIV)

* Calculate power deficit

* A "clear all customs maps" function, i.e restore original /MAP

* A more useful overview of all registered maps

* Make a native DOS-version using Borland C++/Turbo Vision

### Schedule
It is my first attempt to write C++ ever since a 7 hour C++ graduating test in 1997. But it is like java or python in french. 
Should be able to manage it. So my first attempts to refactor the code to avoid redundancy in the dialogs, and change the hardcoded way
constructing a path is actually my learning lessons to not just get familiar with the code but also get familiar with the Qt C++ system. 
I have not yet managed to produce an .EXE as DragonSphere, but that is the entire point. The goal is to make it fluid with a debian-like system, 
while it also can be compiled into an EXE.
 
