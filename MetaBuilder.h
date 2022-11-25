#include "AssemblyCFG.h"
#include "Filter.h"
#include "MetaAsmFilter.h"
#include "MetaTrans.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

class PHI;

namespace MetaTrans {

class MetaAsmBuilder : public FilterTarget {

protected:

friend class AsmArgFilter;
friend class AsmConstantFilter;
friend class AsmInstFilter;
friend class AsmBBFilter;
friend class AsmFuncFilter;

AssemblyCFG* cfg;

MetaFunction* mF;

std::unordered_map<string, std::vector<InstType>>* typeMap;

// Auxiliary map, record the reflection between primitive type and Meta type.

std::unordered_map<uint64_t, MetaBB*> bbMap;

std::unordered_map<uint64_t, MetaInst*> instMap;

// std::unordered_map<class PHI*, MetaPhi*> phiMap;

std::unordered_map<uint64_t, MetaConstant*> constantMap;

std::unordered_map<uint64_t, MetaArgument*> argMap;

// Assembly instructions that with global edges.
std::vector<AssemblyInstruction*> crossInsts;

FilterManager filterManager;

MetaAsmBuilder& clearAuxMaps();

MetaAsmBuilder& buildMetaFunction();

MetaAsmBuilder& buildMetaElements();

// build denpendency graph between instructions.
MetaAsmBuilder& buildGraph();

// use responsibility chain pattern to create meta data.
MetaAsmBuilder& buildMetaData();

// create a meta bb correspond to a llvm bb insde a meta function.
MetaAsmBuilder& createMetaBB(AssemblyBasicBlock& b);

// create a meta instruction correspond to a llvm instruction inside a meta
// bb.
MetaAsmBuilder& createMetaInst(AssemblyInstruction& i, MetaBB& b);

// create meta operand for an llvm instruction.
MetaAsmBuilder& createMetaOperand(AssemblyInstruction& i);

MetaOperand* findMetaOperand(Value *v);

AssemblyBasicBlock* findAsmBB(AssemblyInstruction* asmInst);

// copy instruction dependencies from LLVM IR.
void copyLocalEdge(AssemblyInstruction* curInst);

void copyGlobalEdge(AssemblyInstruction* curInst);

MetaAsmBuilder& printAsmInfo();

MetaAsmBuilder& printTIRinfo();

pair<MetaBB*, MetaInst*> visit(AssemblyBasicBlock* root, std::vector<AssemblyInstruction*>& leaves, std::unordered_set<AssemblyBasicBlock*>& visited, bool base = false);

public:

MetaAsmBuilder();

MetaAsmBuilder& setAsmCFG(AssemblyCFG* c);

MetaAsmBuilder& setTypeMap(std::unordered_map<std::string, std::vector<InstType>>* map);

MetaFunction* build();

void paintColor(int startColor);

void paintInsColorRecursive(MetaInst* ins, int color, int depth);

};

} // namespace MetaTrans