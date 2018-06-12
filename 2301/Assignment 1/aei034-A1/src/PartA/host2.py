import socket
import os

#initiate the server
s = socket.socket()
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(("127.0.0.1", 8080))
s.listen(1)

while True:
	try:
		#Status response messages
		ok = "HTTP/1.1 200 OK\r\n\r\n"
		nf = "HTTP/1.1 404 NOT FOUND\r\n\r\n"
		headers = {} #dictionairy containing the headers
		conn, addr = s.accept() #accept any incoming connection

		#makefiles for recieving and sending data from/to client
		datafile = conn.makefile("r")
		respfile = conn.makefile("w")

		#fetching the requestline, then splitting it up to recieve the essentials of the requestline
		requestline = datafile.readline()
		method = requestline.split()[0]
		url = requestline.split()[1]
		path = url[1:]

		#make sure 'index.html' is opened if no path is specified
		if len(path) == 0:
			path = "."
		if os.path.isdir(path):
			path = os.path.join(path, 'index.html')

		version = requestline.split()[2]

		while True: 

			header = datafile.readline().strip()
			if header == '':
				break
			name = header[:header.find(':')].strip()
			value = header[header.find(':'):].strip()

			headers[name] = value

		if method == "GET":
			"""This is the GET method"""
			respfile.write("HTTP/1.1 200 OK\r\n\r\n")
			with open(path, 'r') as index:
				respfile.write(index.read())
				respfile.flush()
				respfile.close()

		if method == "POST":
			"""This is the POST method"""
			#finding length of the 'Content-Length' header
			if 'Content-Length' in headers:
				length = int(headers['Content-Length'][2:])
			else:
				length = 0

			payload = datafile.read(length) #payload is based off of the 'Content-Length' header
			#append whatever is posted, to the file
			with open(path, 'a') as f:
				f.write("\n" + payload)
				respfile.write(ok + payload)
				respfile.flush()
				respfile.close()

		datafile.close() #close the stream connection
		conn.shutdown(socket.SHUT_RDWR) #shutdown the connection socket object
		conn.close() #close the connection socket object
	except KeyboardInterrupt:
		break
s.close() #close the server socket object