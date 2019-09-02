
# Routing

Request routing is performed by matching request url to a routing pattern. This is performed by the Route class that parses the url pattern and performs the matching with an input url.
```C++
Route user("/user/{id:\d+}/{action:show|hide}/");

Dictionary params;
bool res = user.matches("/user/2/show/", params);
```
The above will return `true` and set `params["id"] = "2"`, `params["action"] = "show"`.

The route pattern is of the form `/<pat1>/<pat2>/<pat3> ... /<patn>/` where each sub-pattern has the format
`({[<param key>][:<regex>]}|<characters>)[?]`. Use a `?` after a sub-pattern to indicate that the element (and the subsequent elements) is optional. Leading and trailing slashes are ignored.

For convenience the `Request` class defines a `matches()` function that will check if the request method matches and the request path matches a Route pattern:
```
Request req;
Dictionary params;
UserController user;

if ( req.matches("GET", "/user/{id:\d+}/show/", params) ) user.show(params["id"]);
```

Reverse routing is performed by calling `url()` member function of a route passing a dictionary of key/value pairs which should correspond to the named route parameters.
