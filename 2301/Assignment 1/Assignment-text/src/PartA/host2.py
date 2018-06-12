import socket
import os

s = socket.socket()

s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

s.bind(("127.0.0.1", 8080))

s.listen(1)

while True:
	try:
		ok = "HTTP/1.1 200 OK\r\n\r\n"
		nf = "HTTP/1.1 404 NOT FOUND\r\n\r\n"
		headers = {}
		conn, addr = s.accept()
		print "Connected by: ", addr

		datafile = conn.makefile("r")
		respfile = conn.makefile("w")

		requestline = datafile.readline()
		method = requestline.split()[0]
		url = requestline.split()[1]
		path = url[1:]
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
		print "Headers:\n",headers
		if method == "GET":
			print "Method:", method
			respfile.write("HTTP/1.1 200 OK\r\n\r\n")
			with open(path, 'r') as index:
				respfile.write(index.read())
				respfile.flush()
				respfile.close()

		if method == "POST":
			print "Method:", method
			if 'Content-Length' in headers:
				length = int(headers['Content-Length'][2:])
			else:
				length = 0
			payload = datafile.read(length)
			print payload
			with open(path, 'a') as f:
				f.write("\n" + payload)
				respfile.write(ok + payload)
				respfile.flush()
				respfile.close()

		datafile.close()
		conn.shutdown(socket.SHUT_RDWR)
		conn.close()
	except KeyboardInterrupt:
		break
s.close()