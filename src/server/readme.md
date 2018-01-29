**Simple HTTP server basing on boost::asio**

Syntax:

```
Server server("<address>", "<port>") ;

Application *service = new Application() ;

server.setHandler(service) ;

DefaultLogger logger("/tmp/logger", true) ;
server.addFilter(new RequestLoggerFilter(logger)) ;

server.run() ;
```
The service `Application` should be an instance of RequestHandler:
```
class Application: public RequestHandler {
public:

    Application() ; // do all one-time initialization here

    // called for every server request. Fill the response object or throw a HttpResponseException
    
    void handle(const Request &req, Response &resp) override ;
};
```

`Request` contains dictionaries of parsed HTTP variables similar to PHP. `SERVER_` contains server related parameters, `GET_` contains query parameters, `POST_` data submitted via POST, `COOKIES_` contains client cookies and `FILE_` all uploaded files. 

The `Response` variables `headers_`, `content_` and `status_` have to be filled in for a valid request. Normally you will use one of the helper functions such as `write(<content_string>, <mime>)` or `encode_file(<file_path>)`. If the request cannot be handled you should throw a `HttpResponseException` using the appropriate status id e.g. `throw HttpResponseException(Response::not_found)`.
