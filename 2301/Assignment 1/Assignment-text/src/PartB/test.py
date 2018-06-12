import httplib, xml.etree.ElementTree as xml

# This is a test client to test the implementation of the 
# INF-2301 first assignenet.
#
# Note that this is just some simple tests, they cannot 
# guarantee that you server is 100% correct.


def xmlFromString(xmlString):
    # Create a xml file from result
    f = open("v.xml", 'w')
    f.write(xmlString)
    f.close()
    
    return xml.parse("v.xml")

def verify_messages(xmlString):
    """ Verify if the xml is correctly structured. """
    retval = True
    
    mXml = xmlFromString(xmlString)
    
    # Check for root element
    root = mXml.getroot()
    if root.tag != "messages":
        print "Root element invalid, Found:", root.tag
        retval = False
    
    # Check messages
    msgs = root.getchildren()
    for msg in msgs:
        if not msg.get("id") or not msg.get("value"):
            print "Message invalid."
            retval = False
     
    return retval

def num_messages(xmlString):
    mXml = xmlFromString(xmlString)
    return len(mXml.getroot().getchildren())
    

class TestClient:
    
    def __init__(self):
        self.conn = httplib.HTTPConnection("localhost", 8080)
    
    def run(self):
        self.test_get()
        self.test_post()
        self.test_put()
        self.test_delete()
       
    
    def test_get(self):
        print "\nTesting GET:"
        
        status, res = self.request("GET", "/messages")
        print status
        if status != 200:
            print "GET response is a fail!"
            return
        print "GET response OK!,","Verifying xml resposne:"
        
        if verify_messages(res) == False:
            print "xml response could not be verified!"
        else:
            print "Verified!"
            print "GET seems to work!"

    def test_post(self):
        print "\nTesting POST:"
        
        status, res = self.request("POST", "/messages", "value=\"Test+post+three...\"")
        print "POST",res
        if status != 200:
            print "POST response if a fail!"
            return
        else:
            if res.find("id=") == -1:
                print "No id returned from server!"
                print "POST failed."
                return
            else:
                print "POST response ok!", "Verifying presence of new message:"
        
        # Get id of the new message
        id = res.split("=")[1][1:-1]
        id = id[:id.find("\"")]
        
        # Get to see if message was posted
        status, res = self.request("GET", "/messages")
        
        mXml = xmlFromString(res)
        msgs = mXml.getroot().getchildren()
        present = False
        for msg in msgs:
            if msg.get("id") == id:
                present = True
        
        if present:
            print "Message successfully posted"
            print "POST seems to work"
        else:
            print "Message not posted correctly. Try again"
            
        
    def test_put(self):
        print "\nTesting PUT:"
        
        # Post a new message
        status, res = self.request("POST", "/messages", "value=\"To+be+edited.\"")
        id = res.split("=")[1][1:-1]
        
        # Edit it        
        status, res = self.request("PUT", "/messages", "id=\""+id+"\""+" value=\"Edited.\"")
        if status != 200:
            print "PUT response if a fail!"
            return
        else:
            print "PUT response ok!", "Verifying new message:"

        # Check if it was edited correctly
        status, res = self.request("GET", "/messages")
        
        mXml = xmlFromString(res)
        msgs = mXml.getroot().getchildren()
        present = False
        for msg in msgs:
            if msg.get("value") == "Edited.":
                present = True
        
        if present:
            print "Message succesfully posted"
            print "PUT seems OK!"
        else:
            print "Message not edited correctly."
    
    def test_delete(self):
        print "\nTesting DELETE:"
        
        # Get all messages
        status, res = self.request("GET", "/messages")
        num = num_messages(res)
        
        # Get id from one of them
        i = res.find("id=")
        res = res[i:]
        i = res.find(" ")
        id = res[:i]
        
        # Delete it
        status, res = self.request("DELETE", "/messages", id)
        if status != 200:
            print "DELETE response is a fail!"
            return
        
        # Verify that the number of messages has decreased by 1
        status, res = self.request("GET", "/messages")
        newnum = num_messages(res)
        if num - newnum != 1:
            print "DELETE failed!"
        else:
            print "DELETE seems OK!"

    def request(self, method, path, payload = None):
        self.conn.request(method, path, payload)
        response = self.conn.getresponse()
        status = response.status
        data = response.read()
        response.close()
        self.conn.close()
        return status, data
    
if __name__ == "__main__":
    c = TestClient()
    c.run()
