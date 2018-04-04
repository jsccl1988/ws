### Twig Template rendering engine

The TemplateRender class supports rendering of Twig/Nunjunks/Jinja2 templates:

```
TemplateRenderer engine(std::shared_ptr<TemplateLoader>(new FileSystemTemplateLoader(
    {{root + "/templates/"}, {root + "/templates/bootstrap-partials/"}}
));
```
The above initializes the renderer by specifing a loader that will load templates from the filesystem (by default .twig is appended
to each key) looking at the given folders.
```
Variant::Object ctx{{"name", "john"}}
string result = engine.render("template", ctx) ;
```
The above will render the template file "template.twig" using the context variable "ctx".
Alternatively:
```
Variant::Object ctx{{"name", "John"}}
string result = engine.renderString("Hello {{name}}!", ctx) ;
```
will produce "Hello John!.
Custom loaders may be created by overriding the TemplateLoader class (e.g. to render in-memory templates). Included and imported templates are also loaded using the defined loader.

### Twig implementation state

The following tags have been implemented:
  - autoescape
  - block
  - extends
  - filter
  - for
  - from
  - if
  - import
  - include
  - macro
  - set
  - verbatim
  - with
  
The following filters have been implemented so far:
   - batch
   - escape (html only)
   - first
   - join
   - last
   - length
   - lower
   - raw/safe
   - upper 
   - render (render a template string using current context)
   
### Extending

You may define custom filters/functions by accesing the global FunctionFactory:
```
 FunctionFactory::instance().registerFunction("user_function", [&](const Variant &args, TemplateEvalContext &ctx) -> Variant {
            Variant::Array unpacked ;
            unpack_args(args, { "str", "option?" }, unpacked) ;
            ...
	    return ...
        }) ;
```
Each function/filter is passed an object containing an array "args" with the positional arguments and an object "kw" with the passed keyword arguments key/value pairs. Use function unpack_args to map them to a list of args with given names. The '?' at the end means that the arg is optional. This will throw an exception if not all required arguments are provided. The missing optional arguments are marked as "undefined" values.

Similarly one can pass function objects in the context variable before calling render. e.g.
```
renderer.render('template', { { "my_function": Variant::Function([&](const Variant &args, TemplateEvalContext &ctx) -> Variant {
          ...
        }) 
}}) ;
```
Which can then be called from the template as ``` {{ my_function(args...) }}```.
