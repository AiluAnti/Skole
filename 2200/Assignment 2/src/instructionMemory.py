'''
Implements CPU element for Instruction Memory in MEM stage.

Code written for inf-2200, University of Tromso
'''
import unittest
from cpuElement import CPUElement
from memory import Memory
from testElement import TestElement

class InstructionMemory(Memory):
  def __init__(self, filename):
    Memory.__init__(self, filename)
  
  def connect(self, inputSources, outputValueNames, control, outputSignalNames):
    CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)

    assert(len(inputSources) == 1), 'InstructionMemory should have one input'
    assert(len(outputValueNames) == 6), 'InstructionMemory should have six outputs'
    assert(len(control) == 0), 'InstructionMemory has no control signal'
    assert(len(outputSignalNames) == 0), 'InstructionMemory has no output signal'

    self.inputName = inputSources[0][1]

    self.op = outputValueNames[0]
    self.rs = outputValueNames[1]
    self.rt = outputValueNames[2]
    self.rd = outputValueNames[3]
    self.shamt = outputValueNames[4]
    self.funct = outputValueNames[5]


  
  def writeOutput(self):
    address = self.inputValues[self.inputName]
    instruction = self.memory[address]

    self.outputValues[self.funct] = instruction & 0x3f
    instruction = instruction >> 6
    self.outputValues[self.shamt] = instruction & 0x1f
    instruction = instruction >> 5
    self.outputValues[self.rd] = instruction & 0x1f
    instruction = instruction >> 5
    self.outputValues[self.rt] = instruction & 0x1f
    instruction = instruction >> 5
    self.outputValues[self.rs] = instruction & 0x1f
    instruction = instruction >> 5
    self.outputValues[self.op] = instruction & 0x3f


  def printAll(self):
    for key in self.outputValues:
      print key, 'field = ', self.outputValues[key]

class TestInstructionMemory(unittest.TestCase):
  def setUp(self):
    self.im = InstructionMemory('add.mem')
    self.testInput = TestElement()
    self.testOutput = TestElement()

    self.testInput.connect(
      [],
      ['Data'],
      [],
      [])

    self.im.connect(
      [(self.testInput, 'Data')],
      ['op', 'rs', 'rt', 'rd', 'shamt', 'funct'],
      [],
      [])

    self.testOutput.connect(
      [(self.im, 'op'), (self.im, 'rs'), (self.im, 'rt'), (self.im, 'rd'), (self.im, 'shamt'), (self.im, 'funct')],
      [],
      [],
      [])

  def test_correct_behavior(self):
    self.testInput.setOutputValue('Data', 0xbfc00218)

    self.im.readInput()
    self.im.writeOutput()
    self.im.printAll()

    '''
    Manual testing confirmed output of 0x152bfffd
    '''

if __name__ == '__main__':
  unittest.main()
