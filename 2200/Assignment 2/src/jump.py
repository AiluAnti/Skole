'''
Implements CPU element for jump.

Author: Alexander Einshoj and Vebjorn Haugland, University of Tromso
'''
import unittest
from common import *
from cpuElement import CPUElement
from testElement import TestElement

class Jump(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)

		assert(len(inputSources) == 2)
		assert(len(outputValueNames) == 1)
		assert(len(control) == 0)
		assert(len(outputSignalNames) == 0)

		self.inputZero 	= inputSources[0][1]
		self.inputOne 	= inputSources[1][1]

		self.outputName = outputValueNames[0]

	def writeOutput(self):
		inputZero = long(self.inputValues[self.inputZero])
		inputOne = long(self.inputValues[self.inputOne])

		inputOne = inputOne & 0xf0000000

		self.outputValues[self.outputName] = inputZero
		self.outputValues[self.outputName] = self.outputValues[self.outputName] + inputOne 

class TestJump(unittest.TestCase):
	def setUp(self):
		self.jump = Jump()
		self.testInput = TestElement()
		self.testOutput = TestElement()

		self.testInput.connect(
			[],
			['DataZero', 'DataOne'],
			[],
			[]
		)
		self.jump.connect(
			[(self.testInput, 'DataZero'), (self.testInput, 'DataOne')],
			['output'],
			[],
			[]
		)
		self.testOutput.connect(
			[(self.jump, 'output')],
			[],
			[],
			[]
		)

	def test_correct_behaviour(self):
		self.testInput.setOutputValue('DataZero', long(0x3f00080)<<2)
		self.testInput.setOutputValue('DataOne', long(0xbfc00004))

		self.jump.readInput()
		self.jump.writeOutput()
		self.testOutput.readInput()
		output = self.testOutput.inputValues['output']

		self.assertEqual(output, 0xbfc00200)

if __name__ == '__main__':
	unittest.main()