#include <wspp/views/forms.hpp>
#include <wspp/twig/renderer.hpp>
#include <wspp/server/request.hpp>
#include <wspp/server/response.hpp>

#include <memory>

using namespace std ;
using namespace wspp::twig ;
using namespace wspp::util ;
using namespace wspp::web ;


int main(int argc, char *argv[]) {

    string root = "/home/malasiot/source/ws/data/test/" ;
    std::shared_ptr<TemplateLoader> loader(new FileSystemTemplateLoader({{root}, {root + "/bootstrap3/"}})) ;
    TemplateRenderer rdr(loader) ;
    rdr.setDebug() ;


}
