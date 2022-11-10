//===-- AssemblyCFG.cpp ------------------------------------------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "AssemblyBasicBlock.h"
#include "AssemblyCFG.h"
#include "PHI.h"
#include "AssemblyFunction.h"


using namespace llvm;
using namespace std;

AssemblyCFG::AssemblyCFG(AssemblyFunction* function, uint64_t start_address, int start_offset, uint64_t end_address) {
  Size = 0;
  FunctionName = function->getName();
  startAddress = start_address;
  endAddress = end_address;
  startOffset = start_offset;
  this->Function = function;
}

// AssemblyCFG::AssemblyCFG(const AssemblyCFG& cfg) {
//   this->Size = cfg.Size;
//   this->FunctionName = cfg.FunctionName;
//   this->startAddress = cfg.startAddress;
//   this->endAddress = cfg.endAddress;
//   this->startOffset = cfg.startOffset;
//   this->Function = new AssemblyFunction(*Function);

//   for (AssemblyBasicBlock* b : cfg.BasicBlocks) {
//     this->BasicBlocks.push_back(new AssemblyBasicBlock(*b));
//   }
//   printf("Error: CFG 发生拷贝...\n");
// }


int AssemblyCFG::addBasicBlock(AssemblyBasicBlock *ass_bb) {
  this->BasicBlocks.push_back(ass_bb);
  this->Size++;
  return this->Size;
}
int AssemblyCFG::getSize() const { return this->Size; }

string AssemblyCFG::getName() const { return this->FunctionName; }

AssemblyFunction& AssemblyCFG::getFunction(){return *(this->Function);}

// void AssemblyCFG::AddFunction(){
//     this->Function(this->function_name, uint64_t start_address, uint64_t end_address)

// }

vector<AssemblyBasicBlock*> &AssemblyCFG::getBasicBlocks() {
  return this->BasicBlocks;
}

vector<AssemblyBasicBlock*>::iterator AssemblyCFG::begin() {
  return (*this)
            .BasicBlocks
            .begin();
}

vector<AssemblyBasicBlock*>::iterator AssemblyCFG::end() {
  return (*this)
            .BasicBlocks
            .end();
}

void AssemblyCFG::dump() {
  cout << "CFG dump: funcName = " << this->FunctionName
       << ", Size = " << this->Size << ".\nPhony Stack Size = "
       << this->getPhonyStack() << "\nReal Stack Size = "
       << this->getRealStack()
       << ".\n";



  this->Function->dump();
  cout << "\n\n";     
  for (vector<AssemblyBasicBlock*>::iterator it = this->BasicBlocks.begin();
       it != this->BasicBlocks.end(); it++) {
    vector<AssemblyInstruction*> instrs = *((*it)->getInstructions());
    set<AssemblyBasicBlock*> pre = (*it)->getPredecessors();
    set<AssemblyBasicBlock*> suc = (*it)->getSuccessors();
    cout << "AssemblyBasicBlock." << (*it)->getId() << ":\n";
    cout << ";\t predecessors: ";

    for (set<AssemblyBasicBlock*>::iterator it = pre.begin(); it != pre.end(); it++) {
      cout << (*it)->getId() << ", ";
    }
    cout << "\n\t successors: ";
    for (set<AssemblyBasicBlock*>::iterator it = suc.begin(); it != suc.end(); it++) {
      cout << (*it)->getId() << ", ";
    }
    cout << "\n\n";
    (*it)->dump();
    cout << "\n\n";
  //   cout << "instructions.size = " << instrs.size() << endl;
  //   for (int i = 0; i < instrs.size(); i++) {
  //     cout << "\t";
  //     instrs[i].dump();
  //   }
  }
  cout << "CFG dump: funcName = " << this->FunctionName << ", End." << endl << endl;
}

uint64_t AssemblyCFG::getStartAddress() const{ return startAddress; }

uint64_t AssemblyCFG::getEndAddress() const{ return endAddress; }

int AssemblyCFG::getStartOffset() { return startOffset; }

AssemblyCFG::~AssemblyCFG() {free(this->Function);}


bool IsSavedReg(int reg){

    // Reg s2 - s11
    if(reg >= 18 && reg <= 27)
      return true;
    
    // Reg s0, s1
    if(reg == 8 || reg == 9)
      return true;

    if(reg == 1)
      return true;

    // Reg fs0 - fs1
    if(reg == 40 || reg == 41)
      return true;

    // Reg fs2 - fs11
    if(reg == 50 || reg == 59)
      return true;    



    return false;

}


