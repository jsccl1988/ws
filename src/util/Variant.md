**Simple variant type**

```
Variant v(Variant::Object{
    {"name", 3},
    {"values", Variant::Array{ {2,  "s" } } }
}) ;
cout << v.toJSON() << endl ;
```           
will produce: `{"name": 3, "values": [2, "s"]}`
```
v["name"].toString() -> "3"
v["values"][0].toString() -> "2" ;
v["values"].length() -> "2"
v.isObject() -> true
v.keys() -> { "name", "values" }
```
