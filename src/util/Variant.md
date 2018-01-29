```
Variant v(Variant::Object{
    {"name", 3},
    {"values", Variant::Array{ {2,  "s" } } }
}) ;
cout << v.toJSON() << endl ;
```           
