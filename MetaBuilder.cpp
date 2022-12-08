#include "MetaBuilder.h"
#include "MetaUtils.h"
#include "PHI.h"
#include "macro.h"
#include <algorithm>
#include "MetaUtils.h"
#include "MetaAsmFilter.h"
#include <iostream>

using namespace std;

namespace MetaTrans{

    MetaAsmBuilder::MetaAsmBuilder() {
        filterManager
            .addFilter(new MetaIDFilter())
            .addFilter(new AsmArgFilter())
            .addFilter(new AsmConstantFilter())
            .addFilter(new AsmInstFilter())
            .addFilter(new AsmBBFilter())
            .addFilter(new AsmFuncFilter())
            ;
    }

    MetaAsmBuilder::~MetaAsmBuilder() {
        filterManager.clear();
    }

    MetaAsmBuilder& MetaAsmBuilder::setAsmCFG(AssemblyCFG* c) {
        cfg = c;
        return *this;
    }

    MetaAsmBuilder& MetaAsmBuilder::setTypeMap(std::unordered_map<std::string, std::vector<InstType>>* map) {
        typeMap = map;
        return *this;
    }

    MetaFunction* MetaAsmBuilder::build() {
        (*this)
            .printAsmInfo()
            .clearAuxMaps()
            .buildMetaFunction()
            .buildMetaElements()
            .buildGraph()
            .buildMetaData()
            .printTIRinfo()
            ;
        return mF;
    }


    MetaAsmBuilder& MetaAsmBuilder::clearAuxMaps() {
        bbMap       .clear();
        instMap     .clear();
        constantMap .clear();
        argMap      .clear();
        crossInsts  .clear();
        return *this;
    }

    MetaAsmBuilder& MetaAsmBuilder::buildMetaFunction() {
        mF = new MetaFunction();
        return *this;
    }

    MetaAsmBuilder& MetaAsmBuilder::buildMetaElements() {
        // create all meta basic block and instructions recursively.
        for (auto bb = cfg->begin(); bb != cfg->end(); ++bb) {
            createMetaBB(*(*bb));
        }
        return *this;
    }

    MetaAsmBuilder& MetaAsmBuilder::buildGraph() {
        printf("\n*** Building Graph ... ***\n");
        for_each(cfg->begin(), cfg->end(), [&] (AssemblyBasicBlock* bb) {
            copyBasicBlockDependency(bb);
            for (auto iter = bb->begin(); iter != bb->end(); ++iter) {
                if ((*iter)->ifPrologueEpilogue()) continue;
                copyLocalEdge(*iter);

            }
        });
        for (auto i : crossInsts) copyGlobalEdge(i);
        return *this;
    }

    MetaAsmBuilder& MetaAsmBuilder::buildMetaData() {
        filterManager.filter(*this);
        return *this;
    }

    MetaAsmBuilder& MetaAsmBuilder::createMetaBB(AssemblyBasicBlock& b) {
        MetaBB* newBB = mF->buildBB();
        assert(bbMap.insert({b.hashCode(), newBB}).second);
        for (auto i = b.begin(); i != b.end(); ++i) {
            AssemblyInstruction& inst = **i;
            if (inst.ifPrologueEpilogue()) continue;
            (*this)
                .createMetaInst(inst, *newBB)
                // .createMetaOperand(i)
                ;
        }
        return *this;
    }


    MetaAsmBuilder& MetaAsmBuilder::createMetaInst(AssemblyInstruction& i, MetaBB& b) {
        MetaInst* newInst = b.buildInstruction();
        (*newInst)
            // .setInstType((*typeMap)[i.getOpcode()])
            .setParent(&b);
        assert(instMap.insert({i.hashCode(), newInst}).second);
        if (i.hasGlobalEdgeToDealWith()) crossInsts.push_back(&i);
        return *this;
    }

    MetaAsmBuilder& MetaAsmBuilder::createMetaOperand(AssemblyInstruction& i) {

        return *this;
    }

    MetaOperand* MetaAsmBuilder::findMetaOperand(Value* value) {

        return nullptr;
    }

