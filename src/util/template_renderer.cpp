#include <wspp/util/template_renderer.hpp>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace fs = boost::filesystem ;
using namespace std ;

namespace wspp {

std::string TemplateRenderer::renderFile(const std::string &path, const Variant::Object &context, bool cache) {
    std::ifstream strm(path) ;

    std::string str((istreambuf_iterator<char>(strm)),
                     istreambuf_iterator<char>());
    return render(str, context) ;

}


std::string TemplateRenderer::renderString(const std::string &str, const Variant::Object &context) {
    return render(str, context) ;
}

static bool expect(const std::string &src, uint &idx, char c) {
    if ( idx < src.size() ) {
        if ( src[idx] == c ) {
            ++idx ;
            return true ;
        } else return false ;
    } else return false ;
}

static string eat_tag(const std::string &src, uint &idx) {
    string res ;

    uint nc = src.size() ;

    while ( idx < nc ) {
        if ( expect(src, idx, '}' ) ) {
            if ( expect(src, idx, '}' ) ) {
                return res ;
            }
            else if ( idx < nc ) res.push_back(src[idx++]) ;
        }
        else if ( idx < nc ) res.push_back(src[idx++]) ;
    }
}

static void parse_tag_var(const string &src, char &cmd, string &name) {

    uint nc = src.size(), idx = 0 ;

    if ( nc == 0 ) return ;

    switch ( src[0] ) {
        case '#':
        case '/':
        case '^': cmd = src[0] ; idx++ ; break ;
        default: cmd = 0 ; break ;
    }

    while ( idx < nc ) {
        char c = src[idx++] ;
        switch ( c ) {
            case ' ':
            case '\r':
            case '\n':
            case '\t': break ;
            default: name.push_back(c) ; break ;
        }
    }
}

string TemplateRenderer::render(const string &src, const Variant::Object &ctx) {

    deque<Variant> ctx_stack ;
    ctx_stack.push_back(ctx) ;

    string out ;

    uint nc = src.size(), idx = 0 ;

    while ( idx < nc ) {
        if ( expect(src, idx, '{') ) {
            if ( expect(src, idx, '{') ) {
                string tag = eat_tag(src, idx), tag_name ;
                char cmd ;

                parse_tag_var(tag, cmd, tag_name) ;

                if ( cmd == '#' ) { // section
                    Variant v = ctx_stack.back() ;
                    if ( v.isObject() ) {
                        Variant child = v.at(tag_name) ;
                        if ( !child.isNull() )
                            ctx_stack.push_back(child) ;
                    }


                } else if ( cmd == '/' ) { // section end
                    ctx_stack.pop_back() ;

                } else if ( cmd == '^' ) { // inverse list

                }
                else {
                    Variant v = ctx_stack.back() ;
                    if ( v.isObject() ) {
                        Variant child = v.at(tag_name) ;
                        if ( !child.isNull() )
                            ctx_stack.push_back(child) ;
                }
            }
            else if ( idx < nc ) out.push_back(src[idx++]) ;
        }
        else if ( idx < nc ) out.push_back(src[idx++]) ;
    }
}





}
