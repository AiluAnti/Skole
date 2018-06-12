'''
Code written for inf-2200, University of Tromso
'''

import unittest
from cpuElement import CPUElement
from testElement import TestElement
import common

class RegisterFile(CPUElement):
  def __init__(self):
    # Dictionary mapping register number to register value
    self.register = {}
    
    # All registers default to 0
    for i in range(0, 32):
      self.register[i] = 0
  
  def connect(self, inputSources, outputValueNames, control, outputRegWriteNames):
    CPUElement.connect(self, inputSources, outputValueNames, control, outputRegWriteNames)
    
    #input names
    self.reg = inputSources[0][1]
    self.reg2 = inputSources[1][1]
    self.writeReg = inputSources[2][1]
    self.writeData = inputSources[3][1]
    self.regWrite = control[0][1]

    self.readData = outputValueNames[0]
    self.readData2 = outputValueNames[1]

  def printAll(self):
    '''
    Print the name and value in each register.
    '''
    
    # Note that we won't actually use all the registers listed here...
    registerNames = ['$zero', '$at', '$v0', '$v1', '$a0', '$a1', '$a2', '$a3',
                      '$t0', '$t1', '$t2', '$t3', '$t4', '$t5', '$t6', '$t7',
                      '$s0', '$s1', '$s2', '$s3', '$s4', '$s5', '$s6', '$s7',
                      '$t8', '$t9', '$k0', '$k1', '$gp', '$sp', '$fp', '$ra']
    
    print
    print "Register file"
    print "================"
    for i in range(0, 32):
      print  "%s \t=> %s (%s)" % (registerNames[i], common.fromUnsignedWordToSignedWord(self.register[i]), hex(long(self.register[i]))[:-1])
    print "================"
    print
    print

  def writeOutput(self):
    address = self.inputValues[self.reg]
    secAddress = self.inputValues[self.reg2]

    assert(address <= long(0b11111)) # cant be more than 5 bit
    assert(secAddress <= long(0b11111)) # cant be more than 5 bit

    if self.controlSignals[self.regWrite] == 1:
      wAddress = self.inputValues[self.writeReg]
      data = self.inputValues[self.writeData]
      self.register[wAddress] = data


    self.outputValues[self.readData] = self.register[address]
    self.outputValues[self.readData2] = self.register[secAddress]

class TestRegisterFile(unittest.TestCase):
  def setUp(self):
    self.regFile = RegisterFile()
    self.testInput = TestElement()
    self.testOutput = TestElement()

    self.testInput.connect(
      [],
      ['Reg1', 'Reg2', 'WriteReg', 'WriteData'],
      [],
      ['RegWrite'],
      )

    self.regFile.connect(
      [(self.testInput, 'Reg1'), (self.testInput, 'Reg2'), (self.testInput, 'WriteReg'), (self.testInput, 'WriteData')],
      ['Output1', 'Output2'],
      [(self.testInput, 'RegWrite')],
      [])

    self.testOutput.connect(
      [(self.regFile, 'Output1'), (self.regFile, 'Output2')],
      [],
      [],
      [])
    
  def test_correct_behavior(self):
    self.testInput.setOutputValue('Reg1', long(5))
    self.testInput.setOutputValue('Reg2', long(6))
    self.testInput.setOutputValue('WriteReg', long(5))
    self.testInput.setOutputValue('WriteData', long(1337))

    self.testInput.setOutputControl('RegWrite', 0)

    self.regFile.readInput()
    self.regFile.readControlSignals()
    self.regFile.writeOutput()
    self.testOutput.readInput()
    output = self.testOutput.inputValues['Output1']
    output2 = self.testOutput.inputValues['Output2']

    self.assertEqual(output, long(0))

    self.testInput.setOutputControl('RegWrite', 1)

    self.regFile.readInput()
    self.regFile.readControlSignals()
    self.regFile.writeOutput()
    self.testOutput.readInput()
    output = self.testOutput.inputValues['Output1']
    output2 = self.testOutput.inputValues['Output2']

    self.regFile.printAll()
    self.assertEqual(output, long(1337))
    print 'Output 2 =', output2



if __name__ == '__main__':
  unittest.main()