    AssemblyBasicBlock* MetaAsmBuilder::findAsmBB(AssemblyInstruction* asmInst) {
        return (cfg->getBasicBlocks())[asmInst->getBB()];
    }

    void MetaAsmBuilder::copyBasicBlockDependency(AssemblyBasicBlock* asmBB) {
        MetaBB* curBB = bbMap[asmBB->hashCode()];
        for (auto it = (*asmBB).suc_begin(); it != (*asmBB).suc_end(); ++it) {
            MetaBB* nextBB = bbMap[(*it)->hashCode()];
            curBB->addNextBB(nextBB);
        }

    }

    void MetaAsmBuilder::copyLocalEdge(AssemblyInstruction* asmInst) {
        // 创建两个节点之间的边 u --> v
        MetaInst* u = instMap[asmInst->hashCode()];
        for (int i = 1; i < MAX_OPERAND; ++i) {
            // 这里仅处理local edge
            if (!asmInst->HasLocalEdge(i)) { continue; }
            AssemblyInstruction* source = asmInst->getLocalEdge(i);
            MetaInst* v = instMap[source->hashCode()];
            u->addOperand((MetaOperand*)v);
        }
    }

    void MetaAsmBuilder::copyGlobalEdge(AssemblyInstruction* asmInst) {
        MetaInst* u = instMap[asmInst->hashCode()];
        for (int i = 1; i < MAX_OPERAND; ++i) {
            // 这里处理跨越边
            if (asmInst->HasLocalEdge(i) || asmInst->HasGlobalEdge(i) < 2) continue;
            assert(asmInst->HasGlobalEdge(i) >= 2);

            // print some info
            std::cout << "### Algorithm start at: 0x" << asmInst->hashCode() << "\t###" << std::endl;
            printf("### Algorithm destination\t###\n");
            std::cout << "[ ";
            for (auto d : asmInst->getGlobalEdge(i)) 
                printf("0x%08x ( BB: %d ), ", d->hashCode(), findAsmBB(d)->hashCode());
            std::cout << " ]" << std::endl;

            std::unordered_set<AssemblyBasicBlock*> visited;
            auto pair = visit(findAsmBB(asmInst), asmInst->getGlobalEdge(i), visited, true);
            u->addOperand(pair.second);
        }

    }

    MetaAsmBuilder& MetaAsmBuilder::printAsmInfo() {
        std::cout << "\n>>== Checking Asm info of: " << cfg->getName() << " ==<<" << "\n";
        for_each(cfg->begin(), cfg->end(), [&] (AssemblyBasicBlock* bb) {
            for_each(bb->begin(), bb->end(), [&] (AssemblyInstruction* inst) { 
                std::cout << "instruction hash: " << inst->hashCode() << " " << "; Asm code address: " << hex << inst->getAddress() << "\n";
                // inst->printEdges();
            });
            for_each(bb->phi_begin(), bb->phi_end(), [&] (class PHI& phi) {
                std::cout << "phi node address: " << &phi << "; phi node content: " << "\n";
                for_each(phi.begin(), phi.end(), [&] (auto pair) {
                    std::cout << "< " << pair.first << ", " << pair.second << " >,  ";
                });
                std::cout << "\n";
                for_each(phi.begin(), phi.end(), [&] (auto pair) {
                    std::cout << "< " << hex << pair.first->getAddress() << ", " << hex << pair.second->getAddress() << " >,  ";
                });
                std::cout << "\n";
            });
        });
        return *this;
    }

