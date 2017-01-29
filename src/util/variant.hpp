#ifndef __VARIANT_HPP__
#define __VARIANT_HPP__

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <functional>

#include <boost/variant.hpp>

struct Bool {
    bool v_ ;
};

using vnode = boost::make_recursive_variant<
    std::nullptr_t, std::string,  int64_t, uint64_t, double, Bool,
    std::map<const std::string, boost::recursive_variant_>,
    std::vector<boost::recursive_variant_>>::type;

using vmap = std::map<const std::string, vnode>;
using varray = std::vector<vnode>;

class VariantRef {
public:
    VariantRef(vnode &v): data_(v) {}

    VariantRef & operator = (int64_t v) { data_ = vnode(v) ; return *this ;}
    VariantRef & operator = (const std::string &v) { data_ = vnode(v) ; return *this ;}

private:
    vnode &data_ ;
};

class Variant {
public:

    typedef std::map<const std::string, vnode> Object;
    typedef std::vector<vnode> Array ;

    Variant(nullptr_t v): data_(v) {}
    Variant(int v): data_(int64_t(v)) {}
    Variant(uint64_t v): data_(v) {}
    Variant(int64_t v): data_(v) {}
    Variant(double v): data_(v) {}
    Variant(const std::string &v): data_(v) {}

    Variant(const vmap &v): data_(v) {}
    Variant(const varray &v): data_(v) {}
    Variant(const vnode &v): data_(v) {}

    struct FieldAccesor : boost::static_visitor<Variant>
    {
        FieldAccesor(const std::string &field): field_(field) {}

        Variant operator ()(nullptr_t t) const { assert("Error"); }
        Variant operator ()(uint64_t) const { assert("Error"); }
        Variant operator ()(int64_t) const { assert("Error"); }
        Variant operator ()(const Bool &) const { assert("Error"); }

        Variant operator ()(double) const { assert("Error"); }
        Variant operator ()(const std::string &) const { assert("Error"); }
        Variant operator ()(const vmap &v) const { return Variant(v.find(field_)->second) ; }
        Variant operator ()(const varray &) const { assert("Error"); }


        std::string field_ ;
    };

    struct RefFieldAccesor : boost::static_visitor<VariantRef>
    {
        RefFieldAccesor(const std::string &field): field_(field) {}

        VariantRef operator ()(nullptr_t t) const { assert("Error"); }
        VariantRef operator ()(int64_t) const { assert("Error"); }
        VariantRef operator ()(uint64_t) const { assert("Error"); }
        VariantRef operator ()(const Bool &b) const { assert("Error"); }
        VariantRef operator ()(double) const { assert("Error"); }
        VariantRef operator ()(const std::string &) const { assert("Error"); }
        VariantRef operator ()(vmap &v) const { return VariantRef(v.find(field_)->second) ; }
        VariantRef operator ()(const varray &) const { assert("Error"); }

        std::string field_ ;
    };

    struct CastStringVisitor : boost::static_visitor<std::string>
    {
        template <class T>
        std::string operator () ( T t ) const { assert("Errpr") ;  }

       // std::string operator ()(nullptr_t t) const { assert("Error"); }
       // std::string operator ()(int64_t) const { assert("Error"); }
        std::string operator ()(const std::string &s) const { return s ; }
      //  std::string operator ()(const vmap &v) const { assert("error") ; }
      //  std::string operator ()(const varray &) const { assert("Error"); }


    };

    struct PrintVisitor : boost::static_visitor<>
    {
        PrintVisitor(std::ostream &strm): strm_(strm) {}

        void operator ()(nullptr_t t) const { strm_ << "null" ; }
        void operator ()(uint64_t v) const { strm_ << v ; }
        void operator ()(int64_t v) const { strm_ << v ; }
        void operator ()(double v) const { strm_ << v ; }
        void operator ()(Bool v) const { strm_ << (v.v_ ? "true" : "false") ; }
        void operator ()(const std::string &v) const { strm_ << '"' << v << '"'; }
        void operator ()(const vmap &v) const {
            strm_ << "{" ;
            bool is_first = true ;
            for( auto &e: v) {
                if ( !is_first ) strm_ << "," ;
                else is_first = false ;
                strm_ << '"' << e.first << '"' << ":" ;
                boost::apply_visitor(PrintVisitor(strm_), e.second);
            }
            strm_ << "}" ;
        }
        void operator ()(const varray &v) const {
            strm_ << "[" ;
            bool is_first = true ;
            for( auto &e: v) {
                if ( !is_first ) strm_ << "," ;
                else is_first = false ;
                boost::apply_visitor(PrintVisitor(strm_), e);
            }
            strm_ << "]";
        }

        std::ostream &strm_ ;
    };

    Variant operator [] (const std::string &name) const {
       return boost::apply_visitor(FieldAccesor(name), data_);
    }

    VariantRef operator [] (const std::string &name) {
       return boost::apply_visitor(RefFieldAccesor(name), data_);
    }

    std::string toString() const {
        return boost::apply_visitor(CastStringVisitor(), data_);
    }

    void dumb(std::ostream &strm = std::cout) const {
        boost::apply_visitor(PrintVisitor(strm), data_);
    }

    vnode data_ ;
};




#endif
