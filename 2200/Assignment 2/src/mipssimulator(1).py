'''
Code written for inf-2200, University of Tromso
'''

from add import Add
from alu import Alu
from aluControl import AluControl
from branch import Branch
from constant import Constant
from control import Control
from dataMemory import DataMemory
from instructionMemory import InstructionMemory
from mux import Mux
from pc import PC
from registerFile import RegisterFile
from shiftLeft2 import ShiftLeft2, ShiftLeft2Jump
from signExtend import SignExtend
from bne import BranchNotEqual
from jump import Jump


class MIPSSimulator():
  '''Main class for MIPS pipeline simulator.
  
  Provides the main method tick(), which runs pipeline
  for one clock cycle.
  
  '''
  def __init__(self, memoryFile):
    self.nCycles = 1 # Used to hold number of clock cycles spent executing instructions
    
    self.adder              = Add()
    self.branchAdder        = Add()
    self.alu                = Alu()
    self.aluControl         = AluControl()
    self.branchNE           = BranchNotEqual()
    self.branch             = Branch()
    self.constant           = Constant(4)
    self.control            = Control()
    self.dataMemory         = DataMemory(memoryFile)
    self.instructionMemory  = InstructionMemory(memoryFile)
    self.regMux             = Mux()
    self.aluMux             = Mux()
    self.dmMux              = Mux()
    self.branchMux          = Mux()
    self.jmpMux             = Mux()
    self.registerFile       = RegisterFile()
    self.shiftLeft2         = ShiftLeft2()
    self.signExtend         = SignExtend()
    self.jump               = Jump()
    self.leftShift2         = ShiftLeft2()
    self.leftShift2Jump     = ShiftLeft2Jump()


    
    self.pc = PC(0xbfc00000) # hard coded "boot" address
    
    self.elements = [ self.constant, self.adder, self.instructionMemory, self.control,
                      self.regMux, self.leftShift2Jump, self.jump , self.registerFile, self.signExtend,
                      self.shiftLeft2, self.branchAdder, self.aluMux, self.aluControl,
                      self.alu, self.branchNE, self.branch, self.branchMux, self.jmpMux, self.dataMemory,
                      self.dmMux ]
    
    self._connectCPUElements()

  def _connectCPUElements(self):
    self.pc.connect(
      [(self.jmpMux, 'newAddress')],
      ['address'],
      [],
      []
      )

    self.constant.connect(
      [],
      ['constant'],
      [],
      []
      )

    self.adder.connect(
      [(self.constant, 'constant'), (self.pc, 'address')],
      ['sum'],
      [],
      []
      )

    self.instructionMemory.connect(
      [(self.pc, 'address')],
      ['op', 'rs', 'rt', 'rd', 'shamt', 'funct'],
      [],
      []
      )

    self.control.connect(
      [(self.instructionMemory, 'op')],
      [],
      [],
      ['regDst', 'aluSrc', 'memtoReg', 'regWrite', 'memRead', 'memWrite', 'branch', 'aluOp1', 'aluOp0', 'bne', 'jump']
      )

    self.regMux.connect(
      [(self.instructionMemory, 'rt'), (self.instructionMemory, 'rd')],
      ['wrSignal'],
      [(self.control, 'regDst')],
      []
      )

    self.leftShift2Jump.connect(
      [(self.instructionMemory, 'rs'), (self.instructionMemory, 'rt'), (self.instructionMemory, 'rd'), (self.instructionMemory, 'shamt'), (self.instructionMemory, 'funct')],
      ['jmpAddress'],
      [],
      []
      )
    
    self.jump.connect(
      [(self.leftShift2Jump, 'jmpAddress'), (self.adder, 'sum')],
      ['newJmp'],
      [],
      []
      )

    self.registerFile.connect(
      [(self.instructionMemory, 'rs'), (self.instructionMemory, 'rt'), (self.regMux, 'wrSignal'), (self.dmMux, 'regWrite')],
      ['rd1', 'rd2'],
      [(self.control, 'regWrite')],
      []
      )

    self.signExtend.connect(
      [(self.instructionMemory, 'rd'), (self.instructionMemory, 'shamt'), (self.instructionMemory, 'funct')],
      ['extendedOutput'],
      [],
      []
      )

    self.shiftLeft2.connect(
      [(self.signExtend, 'extendedOutput')],
      ['toBAdder'],
      [],
      []
      )

    self.branchAdder.connect(
      [(self.adder, 'sum'), (self.shiftLeft2, 'toBAdder')],
      ['bAdderResult'],
      [],
      []
      )

    self.aluMux.connect(
      [(self.registerFile, 'rd1'), (self.signExtend, 'extendedOutput')],
      ['toAlu'],
      [(self.control, 'aluSrc')],
      []
      )

    self.aluControl.connect(
      [(self.instructionMemory, 'funct')],
      [],
      [(self.control, 'aluOp0'), (self.control, 'aluOp1')],
      ['aluControlSignal']
      )

    self.alu.connect(
      [(self.registerFile, 'rd1'), (self.aluMux, 'toAlu')],
      ['aluResult'],
      [(self.aluControl, 'aluControlSignal')],
      ['zero']
      )

    self.branchNE.connect(
      [],
      [],
      [(self.control, 'bne'), (self.alu, 'zero')],
      ['inverted']
      )

    self.branch.connect(
      [],
      [],
      [(self.control, 'branch'), (self.branchNE, 'inverted')],
      ['branched']
      )

    self.branchMux.connect(
      [(self.adder, 'sum'), (self.branchAdder, 'bAdderResult')],
      ['bMuxOutput'],
      [(self.branch, 'branched')],
      []
      )

    self.jmpMux.connect(
      [(self.branchMux, 'bMuxOutput'), (self.jump, 'newJmp')],
      ['newAddress'],
      [(self.control, 'jump')],
      []
      )

    self.dataMemory.connect(
      [(self.alu, 'aluResult'), (self.registerFile, 'rd2')],
      ['rd'],
      [(self.control, 'memWrite'), (self.control, 'memRead')],
      []
      )

    self.dmMux.connect(
      [(self.dataMemory, 'rd'), (self.alu, 'aluResult')],
      ['regWrite'],
      [(self.control, 'memtoReg')],
      []
      )

  def clockCycles(self):
    '''Returns the number of clock cycles spent executing instructions.'''
    
    return self.nCycles
  
  def dataMemory(self):
    '''Returns dictionary, mapping memory addresses to data, holding
    data memory after instructions have finished executing.'''
    
    return self.dataMemory.memory
  
  def registerFile(self):
    '''Returns dictionary, mapping register numbers to data, holding
    register file after instructions have finished executing.'''
    
    return self.registerFile.register
  
  def printDataMemory(self):
    self.dataMemory.printAll()
  
  def printRegisterFile(self):
    self.registerFile.printAll()
  
  def tick(self):
    '''Execute one clock cycle of pipeline.'''
    
    self.nCycles += 1
    
    # The following is just a small sample implementation
    
    self.pc.writeOutput()
    
    for elem in self.elements:
      elem.readControlSignals()
      elem.readInput()
      elem.writeOutput()
      elem.setControlSignals()
    
    self.pc.readInput()
