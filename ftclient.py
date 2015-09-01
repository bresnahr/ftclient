#!/usr/bin/python

'''
 Rory Bresnahan
 bresnahr@onid.oregonstate.edu
 CS372
 Project 2
 2 connection client-server network application for file transfer
 ftServer accepts a server port command-line argument, this port will be used for the connection.  ftclient and ftserver establish a TCP control connection,
 if ftServer receives valid command a TCP data connection is made with the client.  ftclient can send either -l or -g command via command-line.
 '-l' requests server directory list and '-g' requests a file. 
 Proper command-line argument order is: <SERVER HOST> <SERVER PORT> <COMMAND> <DATA PORT> <FILENAME>
 Data is sent via the data connection, messages are sent via the control connection
'''

import sys
import socket
# from http://stackoverflow.com/questions/3207219/how-to-list-all-files-of-a-directory-in-python
import os
import os.path
from os import listdir
from os.path import isfile, join

filename = ''
data_received = ''

# validate comman-line arguments, make sure there are enough arguments and the commands are valid
if len(sys.argv) > 4 and len(sys.argv) < 7: 
	server_host = sys.argv[1]
	server_port = sys.argv[2]
	command = sys.argv[3]
	
	if command == "-l" or command == "-g":
		pass
	else:
		print "Error: enter valid command '-l' to list directory or '-g' to request file\n"
		exit(1)

	# make sure if command '-l' was entered there are no extra arguments
	if len(sys.argv) > 5 and command == '-l':
		print "Error: too many command-line arguments\n"
		exit(1)
	
	data_port = sys.argv[4]

	if server_port and not server_port.isdigit():
		print "Server port is not a valid number\nUsage: <SERVER HOST> <SERVER PORT> <COMMAND> <DATA PORT> <FILENAME>"
		exit(1)
	
	if data_port and not data_port.isdigit():
                print "Data port is not a valid number\nUsage: <SERVER HOST> <SERVER PORT> <COMMAND> <DATA PORT> <FILENAME>"
                exit(1)

	
	if len(sys.argv) == 6:
		filename = sys.argv[5]

	#make sure if command '-g' was entered a filename follows
	if command == "-g" and not filename:
		print "Error: enter command-line arguments as <SERVER HOST> <SERVER PORT> <COMMAND> <DATA PORT> <FILENAME>\n"
		exit(1)
else:
	print "Error: enter command-line arguments as <SERVER HOST> <SERVER PORT> <COMMAND> <DATA PORT> <FILENAME>\n"
	sys.exit(1)

command_msg = command + ' ' + data_port
if filename != '':
	command_msg += ' ' + filename

#initiate contact  make request  receiveFile
# from https://docs.python.org/2/howto/sockets.html
class mySocket:

    def __init__(self, sock=None):
	if sock is None:
	    self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	    
	else:
	    self.sock = sock
	    
    def bind(self, host_name, data_port):
	try:
		self.sock.bind((socket.gethostbyname(server_host), int(data_port)))
	except socket.error, e:
		print "Binding error: %s\n" % e
		sys.exit(1)

    def connect(self, host, port):
	try:
		self.sock.connect((host, port))
	except socket.gaierror, e:
		print "Address related error connecting to server: %s\n" % e
		sys.exit(1)
	except socket.error, e:
		print "Connection error: %s\n" % e
		sys.exit(1)

    def setsocket(self):
	try:
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	except socket.error, e:
		print "Setsockopt error: %s\n" % e

    def makeRequest(self, msg):
	totalsent = 0
	while totalsent < len(msg):
	    sent = self.sock.send(msg[totalsent:])
	    if sent == 0:
		raise RuntimeError("socket connection broke")
	    totalsent = totalsent + sent

    def recvall(self):
	data = ''
	part = None
	while part != '':
	    part = self.sock.recv(4096)
	    data += part
	return data

# set up control connecction for communication message
# set up data connecton to receive files/ directory list
def initiateContact(controlSock, dataSock, host, server_port, data_port):
	controlSock = mySocket()# create TCP socket for control connection
	controlSock.setsocket()# set socket options
	controlSock.connect(host, int(server_port))# connect the socket to port server where server is listening

	dataSock = mySocket()# create TCP socket for data connection
	dataSock.setsocket()# set socket options
	dataSock.bind(host, data_port)# bind connection
	dataSock.sock.listen(1)# listen for connection

	return controlSock, dataSock

# same as recvall except not part of a class
def receiveFile(sock):
	data = ''
        part = None
        while part != '':
            part = sock.recv(4096)
            data += part
        return data

	#testing variation of this function
	'''data = ''
	part = sock.recv(4096)
        while part:
            data += part
	    part = sock.recv(4096)
	    print data
	   
        return data'''


controlSock = ''
dataSock = '' 
controlSock, dataSock = initiateContact(controlSock, dataSock, server_host, server_port, data_port)

try:
	controlSock.makeRequest(command_msg)# send command request to server
except socket.error, msg:
	sys.stderr.write("[ERROR] %s\n" & msg[1])
	controlSock.close()
        sys.exit(1)

try:
	control_message_received = controlSock.recvall()
except socket.error, msg:	
	sys.stderr.write("[ERROR] %s\n" & msg[1])
	controlSock.close()
	sys.exit(1)

if(control_message_received):
                print "%s: %d says " % (server_host, int(server_port)) + control_message_received + "\n"
		controlSock.sock.close()
		sys.exit(1)

# from https://docs.python.org/3/library/socket.html

try:
	(connectedDataSock, address) = dataSock.sock.accept()
except IOError, e:
	print e.errno
	print e
	sys.exit(1)

connectedDataSocket = mySocket()

if command == '-l':
	print "Receiving directory structure from %s: %d\n" % (server_host, int(data_port))
	data_received = receiveFile(connectedDataSock)
elif command == '-g':
	print "Receiving %s from %s: %d\n" % (filename, server_host, int(data_port))
	if os.path.isfile(filename):
		print "File requested has duplicate name: file rejected\n"
	else:
		print "Receiving file..."
		with open(filename, 'wb') as file_to_write:
			data_received = receiveFile(connectedDataSock)
			file_to_write.write(data_received)

#data_received = receiveFile(connectedDataSock)

if data_received and command == '-g':
	print "File transfer complete\n"
elif data_received and command == '-l':
	print data_received

#close all sockets, client is no longer in use
#connectedDataSock.close()
controlSock.sock.close()
dataSock.sock.close()
sys.exit(0)
