#ifndef __WSPP_TEMPLATE_LOADER_HPP__
#define __WSPP_TEMPLATE_LOADER_HPP__

#include <string>
#include <vector>

// abstract template loader

class TemplateLoader {
public:
    // override to return a template string from a key
    virtual std::string load(const std::string &src) =0 ;
};

// loads templates from file system relative to root folders.

class FileSystemTemplateLoader: public TemplateLoader {

public:
    FileSystemTemplateLoader(const std::initializer_list<std::string> &root_folders, const std::string &suffix = ".twig") ;

    virtual std::string load(const std::string &src) override ;

private:
    std::vector<std::string> root_folders_ ;
    std::string suffix_ ;
};


#endif
