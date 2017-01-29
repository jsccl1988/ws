#include "util/variant.hpp"

#include <iostream>

using namespace std ;

int main() {


    Variant var = {
        Variant::Object{
            { "name", "val" },
            { "val", Variant::Array{"10", (uint64_t)20 } }
        }
    } ;


    var.dumb(std::cout) ;

    var["name"] = "2";
    var.dumb() ;
    var = std::string("test") ;
    var.dumb() ;

}
