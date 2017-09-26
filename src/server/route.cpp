#include <wspp/server/route.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

#include <cassert>

using namespace std ;
using namespace wspp::util ;

namespace wspp { namespace server {

struct RouteElement {

    RouteElement(const string &name, const string &pattern, bool optional):
        name_(name), pattern_(pattern), optional_(optional) {}

    string name_ ;
    string pattern_ ;
    bool optional_ ;
};

struct RouteImpl {
public:
    RouteImpl(const std::string &pattern) {
        if ( pattern.back() == '/' ) pattern_ = pattern ;
        else pattern_ = pattern + '/' ;
        assert(parse(pattern_)) ;
    }
    bool parse(const string &pattern) ;
    bool makeRegex() ;
    bool match(const string &path, Dictionary &vars);
    string url(const Dictionary &params, bool relative) const;

    vector<RouteElement> elements_ ;
    string pattern_ ;
};


bool RouteImpl::parse(const string &pattern) {
    string::const_iterator cursor = pattern.begin(), end = pattern.end() ;

    enum State { Token, Element, Param, Regex } ;

    State state = Element ;

    string name, pat ;
    bool is_optional = false ;

    if ( cursor != end && *cursor == '/' ) ++cursor ; // omit leading /

    while ( cursor != end ) {
        char c = *cursor++ ;
        if ( state == Token ) {
            if ( c == '/' || cursor == end ) {
                elements_.emplace_back(RouteElement(name, pat, is_optional )) ;
                name.clear() ; pat.clear() ; is_optional = false ;
                state = Element ;
            }
            else if ( c == '?') { is_optional = true ; }
            else return false ;
        }
        else if ( state == Param ) {
            if ( c == ':' )
                state = Regex ;
            else if ( c == '}' )
                state = Token ;
            else if ( isalnum(c) || c == '_' || c == '-' ) name.push_back(c) ;
            else return false ;
        }
        else if ( state == Regex ) {
            if ( c == '}' ) state = Token ;
            else pat.push_back(c) ;
        }
        else if ( state == Element ) {
            if ( c == '{' ) {
                if ( !pat.empty() ) return false ;
                state = Param ;
            }
            else if ( cursor == end )
                elements_.emplace_back(RouteElement(name, pat, is_optional)) ;
            else if ( c == '/' ) {
                state = Token ;
                cursor -- ;
            }
            else pat.push_back(c) ;
        }
    }


    return true ;
}


class UriPatternMatcher {
public:

    bool matches(const string &pattern, const RouteImpl &route, const string &uri, Dictionary &vars) ;

    static UriPatternMatcher &instance() {
        static UriPatternMatcher the_instance ;
        return the_instance;
    }

private:
    boost::regex makeRegexFromPattern(const string &pat, const RouteImpl &route_data);
    bool matchPattern(const boost::regex &rx, const RouteImpl &route, const string &path, Dictionary &vars) ;

    std::map<std::string, boost::regex> cache_ ;
    mutable boost::mutex cache_mutex_ ;
};

bool UriPatternMatcher::matchPattern(const boost::regex &rx, const RouteImpl &route, const string &path, Dictionary &vars)
{
    boost::smatch results ;
    if ( boost::regex_match(path, results, rx) ) {
        for( const RouteElement &e: route.elements_ ) {
            if ( !e.name_.empty() ) {
                string val = results[e.name_].str() ;
                if ( !val.empty() ) vars[e.name_] = val ;
            }
        }
        return true ;
    }

    return false ;
}

bool UriPatternMatcher::matches(const string &pattern, const RouteImpl &route, const string &uri, Dictionary &vars) {
    boost::unique_lock<boost::mutex> l(cache_mutex_) ;

    auto it = cache_.find(pattern) ;
    if ( it != cache_.end() )  // pattern exists in cache
        return matchPattern(it->second, route, uri, vars) ;
    else {
        boost::regex rxs = makeRegexFromPattern(pattern, route) ;
        bool res = matchPattern(rxs, route, uri, vars) ;
        cache_[pattern] = std::move(rxs) ;
        return res ;
    }
}

boost::regex  UriPatternMatcher::makeRegexFromPattern(const string &pat, const RouteImpl &route) {
    string rx ;

    vector<string> patterns ;
    for( const RouteElement &e: route.elements_ ) {
        string param = e.name_ ;
        string pattern = e.pattern_ ;

        if ( pattern.empty() ) pattern = "[^\\/]+" ;

        if ( !param.empty() ) {
            patterns.push_back("(?<" + param + ">" + pattern +  ")" ) ;
        }
        else
            patterns.push_back( "(?:" + pattern + ")") ;
    }

    auto it = route.elements_.rbegin() ;
    auto pit = patterns.rbegin() ;

    for(  ; it != route.elements_.rend() ; ++it, ++pit ) {
        const RouteElement &e = *it ;

        if ( e.optional_ ) rx = "(?:/" + *pit + rx + ")?" ;
        else rx = "/" + *pit + rx ;
    }

    rx = "^" + rx + '$';


    try {
        boost::regex res(rx, boost::regex::perl) ;
        return res ;
    }
    catch ( boost::bad_expression &e ) {
        cout << e.what() << endl ;
        return boost::regex() ;
    }
}

bool RouteImpl::match(const string &path, Dictionary &data) {
    return UriPatternMatcher::instance().matches(pattern_, *this, path, data) ;

}

string RouteImpl::url(const Dictionary &params, bool relative) const {
    string res ;
    if ( !relative ) res += '/' ;
    for( const RouteElement &e: elements_ ) {
        if ( e.name_.empty() ) {
            res += e.pattern_ + '/' ;
        }
        else {
            auto it = params.find(e.name_) ;
            if ( it == params.end() ) {
                if ( e.optional_ ) return res ;
                else assert(1) ;
            }
            else {
                res += it->second + '/' ;
            }
        }
    }

    return res ;
}


Route::Route(const string &pattern): impl_(new RouteImpl(pattern)) {
}

Route::~Route() {}

static string get_clean_path(const std::string &path)
{
    string uri = path ;

    // remove ererything after the query string if any
    uri = boost::regex_replace(uri, boost::regex("\\?.*"), "") ;

            // trim beginning and trailing slashes
    uri = boost::algorithm::trim_copy_if(uri, boost::is_any_of("/")) ;

         // Replace multiple slashes in a url, such as /my//dir/url
    uri = boost::regex_replace(uri, boost::regex("\\/+"), "/");

    return '/' + uri + '/' ;
}

bool Route::matches(const string &path, Dictionary &data) const {
    return impl_->match(path, data) ;

}

bool Route::matches(const string &path) const {
    Dictionary data ;
    return matches(path, data) ;
}

string Route::url(const Dictionary &params, bool relative) const  {
    return impl_->url(params, relative) ;
}

} // namespace server
} // namespace wspp

