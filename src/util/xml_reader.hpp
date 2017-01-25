#ifndef __XML_READER_H__
#define __XML_READER_H__

#include <istream>
#include <map>
#include <sstream>

// Simple and fast XML pull style reader

class XmlReader {
public:
    XmlReader(std::istream &strm) ;
    XmlReader(const std::string &fileName) ;

    ~XmlReader() ;

    enum NodeType
    {
        None,
        Invalid,
        Ignored,
        StartElement,
        EndElement,
        Characters
    };

    /**
     * @brief read next node from stream
     */
    bool read();

    // read until a start element with the given name is encountered (optional) or an error occurs
    bool readNextStartElement(const std::string &name = std::string());

    /**
     * @brief get type of the current node
     */
    NodeType nodeType() const { return current_node_type_ ; }

    /**
     * @brief get the name of the current node (element name or text)
     */
    std::string nodeName() const { return current_node_name_ ; }

    /**
     * @brief get attribute value with given name if tit exists otherwise return the default value
     */
    std::string attribute(const std::string &name, const std::string &val = std::string()) const ;

    /**
     * @brief return the text between start and end element nodes or an empty string in case a start element was found between
     */

    std::string elementText() ;

    bool isWhiteSpace() const ;

    /**
     * @brief skip current element children until end of element tag encountered
     */
    void skipElement();

    bool isStartElement(const std::string &name = std::string()) ;
    bool isEndElement(const std::string &name = std::string()) ;

    const std::map<std::string , std::string> &attributes() const { return attrs_ ; }

    int currentLine() const { return lineno_ ; }

private:

    bool parseCDATA();
    void parseClosingXMLElement();
    void parseOpeningXMLElement();
    void parseComment();
    void ignoreDefinition();
    bool setText(char *start, char *end);
    void parseCurrentNode();

    void flush();
    void advance();

private:
    char *cp_, *be_ ;
    std::string current_node_name_ ;
    NodeType current_node_type_ ;
    char * buf_ ;
    size_t buf_size_ ;
    std::istream *strm_ ;
    FILE *fdesc_ ;
    std::map<std::string, std::string> attrs_ ;
    bool is_empty_elem_ ;
    int lineno_, colno_ ;



};









#endif
