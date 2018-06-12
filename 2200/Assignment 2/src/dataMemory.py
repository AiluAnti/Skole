'''
Implements CPU element for Data Memory in MEM stage.

Code written for inf-2200, University of Tromso
'''

import unittest
from testElement import TestElement
from cpuElement import CPUElement
from memory import Memory
import common

class DataMemory(Memory):
  def __init__(self, filename):
    Memory.__init__(self, filename)
    
  def connect(self, inputSources, outputValueNames, control, outputSignalNames):
    CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)
    '''
    Connect dataMemory to input sources and controllers
    '''
    
    assert(len(inputSources) == 2), 'dataMemory should have two inputs'
    assert(len(outputValueNames) == 1), 'dataMemory should have one output'
    assert(len(control) == 2), 'dataMemory should have two control signals'
    assert(len(outputSignalNames) == 0), 'dataMemory should not have an output signal'

    self.address = inputSources[0][1]
    self.wd = inputSources[1][1]
    self.memW = control[0][1]
    self.memR = control[1][1]
    self.output = outputValueNames[0]
  
  def writeOutput(self):
    memW = self.controlSignals[self.memW]
    memR = self.controlSignals[self.memR]
    address = self.inputValues[self.address]
    if memR == 0x1:
    	if memW == 0x0: #LW
    		self.outputValues[self.output] = self.memory[address]
    elif memR == 0x0:
    	if memW == 0x1: # SW
    		data = self.inputValues[self.wd]
    		self.memory[address] = data

class TestDataMemory(unittest.TestCase):
	def setUp(self):
		self.dm = DataMemory('add.mem')
		self.testInput = TestElement()
		self.testOutput = TestElement()

		self.testInput.connect(
			[],
			['Address', 'WriteData'],
			[],
			['memW', 'memR'])

		self.dm.connect(
			[(self.testInput, 'Address'), (self.testInput, 'WriteData')],
			['Output'],
			[(self.testInput, 'memW'), (self.testInput, 'memR')],
			[])

		self.testOutput.connect(
			[(self.dm, 'Output')],
			[],
			[],
			[])

	def test_correct_behavior(self):
		self.testInput.setOutputValue('Address', 0xbfc0020c)
		self.testInput.setOutputValue('WriteData', long(1337))

		############ TESTING LW OPERATION ####################################
		self.testInput.setOutputControl('memW', 0x0)
		self.testInput.setOutputControl('memR', 0x1)

		self.dm.readInput()
		self.dm.readControlSignals()
		self.dm.writeOutput()
		self.testOutput.readInput()
		output = self.testOutput.inputValues['Output']
		self.assertEqual(output, 0x8d6b0008)

		############ TESTING SW OPERATION ####################################
		self.testInput.setOutputControl('memW', 0x1)
		self.testInput.setOutputControl('memR', 0x0)

		self.dm.readInput()
		self.dm.readControlSignals()
		self.dm.writeOutput()
		self.testOutput.readInput()
		address = self.dm.memory[0xbfc0020c]

		self.assertEqual(address, long(1337))
		self.dm.printAll()
if __name__ == '__main__':
	unittest.main()


