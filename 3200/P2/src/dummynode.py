#!/usr/bin/env python
import argparse
import httplib
import re
import signal
import socket
import threading

from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer


object_store = {}
neighbours = []

class NodeHttpHandler(BaseHTTPRequestHandler):

    def send_whole_response(self, code, content, content_type="text/plain"):
        self.send_response(code)
        self.send_header('Content-type', content_type)
        self.send_header('Content-length',len(content))
        self.end_headers()
        self.wfile.write(content)


    def extract_key_from_path(self, path):
        return re.sub(r'/storage/?(\w+)', r'\1', path)

    def do_PUT(self):
        content_length = int(self.headers.getheader('content-length', 0))

        key = self.extract_key_from_path(self.path)
        value = self.rfile.read(content_length)

        object_store[key] = value

        # Send OK response
        self.send_whole_response(200, "Value stored for " + key)


    def do_GET(self):
        if self.path.startswith("/storage"):
            key = self.extract_key_from_path(self.path)

            if key in object_store:
                self.send_whole_response(200, object_store[key])
            else:
                self.send_whole_response(404,
                        "No object with key '%s' on this node" % key)

        if self.path.startswith("/neighbours"):
            self.send_whole_response(200, "\n".join(neighbours) + "\n")

        else:
            self.send_whole_response(404, "Unknown path: " + self.path)


def parse_args():
    PORT_DEFAULT = 8000
    DIE_AFTER_SECONDS_DEFAULT = 20 * 60
    parser = argparse.ArgumentParser(prog="node", description="DHT Node")

    parser.add_argument("-p", "--port", type=int, default=PORT_DEFAULT,
            help="port number to listen on, default %d" % PORT_DEFAULT)

    parser.add_argument("--die-after-seconds", type=float,
            default=DIE_AFTER_SECONDS_DEFAULT,
            help="kill server after so many seconds have elapsed, " +
                "in case we forget or fail to kill it, " +
                "default %d (%d minutes)" % (DIE_AFTER_SECONDS_DEFAULT, DIE_AFTER_SECONDS_DEFAULT/60))

    parser.add_argument("neighbours", type=str, nargs="+",
            help="addresses (host:port) of neighbour nodes")

    return parser.parse_args()


if __name__ == "__main__":

    args = parse_args()

    server = HTTPServer(('', args.port), NodeHttpHandler)
    neighbours = args.neighbours

    def run_server():
        print "Starting server on port" , args.port
        server.serve_forever()
        print "Server has shut down"

    def shutdown_server_on_signal(signum, frame):
        print "We get signal (%s). Asking server to shut down" % signum
        server.shutdown()

    # Start server in a new thread, because server HTTPServer.serve_forever()
    # and HTTPServer.shutdown() must be called from separate threads
    thread = threading.Thread(target=run_server)
    thread.daemon = True
    thread.start()

    # Shut down on kill (SIGTERM) and Ctrl-C (SIGINT)
    signal.signal(signal.SIGTERM, shutdown_server_on_signal)
    signal.signal(signal.SIGINT, shutdown_server_on_signal)

    # Wait on server thread, until timeout has elapsed
    #
    # Note: The timeout parameter here is also important for catching OS
    # signals, so do not remove it.
    #
    # Having a timeout to check for keeps the waiting thread active enough to
    # check for signals too. Without it, the waiting thread will block so
    # completely that it won't respond to Ctrl-C or SIGTERM. You'll only be
    # able to kill it with kill -9.
    thread.join(args.die_after_seconds)
    if thread.isAlive():
        print "Reached %.3f second timeout. Asking server to shut down" % args.die_after_seconds
        server.shutdown()

    print "Exited cleanly"
