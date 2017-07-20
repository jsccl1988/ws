#include <wspp/server/router.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
using namespace std ;

namespace wspp {

bool Router::handle(Request &req, Response &rep, SessionManager &session) {
    std::string method = req.method_ ;
    std::string uri = getCleanPath(req.path_) ;

    for( const Entry &entry: entries_ ) {
        if ( std::find(entry.methods_.begin(), entry.methods_.end(), method) == entry.methods_.end() ) continue ;
        if ( matchRoute(entry.patterns_, uri, req.GET_) ) {

        }
    }

    return false ;
}

std::string Router::getCleanPath(const std::string &path)
{
    string uri = path.substr(base_path_.length());

    // remove ererything after the query string if any
    uri = boost::regex_replace(uri, boost::regex("\\?.*"), "") ;

            // trim beginning and trailing slashes
    uri = boost::algorithm::trim_copy_if(uri, boost::is_any_of("/")) ;

         // Replace multiple slashes in a url, such as /my//dir/url
    uri = boost::regex_replace(uri, boost::regex("\\/+"), "/");

    return uri ;
}


static map<string, string> verify_patterns = {
    { "n", "[0-9]+"},
    { "a", "[a-zA-Z0-9\\-\\_]+"},
    { "w", "[a-zA-Z]"},
    { "*", "[^\\/]+"},
    { "**", ".+"}
} ;

static string replace(const string &src, const boost::regex &re, std::function<string (const boost::smatch &)> callback)
{
    string res ;
    size_t pos = 0, start_pos = 0;

    boost::sregex_iterator  begin(src.begin(), src.end(), re),  end;
    std::for_each(begin, end, [&](const boost::smatch &match) {
        pos = match.position((size_t)0);
        res.append(src.substr(start_pos, pos - start_pos)) ;
        res.append(callback(match)) ;
        pos += match.length(0) ;
        start_pos = pos ;
    });

    return res;
}
vector<boost::regex> Router::makeRegexPatterns(const string &pat) {
    string pattern = boost::trim_copy_if(pat, boost::is_any_of("/")) ;
    vector<string> patterns ;

    if ( pattern.rfind('?', 0) == pattern.length() - 1 ) { // last arg is optional so we create two expressions one with and one without the argument
        patterns.emplace_back(boost::trim_right_copy_if(pattern, boost::is_any_of("?"))) ;
        patterns.emplace_back(boost::regex_replace(pattern, boost::regex("\\/(?:.(?!\\/))+\\?$"), "")) ;
    }
    else
        patterns.emplace_back(pattern) ;

    vector<boost::regex> rxs ;

    for( string pattern: patterns ) {

        pattern = boost::regex_replace(pattern, boost::regex("\\/+"), "\\\\/");

        string rx = replace(pattern, boost::regex("(?:\\{([^\\{\\}]*?)(?:\\:([^\\{\\}]+))?\\})"), [&](const boost::smatch &matches) -> string {
                    string param = matches[1] ;
                    string vmatch = matches[2] ;
                    string verifier = vmatch.empty() ? "*" : vmatch ;
                    string omatch = matches[3] ;
                    string option = omatch.empty() ? "" : "?" ;
                    if ( !param.empty() )
                        return  "(?<" + param + ">" + verify_patterns[verifier] +  ")" + option ;
                    else if ( !verifier.empty() )
                        return "(" + verify_patterns[verifier] + ")" + option ;
        }) ;

        try {
        boost::regex res("^" + rx + "$", boost::regex::perl) ;
        rxs.emplace_back(res) ;
        }
        catch ( boost::bad_expression &e) {
            cout << e.what() << endl ;
        }


    }

    return rxs ;
}


bool Router::matchRoute(const vector<boost::regex> &patterns, const string &uri, Dictionary &vars)
{
    for( const boost::regex &rx: patterns ) {
        boost::smatch results ;
        if ( boost::regex_match(uri, results, rx) ) {

            return true ;

        }
    }

    return false ;

}



}
