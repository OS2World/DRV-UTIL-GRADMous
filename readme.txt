README.TXT for GRADMOUS 1.0.0
-----------------------------

INSTALLATION
------------
To install Gradmous, simply copy the file gradmous.exe into a
suitable directory and run it from a WinOS/2 Fullscreen.


USAGE
-----
Start the program from a fullscreen WinOS/2 session. 
It will not run in seamless mode.
It creates an icon on the WinOS/2 desktop.
You can disable the correction in it's systemmenu.
There are some parameters but normally you do not need it.

tip: create a program-item in the WinOS/2 StartUp folder without parameter.


PARAMETERS
----------
/q
  quiet, less messages

/v
  verbose, more messages

/d
  initially disabled

/h
  hidden, does not create an icon on the desktop nor an entry in the tasklist

/x
  executes the specified program incl. additional parameters
  therefore /x has to be the latest parameter.


REMARKS
-------
Unfortunately it does not correct the speed of the mouse while a 
systemmodal dialog is displayed. (e.g. the "Exit Win-OS/2" dialog)


STATUS
------
This program is free software; you can redistribute it and/or modify it.


DISCLAIMER
----------
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
The entire risk is with you.


Content of package
------------------
GRADMOUS.EXE      The executable
README.TXT        This file
SOURCE.ZIP        Source (Borland C++ 3.1 project)
