*****************************
Rory Bresnahan
bresnahr@onid.oregonstate.edu
CS372
Project 2
README.txt
*****************************
File Transfer Program between a server and client

ftserver opens a socket connection and waits for ftclient to connect to the
specified host/control port.
ftclient can request the server's directory listing or request a file to be
transferred.
Message communication happens on a control connection while data tranport
occurs on a data connection.


File included: ftserver.c  ftclient.py  Makefile


***********
To Compile:
***********

Use the Makefile to compile ftserver.c

Place ftserver.c and Makefile in the same directory, then enter 'make all'

ftclient.py does not need to be compiled because it is Python.


*******
To Run:
*******


Ideally place ftclient.py in a different directory (keep Makefile and
ftserver.c together), so when you run both ftclient and ftserver you can see the file transfer in action.

You will have to run the client and server simultaneously, therefore open two
terminals and run the programs.

First begin the server program:

After 'make all', you can run the program by entering: ftserver <SERVER
PORT>
<SERVER PORT> is the port you want the server to be connected to
Example: ftserver 3333


Then switch to your second terminal and run ftclient.py
Do this by running a python program: python ftclient.py 
and add command-line arguments.

You have a choice of using command '-l' to list the server's directory
or command '-g', which combined with a file name, will request that file from the
server.

For command '-l':
You need to enter valid command-line arguments as follows:

<SERVER HOST> <SERVER_PORT> <COMMAND> <DATA_PORT> 
Example: python ftclient.py localhost 3333 -l 33334

For command '-g':
Enter command-line arguments as follows:

<SERVER HOST> <SERVER_PORT> <COMMAND> <DATA_PORT> <FILENAME>
Example: python ftclient.py localhost 3333 -g 3334 filename.txt


ftServer will run until the operator terminates it.  ftclient terminates after
submitting and receiving command data, and/or error messages that require
restart


Enjoy!!
