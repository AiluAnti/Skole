'''
Implements CPU element for control.

Author: Alexander Einshoj and Vebjorn Haugland, University of Tromso
'''
import unittest
from cpuElement import CPUElement
from testElement import TestElement

class Control(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)

		assert(len(inputSources) == 1)
		assert(len(outputValueNames) == 0)
		assert(len(control) == 0)
		assert(len(outputSignalNames) == 11)

		self.inputName = inputSources[0][1]

		self.regDst 	= outputSignalNames[0]
		self.aluSrc 	= outputSignalNames[1]
		self.memtoReg 	= outputSignalNames[2]
		self.regWrite 	= outputSignalNames[3]
		self.memRead 	= outputSignalNames[4]
		self.memWrite 	= outputSignalNames[5]
		self.branch 	= outputSignalNames[6]
		self.aluOp1 	= outputSignalNames[7]
		self.aluOp0		= outputSignalNames[8]
		self.bne		= outputSignalNames[9]
		self.jump 		= outputSignalNames[10]

	def writeOutput(self):

		"""IF OPCODE == 0b000000"""
		if self.inputValues[self.inputName] == 0x0:
			self.outputControlSignals[self.regDst] 		= 0x1
			self.outputControlSignals[self.aluSrc] 		= 0x0
			self.outputControlSignals[self.memtoReg] 	= 0x0
			self.outputControlSignals[self.regWrite] 	= 0x1
			self.outputControlSignals[self.memRead] 	= 0x0
			self.outputControlSignals[self.memWrite] 	= 0x0
			self.outputControlSignals[self.branch] 		= 0x0
			self.outputControlSignals[self.aluOp1] 		= 0x1
			self.outputControlSignals[self.aluOp0]		= 0x0
			self.outputControlSignals[self.bne]			= 0x0
			self.outputControlSignals[self.jump]		= 0x0

			"""IF OPCODE == 0b100011"""
		elif self.inputValues[self.inputName] == 0x23:
			self.outputControlSignals[self.regDst] 		= 0x0
			self.outputControlSignals[self.aluSrc] 		= 0x1
			self.outputControlSignals[self.memtoReg] 	= 0x1
			self.outputControlSignals[self.regWrite] 	= 0x1
			self.outputControlSignals[self.memRead] 	= 0x1
			self.outputControlSignals[self.memWrite] 	= 0x0
			self.outputControlSignals[self.branch] 		= 0x0
			self.outputControlSignals[self.aluOp1] 		= 0x0
			self.outputControlSignals[self.aluOp0]		= 0x0	
			self.outputControlSignals[self.bne]			= 0x0	
			self.outputControlSignals[self.jump]		= 0x0

			"""IF OPCODE == 101011"""
		elif self.inputValues[self.inputName] == 0x2B:
			self.outputControlSignals[self.aluSrc] 		= 0x1
			self.outputControlSignals[self.regWrite] 	= 0x0
			self.outputControlSignals[self.memRead] 	= 0x0
			self.outputControlSignals[self.memWrite] 	= 0x1
			self.outputControlSignals[self.branch]	 	= 0x0
			self.outputControlSignals[self.aluOp1]	 	= 0x0
			self.outputControlSignals[self.aluOp0]		= 0x0
			self.outputControlSignals[self.bne]			= 0x0
			self.outputControlSignals[self.jump]		= 0x0

			"""IF OPCODE == 000100"""
		elif self.inputValues[self.inputName] == 0x4:
			self.outputControlSignals[self.aluSrc] 		= 0x0
			self.outputControlSignals[self.regWrite] 	= 0x0
			self.outputControlSignals[self.memRead] 	= 0x0
			self.outputControlSignals[self.memWrite] 	= 0x0
			self.outputControlSignals[self.branch] 		= 0x1
			self.outputControlSignals[self.aluOp1] 		= 0x0
			self.outputControlSignals[self.aluOp0]		= 0x1
			self.outputControlSignals[self.bne]			= 0x0
			self.outputControlSignals[self.jump]		= 0x0

			"""IF OPCODE == 000101"""
		elif self.inputValues[self.inputName] == 0x5:
			self.outputControlSignals[self.aluSrc] 		= 0x0
			self.outputControlSignals[self.regWrite] 	= 0x0
			self.outputControlSignals[self.memRead] 	= 0x0
			self.outputControlSignals[self.memWrite] 	= 0x0
			self.outputControlSignals[self.branch] 		= 0x1
			self.outputControlSignals[self.aluOp1] 		= 0x0
			self.outputControlSignals[self.aluOp0]		= 0x1
			self.outputControlSignals[self.bne]			= 0x1
			self.outputControlSignals[self.jump]		= 0x0

			"""IF OPCODE == 000010"""
		elif self.inputValues[self.inputName] == 0x2:
			self.outputControlSignals[self.aluSrc] 		= 0x0
			self.outputControlSignals[self.regWrite] 	= 0x0
			self.outputControlSignals[self.memRead] 	= 0x0
			self.outputControlSignals[self.memWrite] 	= 0x0
			self.outputControlSignals[self.branch] 		= 0x0
			self.outputControlSignals[self.aluOp1] 		= 0x0
			self.outputControlSignals[self.aluOp0]		= 0x0
			self.outputControlSignals[self.bne]			= 0x0
			self.outputControlSignals[self.jump]		= 0x1		

