#include <wspp/server/request.hpp>

#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

using namespace std ;

namespace wspp { namespace server {

typedef std::pair<vector<boost::regex>, std::vector<string>> RxPattern ;

class UriPatternMatcher {
public:

    bool matches(const string &pattern, const string &uri, Dictionary &vars) ;

    static UriPatternMatcher &instance() {
        static UriPatternMatcher the_instance ;
        return the_instance;
    }

private:
    RxPattern makeRegexFromPattern(const string &pat);
    bool matchPattern(const RxPattern &patterns, const string &uri, Dictionary &vars);

    std::map<std::string, RxPattern> cache_ ;
    mutable boost::mutex cache_mutex_ ;
};



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
RxPattern UriPatternMatcher::makeRegexFromPattern(const string &pat) {
    string pattern = boost::trim_copy_if(pat, boost::is_any_of("/")) ;
    vector<string> patterns ;

    if ( pattern.back() == '?' ) { // last arg is optional so we create two expressions one with and one without the argument
        patterns.emplace_back(boost::trim_right_copy_if(pattern, boost::is_any_of("?"))) ;
        patterns.emplace_back(boost::regex_replace(pattern, boost::regex("\\/(?:.(?!\\/))+\\?$"), "")) ;
    }
    else
        patterns.emplace_back(pattern) ;

    RxPattern rxs ;

    bool first_pass = true ;

    for( string pattern: patterns ) {

        pattern = boost::regex_replace(pattern, boost::regex("\\/+"), "\\\\/");

        string rx = boost::regex_replace(pattern, boost::regex("(?:\\{([^\\{\\}]*?)(?:\\:([^\\{\\}]+))?\\})"), [&](const boost::smatch &matches) -> string {
                    string param = matches[1] ;
                    string vmatch = matches[2] ;
                    string verifier = vmatch.empty() ? "*" : vmatch ;
                    string omatch = matches[3] ;
                    string option = omatch.empty() ? "" : "?" ;
                    if ( !param.empty() ) {
                        if ( first_pass ) rxs.second.push_back(param) ;
                        return  "(?<" + param + ">" + verify_patterns[verifier] +  ")" + option ;
                    }
                    else if ( !verifier.empty() )
                        return "(" + verify_patterns[verifier] + ")" + option ;
        }) ;


        first_pass = false ;

        try {
            boost::regex res("^" + rx + "$", boost::regex::perl) ;
            rxs.first.emplace_back(res) ;
        }
        catch ( boost::bad_expression &e) {
            cout << e.what() << endl ;
            throw(e) ;
        }
    }

    return rxs ;
}


bool UriPatternMatcher::matchPattern(const RxPattern &pattern, const string &uri, Dictionary &vars)
{
    for( const boost::regex &rx: pattern.first ) {
        boost::smatch results ;
        if ( boost::regex_match(uri, results, rx) ) {

            for ( const string &name: pattern.second ) {
                string val = results[name].str() ;
                if ( !val.empty() ) vars[name] = val ;
            }

            return true ;
        }
    }

    return false ;
}

bool UriPatternMatcher::matches(const string &pattern, const string &uri, Dictionary &vars) {
    boost::unique_lock<boost::mutex> l(cache_mutex_) ;

    auto it = cache_.find(pattern) ;
    if ( it != cache_.end() )  // pattern exists in cache
        return matchPattern(it->second, uri, vars) ;
    else {
        RxPattern rxs = makeRegexFromPattern(pattern) ;
        bool res = matchPattern(rxs, uri, vars) ;
        cache_[pattern] = std::move(rxs) ;
        return res ;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Request::getCleanPath(const std::string &path) const
{
    string uri = path ;

    // remove ererything after the query string if any
    uri = boost::regex_replace(uri, boost::regex("\\?.*"), "") ;

            // trim beginning and trailing slashes
    uri = boost::algorithm::trim_copy_if(uri, boost::is_any_of("/")) ;

         // Replace multiple slashes in a url, such as /my//dir/url
    uri = boost::regex_replace(uri, boost::regex("\\/+"), "/");

    return uri ;
}

bool Request::matches(const string &method, const string &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && UriPatternMatcher::instance().matches(pattern, getCleanPath(path_), attributes) ;
}

bool Request::matches(const string &method, const string &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && UriPatternMatcher::instance().matches(pattern, getCleanPath(path_), attributes) ;
}

bool Request::matchesMethod(const string &method) const
{
    vector<string> methods ;
    boost::split( methods, method, boost::is_any_of(" |"), boost::token_compress_on );
    return std::find(methods.begin(), methods.end(), method_) != methods.end() ;
}

} // namespace server
} // namespace wspp
