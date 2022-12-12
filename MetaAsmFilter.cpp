#include "MetaAsmFilter.h"
#include "MetaBuilder.h"
#include "MetaUtils.h"

using namespace std;

namespace MetaTrans {


    void AsmArgFilter::doFilter(FilterTarget& target, FilterChain& chain) {

        chain.doFilter(target);

    }

    void AsmConstantFilter::doFilter(FilterTarget& target, FilterChain& chain) {

        chain.doFilter(target);

    }

    void AsmInstFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaAsmBuilder&                             builder     =   dynamic_cast<MetaAsmBuilder&>(target);
        AssemblyCFG&                                cfg         =   *(builder.cfg);
        unordered_map<string, vector<InstType>>&    typeMap     =   *(builder.typeMap);

        // for (auto iter = typeMap.begin(); iter != typeMap.end(); ++iter) {
        //     std::cout << iter->first << " -->> ";
        //     for (auto type : iter->second) std::cout << type << ", ";
        //     std::cout << "\n"; 
        // }
        
        printf("\n");
        for (auto bb_iter = cfg.begin(); bb_iter != cfg.end(); ++bb_iter) {
            for (auto inst_iter = (**bb_iter).begin(); inst_iter != (**bb_iter).end(); ++inst_iter) {
                if ((*inst_iter)->ifPrologueEpilogue()) continue;
                cout << "instruction mnemonic: " << (*inst_iter)->getMnemonic() << "\n";
                string mnemonic = (*inst_iter)->getMnemonic();

                // 去除空白字符、转化为大写
                mnemonic.erase(remove_if(mnemonic.begin(), mnemonic.end(), ::isspace), mnemonic.end());
                transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);

                if (typeMap.find(mnemonic) == typeMap.end()) {
                    cout << "Error: op list of instruction " << mnemonic << " not found!\n";
                }

                MetaInst* metaInst = builder.instMap[(**inst_iter).hashCode()];
                vector<InstType> opList = typeMap[mnemonic];
                (*metaInst)
                    .setOriginInst(mnemonic)
                    .setInstType(opList)
                    ;

            }
        }

        chain.doFilter(target);

    }

    void AsmBBFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaAsmBuilder&                             builder     =   dynamic_cast<MetaAsmBuilder&>(target);
        MetaFunction&                               metaFunc    =   *(builder.mF);
        for (auto bb_iter = metaFunc.begin(); bb_iter != metaFunc.end(); ++bb_iter) {
            vector<MetaInst*>& instList = (**bb_iter).getInstList();
            auto inst_iter = instList.begin();
            while (inst_iter != instList.end() && (**inst_iter).isMetaPhi()) ++inst_iter;
            if (inst_iter == instList.end()) continue;
            (**bb_iter)
                    .setEntry(*inst_iter)
                    .setTerminator(instList[instList.size() - 1])
                    ;
        }

        chain.doFilter(target);

    }

    void AsmFuncFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaAsmBuilder&                             builder     =   dynamic_cast<MetaAsmBuilder&>(target);
        AssemblyCFG&                                cfg         =   *(builder.cfg);
        MetaFunction&                               metaFunc    =   *(builder.mF);
        metaFunc
            .setRoot(builder.bbMap[cfg.getBasicBlocks()[0]->hashCode()])
            .setFunctionName(cfg.getName())
            .setReturnType(DataType::INT)   // 暂时先设置为int
            ;

        chain.doFilter(target);

    }

    void MetaIDFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaAsmBuilder&                             builder     =   dynamic_cast<MetaAsmBuilder&>(target);
        MetaFunction&                               metaFunc    =   *(builder.mF);

        // set id fro each meta basic block and meta operand.
        int operand_id = 0, bb_id = 0;

        for (auto const_iter = metaFunc.const_begin(); const_iter != metaFunc.const_end(); ++const_iter) {
            (**const_iter).setID(operand_id++);
        }

        for (auto arg_iter = metaFunc.arg_begin(); arg_iter != metaFunc.arg_end(); ++arg_iter) {
            (**arg_iter).setID(operand_id++);
        }

        for (auto bb_iter = metaFunc.begin(); bb_iter != metaFunc.end(); ++bb_iter) {
            MetaBB& bb = **bb_iter;
            for (auto inst_iter = bb.inst_begin(); inst_iter != bb.inst_end(); ++inst_iter) {
                MetaInst& inst = **inst_iter;
                inst.setID(operand_id++);
            }
            bb.setID(bb_id++) ;
        }

        chain.doFilter(target);

    }

    void MetaFeatureFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaAsmBuilder&                             builder     =   dynamic_cast<MetaAsmBuilder&>(target);
        MetaFunction&                               metaFunc    =   *(builder.mF);

        for (auto bb_iter = metaFunc.begin(); bb_iter != metaFunc.end(); ++bb_iter) {
            MetaBB& bb = **bb_iter;
            int load_count = 0, store_count = 0;
            for (auto inst_iter = bb.inst_begin(); inst_iter != bb.inst_end(); ++inst_iter) {
                MetaInst& inst = **inst_iter;
                if (inst.isLoad()) load_count++;
                if (inst.isStore()) store_count++;
            }
            
            bb
                .addFeature(load_count)
                .addFeature(store_count)
                ;
        }

        chain.doFilter(target);

    }


}