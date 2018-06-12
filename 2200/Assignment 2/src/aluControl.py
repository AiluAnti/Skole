'''
Implements CPU element for ALU.

Author: Alexander Einshoj and Vebjorn Haugland, University of Tromso
'''
import unittest
from common import *
from cpuElement import CPUElement
from testElement import TestElement

class AluControl(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)
		'''
		Connect aluControl to input sources and controller
		'''

		assert(len(inputSources) == 1), 'AluControl has only one input'
		assert(len(outputValueNames) == 0), 'AluControl has no output'
		assert(len(control) == 2), 'AluControl has one control signal'
		assert(len(outputSignalNames) == 1), 'AluControl has one output signal'

		self.input = inputSources[0][1]
		self.aluOp1 = control[0][1]
		self.aluOp0 = control[1][1]
		self.outputSignal = outputSignalNames[0]

	def writeOutput(self):
		aluOp = self.controlSignals[self.aluOp1]
		aluOp = aluOp << 1
		aluOp = aluOp + self.controlSignals[self.aluOp0]

		assert(isinstance(aluOp, int))
		assert(not isinstance(aluOp, bool))

		if aluOp == 0x0: # LW or SW, send ADD to ALU
			self.outputControlSignals[self.outputSignal] = 0x2
		elif aluOp == 0x1: # BEQ, send SUB to ALU
			self.outputControlSignals[self.outputSignal] = 0x6
		elif aluOp == 0x2: # R TYPE
			if self.inputValues[self.input] == 0x20: # ADD
				self.outputControlSignals[self.outputSignal] = 0x2
			elif self.inputValues[self.input] == 0x22: # SUB
				self.outputControlSignals[self.outputSignal] = 0x6
			elif self.inputValues[self.input] == 0x24: # AND
				self.outputControlSignals[self.outputSignal] = 0x0
			elif self.inputValues[self.input] == 0x25: # OR
				self.outputControlSignals[self.outputSignal] = 0x1
			elif self.inputValues[self.input] == 0x2A: # SET ON LESS THAN
				self.outputControlSignals[self.outputSignal] = 0x7

class TestAluControl(unittest.TestCase):
	def setUp(self):
		self.aluControl = AluControl()
		self.testInput = TestElement()
		self.testOutput = TestElement()

		self.testInput.connect(
			[],
			['Data'],
			[],
			['aluOPZero', 'aluOPOne'])

		self.aluControl.connect(
			[(self.testInput, 'Data')],
			[],
			[(self.testInput, 'aluOPZero'), (self.testInput, 'aluOPOne')],
			['signal'])

		self.testOutput.connect(
			[],
			[],
			[(self.aluControl, 'signal')],
			[])

	def test_correct_behavior(self):
		self.testInput.setOutputValue('Data', 0x22)

		############ TESTING LW and SW OPERATION ####################################
		self.testInput.setOutputControl('aluOPZero', 0x0)
		self.testInput.setOutputControl('aluOPOne', 0x0)

		self.aluControl.readInput()
		self.aluControl.readControlSignals()
		self.aluControl.writeOutput()
		self.testOutput.readControlSignals()
		output = self.testOutput.controlSignals['signal']

		self.assertEqual(output, 0x2)

		############ TESTING BEQ OPERATION ####################################
		self.testInput.setOutputControl('aluOPZero', 0x0)
		self.testInput.setOutputControl('aluOPOne', 0x1)

		self.aluControl.readInput()
		self.aluControl.readControlSignals()
		self.aluControl.writeOutput()
		self.testOutput.readControlSignals()
		output = self.testOutput.controlSignals['signal']

		self.assertEqual(output, 0x6)
		############ TESTING SET ON LESS THAN OPERATION ####################################
		self.testInput.setOutputValue('Data', 0x2A)
		self.testInput.setOutputControl('aluOPZero', 0x0)
		self.testInput.setOutputControl('aluOPOne', 0x2)

		self.aluControl.readInput()
		self.aluControl.readControlSignals()
		self.aluControl.writeOutput()
		self.testOutput.readControlSignals()
		output = self.testOutput.controlSignals['signal']

		self.assertEqual(output, 0x7)

if __name__ == '__main__':
  unittest.main()