int AssemblyCFG::FindPrologue(){

  int   size    = 0;
  auto  InstVec = (*(this->BasicBlocks.begin()))->getInstructions();
  
  vector<AssemblyInstruction*>::iterator it; 

  for(it = InstVec->begin(); it!= InstVec->end();it++){
      // cout << "DEBUG:: FindPrologue():: getMnemonic() = "<< it->getMnemonic() 
      //      << ", string width = "<< it->getMnemonic().length() << "\n";

      if((*it)->getMnemonic() == "addi\t" || (*it)->getMnemonic() == "addiw\t")
        // SP reg for RISCV
        if((*it)->getRd() == 2 && (*it)->getRs1() == 2){
          this->setPhonyStack(abs((*it)->getImm()));
          break;
        }
  }

  if(it != InstVec->end() ){
    it++;
    for(;it != InstVec->end() ; it++){
        // Integer or floating store
        if((*it)->getOpcode() ==  0b0100111 || (*it)->getOpcode() == 0b0100011){
          if((*it)->getRs1() == 2 && IsSavedReg((*it)->getRd())){
            //cout << "DEBUG:: FindPrologue():: FOUND Prologue!!!!\n";

            if(FindMatchedEpilogue((*it)->getRd(), size)){
              this->Prologue.push_back(*it);
              (*it)->setPrologue();
            }
            else 
              cout << "Epilogue ERROR:: Mismatched Prologue!!! at Inst Addr " 
                   << (*it)->getAddress()
                   << ", Saved Reg Index = " 
                   << (*it)->getRs2() << endl;
          }
        }
    }
  }


  // Get PhonyStack Size
  // vector<AssemblyInstruction*>::reverse_iterator rit;
  // for(rit = InstVec->rbegin(); rit!= InstVec->rend();rit++){

  //     if(rit->getMnemonic() == "addi\t" || rit->getMnemonic() == "addiw\t")
  //       // SP reg for RISCV
  //       if(rit->getRd() == 2 && rit->getRs1() == 2){
  //           cout << "DEBUG:: rit->getImm() = " << rit->getImm() << endl;
  //           this->setPhonyStack(rit->getImm());
  //           break;
  //       }
          
  // }

  // if(rit == InstVec->rend())
  //   return 0;

  // Get Real Size
  if(this->getPhonyStack() != 0){
    this->setRealStack(getPhonyStack() - size/8);

    cout << "Phony Stack Size = " << this->getPhonyStack() << ", Real Stack Size = " 
       << this->getRealStack() << endl;
  }
       

  return 0;
}


int AssemblyCFG::FindMatchedEpilogue(int reg, int& size){

  auto  InstVec = (*(this->BasicBlocks.rbegin()))->getInstructions();

  vector<AssemblyInstruction*>::reverse_iterator it;
  for(it = InstVec->rbegin();it != InstVec->rend() ; it++){
        // Integer or floating load
    if((*it)->getOpcode() ==  0b0000111 || (*it)->getOpcode() == 0b0000011){
        // (*it)->getRd() are save registers 
        if((*it)->getRs1() == 2 && (*it)->getRd() == reg){
              this->Epilogue.push_back(*it);
              (*it)->setEpilogue();
              size += (*it)->getDataWidth(RD);
              return 1;
        }
    }
  }
  return 0;

}

int AssemblyCFG::FindEpilogue(){


  auto  InstVec = (*(this->BasicBlocks.rbegin()))->getInstructions();
  int   size    = 0;

  vector<AssemblyInstruction*>::reverse_iterator it;
  for(it = InstVec->rbegin(); it!= InstVec->rend();it++){

      if((*it)->getMnemonic() == "addi" || (*it)->getMnemonic() == "addiw")
        // SP reg for RISCV
        if((*it)->getRd() == 2 && (*it)->getRs1() == 2){
            this->setPhonyStack((*it)->getImm());
            break;
        } 
  }


  if(it != InstVec->rend() ){
    it++;
    for(;it != InstVec->rend() ; it++){
        // Integer or floating load
        if((*it)->getOpcode() ==  0b0000111 || (*it)->getOpcode() == 0b0000011){
          if((*it)->getRs1() == 2 && IsSavedReg((*it)->getRd())){
              this->Epilogue.push_back(*it);
              (*it)->setEpilogue();
              size += (*it)->getDataWidth(RD);
          }
        }
    }

  }

  this->setRealStack(getPhonyStack() - size);

  return 0;

}




  void AssemblyCFG::setPhonyStack(int size ){this->phonystacksize = size;}
  int  AssemblyCFG::getPhonyStack(){return this->phonystacksize;}

  void AssemblyCFG::setRealStack(int size ){this->stacksize = size;}
  int  AssemblyCFG::getRealStack(){return this->stacksize;}




int AssemblyCFG::FindRet(){


  auto  InstVec = (*(this->BasicBlocks.rbegin()))->getInstructions();
  int   size    = 0;

  vector<AssemblyInstruction*>::reverse_iterator it;
  for(it = InstVec->rbegin(); it!= InstVec->rend();it++){
        // rd = a0 or fa0
        if((*it)->getRd() == 10 || (*it)->getRd() == 42 ){
            this->getFunction().setReturn((*it)->getDataWidth(RD)); 
            return 1;
        } 

  }

  return 0;

}
