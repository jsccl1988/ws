#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <deque>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem ;
using namespace std ;

class StreamWrapper {
public:
    StreamWrapper(istream &strm): strm_(strm) {
        line_ = 1 ; chars_ = 0 ; column_ = 1 ;
    }

    char next() {
        char c ;
        if ( !la_.empty() ) {
            c = la_.back() ;
            la_.pop_back();
        }
        else {
            c = strm_.get() ;
            if ( c == '\r' ) c = strm_.get() ;
        }

        chars_ ++ ;
        column_ ++ ;

        if ( c == '\n' ) {
            line_++ ; column_ = 1 ;
        }

        return c ;
    }

    void putback(char c) {
        la_.push_back(c) ;
        chars_ -- ;
        column_ -- ;
        if ( c == '\n' ) {
            line_-- ; column_ = 1 ;
        }
    }

    bool good() {
        return strm_.good() ;
    }

    uint line() const { return line_ ; }

    istream &strm_ ;
    uint line_, chars_, column_ ;
    std::deque<char> la_ ;
};

bool parse(istream &in, const string &filename, ostream &out) {



    enum ParserState { RawText, Expression, Block } ;

    StreamWrapper strm(in) ;

    string raw, block, expr ;
    ParserState state = RawText ;

    while ( strm.good() ) {
        char c = strm.next() ;

        if ( c == '<' && state == RawText ) {
            char nc1 = strm.next() ;
            if ( nc1 == '%' ) {
                char nc2 = strm.next() ;
                if ( nc2 == '=' ) {
                    state = Expression ;
                }
                else {
                    state = Block ;
                    strm.putback(nc2) ;
                }

                if ( !raw.empty() ) {
                    out << "response.append(R\"(" << raw << ")\");" << endl ;
                    raw.clear() ;
                }

            }
            else {
                raw.push_back(c) ;
                strm.putback(nc1) ;
            }
        }
        else if ( c == '%' && ( state == Block || state == Expression )) {
            char nc = strm.next() ;
            if ( nc == '>' ) {

                if ( state == Expression ) {
                    out << "#line " << strm.line_ << " \"" << filename << "\"" << endl ;
                    out << "response.append(" << expr << ");" << endl ;
                    out.flush() ;
                    expr.clear() ;
                }
                else {
                    out << block ;
                    out.flush() ;
                    block.clear() ;
                }

               state = RawText ;
            }
            else {
                strm.putback(nc) ;

                if ( state == Expression ) {
                    expr.push_back(c) ;
                }
                else if ( state == Block ) {
                    if ( c == '\n' ) {
                        out << "#line " << strm.line_-1 << " \"" << filename << "\"" << endl ;
                        out << block << endl ;
                        block.clear() ;
                    }
                    else block.push_back(c) ;
                }
             }
        }
        else {
            if ( state == RawText ) {
                if ( strm.good() )
                    raw.push_back(c) ;
                else
                    out << "response.append(R\"(" << raw << ")\");" << endl ;
            }
            else if ( state == Expression ) {
                expr.push_back(c) ;
            }
            else if ( state == Block ) {
                if ( c == '\n' ) {
                    out << "#line " << strm.line_-1 << " \"" << filename << "\"" << endl ;
                    out << block << endl ;
                    block.clear() ;
                }
                else block.push_back(c) ;
            }
        }
    }

    return true ;
}

int main(int argc, const char *argv[]) {

    if ( argc < 3 ) {
        cerr << "Invalid number of args" << endl ;
        exit(1) ;
    }

    string src(argv[1]), dst(argv[2]) ;

    if ( !fs::exists(src) ) {
        cerr << "source file does not exist: " << src << endl ;
        exit(1) ;
    }

    fs::create_directories(fs::path(dst).parent_path()) ;

    ifstream srcstream(src);
    ofstream dststream(dst) ;

    if ( !dststream.good() ) {
        cerr << "cannot create output file: " << dst << endl ;
        return 1 ;
    }

    if ( !parse(srcstream, src, dststream) ) {
        cerr << "cannot parse input file: " << src << endl ;
    }

}
