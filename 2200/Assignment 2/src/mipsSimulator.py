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
from IFID import IFID
from IDEX import IDEX
from EXMEM import EXMEM
from MEMWB import MEMWB

class MIPSSimulator():
  '''Main class for MIPS pipeline simulator.
  
  Provides the main method tick(), which runs pipeline
  for one clock cycle.
  
  '''
  def __init__(self, memoryFile):
    self.nCycles = 0 # Used to hold number of clock cycles spent executing instructions
    
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

    self.IFID = IFID()
    self.IDEX = IDEX()
    self.EXMEM = EXMEM()
    self.MEMWB = MEMWB()

    self.pc = PC(0xbfc00200) # hard coded "boot" address
    
    self.IFIDelements = [ self.constant, self.branchMux, self.jmpMux, self.instructionMemory, self.adder ]

    self.IDEXelements = [ self.control, self.registerFile, self.signExtend, self.leftShift2Jump, self.jump ]

    self.EXMEMelements = [ self.regMux, self.aluControl, self.aluMux, self.alu, self.shiftLeft2, self.branchAdder, self.branchNE, self.branch ]
    
    self.MEMWBelements = [ self.dataMemory ]

    self.WBelements = [ self.dmMux ] 

    self.elements = [ self.IFIDelements, self.IDEXelements, self.EXMEMelements, self.WBelements]

    self.pipes = [ self.IFID, self.IDEX, self.EXMEM, self.MEMWB]
    
    self._connectCPUElements()
    
  def _connectCPUElements(self):
    ###################### IFID elements ###############################
    self.pc.connect(
      [(self.jmpMux, 'newAddr')],
      ['address'],
      [],
      []
      )

    self.constant.connect(
      [],
      ['Constant'],
      [],
      []
      )

    self.branchMux.connect(
      [(self.adder, 'toBranchSignal'), (self.EXMEM, 'branchMuxIn')],
      ['branchMuxOutput'],
      [(self.EXMEM, 'branchMuxControl')],
      []
      )

    self.jmpMux.connect(
      [(self.branchMux, 'branchMuxOutput'), (self.IDEX, 'newAddr')],
      ['newAddr'],
      [(self.IDEX, 'jump')],
      []
      )

    self.instructionMemory.connect(
      [(self.pc, 'address')],
      ['op', 'rs', 'rt', 'rd', 'shamt', 'funct'],
      [],
      []
      )

    self.adder.connect(
      [(self.pc, 'address'), (self.constant, 'Constant')],
      ['sum'],
      [],
      []
      )

    ###################### IDEX elements ###############################
    self.control.connect(
      [(self.IFID, 'op')],
      [],
      [],
      ['regDst', 'aluSrc', 'memtoReg', 'regWrite', 'memRead', 'memWrite', 'branch', 'aluOp1', 'aluOp0', 'bne', 'jump']
      )

    self.registerFile.connect(
      [(self.IFID, 'rs'), (self.IFID, 'rt'), (self.MEMWB, 'writeReg'), (self.dmMux, 'writeData')],
      ['rd1', 'rd2'],
      [(self.MEMWB, 'regWrite')],
      []
      )
    
    self.signExtend.connect(
      [(self.IFID, 'rd'), (self.IFID, 'shamt'), (self.IFID, 'funct')],
      ['aluMuxIn'],
      [],
      []
      )
    
    self.leftShift2Jump.connect(
      [(self.IFID, 'rs'), (self.IFID, 'rt'), (self.IFID, 'rd'), (self.IFID, 'shamt'), (self.IFID, 'funct')],
      ['jumpaddr'],
      [],
      []
      )
    
    self.jump.connect(
      [(self.leftShift2Jump, 'jumpaddr'), (self.IFID, 'sum')],
      ['newAddr'],
      [],
      []
      )
    ###################### EXMEM elements ###############################
    self.regMux.connect(
      [(self.IDEX, 'rt'), (self.IDEX, 'rd')],
      ['regMuxOutput'],
      [(self.IDEX, 'regDst')],
      []
      )
    self.aluControl.connect(
      [(self.IDEX, 'funct')],
      [],
      [(self.IDEX, 'aluOp1'), (self.IDEX, 'aluOp0')],
      ['aluOutSignal']
      )
    self.aluMux.connect(
      [(self.IDEX, 'rd2'), (self.IDEX, 'aluMuxIn')],
      ['aluSecIn'],
      [(self.IDEX, 'aluSrc')],
      []
      )
    self.alu.connect(
      [(self.IDEX, 'rd1'), (self.aluMux, 'aluSecIn')],
      ['aluOutput'],
      [(self.aluControl, 'aluOutSignal')],
      ['zero']
      )

    self.shiftLeft2.connect(
      [(self.IDEX, 'aluMuxIn')],
      ['toAdder'],
      [],
      []
      )
    self.branchAdder.connect(
      [(self.IDEX, 'sum'), (self.shiftLeft2, 'toAdder')],
      ['branchMuxIn'],
      [],
      []
      )
    self.branchNE.connect(
      [],
      [],
      [(self.IDEX, 'bne'), (self.alu, 'zero')],
      ['toBranchSignal']
      )
    self.branch.connect(
      [],
      [],
      [(self.IDEX, 'branch'), (self.alu, 'zero')],
      ['branchMuxControl']
      )
    ###################### MEMWB elements ###############################

    self.dataMemory.connect(
      [(self.EXMEM, 'aluOutput'), (self.EXMEM, 'rd2')],
      ['toFinalMux'],
      [(self.EXMEM, 'memRead'), (self.EXMEM, 'memWrite')],
      []
      )

    ###################### WB elements ###############################
    self.dmMux.connect(
      [(self.MEMWB, 'aluOutput'), (self.MEMWB, 'toFinalMux')],
      ['writeData'],
      [(self.MEMWB, 'memtoReg')],
      []
      )

    ###################### PIPE elements ###############################
    self.IFID.connect(
      [(self.adder, 'sum'), (self.instructionMemory, 'op'),
       (self.instructionMemory, 'rs'), (self.instructionMemory, 'rt'),
       (self.instructionMemory, 'rd'), (self.instructionMemory, 'shamt'),
       (self.instructionMemory, 'funct')],
      ['sum', 'op', 'rs', 'rt', 'rd', 'shamt', 'funct'],
      [],
      []
      )
    
    self.IDEX.connect(
      [(self.IFID, 'sum'), (self.IFID, 'op'), (self.IFID, 'rt'), 
      (self.IFID, 'rd'), (self.IFID, 'shamt'), (self.IFID, 'funct'),
       (self.registerFile, 'rd1'), (self.registerFile, 'rd2'), (self.signExtend, 'aluMuxIn'),
       (self.jump, 'newAddr') ],
      ['sum', 'op', 'rt', 'rd', 'shamt', 'funct', 'rd1', 'rd2', 'aluMuxIn', 'newAddr' ],
      [(self.control, 'regDst'), (self.control, 'aluSrc'), (self.control, 'memtoReg'), (self.control, 'regWrite'),
       (self.control, 'memRead'), (self.control, 'memWrite'), (self.control, 'branch'),
       (self.control, 'aluOp1'), (self.control, 'aluOp0'), (self.control, 'bne'), (self.control, 'jump')],
      ['regDst', 'aluSrc', 'memtoReg', 'regWrite', 'memRead', 'memWrite', 'branch', 'aluOp1', 'aluOp0', 'bne', 'jump']
      )
    
    self.EXMEM.connect(
      [(self.IDEX, 'rd2'), (self.alu, 'aluOutput'), (self.regMux, 'writeReg'), (self.branchAdder, 'branchMuxIn')],
      ['rd2', 'aluOutput', 'writeReg', 'branchMuxIn'],
      [(self.IDEX, 'memtoReg'), (self.IDEX, 'memtoRead'), (self.IDEX, 'memWrite'), (self.branch ,'branchMuxControl'), (self.IDEX, 'regWrite')],
      ['memtoReg', 'memtoRead', 'memWrite', 'branchMuxControl', 'regWrite']
      )
    
    self.MEMWB.connect(
      [(self.EXMEM, 'writeReg'), (self.EXMEM, 'aluOutput'), (self.EXMEM, 'toFinalMux')],
      ['writeReg', 'aluOutput', 'toFinalMux'],
      [(self.EXMEM, 'memtoReg'), (self.EXMEM, 'regWrite')],
      ['memtoReg', 'regWrite']
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
      for e in elem:
        e.readControlSignals()
        e.readInput()
        e.writeOutput()
        e.setControlSignals()
    
    self.pc.readInput()
if __name__ == '__main__':
  mips = MIPSSimulator('add_modified.mem')
  mips.tick()