import httplib, socket, xml.etree.ElementTree as xml
from xml.etree.ElementTree import ElementTree

class RESThandler():

	def __init__(self):
		self.s = socket.socket()   																
		self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) 							
		self.s.bind(("", 8080))
		self.s.listen(1)

	def get(self):		
		ok = "HTTP/1.1 200 OK\r\n\r\n"
		nf = "HTTP/1.1 404 NOT FOUND\r\n\r\n"
										
		print "Request:  ", self.data.split()[0] 							
		#if self.data.split()[1] == '/messages':
		with open('storage.xml', 'r') as f:
			#tree = ElementTree() 									
			#tree.parse("storage.xml")
			#t = tree.find("messages/message")
			#msg = tree.getiterator("message")
			#print msg
			#for i in msg:
			#	print str(i)							
			self.connection.sendall(ok + f.read())
		print "Hva apnes: ", self.data.split()[2]								
		#elif self.data.split()[1] != '/':
		#	try:
		#		with open(self.data.split()[1][1:], 'r') as page:
		#			connection.sendall(ok + page.read()) 							
		#	except IOError:
		#		with open('404.html', 'r') as wrong:
		#			connection.sendall(nf + wrong.read())

	def post(self, message, id):
		pass

	def put(self, message, id):
		pass

	def delete(self, message, id):
		pass

	def run(self):
		while True:
			try:
				self.connection, self.addr = self.s.accept()
				print "Connected by: ", self.addr  
				self.data = self.connection.recv(1024)
				if self.data.split()[0] == "GET":
					self.get()
				self.connection.close()
			except KeyboardInterrupt:
				break
		self.s.close()

if __name__ == "__main__":
	r = RESThandler()
	r.run()