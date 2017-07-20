#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <regex>
#include <map>

namespace wspp {
// A class of key/value pairs of strings. 

class Dictionary: public std::map<std::string, std::string>
{
	public:

	Dictionary() ;
	~Dictionary() {}

	// add a key/val pair 
    void add(const std::string &key, const std::string &val) ;
	// remove entry with given key if exists
    void remove(const std::string &key) ;
    void removeSome(const std::regex &rx) ;

	// remove all items
	void clear() ;
		
	// get a the value of the given key if exists. Otherwise return defaultValue

    std::string get(const std::string &key, const std::string &defaultVal = std::string()) const ;

	// check the existance of a key

    bool contains(const std::string &key) const;

	// get a list of the keys in the dictionary

    std::vector<std::string> keys() const ;
    std::vector<std::string> keys(const std::regex &rx) const ;

	// get values 

    std::vector<std::string> values() const ;
    std::vector<std::string> values(const std::regex &key) const ;

	// number of entries

	int count() const ;
    int count(const std::regex &kx) const ;
    int count(const std::string &) const ;

    uint64_t capacity() const ;

	bool empty() const ;

    void dump() const ;

    std::string serialize(const char *sep) ;

} ;

class DictionaryIterator
{
	public:

    DictionaryIterator(const Dictionary &dic): dict_(dic), it_(dic.begin()) {}
    DictionaryIterator(const DictionaryIterator &other): dict_(other.dict_), it_(other.it_) {}

    bool operator == (const DictionaryIterator &other) const { return it_ == other.it_ ; }
    bool operator != (const DictionaryIterator &other) const { return it_ != other.it_ ; }

    operator int () const { return it_ != dict_.end() ; }
	
    DictionaryIterator & operator++() { ++it_ ; return *this ; }
    DictionaryIterator operator++(int) { DictionaryIterator tmp(*this) ; ++it_; return tmp ; }

    std::string key() const { return (*it_).first ; }
    std::string value() const { return (*it_).second ; }

	private:

    const Dictionary &dict_ ;
    Dictionary::const_iterator it_ ;
} ;

}

#endif