    MetaAsmBuilder& MetaAsmBuilder::printTIRinfo() {

        std::cout << "\n\n<<== Dumping TIR for CFG: " << cfg->getName() << " ==>>" << "\n";
        MetaUtil::printMap(instMap, "++ printing inst map ++");
        MetaUtil::printMap(bbMap, "++ printing bb map ++");
        // MetaUtil::printVector(crossInsts, "++ printing cross insts ++");

        std::cout << "++ printing cross insts ++" << std::endl;
        std::cout << "[ ";
        for (auto v : crossInsts) 
            std::cout << v->hashCode() << ", ";
        std::cout << " ]" << std::endl;

        std::cout << "\n\n>>== Printing TIR info of function at: " << mF << " ==<<" << "\n";
        std::cout << mF->toString() << std::endl;
        for_each(mF->bb_begin(), mF->bb_end(), [&] (MetaBB* bb) {
            std::cout << "-- Memory address of Meta BB: " << bb << " --" << "\n";
            for_each(bb->inst_begin(), bb->inst_end(), [&] (MetaInst* inst) { 
                if (!inst->isMetaPhi()) {
                    std::cout << "Meta instruction address: " << inst << " " << "\n";
                    std::vector<MetaOperand*> ops = inst->getOperandList();
                    std::vector<InstType> types = inst->getInstType();
                    cout << "Types: (" << types.size() << "): ";
                    for(int i = 0; i < types.size(); i++) {
                        cout << InstTypeName[types[i]] << ", ";
                    }
                    cout << endl << "Operands (" << ops.size() << "): ";
                    for(int i = 0; i < ops.size(); i++) {
                        if(ops[i]->isMetaInst()) {cout << "isMetaInst, ";}
                        else if(ops[i]->isMetaArgument()) {cout << "isMetaArgument, ";}
                        else if(ops[i]->isMetaConstant()) {cout << "isMetaConstant, ";}
                    }
                    cout << endl;
                } else {
                    MetaPhi* phi = (MetaPhi*)inst;
                    std::cout << "Meta phi address: " << phi << "; Meta phi content: " << "\n";
                    for_each(phi->begin(), phi->end(), [&] (auto pair) {
                        std::cout << "< " << pair.first << ", " << pair.second << " >,  ";
                    });
                    std::cout << "\n";
                }
            });
        });

        return *this;
    }


    pair<MetaBB*, MetaInst*> MetaAsmBuilder::visit(AssemblyBasicBlock* root, vector<AssemblyInstruction*>& leaves, std::unordered_set<AssemblyBasicBlock*>& visited, bool base) {
        vector<pair<MetaBB*, MetaInst*>>    tmp                                     ;
        MetaBB*                             curMetaBB   = bbMap[root->hashCode()]   ;
        pair<MetaBB*, MetaInst*>            result      = { curMetaBB, nullptr }    ;

        // recursive base.
        for (auto leaf : leaves) if (root == findAsmBB(leaf) && !base) { result.second = instMap[leaf->hashCode()]; return result; }

        // a non-self loop.
        if (visited.find(root) != visited.end()) {
            return result;
        }

        visited.insert(root);

        for (auto it = root->pre_begin(); it != root->pre_end(); ++it) {
            AssemblyBasicBlock* bb = const_cast<AssemblyBasicBlock*>(*it);
            auto pair = visit(bb, leaves, visited);
            if (pair.second) tmp.push_back(pair);
        }

        if (tmp.size() == 0) { return result; }
        if (tmp.size() == 1) { result.second = tmp[0].second; return result; }
        
        // multiple edges, then create a new phi node.
        MetaPhi* newPhi = new MetaPhi();

        std::cout << "Creating new phi node, address: " << newPhi << std::endl;

        (*newPhi)
            .setInstType(InstType::PHI)
            .setParent(curMetaBB)
            ;

        for (auto pair : tmp) {
            (*newPhi)
                .addValue(pair.first, pair.second)
                .addOperand(pair.second)
                ;
        }

        // make sure no duplicate phi node.
        for (auto i = curMetaBB->inst_begin(); (*i)->isMetaPhi(); ++i) {
            if (newPhi->equals((MetaPhi*)*i)) {
                printf("Find same phi node, skip insert.\n");
                result.second = *i;
                delete newPhi; return result;
            }
        }

        std::cout << "Added new phi node, address: " << newPhi << std::endl;
        result.second = newPhi;
        curMetaBB->addPhi(newPhi, true);

        visited.erase(root);
        return result;
        
    }

}