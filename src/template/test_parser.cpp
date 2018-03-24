#include "parser.hpp"

using namespace std ;
string data = R"(
        ksks <d% x + 3 %>
        ggggg <d% shh %>)";

int main(int argc, char *argv[]) {

    istringstream strm(data) ;


    TemplateParser parser(strm) ;

    parser.parse();



}
