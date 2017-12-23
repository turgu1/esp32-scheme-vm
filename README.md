# esp32-scheme-vm

A Scheme interpreter to be used both on a linux based host and on an ESP32 IOT processor.

**__Very very very__** early piece of code. This is work in progress and is not ready for use.

This is a port of the PicoBit Scheme system you can find at the following
[location](https://github.com/stamourv/picobit).

The ESP32 processor provides an environment that requires a lot of changes to
the way the scheme vm operate. As such, I decided to develop a new interpreter
from scratch, using as much as possible from the PicoBit vm implementation.


esp32-scheme-vm is released under the GPLv3.

Guy Turcotte..
December 2017
