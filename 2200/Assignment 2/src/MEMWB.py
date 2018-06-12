from cpuElement import CPUElement

class MEMWB(CPUElement):
	def connect(self, inputSources, outputValueNames, control, outputSignalNames):
		CPUElement.connect(self, inputSources, outputValueNames, control, outputSignalNames)
		'''
		assert(len(inputSources) == 7)
		assert(len(outputValueNames) == 7)
		assert(len(control) == 0)
		assert(len(outputSignalNames) == 0)
		'''

	def writeOutput(self):
		for src, name in self.inputSources:
			self.outputValues[name] = self.inputValues[name]

	def setControlSignals(self):
		for src, name in self.control:
			self.outputControlSignals[name] = self.control[name]

