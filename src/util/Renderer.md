### Template rendering engine

The TemplateRender class supports rendering of Mustache templates:

```
TemplateRenderer engine(std::shared_ptr<TemplateLoader>(new FileSystemTemplateLoader(
    {{root + "/templates/"}, {root + "/templates/bootstrap-partials/"}}
));
```
The above initializes the renderer by specifing a loader that will load templates from the filesystem (by default .mst is appended
to each key) looking at the given folders.
```
Variant::Object ctx{{"name", "john"}}
string result = engine.render("template", ctx) ;
```
The above will render the template file "template.mst" using the context variable "ctx".
Alternatively:
```
Variant::Object ctx{{"name", "John"}}
string result = engine.renderString("Hello {{name}}!", ctx) ;
```
will produce "Hello John!.
Custom loaders may be created by overriding the TemplateLoader class (e.g. to render in-memory templates). Partials are also loaded
using the defined loader.

### Extended Mustache syntax

The syntax of Mustache is extended in the following manner.

#### Inheritance

Child template may inherit from parent templates using block substitution.

```
{{!layout.mst}}
<html>
  <head>
  {{> head }}
  {{$extra_headers}}
  {{/extra_headers}}
  </head>

  <body>
    <div class="container">
       {{$contents}}
       {{/contents}}
    </div>

    <script  src="https://code.jquery.com/jquery-3.2.1.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
       
    {{$extra_footers}}
    {{/extra_footers}}
  </body>
</html>

{{!page.mst}}
{{<layout }}

{{$contents}}
  <ul class="nav nav-tabs">
    <li class="active"><a href="#edit" data-toggle="tab">Page</a></li>
    <li><a href="#preview" data-toggle="tab">Preview</a></li>
  </ul>
           
  <div class="tab-content clearfix" style="margin-top: 10px">
     <div class="tab-pane active" id="edit">
        <form id="edit-content" action="/page/publish/" method="POST">
           <div class="form-group">
              <textarea class="mce-editable form-control" name="content" id="content">{{{content}}}</textarea>
           </div>
           <p class="form-control-static" id="message"></p>
           <button type="submit" class="btn btn-default">Publish</button>
        </form> 
     </div>
		   		
		 <div class="tab-pane" id="preview"></div>
  </div>
{{/contents}}

{{$extra_footers}}
  <script language="javascript" type="text/javascript" src="js/tinymce/js/tinymce/tinymce.min.js"></script>
{{/extra_footers}}        
```
Rendering of "page.mst" will result in replacing "contents" and "extra_footers" blocks in "layout.mst"

### Variable substitution

We support complex keys of the form `{{person.info.first_name}}`. The tag `{{.}}` is substituted with the current array element.

### Block Helpers

Block helpers are definded globally using the "addBlockHelper" function. In the Mustache template helpers are called as:
```
{{# helper_name(arg1, "arg2", arg3="val" ...) }}
... 
{{/ helper_name }}
```
or alternatively 
```
{{# helper_name arg1 "arg2" arg3="val" }}
... 
{{/ helper_name }}
```
If the helper with the name "helper_name" has been defined it will be called by passing an array of arguments. Specifically, 
literals such as arg2 are passed as is, variables (e.g. arg1) are evaluated first using the current context and the result 
is passed. Key-value pairs are pushed in the current context before calling the function and popped afterwards.

The helper lambda has the following form 
```
 [&](const string &src, ContextStack &ctx, Variant::Array params) -> string { ...  } 
```
"src" is the contents of the block unrenderer and "ctx" is the current context. "params" will contain the list of params. The function should assure that the correct value of parameters has been provided. Normally 
the lambda should capture an instance of the TemplateRenderer which gives access to the render function "renderString". For example:
```
engine.registerBlockHelper("b", [&](const string &src, ContextStack &ctx, Variant::Array params) {
      return "<bold>" + engine.renderString(src, ctx) + "</bold>" ;
}) ;
```
will wrap the text enclosed in a `{{#bold}}` section. 
