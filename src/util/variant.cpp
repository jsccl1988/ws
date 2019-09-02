#include <wspp/util/variant.hpp>
#include <wspp/util/detail/value_holder.hpp>

namespace wspp {
namespace util {
Variant::Variant(): value_(new NullValueHolder()) {}
Variant::Variant(const char *val): value_(new ValueHolder<std::string>(val)) {}
Variant::Variant(const std::string &val): value_(new ValueHolder<std::string>(val)) {}
Variant::Variant(double val): value_(new ValueHolder<double>(val)) {}
Variant::Variant(bool val): value_(new ValueHolder<bool>(val)) {}
Variant::Variant(const Variant::Object &values): value_(new ObjectValueHolder(values)) {}
Variant::Variant(const Variant::Array &values): value_(new ArrayValueHolder(values)) {}
Variant Variant::fromDictionary(const Dictionary &dict) {
    Variant::Object obj;
    for( const auto &p: dict )
        obj.insert({p.first, p.second});
    return obj;
}

Variant Variant::fromDictionaryAsArray(const Dictionary &dict, const std::string &keyname, const std::string &valname) {
    Variant::Array ar;
    for( const auto &p: dict )
        ar.emplace_back(Variant::Object({{keyname, p.first}, {valname, p.second}}));
    return ar;
}

bool Variant::isObject() const { return value_->type() == IValueHolder::Object; }
bool Variant::isArray() const { return value_->type() == IValueHolder::Array; }
bool Variant::isNull() const { return value_->type() == IValueHolder::Null; }
bool Variant::isValue() const {
    return
            ( value_->type() == IValueHolder::String ||
              value_->type() == IValueHolder::Integer ||
              value_->type() == IValueHolder::Float ||
              value_->type() == IValueHolder::Boolean );
}

bool Variant::isFalse() const {
    return value_->isFalse();
}

std::string Variant::toString() const {
    if ( isValue() ) {
        return value_->toString();
    }
}

std::vector<std::string> Variant::keys() const {
    std::vector<std::string> res;

    if ( !isObject() ) return res;

    boost::shared_ptr<ObjectValueHolder> e = boost::dynamic_pointer_cast<ObjectValueHolder>(value_);
    for( const auto &p: e->values_ )
        res.push_back(p.first);

    return res;
}

size_t Variant::length() const {
    if ( isObject() ) {
        boost::shared_ptr<ObjectValueHolder> e = boost::dynamic_pointer_cast<ObjectValueHolder>(value_);
        return e->values_.size();
    } else if ( isArray() ) {
        boost::shared_ptr<ArrayValueHolder> e = boost::dynamic_pointer_cast<ArrayValueHolder>(value_);
        return e->values_.size();
    }
    else return 0;
}

Variant Variant::at(const std::string &key) const {
    using namespace boost;
    typedef tokenizer<char_separator<char> > tokenizer;
    char_separator<char> sep(".");

    tokenizer tokens(key, sep);

    boost::shared_ptr<IValueHolder> current = value_;
    for ( tokenizer::iterator it = tokens.begin(); it != tokens.end();  ) {
        Variant val = current->fetchKey(*it++);
        if ( val.isNull() ) return val;
        else if ( it != tokens.end() )
            current = val.value_;
        else return val;
    }

    return Variant();
}

Variant Variant::at(uint idx) const { return value_->fetchIndex(idx); }
std::string Variant::toJSON() const {
    std::ostringstream strm;
    value_->toJSON(strm);
    return strm.str();
}
}
}