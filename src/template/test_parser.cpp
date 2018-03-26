#include "parser.hpp"
#include "template_ast.hpp"
using namespace std ;
string data = R"(-----
             {{ (1 + 2) IF name.k1 == 'v1' ELSE 5 }}
 ----       )";

int main(int argc, char *argv[]) {

    istringstream strm(data) ;


    TemplateParser parser(strm) ;

    parser.parse();

    ast::TemplateEvalContext ctx ;

    auto &&t = ctx.push() ;

    t["name"] = Variant::Object{{"k1", "v1"}, {"k2", Variant::Array{{"a", "b"}}}} ;
    t["vals"] = Variant::Array{{1, 2, 3, 4}}  ;

    t["food"] = Variant::Object{{"ketchup", "5 tbsp"}, {"mustard", "1 tbsp"}, {"pickle", "0 tbsp"}} ;

    t["items"] = Variant::Array{{ Variant::Object{{ "title", "foo"}, {"id", 1 }}, Variant::Object{{ "title", "bar" }, {"id", "2"}} }};

    t["hunsgry"] = true ;
    t["tired"] = true ;

    string res ;
    parser.eval(ctx, res) ;
    cout << res << endl ;
}
