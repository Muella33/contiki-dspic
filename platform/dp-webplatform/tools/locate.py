#!/usr/bin/python
# -*- coding: iso-8859-1 -*-

import Tkinter, tkFileDialog
from itertools import izip
import socket
import os
import Queue, threading
from tkMessageBox import *

## http://stackoverflow.com/questions/5389507/iterating-over-every-two-elements-in-a-list
def pairwise(iterable):    
	a = iter(iterable)
	return izip(a, a)

class simpleapp_tk(Tkinter.Tk):
    def __init__(self,parent,port=4321):
        Tkinter.Tk.__init__(self,parent)
        self.parent = parent
        self.msgqueue = Queue.Queue()
        self.webnodes = dict()
        self.port = port
        self.recv = udp_listen(port, self.msgqueue)
        try:
            scriptdir = os.path.dirname(os.path.abspath(__file__))
            self.iconbitmap(os.path.join(scriptdir, 'dp-favicon.ico'))
        except:
            print 'failed to set icon'
        self.initialize()
		
    def initialize(self):
        self.grid()
        self.minsize(600,250)
		
        listbox = Tkinter.Listbox(self, font=('Lucida Console', 9) )
        listbox.grid(column=0,row=1,sticky='NSEW')
        listbox.insert(Tkinter.END, 'Searching for devices...')
        self.listbox = listbox
		
        # button = Tkinter.Button(self,text=u"Click me !", command=self.OnButtonClick)
        # button.grid(column=1,row=0)

        label1 = Tkinter.Label(self,text=("Listening to UDP port %d" % (self.port)), anchor="w")
        label1.grid(column=0,row=0,columnspan=1,sticky='EW')
        
        self.labelVariable = Tkinter.StringVar()
        label = Tkinter.Label(self,textvariable=self.labelVariable, anchor="w")
        label.grid(column=0,row=2,columnspan=1,sticky='EW')
        self.labelVariable.set(u"Found 0 devices")

        self.grid_columnconfigure(0,weight=1)
        self.grid_rowconfigure(1,weight=1)
        self.resizable(True,True)
        self.update()
        self.geometry(self.geometry())       
        
        self.timertask = self.after(100, self.pollQueue)

    def pollQueue(self):
        self.timertask = self.after(100, self.pollQueue)
		
        while self.msgqueue.qsize():
            try:
                msg = self.msgqueue.get(0)
                print 'GUI thread recieved ' + str(msg)
                ## merge/store node entries by MAC
                self.webnodes[ msg['ETH'] ] = msg
                
                self.listbox.delete(0, Tkinter.END)
                self.listbox.insert(0, "MAC               IP             RTC                  FIRMWARE " )
                for k,v in self.webnodes.items():
					self.listbox.insert(Tkinter.END, "%-16s %-14s %-20s %s" % (v['ETH'], v['IP'], v['RTC'], v['HELLO'] ) )
				
                self.labelVariable.set(u"Found %d devices" % (len(self.webnodes)) )
				
            except Queue.Empty:
                pass		

class udp_listen():
    def __init__(self,port,queue,buffersize=1024):
        self.queue = queue
        self.buffersize = buffersize
		
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # allow multiple instances of the GUI to 'share' listening on UDP port 
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:            
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        except AttributeError:
            pass  # only required SO_REUSEPORT supported only on some platforms
        self.sock.bind(('', port))
        self.sock.setblocking(1)
		
        self.thread = threading.Thread(target=self.run)
        self.thread.setDaemon(1)
        self.thread.start()
        
    def run(self):		
        while (True):            
            buf,addr = self.sock.recvfrom(self.buffersize)
            print 'Got UDP packet: ' + str(buf).strip()
            data = dict(pairwise(buf.split()))
            self.queue.put(data);

if __name__ == "__main__":
    app = simpleapp_tk(None)
    app.title('DP Webplatform announce listener')
    app.mainloop()
    
