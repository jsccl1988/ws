#include <wspp/twig/renderer.hpp>
#include <wspp/util/filesystem.hpp>

#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include "template_parser.hpp"

using namespace std;
using namespace wspp::util;

namespace wspp { namespace web {

FileSystemTemplateLoader::FileSystemTemplateLoader(const std::initializer_list<string> &root_folders, const string &suffix):
    root_folders_(root_folders), suffix_(suffix) {
}

string FileSystemTemplateLoader::load(const string &key) {

    using namespace boost::filesystem;

    for ( const string &r: root_folders_ ) {
        path p(r);

        p /= key + suffix_;

        if ( !exists(p) ) continue;

        return readFileToString(p.string());

    }

    return string();
}



string TemplateRenderer::render(const string &src, const Variant &ctx) {

    detail::ParseContext parse_ctx(loader_,  block_helpers_, value_helpers_, caching_);
    detail::Parser parser(parse_ctx);

    string res;
    detail::SectionNode::Ptr ast = parser.parse(src);

    if ( ast ) {
        Variant rc(Variant::Object{{"$root", true}});
        ContextStack stack;
        stack.push(rc);
        stack.push(ctx);
        ast->eval(stack, res);
    }

    return res;
}

string TemplateRenderer::renderString(const string &src, ContextStack &ctx) {

    detail::ParseContext parse_ctx(loader_,  block_helpers_, value_helpers_, caching_);
    detail::Parser parser(parse_ctx);

    string res;
    detail::SectionNode::Ptr ast = parser.parseString(src);

    if ( ast ) {
        ast->eval(ctx, res);
    }

    return res;
}

void TemplateRenderer::registerBlockHelper(const string &name, TemplateRenderer::BlockHelper helper){
    block_helpers_.insert({name, helper});
}

void TemplateRenderer::registerValueHelper(const string &name, TemplateRenderer::ValueHelper helper){
    value_helpers_.insert({name, helper});
}

void TemplateRenderer::registerDefaultHelpers() {
    registerValueHelper("render", [&](ContextStack &ctx, Variant::Array params) -> pair<bool, string> {
        Variant key = params.at(0);
        if ( key.isNull() ) return make_pair(true, string());
        return make_pair(true, renderString(key.toString(), ctx));
    });
}


} // namespace web
               } // namespace wspp
