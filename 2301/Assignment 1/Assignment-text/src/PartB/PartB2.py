import httplib, socket, xml.etree.ElementTree as xml
import os
import urllib

class RESThandler():

	def __init__(self):
		self.s = socket.socket()
		self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.s.bind(("", 8080))
		self.s.listen(1)
		self.ok = "HTTP/1.1 200 OK\r\n\r\n"
		self.nf = "HTTP/1.1 404 NOT FOUND\r\n"

	def get(self, path, sendstream):
		if os.stat(path).st_size == 0:
			sendstream.write(self.nf)
			sendstream.flush()
		else:
			print "size in bytes:", os.stat(path).st_size
			with open(path, 'r') as xml:
				sendstream.write(self.ok)
				sendstream.write(xml.read())
				sendstream.flush()

	def post(self, recvstream, sendstream, headers, path):
		print "Path:", path

		tree = xml.parse(path)
		root = tree.getroot()

		num = 0
		for i in root:
			if i.attrib['id'] >= num:
				num = int(i.attrib['id']) + 1

		if 'Content-Length' in headers:
			length = int(headers['Content-Length'][2:])
		else:
			length = 0
		payload = recvstream.read(length)
		payload = payload[6:]
		payload = urllib.unquote_plus(payload)
		print "Payload:", payload
		payload = payload[1:-1]
		node = {"id": str(num), "value": str(payload)}

		newElem = xml.Element('message', node)
		root.append(newElem)
		tree.write(path)
		sendstream.write(self.ok)

	def put(self, recvstream, sendstream, headers, path):
		tree = xml.parse(path)
		root = tree.getroot()

		if 'Content-Length' in headers:
			length = int(headers['Content-Length'][2:])
		else:
			length = 0
		payload = recvstream.read(length)
		print "Payload:", payload

		newId = payload[payload.find('\"')+1:payload.find(' ')-1]
		print "nyid:", newId

		newValue = payload[payload.rfind('=')+2:-1]
		print "nyvalue:", newValue

		for i in root:
			if i.attrib['id'] == newId:
				i.attrib['value'] = newValue
		tree.write(path)
		sendstream.write(self.ok)

	def delete(self, recvstream, sendstream, headers, path):
		tree = xml.parse(path)
		root = tree.getroot()

		if 'Content-Length' in headers:
			length = int(headers['Content-Length'][2:])
		else:
			length = 0
		payload = recvstream.read(length)
		print "Payload:", payload

		ret = payload[payload.find('\"'):]
		ret = ret[1:-1]
		print "ret:", ret

		for i in root:
			if i.attrib['id'] == ret:
				root.remove(i)

		tree.write(path)
		sendstream.write(self.ok)

	def run(self):
		while True:
			try:
				conn, addr = self.s.accept()
				print "Connected by:", addr
				recvstream = conn.makefile('r')
				sendstream = conn.makefile('w')

				requestline = recvstream.readline()
				method = requestline.split()[0]
				url = requestline.split()[1]
				
				print "url:", url
				path = url[1:]
				
				if len(path) == 0:
					path = "."
				
				if os.path.isdir(path):
					path = os.path.join(path, 'messages.xml')
				
				if path == 'messages':
					path = 'messages.xml'
				
				headers = {}
				while True:
					
					header = recvstream.readline().strip()
					if header == '':
						break
					name = header[:header.find(':')].strip()
					value = header[header.find(':'):].strip()

					headers[name] = value
				if method == 'GET':
					print "Method:", method
					self.get(path, sendstream)
				if method == 'POST':
					print "Method:", method
					self.post(recvstream, sendstream, headers, path)
					self.get(path, sendstream)
				if method == 'PUT':
					print "Method:", method
					self.put(recvstream, sendstream, headers, path)
					self.get(path, sendstream)
				if method == 'DELETE':
					print "Method:", method
					self.delete(recvstream, sendstream, headers, path)
				sendstream.close()
			except KeyboardInterrupt:
				break
			conn.shutdown(socket.SHUT_RDWR)
			conn.close()
		self.s.close()

if __name__ == "__main__":
	r = RESThandler()
	r.run()
