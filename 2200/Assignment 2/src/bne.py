'''
Implements CPU element for branch.

Author: Alexander Einshoj and Vebjorn Haugland, University of Tromso
'''
import unittest
from common import *
from cpuElement import CPUElement
from testElement import TestElement

class BranchNotEqual(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)
		'''
		Connect branch to input controllers
		'''

		assert(len(inputSources) == 0), 'Branch should have no inputs'
		assert(len(outputValueNames) == 0), 'Branch has no output'
		assert(len(control) == 2), 'Branch has two control signals'
		assert(len(outputSignalNames) == 1), 'Branch has one output signal'

		self.controlBranch = control[0][1]
		self.zero 	= control[1][1]
		self.output = outputSignalNames[0]

	def writeOutput(self):
		controlBranch = self.controlSignals[self.controlBranch]
		zero = self.controlSignals[self.zero]

		if controlBranch == 0x1:
			if zero == 0x1:
				zero = 0x0
				self.outputControlSignals[self.output] = 0x0 
			else:
				zero = 0x1
				self.outputControlSignals[self.output] = 0x1
		else:
			self.outputControlSignals[self.output] = zero

class TestBranch(unittest.TestCase):
	def setUp(self):
		self.branch = Branch()
		self.testInput = TestElement()
		self.testOutput = TestElement()

		self.testInput.connect(
			[],
			[],
			[],
			['controlBranch', 'zero'])

		self.branch.connect(
			[],
			[],
			[(self.testInput, 'controlBranch'), (self.testInput, 'zero')],
			['outputSignal'])

		self.testOutput.connect(
			[],
			[],
			[(self.branch, 'outputSignal')],
			[])

	def test_correct_behavior(self):
		self.testInput.setOutputControl('controlBranch', 0x1)
		self.testInput.setOutputControl('zero', 0x1)


		self.branch.readControlSignals()
		self.branch.writeOutput()
		self.testOutput.readControlSignals()
		output = self.testOutput.controlSignals['outputSignal']

		self.assertEqual(output, 0x0)

if __name__ == '__main__':
  unittest.main()
