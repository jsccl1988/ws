#include "parser.hpp"
#include "template_renderer.hpp"

#include <memory>

using namespace std ;

int main(int argc, char *argv[]) {


    std::shared_ptr<TemplateLoader> loader(new FileSystemTemplateLoader({"/home/malasiot/source/ws/data/test"})) ;
    TemplateRenderer rdr(loader) ;
    rdr.setDebug() ;

    ast::TemplateEvalContext ctx ;
    ctx.rdr_ = &rdr ;

    auto &&t = ctx.data() ;

    t["name"] = Variant::Object{{"k1", "v1"}, {"k2", Variant::Array{{"a", "b"}}}} ;
    t["vals"] = Variant::Array{{1, 2, 3, 4, "a"}}  ;

    t["food"] = Variant::Object{{"ketchup", "5 tbsp"}, {"mustard", "1 tbsp"}, {"pickle", "0 tbsp"}} ;

    t["items"] = Variant::Array{{ Variant::Object{{ "title", "foo"}, {"id", 1 }}, Variant::Object{{ "title", "bar" }, {"id", "2"}} }};

    t["users"] = Variant::Array{{ Variant::Object{{ "name", "john"}, {"hidden", true }}, Variant::Object{{ "name", "tom" }, {"hidden", false}} }};
    t["tired"] = true ;

    t["posts"] = Variant::Array{{ Variant::Object{{ "title", "xxx"}, {"body", "yyy"}, {"text", "bb"}},
            Variant::Object{{ "title", "aaa"}, {"body", "bbb"}, {"text", "ccc"}}
    } };

    string res ;

    ast::DocumentNodePtr c = rdr.compile("index") ;
    c->eval(ctx, res) ;
    cout.flush() ;
    cout << res << endl ;
}
