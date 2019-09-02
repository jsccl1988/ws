#include "template_parser.hpp"

using namespace std;

namespace wspp {
namespace web {
namespace detail {
string Node::escape(const string &src) {
    string buffer;
    for ( char c: src ) {
        switch(c) {
        case '&':  buffer.append("&amp;");       break;
        case '\"': buffer.append("&quot;");      break;
        case '\'': buffer.append("&apos;");      break;
        case '<':  buffer.append("&lt;");        break;
        case '>':  buffer.append("&gt;");        break;
        default:   buffer.push_back(c);          break;
        }
    }

    return buffer;
}

bool Parser::parseComplexTag(const string &tag, string &name, vector<Arg> &args) {
    static boost::regex rx_args(R"%((\w+)\s*=\s*"([^"]*)|(\w+)|"([^"]*)")%");
    static boost::regex rx_name(R"(^\s*([^\s\()]+))");

    boost::smatch tmatch;
    if ( !boost::regex_search(tag, tmatch, rx_name) ) return false;
    name = tmatch[1];

    std::string::const_iterator start = tmatch[0].second;
    boost::sregex_iterator it(start, tag.end(), rx_args), end;

    while ( it != end ) {

        string key = (*it)[1];
        string val = (*it)[2];
        string param = (*it)[3];
        string literal = (*it)[4];
        if ( !key.empty() )
            args.emplace_back(key, val, true);
        else if ( !param.empty() )
            args.emplace_back(string(), param, false);
        else if ( !literal.empty() )
            args.emplace_back(string(), literal, true);

        ++ it;
    }

    return true;
}

bool Parser::parseSimpleTag(const string &tag, string &name) {
    name = boost::trim_copy(tag);
    return !name.empty();
}

bool Parser::expect(Position &cursor, char c) {
    if ( cursor ) {
        if ( *cursor == c ) {
            ++cursor;
            return true;
        } else return false;
    } else return false;
}

string Parser::eatTag(Position &cursor) {
    string res;
    while ( cursor ) {
        if ( expect(cursor, '}') ) {
            if ( expect(cursor, '}') ) {
                return res;
            }
            else if ( cursor ) {
                res.push_back(*cursor);
                ++cursor;
            }
        }
        else if ( cursor ) {
            res.push_back(*cursor);
            ++cursor;
        }
    }
}

void Parser::parseTag(Position &cursor, Tag &tag) {
    if ( !cursor ) return;

    switch ( *cursor ) {
    case '#':  tag.type_ = Tag::SectionBegin; ++cursor; break;
    case '/':  tag.type_ = Tag::SectionEnd; ++cursor; break;
    case '^':  tag.type_ = Tag::InvertedSectionBegin; ++cursor; break;
    case '&':  tag.type_ = Tag::RawSubstitutionAmpersand; ++cursor; break;
    case '{':  tag.type_ = Tag::RawSubstitutionCurlyBracket; ++cursor; break;
    case '!':  tag.type_ = Tag::Comment; ++cursor; break;
    case '>':  tag.type_ = Tag::Partial; ++cursor; break;
    case '<':  tag.type_ = Tag::Extension; ++cursor; break;
    case '$':  tag.type_ = Tag::Block; ++cursor; break;
    default: tag.type_ = Tag::EscapedSubstitution; break;
    }

    // eat comment
    if ( tag.type_ == Tag::Comment ) {
        uint level = 0;
        while ( cursor ) {
            if ( expect(cursor, '}') ) {
                if ( expect(cursor, '}') ) {
                    if ( level == 0 ) break;
                    else level --;
                }
                else ++cursor;
            }
            else if ( expect(cursor, '{') ) {
                if ( expect(cursor, '{'))
                    level++;
                else ++cursor;
            }
            else ++cursor;
        }

        return;
    }

    while ( cursor ) {
        if ( expect(cursor, '}') ) {
            if ( expect(cursor, '}') ) {
                if ( tag.type_ == Tag::RawSubstitutionCurlyBracket )
                    expect(cursor, '}'); // just eat the character otherwise we silently continue
                return;
            }
            else if ( cursor ) {
                //  if ( !isspace(src[idx_]) )
                tag.name_.push_back(*cursor);
                ++cursor;
            }
        }
        else if ( cursor ) {
            //      if ( !isspace(src[idx_]) )
            tag.name_.push_back(*cursor);
            ++cursor;
        }
    }
}

bool Parser::nextTag(Position &cursor, string &raw, Tag &tag, std::string::const_iterator &ecursor) {
    while ( cursor ) {
        if ( expect(cursor, '{') ) {
            if ( expect(cursor, '{') ) {

                ecursor = cursor.cursor_ - 2;
                parseTag(cursor, tag);

                if ( tag.type_ == Tag::Comment ) continue;
                return true;
            }
            else --cursor;
        }

        if ( cursor ) {
            raw.push_back(*cursor);
            ++cursor;
        }
    }

    return false;
}

SectionNode::Ptr Parser::parse(const string &key) {
    static Cache g_cache;

    if ( key.empty() ) return nullptr;

    if ( ctx_.caching_ ) {
        auto stored = g_cache.fetch(key);
        if ( stored ) return stored;
    }

    std::string src = ctx_.loader_->load(key);

    if  ( src.empty() ) return nullptr;

    auto compiled = parseString(src);
    if ( ctx_.caching_ ) g_cache.add(key, compiled);

    return compiled;
}

SectionNode::Ptr Parser::parseString(const string &src) {

    SectionNode::Ptr root(new SectionNode("$root"));

    deque<ContainerNode::Ptr> stack;

    stack.push_back(root);

    Position cursor(src);

    string::const_iterator s_cursor = src.begin(), e_cursor;
    BlockHelperNode::Ptr helper_node;

    while (!stack.empty()) {
        ContainerNode::Ptr parent = stack.back();

        string raw;
        Tag tag;
        bool res = nextTag(cursor, raw, tag, e_cursor) ;

        string name;
        vector<Arg> args;

        if ( helper_node ) { // just skip until we found a valid closing tag
            if ( tag.type_ == Tag::SectionEnd ) {
                if ( parseSimpleTag(tag.name_, name)) {
                    if ( name == helper_node->key_ ) {
                        helper_node->content_.assign(s_cursor, e_cursor);
                        helper_node = nullptr;
                    }
                }
            }
        } else {
            if ( !raw.empty() ) {
                parent->children_.push_back(std::make_shared<RawTextNode>(raw));
            }

            if ( tag.type_ == Tag::SectionBegin  ) {
                if ( parseComplexTag(tag.name_, name, args)) {
                    auto it = ctx_.block_helpers_.find(name);
                    if ( it != ctx_.block_helpers_.end()) { // if it is a registered helper
                        helper_node.reset(new BlockHelperNode(name, args, it->second));
                        parent->children_.push_back(helper_node);
                        s_cursor = cursor.cursor_;
                    }
                    else {
                        SectionNode::Ptr new_section(new SectionNode(name));
                        parent->children_.push_back(new_section);
                        stack.push_back(new_section);
                    }
                }
            } else if ( tag.type_ == Tag::InvertedSectionBegin  ) {
                if ( parseSimpleTag(tag.name_, name)) {
                    SectionNode::Ptr new_section(new SectionNode(name, true));
                    parent->children_.push_back(new_section);
                    stack.push_back(new_section);
                }
            } else if ( tag.type_ == Tag::SectionEnd ) {
                if ( parseSimpleTag(tag.name_, name)) {
                    if ( name != parent->name_ ) return nullptr;
                    else stack.pop_back();
                }
            } else if ( tag.type_ == Tag::RawSubstitutionAmpersand || tag.type_ == Tag::RawSubstitutionCurlyBracket ) {
                if ( parseComplexTag(tag.name_, name, args)) {
                        auto it = ctx_.value_helpers_.find(name);
                        if ( it != ctx_.value_helpers_.end()) { // if it is a registered helper
                           parent->children_.push_back(std::make_shared<ValueHelperNode>(name, args, it->second, false));
                        } else
                            parent->children_.push_back(std::make_shared<SubstitutionNode>(name, false));
                }
            } else if ( tag.type_ == Tag::EscapedSubstitution ) {
                if ( parseComplexTag(tag.name_, name, args)) {
                    auto it = ctx_.value_helpers_.find(name);
                    if ( it != ctx_.value_helpers_.end()) { // if it is a registered helper
                       parent->children_.push_back(std::make_shared<ValueHelperNode>(name, args, it->second, true));
                    } else
                        parent->children_.push_back(std::make_shared<SubstitutionNode>(name, true));
                }

            } else if ( tag.type_ == Tag::Partial ) {
                if ( parseComplexTag(tag.name_, name, args) )
                    parent->children_.push_back(std::make_shared<PartialNode>(name, args, ctx_));
            } else if ( tag.type_ == Tag::Extension ) {
                if ( parseComplexTag(tag.name_, name, args) ) {
                    ExtensionNode::Ptr new_section(new ExtensionNode(name, args, ctx_));
                    parent->children_.push_back(new_section);
                    stack.push_back(new_section);
                }
            } else if ( tag.type_ == Tag::Block ) {
                if ( parseSimpleTag(tag.name_, name)) {
                    BlockNode::Ptr new_block(new BlockNode(name));
                    parent->children_.push_back(new_block);
                    parent->blocks_.insert({name, new_block});
                    stack.push_back(new_block);
                }
            }
        }

        if ( !res ) stack.pop_back();
    }

    return root;
}

Variant::Object Parser::getDictionaryArgs(const std::vector<Parser::Arg> &args, ContextStack &stack) {
    Variant::Object res;

    for( auto &a: args ) {
        if ( a.key_.empty() ) continue;
        std::string value = ( !a.is_literal_ ) ? stack.find(a.val_).toString() : a.val_;
        res.insert({a.key_, value});
    }
    return res;
}

Variant::Array Parser::getParams(const std::vector<Parser::Arg> &args, ContextStack &ctx) {
    Variant::Array res;

    for( auto &a: args ) {
        if ( !a.key_.empty() ) continue;
        if ( a.is_literal_ ) res.emplace_back(a.val_);
        else res.emplace_back(ctx.find(a.val_));
    }
    return res;
}
} // namespace detail
} // namespace web
} // namespace wspp