#ifndef __GPX_PARSER_HPP__
#define __GPX_PARSER_HPP__

#include <iostream>
#include <wspp/util/xml_sax_parser.hpp>

#include "route_geometry.hpp"
using wspp::util::XMLSAXParser ;

struct RouteGeometry ;

class GpxParser: public XMLSAXParser {
public:
    GpxParser(std::istream &strm, RouteGeometry &geom);

    virtual void startElement(const std::string &qname, const AttributeList &attrs) override ;

    virtual void endElement(const std::string &qname) override;

    virtual void characters(const std::string &text) override;

  //  virtual void error(ErrorCode code, uint line, uint column) override {}

private:

    enum Context { WayPointContext, TrackContext, TrackSegmentContext, TrackPointContext, DocumentContext } ;

    RouteGeometry &geom_ ;
    std::string text_ ;
    bool is_gpx_document_ = false ;
    Waypoint current_wpt_ ;
    TrackPoint current_tpt_ ;
    TrackSegment current_seg_ ;
    Track current_track_ ;
    std::stack<Context> ctx_ ;
};







#endif
