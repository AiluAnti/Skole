#!/usr/bin/env python

# Skeleton chord node
#
# Note that this does not use the chord-config.json file that the Go code uses.
# Port numbers and hosts to run on are hard-coded, and must be changed.

import argparse
import httplib
import os
import random
import re
import signal
import socket
import subprocess
import threading
import urlparse

from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from SocketServer import ThreadingMixIn


server = None
this_node = None
neighbors = set()
neighborlock = threading.Lock()


def launch(host, commandline, stdout=None, stderr=None, wait=False):
    """ Runs a command either locally or on a remote host via SSH """
    cwd = os.getcwd()
    if host == 'localhost':
        pass
    else:
        commandline = "ssh -f %s 'cd %s; %s'" % (host, cwd, commandline)

    print commandline
    process = subprocess.Popen(commandline, shell=True, stdout=stdout, stderr=stderr)
    if wait:
        process.wait()

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    """Handle requests in a separate thread."""

class NodeHttpHandler(BaseHTTPRequestHandler):

    def send_ok_200(self, body):
        self.send_response(200)
        self.send_header('Content-type','text/plain')
        self.send_header('Content-length',len(body))
        self.end_headers()
        self.wfile.write(body)
        self.wfile.flush()
        self.wfile.close()

    def send_error_404(self):
        errortext = "Path not found: " + self.path
        self.send_response(404)
        self.send_header('Content-type','text/plain')
        self.send_header('Content-length',len(errortext))
        self.end_headers()
        self.wfile.write(errortext)

    def do_POST(self):
        content_length = int(self.headers.getheader('content-length', 0))
        self.rfile.read(content_length)

        if self.path == "/addNode":
            host = "localhost"
            port = random.randrange(8000,9000)
            hostport = "{0}:{1}".format(host,port)
            print "Starting a new server on {0}".format(hostport)
            launch(host, "./node.py --port {0} --parent {1}".format(port,this_node))
            with neighborlock:
                neighbors.add(hostport)
            self.send_ok_200(hostport)

        elif self.path == "/shutdown":
            msgadd = "Got HTTP request to shutdown. Shutting down..."
            msgs = [msgadd]
            print msgadd
            self.send_ok_200("\n".join(msgs))
            threading.Thread(target=server.shutdown).start()

        else:
            self.send_error_404()


    def do_GET(self):
        if self.path == "/neighbours":
            self.send_ok_200("\n".join(neighbors))
        else:
            self.send_error_404()

def parse_args():
    PORT_DEFAULT = 8000
    DIE_AFTER_SECONDS_DEFAULT = 20 * 60
    parser = argparse.ArgumentParser(prog="node", description="DHT Node")

    parser.add_argument("-p", "--port", type=int, default=PORT_DEFAULT,
            help="port number to listen on, default %d" % PORT_DEFAULT)

    parser.add_argument("-n", "--parent", type=str, default=None,
            help="parent node that is spawning this node")

    parser.add_argument("--die-after-seconds", type=float,
            default=DIE_AFTER_SECONDS_DEFAULT,
            help="kill server after so many seconds have elapsed, " +
                "in case we forget or fail to kill it, " +
                "default %d (%d minutes)" % (DIE_AFTER_SECONDS_DEFAULT, DIE_AFTER_SECONDS_DEFAULT/60))

    return parser.parse_args()


if __name__ == "__main__":

    args = parse_args()

    server = ThreadedHTTPServer(('', args.port), NodeHttpHandler)
    this_node = "localhost:{0}".format(args.port)
    if args.parent:
        neighbors.add(args.parent)

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
