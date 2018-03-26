#include "parser.hpp"
#include "template_ast.hpp"
using namespace std ;
string data = R"(-----
       {% filter upper %}
            {% for user in users %}
                    {{- user.name -}} is {{ foo.foo }}
        {% endfor %}
        {% end filter %}
        {{ foo.foo }}
 ----       )";

int main(int argc, char *argv[]) {

    istringstream strm(data) ;


    TemplateParser parser(strm) ;

    parser.parse();

    ast::TemplateEvalContext ctx ;

    auto &&t = ctx.data() ;

    t["name"] = Variant::Object{{"k1", "v1"}, {"k2", Variant::Array{{"a", "b"}}}} ;
    t["vals"] = Variant::Array{{1, 2, 3, 4}}  ;

    t["food"] = Variant::Object{{"ketchup", "5 tbsp"}, {"mustard", "1 tbsp"}, {"pickle", "0 tbsp"}} ;

    t["items"] = Variant::Array{{ Variant::Object{{ "title", "foo"}, {"id", 1 }}, Variant::Object{{ "title", "bar" }, {"id", "2"}} }};

    t["users"] = Variant::Array{{ Variant::Object{{ "name", "john"}, {"hidden", true }}, Variant::Object{{ "name", "tom" }, {"hidden", false}} }};
    t["tired"] = true ;

    string res ;
    parser.eval(ctx, res) ;
    cout << res << endl ;
}
