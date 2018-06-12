'''
Implements base class for memory elements.

Note that since both DataMemory and InstructionMemory are subclasses of the Memory
class, they will read the same memory file containing both instructions and data
memory initially, but the two memory elements are treated separately, each with its
own, isolated copy of the data from the memory file.

Code written for inf-2200, University of Tromso
'''

from cpuElement import CPUElement
import common

class Memory(CPUElement):
  def __init__(self, filename):
  
    # Dictionary mapping memory addresses to data
    # Both key and value must be of type 'long'
    self.memory = {}
    
    self.initializeMemory(filename)
  
  def initializeMemory(self, filename):
    '''
    Helper function that reads and initializes the data memory by reading input
    data from a file.
    '''
    with open (filename, 'r') as f:
      while True:
        line = f.readline()
        if line == '':
          break
        if line[0] != '#':
          list = line.split('\t')
          address = long(list[0].strip(), 16)
          instruction = long(list[1].strip(), 16)
          self.memory[address] = instruction


    # Remove this and replace with your implementation!
    # Implementation MUST populate the dictionary in self.memory!
    
  def printAll(self):
    for key in sorted(self.memory.iterkeys()):
      print "%s\t=> %s\t(%s)" % (hex(long(key))[:-1], common.fromUnsignedWordToSignedWord(self.memory[key]), hex(long(self.memory[key]))[:-1])


if __name__ == "__main__":
  memory = Memory('add.mem')
  memory.printAll()