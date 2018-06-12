'''
Implements CPU element for signExtend.

Author: Alexander Einshoj and Vebjorn Haugland, University of Tromso
'''
import unittest
from cpuElement import CPUElement
from testElement import TestElement

class SignExtend(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)
		'''
		Connect SignExtend to input sources
		'''

		assert(len(inputSources) == 3), 'SignExtend has one input'
		assert(len(outputValueNames) == 1), 'SignExtend has one output'
		assert(len(control) == 0), 'SignExtend has no control signal'
		assert(len(outputSignalNames) == 0), 'SignExtend has no output signal'

		self.rd = inputSources[0][1]
		self.shamt = inputSources[1][1]
		self.funct = inputSources[2][1]

		self.output = outputValueNames[0]



	def writeOutput(self):
		i = self.inputValues[self.rd]
		print i
		i = i << 5
		i += self.inputValues[self.shamt]
		print i
		i = i << 6
		i += self.inputValues[self.funct]
		print i

		if (i & 0x8000) > 0:
			i = i|0xffff0000	#ffff = 1111|1111|1111|1111
		else:
			i = i&0xffffffff


		self.outputValues[self.output] = i

	def printAll(self):
		for key in self.outputValues:
			print key, ' field = ', self.outputValues[key]

class TestSignExtend(unittest.TestCase):
	def setUp(self):
		self.se = SignExtend()
		self.testInput = TestElement()
		self.testOutput = TestElement()

		self.testInput.connect(
			[],
			['rd', 'shamt', 'funct'],
			[],
			[])

		self.se.connect(
			[(self.testInput, 'rd'), (self.testInput, 'shamt'), (self.testInput, 'funct')],
			['Output'],
			[],
			[])

		self.testOutput.connect(
			[(self.se, 'Output')],
			[],
			[],
			[])

	def test_correct_behavior(self):
		self.testInput.setOutputValue('rd', 0b11111)
		self.testInput.setOutputValue('shamt', 0b11111)
		self.testInput.setOutputValue('funct', 0b111111)

		self.se.readInput()
		self.se.writeOutput()
		self.se.printAll()
		#return
		self.testOutput.readInput()
		output = self.testOutput.inputValues['Output']

		self.assertEqual(output, 0xffffffff)

		

if __name__ == '__main__':
  unittest.main()
