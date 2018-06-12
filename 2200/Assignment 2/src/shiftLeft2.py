'''
Implements CPU element for ShiftLeft2.

Author: Alexander Einshoj and Vebjorn Haugland, University of Tromso
'''
import unittest
from cpuElement import CPUElement
from testElement import TestElement

class ShiftLeft2(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)
		'''
		Connect ShiftLeft2 to input source
		'''

		assert(len(inputSources) == 1), 'ShiftLeft2 has one input'
		assert(len(outputValueNames) == 1), 'ShiftLeft2 has one output'
		assert(len(control) == 0), 'ShiftLeft2 has no control signal'
		assert(len(outputSignalNames) == 0), 'ShiftLeft2 has no control output'

		self.input = inputSources[0][1]
		self.output = outputValueNames[0]

	def writeOutput(self):
		self.outputValues[self.output] = self.inputValues[self.input] << 2

class ShiftLeft2Jump(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)
		'''
		Connect ShiftLeft2Jump to input source
		'''

		assert(len(inputSources) == 5), 'ShiftLeft2Jump has five inputs'
		assert(len(outputValueNames) == 1), 'ShiftLeft2Jump has one output'
		assert(len(control) == 0), 'ShiftLeft2Jump has no control signal'
		assert(len(outputSignalNames) == 0), 'ShiftLeft2Jump has no control output'

		self.rs = inputSources[0][1]
		self.rt = inputSources[1][1]
		self.rd = inputSources[2][1]
		self.shamt = inputSources[3][1]
		self.funct = inputSources[4][1]
		self.output = outputValueNames[0]

	def writeOutput(self):
		i = self.inputValues[self.rs]
		i = i << 5
		i += self.inputValues[self.rt]
		i = i << 5
		i += self.inputValues[self.rd]
		i = i << 5
		i += self.inputValues[self.shamt]
		i = i << 6
		i += self.inputValues[self.funct]

		self.outputValues[self.output] = i << 2
		
class TestShiftLeft2(unittest.TestCase):
	def setUp(self):
		self.sl2 = ShiftLeft2()
		self.testInput = TestElement()
		self.testOutput = TestElement()

		self.testInput.connect(
			[],
			['Data'],
			[],
			[])

		self.sl2.connect(
			[(self.testInput, 'Data')],
			['Output'],
			[],
			[])

		self.testOutput.connect(
			[(self.sl2, 'Output')],
			[],
			[],
			[])

	def test_correct_behavior(self):
		self.testInput.setOutputValue('Data', 0b111)
		self.sl2.readInput()
		self.sl2.writeOutput()
		self.testOutput.readInput()
		output = self.testOutput.inputValues['Output']

		# result should be 0b11100, which in decimal is 28
		self.assertEqual(output, 0b11100)

if __name__ == '__main__':
  unittest.main()