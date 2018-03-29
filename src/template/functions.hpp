#ifndef __WSPP_TEMPLATE_FUNCTIONS_HPP__
#define __WSPP_TEMPLATE_FUNCTIONS_HPP__

#include <string>
#include <functional>

#include <wspp/util/variant.hpp>

#include "context.hpp"

using TemplateFunction = std::function<wspp::util::Variant(const wspp::util::Variant &, TemplateEvalContext &)>;

class FunctionFactory {
public:

    FunctionFactory() ;

    static FunctionFactory &instance() {
        static FunctionFactory s_instance ;
        return s_instance ;
    }

    bool hasFunction(const std::string &name) ;

    wspp::util::Variant invoke(const std::string &name, const wspp::util::Variant &args, TemplateEvalContext &ctx) ;

    void registerFunction(const std::string &name, const TemplateFunction &f);

private:

    std::map<std::string, TemplateFunction> functions_ ;
};


#endif
