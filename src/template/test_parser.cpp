#include "parser.hpp"
#include "template_ast.hpp"
using namespace std ;
string data = R"(-----
----{% for x, y, z in points %}
  Point: {{ x }}, {{ y }}, {{ z }}
{% endfor %}----
 ----       )";

int main(int argc, char *argv[]) {

    istringstream strm(data) ;


    TemplateParser parser(strm) ;

    parser.parse();

    ast::TemplateEvalContext ctx ;

    ctx.vals_ = Variant::Object{{"name", Variant::Object{{"k1", "v1"}, {"k2", Variant::Array{{"a", "b"}}}}}, {"vals", Variant::Array{{1, 2, 3, 4}} }} ;


    cout << parser.eval(ctx).toJSON()  << endl ;
}
