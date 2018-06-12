import httplib, socket, xml.etree.ElementTree as xml
import os, urllib

class RESThandler():

	def __init__(self):
		#Initiate the server
		self.s = socket.socket() 											#create the socket object
		self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.s.bind(("", 8080))
		self.s.listen(1)
		#Define status messages
		self.ok = "HTTP/1.1 200 OK\r\n"
		self.nf = "HTTP/1.1 404 NOT FOUND\r\n"

	def get(self, path, sendstream):
		"""This is the method for GET"""
		if os.stat(path).st_size == 0: 										#Checking if the xml file is empty
			sendstream.write(self.nf)
			sendstream.flush()
		else:
			with open(path, 'r') as xml: 									#makes us able to read given file at path
				sendstream.write(self.ok)
				sendstream.write(xml.read()) 
				sendstream.flush()

	def post(self, recvstream, sendstream, path, headers, payload, length):
		"""This is the method for POST"""
		tree = xml.parse(path) 												#parsing the xml file
		root = tree.getroot() 												#defining the root of the parsed file

		# providing a new id based on how many already exists in the file
		num = 0
		for i in root:
			if i.attrib['id'] >= num:
				num = int(i.attrib['id']) + 1


		payload = payload[6:] 												#payload will be the message posted
		payload = urllib.unquote_plus(payload) 								#replace '+' with spaces
		payload = payload[1:-1] 											#remove the quotation marks from the message
		node = {"id": str(num), "value": str(payload)} 						#create a new node for the xml tree

		newElem = xml.Element('message', node) 								#define a new element with the 'message' tag in the tree
		root.append(newElem) 												#append the newly created element to the tree
		tree.write(path) 													#write the tree to the path (the xml file)

		sendstream.write(self.ok)
		sendstream.write("Content-Length: " + str(len(str(num)))) 			#provide client with content length
		sendstream.write("\r\n\r\n") 										#end of headers
		sendstream.write(str(num)) 											#provide client with the newly created id
		sendstream.flush()

	def put(self, recvstream, sendstream, path, headers, payload):
		"""This is the method for PUT. This request requires and id and a message field as payload."""
		tree = xml.parse(path)
		root = tree.getroot()

		newId = payload[payload.find('\"')+1:payload.find(' ')-1] 			#find the id to replace provided by the client

		newValue = payload[payload.rfind('=')+2:-1] 						#find the new value provided by the client

		#searching through the tree, replacing the value at the given id
		for i in root:
			if i.attrib['id'] == newId:
				i.attrib['value'] = newValue

		#write tree to the xml file, send status message to client
		tree.write(path)
		sendstream.write(self.ok + "Content-Length: 0\r\n\r\n")
		sendstream.flush()

	def delete(self, recvstream, sendstream, path, headers, payload):
		"""This is the method for DELETE. This request requires an id which message to delete."""
		tree = xml.parse(path)
		root = tree.getroot()

		#find the id which message should be removed
		ret = payload[payload.find('\"'):]
		ret = ret[1:-1]

		#remove the message
		for i in root:
			if i.attrib['id'] == ret:
				root.remove(i)

		#write tree to the xml file, send status message to client
		tree.write(path)
		sendstream.write(self.ok + "Content-Length: 0\r\n\r\n")
		sendstream.flush()

	def run(self):
		"""This is the main function of the program"""
		while True:
			try:
				#accept a connection request
				conn, addr = self.s.accept()
				#create to makefiles, one for reading, and one for writing to
				recvstream = conn.makefile('r')
				sendstream = conn.makefile('w')

				#reading and splitting up the request line
				requestline = recvstream.readline()
				method = requestline.split()[0]
				url = requestline.split()[1]
				
				path = url[1:]
				
				if len(path) == 0:
					path = "."
				
				if os.path.isdir(path):
					path = os.path.join(path, 'messages.xml')
				
				if path == 'messages':
					path = 'messages.xml'
				
				headers = {} 												#create a dictionairy for the headers

				#looping through the headers, giving them each a name and value
				while True:
					
					header = recvstream.readline().strip()
					if header == '':
						break
					name = header[:header.find(':')].strip()
					value = header[header.find(':'):].strip()

					headers[name] = value
				#finding the content length from the 'headers' dictionairy, if there is one
				if 'Content-Length' in headers:
					length = int(headers['Content-Length'][2:])
				else:
					length = 0

				payload = recvstream.read(length) #reading the payload

				#Run the responsible method based on the request from the client
				if method == 'GET':
					self.get(path, sendstream)
				else:
					if method == 'POST':
						self.post(recvstream, sendstream, path, headers, payload, length)
					elif method == 'PUT':
						self.put(recvstream, sendstream, path, headers, payload)
					elif method == 'DELETE':
						self.delete(recvstream, sendstream, path, headers, payload)
					
					self.get(path, sendstream)

				sendstream.close()
			except KeyboardInterrupt:
				break
			conn.shutdown(socket.SHUT_RDWR) 								#shutdown the connection
			conn.close() 													#close the connection socket
		self.s.close() 														#close the main socket object

if __name__ == "__main__":
	r = RESThandler()
	r.run()
