#include "MetaAsmFilter.h"
#include "MetaBuilder.h"
#include "MetaUtils.h"

namespace MetaTrans {


    void AsmArgFilter::doFilter(FilterTarget& target, FilterChain& chain) {

    }

    void AsmConstantFilter::doFilter(FilterTarget& target, FilterChain& chain) {

    }

    void AsmInstFilter::doFilter(FilterTarget& target, FilterChain& chain) {
        MetaAsmBuilder& builder = dynamic_cast<MetaAsmBuilder&>(target);

        AssemblyCFG& cfg = *(builder.cfg);
        std::unordered_map<std::string, std::vector<InstType>>& typeMap = *(builder.typeMap);

        // for (auto iter = typeMap.begin(); iter != typeMap.end(); ++iter) {
        //     std::cout << iter->first << " -->> ";
        //     for (auto type : iter->second) std::cout << type << ", ";
        //     std::cout << "\n"; 
        // }
        
        printf("\n");
        for (auto bb_iter = cfg.begin(); bb_iter != cfg.end(); ++bb_iter) {
            for (auto inst_iter = (*bb_iter)->begin(); inst_iter != (*bb_iter)->end(); ++inst_iter) {
                std::cout << "instruction mnemonic: " << (*inst_iter)->getMnemonic() << "\n";
                std::string mnemonic = (*inst_iter)->getMnemonic();

                // 去除空白字符、转化为大写
                mnemonic.erase(std::remove_if(mnemonic.begin(), mnemonic.end(), ::isspace), mnemonic.end());
                transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);

                if (typeMap.find(mnemonic) == typeMap.end()) {
                    std::cout << "Error: op list of instruction " << mnemonic << " not found!\n";
                }

                MetaInst* metaInst = builder.instMap[(*inst_iter)->hashCode()];
                vector<InstType> opList = typeMap[mnemonic];
                metaInst->setInstType(opList);

            }
        }


    }

    void AsmBBFilter::doFilter(FilterTarget& target, FilterChain& chain) {

    }

    void AsmFuncFilter::doFilter(FilterTarget& target, FilterChain& chain) {

    }

}