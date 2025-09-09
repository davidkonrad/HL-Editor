# An attempt to collaborate with some improvements

This is a playground for a fork of the excellent HL-Editor. The goal is to 

1. Make a full-featured version that works natively in Linux. 

2. Try to implement some of the "whishes" that perhaps could make the editor even better.

## Done so far

* Sunset the use of `fopen_s` (at mentioned by Knippert in `main.cpp`). That was obvious, I needed to find `msvcp140_1.dll` and put it in the HL-Editor main directory, in order to run it with wine. That should have been solved. 

* Make paths unix-style, a new flag `QT_TARGET` determines if the paths should be C-like or unix-like. 

* Introduce `utils.h` as container for code redundancy cleanup.


### Whishlist, to be implemented if possible and if I am able to figure it out

* Insertion of HQ's, factories and depots in one single click (you can do it now by right-clicking on an entrance)

* Toolbar with most common actions, like scale, save and so on

* Tile-window "resets". Annoying that you must right click on the tile windows in order to not set a new terrain or unit while clicking

* Map annotations, who did it, story, strategy and so on

* A map generator (like we have in CIV)


