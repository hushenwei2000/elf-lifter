#pragma once
#include "Filter.h"

namespace MetaTrans {

    class AsmArgFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class AsmConstantFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class AsmInstFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class AsmBBFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    class AsmFuncFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

    // fill id for each meta datastructure
    class MetaIDFilter : public Filter {

        public:

            virtual void doFilter(FilterTarget& target, FilterChain& chain) override;

    };

}