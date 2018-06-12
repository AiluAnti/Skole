import socket

s = socket.socket()   																#Create a new socket object

s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) 							#Closes the connection if there is one already existing

s.bind(("127.0.0.1", 8080))

s.listen(1)

while True:
	try:
		ok = "HTTP/1.1 200 OK\r\n\r\n"
		nf = "HTTP/1.1 404 NOT FOUND\r\n\r\n"

		connection, addr = s.accept()

		#print "Connected by: ", addr  												#C-type formatting

		data = connection.recv(4096) 												#The data that is recieved from the client

		if data.split()[0] == "GET": 												#splitting by space
			#print "Request er av type:  ", data.split()[0] 							#request type will be the first word before space
			print "Dette er alle headers: ", data.split('\r\n')[1:]
			if data.split()[1] == '/':
				connection.send(ok) 												#send 200 OK to the client socket, alongside the obligatory newlines and such
				with open('index.html', 'r') as index: 									#allowing us to read the index.html file
					connection.send(index.read()) 										#Send the open index.html to the client socket
			elif data.split()[1] != '/':
				try:
					with open(data.split()[1][1:], 'r') as page:
						connection.sendall(ok + page.read()) 							#send 200 OK to the client socket, alongside the obligatory newlines and such
				except IOError:
					with open('404.html', 'r') as wrong:
						connection.sendall(nf + wrong.read())

		if data.split()[0] == "POST":												#if the first word in the request is POST
			print "Request er av type:  ", data.split()[0] 	
			connection.send(ok) 													#send 200 OK to the client socket, alongside the obligatory newlines and such
			msg = data.split('\r\n\r\n')[1] 										#the message written by the user on the client side will be the second "word" after \r\n\r\n
			#print data.split('\r\n\r\n')[1]
			with open('test.txt', 'a') as test:											#allowing us to append to test.txt file
				test.write('\n' + msg) 													#append whatever the user wrote, to the test.txt file
			connection.sendall(ok + msg)

		#connection.send(data) #Sending the data recieved from the client, back to the client
		connection.close() 															#closes the connection socket between client and server
	except KeyboardInterrupt:
		break

s.close() 																			#closes the socket, NAPPER UT (BUTT)PLUGGEN