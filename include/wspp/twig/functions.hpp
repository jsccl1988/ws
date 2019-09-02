#ifndef __WSPP_TEMPLATE_FUNCTIONS_HPP__
#define __WSPP_TEMPLATE_FUNCTIONS_HPP__

#include <string>
#include <functional>

#include <wspp/util/variant.hpp>

#include "context.hpp"

namespace wspp {
namespace twig {
using TemplateFunction = std::function<wspp::util::Variant(const wspp::util::Variant &)>;

// Unpack positional and named arguments passed to the function to the list of expected arguments
// The named_args is a list of arguments names. If ending with '?' argument is optional. Non supplied arguments are given undefined value.
// Throws TemplateRuntimeException if not all required arguments are provided
void unpack_args(const util::Variant &args, const std::vector<std::string> &named_args, util::Variant::Array &res);

class FunctionFactory {
public:
    FunctionFactory();

    static FunctionFactory &instance() {
        static FunctionFactory s_instance;
        return s_instance;
    }

    bool hasFunction(const std::string &name);
    wspp::util::Variant invoke(const std::string &name, const wspp::util::Variant &args);
    void registerFunction(const std::string &name, const TemplateFunction &f);

private:
    std::map<std::string, TemplateFunction> functions_;
};
} // namespace twig
} // namespace wspp
#endif