class TestControl(unittest.TestCase):
	def setUp(self):
		self.control = Control()
		self.testInput = TestElement()
		self.testOutput = TestElement()

		self.testInput.connect(
			[],
			['opCode'],
			[],
			[]
		)
		self.control.connect(
			[(self.testInput, 'opCode')],
			[],
			[],
			['regDst', 'aluSrc', 'memtoReg', 'regWrite','memRead','memWrite','branch','aluOp1','aluOp0', 'bne', 'jump']
		)
		self.testOutput.connect(
			[],
			[],
			[(self.control, 'regDst'), (self.control, 'aluSrc'), (self.control, 'memtoReg'), (self.control, 'regWrite'), (self.control, 'memRead'), (self.control, 'memWrite'), (self.control, 'branch'), (self.control, 'aluOp1'), (self.control, 'aluOp0'), (self.control, 'bne'), (self.control, 'jump')],
			[]
		)

	def test_correct_behaviour(self):
		"""Testing if opCode == 000000"""
		self.testInput.setOutputValue('opCode', 0x0)

		self.control.readInput()
		self.control.writeOutput()
		self.testOutput.readControlSignals()
		
		regDst 		= self.testOutput.controlSignals['regDst']
		aluSrc 		= self.testOutput.controlSignals['aluSrc']
		memtoReg	= self.testOutput.controlSignals['memtoReg']
		regWrite 	= self.testOutput.controlSignals['regWrite']
		memRead 	= self.testOutput.controlSignals['memRead']
		memWrite 	= self.testOutput.controlSignals['memWrite']
		branch 		= self.testOutput.controlSignals['branch']
		aluOp1 		= self.testOutput.controlSignals['aluOp1']
		aluOp0 		= self.testOutput.controlSignals['aluOp0']
		bne 		= self.testOutput.controlSignals['bne']

		self.assertEqual(regDst, 0x1)
		self.assertEqual(aluSrc, 0x0)
		self.assertEqual(memtoReg, 0x0)
		self.assertEqual(regWrite, 0x1)
		self.assertEqual(memRead, 0x0)
		self.assertEqual(memWrite, 0x0)
		self.assertEqual(branch, 0x0)
		self.assertEqual(aluOp1, 0x1)
		self.assertEqual(aluOp0, 0x0)
		self.assertEqual(bne, 0x0)

		"""Testing if opCode == 100011"""
		self.testInput.setOutputValue('opCode', 0x23)

		self.control.readInput()
		self.control.writeOutput()
		self.testOutput.readControlSignals()
		
		regDst 		= self.testOutput.controlSignals['regDst']
		aluSrc 		= self.testOutput.controlSignals['aluSrc']
		memtoReg	= self.testOutput.controlSignals['memtoReg']
		regWrite 	= self.testOutput.controlSignals['regWrite']
		memRead 	= self.testOutput.controlSignals['memRead']
		memWrite 	= self.testOutput.controlSignals['memWrite']
		branch 		= self.testOutput.controlSignals['branch']
		aluOp1 		= self.testOutput.controlSignals['aluOp1']
		aluOp0 		= self.testOutput.controlSignals['aluOp0']
		bne 		= self.testOutput.controlSignals['bne']

		self.assertEqual(regDst, 0x0)
		self.assertEqual(aluSrc, 0x1)
		self.assertEqual(memtoReg, 0x1)
		self.assertEqual(regWrite, 0x1)
		self.assertEqual(memRead, 0x1)
		self.assertEqual(memWrite, 0x0)
		self.assertEqual(branch, 0x0)
		self.assertEqual(aluOp1, 0x0)
		self.assertEqual(aluOp0, 0x0)
		self.assertEqual(bne, 0x0)

		"""Testing if opCode == 101011"""
		self.testInput.setOutputValue('opCode', 0x2B)

		self.control.readInput()
		self.control.writeOutput()
		self.testOutput.readControlSignals()
		
		regDst 		= self.testOutput.controlSignals['regDst']
		aluSrc 		= self.testOutput.controlSignals['aluSrc']
		memtoReg	= self.testOutput.controlSignals['memtoReg']
		regWrite 	= self.testOutput.controlSignals['regWrite']
		memRead 	= self.testOutput.controlSignals['memRead']
		memWrite 	= self.testOutput.controlSignals['memWrite']
		branch 		= self.testOutput.controlSignals['branch']
		aluOp1 		= self.testOutput.controlSignals['aluOp1']
		aluOp0 		= self.testOutput.controlSignals['aluOp0']
		bne 		= self.testOutput.controlSignals['bne']

		self.assertEqual(regDst, 0x0)
		self.assertEqual(aluSrc, 0x1)
		self.assertEqual(memtoReg, 0x1)
		self.assertEqual(regWrite, 0x0)
		self.assertEqual(memRead, 0x0)
		self.assertEqual(memWrite, 0x1)
		self.assertEqual(branch, 0x0)
		self.assertEqual(aluOp1, 0x0)
		self.assertEqual(aluOp0, 0x0)
		self.assertEqual(bne, 0x0)

		"""Testing if opCode == 000100"""
		self.testInput.setOutputValue('opCode', 0x4)

		self.control.readInput()
		self.control.writeOutput()
		self.testOutput.readControlSignals()

		
		regDst 		= self.testOutput.controlSignals['regDst']
		aluSrc 		= self.testOutput.controlSignals['aluSrc']
		memtoReg	= self.testOutput.controlSignals['memtoReg']
		regWrite 	= self.testOutput.controlSignals['regWrite']
		memRead 	= self.testOutput.controlSignals['memRead']
		memWrite 	= self.testOutput.controlSignals['memWrite']
		branch 		= self.testOutput.controlSignals['branch']
		aluOp1 		= self.testOutput.controlSignals['aluOp1']
		aluOp0 		= self.testOutput.controlSignals['aluOp0']
		bne 		= self.testOutput.controlSignals['bne']

		self.assertEqual(regDst, 0x0)
		self.assertEqual(aluSrc, 0x0)
		self.assertEqual(memtoReg, 0x1)
		self.assertEqual(regWrite, 0x0)
		self.assertEqual(memRead, 0x0)
		self.assertEqual(memWrite, 0x0)
		self.assertEqual(branch, 0x1)
		self.assertEqual(aluOp1, 0x0)
		self.assertEqual(aluOp0, 0x1)
		self.assertEqual(bne, 0x0)

		"""Testing if opCode == 000101"""
		self.testInput.setOutputValue('opCode', 0x5)

		self.control.readInput()
		self.control.writeOutput()
		self.testOutput.readControlSignals()

		
		regDst 		= self.testOutput.controlSignals['regDst']
		aluSrc 		= self.testOutput.controlSignals['aluSrc']
		memtoReg	= self.testOutput.controlSignals['memtoReg']
		regWrite 	= self.testOutput.controlSignals['regWrite']
		memRead 	= self.testOutput.controlSignals['memRead']
		memWrite 	= self.testOutput.controlSignals['memWrite']
		branch 		= self.testOutput.controlSignals['branch']
		aluOp1 		= self.testOutput.controlSignals['aluOp1']
		aluOp0 		= self.testOutput.controlSignals['aluOp0']
		bne 		= self.testOutput.controlSignals['bne']

		self.assertEqual(regDst, 0x0)
		self.assertEqual(aluSrc, 0x0)
		self.assertEqual(memtoReg, 0x1)
		self.assertEqual(regWrite, 0x0)
		self.assertEqual(memRead, 0x0)
		self.assertEqual(memWrite, 0x0)
		self.assertEqual(branch, 0x1)
		self.assertEqual(aluOp1, 0x0)
		self.assertEqual(aluOp0, 0x1)
		self.assertEqual(bne, 0x1)

if __name__ == '__main__':
	unittest.main()
