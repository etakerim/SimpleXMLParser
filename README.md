# SimpleXMLParser
There are a lot of good quality C libraries for parsing content of XML files
(e.g. libxml2), but when you need something dumb and basic you can use this
library. Be warned, that it was designed with learning opportunity in mind, so
do not expect it to be ideal. However, when not supported format is supplied you
get an parse error. 

### Supported XML syntax
This tag illustrates all of valid syntax:
```xml
<tag key="value" key='value'>Paragraph</tag>
```
Self contained tags are also permitted:
```xml
<selfcontained />
```

### Implementation data model
This is **not a validator**! You are not able to supply DTD nor xml-schema. It
is purely based on *tree data structure*.
