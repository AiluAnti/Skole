'''
Implements CPU element for ALU.

Author: Alexander Einshoj and Vebjorn Haugland, University of Tromso
'''
import unittest
from common import *
from cpuElement import CPUElement
from testElement import TestElement


class Alu(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)
		'''
		Connect alu to input sources and controller
		'''

		assert(len(inputSources) == 2), 'Alu should have two inputs'
		assert(len(outputValueNames) == 1), 'Alu has only one output'
		assert(len(control) == 1), 'Alu has one control signal'
		assert(len(outputSignalNames) == 1), 'Alu has one output signal'

		self.inputZero = inputSources[0][1]
		self.inputOne = inputSources[1][1]
		self.outputName = outputValueNames[0]
		self.controlName = control[0][1]
		self.outputSignal = outputSignalNames[0]

	def writeOutput(self):
		aluControl = self.controlSignals[self.controlName]

		assert(isinstance(aluControl, int))
		assert(not isinstance(aluControl, bool))

		if aluControl == 0x0: # ALU action AND
			self.outputValues[self.outputName] = self.inputValues[self.inputZero] & self.inputValues[self.inputOne]
		elif aluControl == 0x1: # ALU action OR
			self.outputValues[self.outputName] = self.inputValues[self.inputZero] | self.inputValues[self.inputOne]
		elif aluControl == 0x2: # ALU action ADD
			self.outputValues[self.outputName] = (self.inputValues[self.inputZero] + self.inputValues[self.inputOne]) & 0xffffffff
		elif aluControl == 0x6: # ALU action SUB
			self.outputValues[self.outputName] = (self.inputValues[self.inputZero] - self.inputValues[self.inputOne]) & 0xffffffff 
		elif aluControl == 0x7: # ALU action set on less than
			if fromUnsignedWordToSignedWord(self.inputValues[self.inputZero]) < fromUnsignedWordToSignedWord(self.inputValues[self.inputOne]):
				self.outputValues[self.outputName] = long(1)
			else:
				self.outputValues[self.outputName] = long(0)
		if self.outputValues[self.outputName] == long(0):
			self.outputControlSignals[self.outputSignal] = 0x1
	def printOutout(self):
		'''
		Debug function that prints the output value
		'''
		print 'alu.output = %d' % (self.outputValues[self.outputName])

class TestAlu(unittest.TestCase):
	def setUp(self):
		self.alu = Alu()
		self.testInput = TestElement()
		self.testOutput = TestElement()

		self.testInput.connect(
			[],
			['Data', 'Data2'],
			[],
			['control'])

		self.alu.connect(
			[(self.testInput, 'Data'), (self.testInput, 'Data2')],
			['Output'],
			[(self.testInput, 'control')],
			['zero'])

		self.testOutput.connect(
			[(self.alu, 'Output')],
			[],
			[(self.alu, 'zero')],
			[])

	def test_correct_behavior(self):
		self.testInput.setOutputValue('Data', long(5354))
		self.testInput.setOutputValue('Data2', long(661))

		############ TESTING AND OPERATION ####################################
		self.testInput.setOutputControl('control', 0x0)

		self.alu.readInput()
		self.alu.readControlSignals()
		self.alu.writeOutput()
		self.testOutput.readInput()
		output = self.testOutput.inputValues['Output']

		self.assertEqual(output, long(128))
		############ TESTING OR OPERATION ####################################
		self.testInput.setOutputControl('control', 0x1)

		self.alu.readInput()
		self.alu.readControlSignals()
		self.alu.writeOutput()
		self.testOutput.readInput()
		output = self.testOutput.inputValues['Output']

		self.assertEqual(output, long(5887))
		############ TESTING ADD OPERATION ####################################
		self.testInput.setOutputControl('control', 0x2)

		self.alu.readInput()
		self.alu.readControlSignals()
		self.alu.writeOutput()
		self.testOutput.readInput()
		output = self.testOutput.inputValues['Output']

		self.assertEqual(output, long(6015))
		############ TESTING SUB OPERATION ####################################
		self.testInput.setOutputControl('control', 0x6)

		self.alu.readInput()
		self.alu.readControlSignals()
		self.alu.writeOutput()
		self.testOutput.readInput()
		output = self.testOutput.inputValues['Output']

		self.assertEqual(output, long(4693))
		############ TESTING SET ON LESS THAN OPERATION ####################################
		self.testInput.setOutputControl('control', 0x7)

		self.alu.readInput()
		self.alu.readControlSignals()
		self.alu.writeOutput()
		self.testOutput.readInput()
		output = self.testOutput.inputValues['Output']

		self.assertEqual(output, long(0))
if __name__ == '__main__':
  unittest.main()

