## Indicator String Format

### Raw indicator string

```
<delimiter><frame><delimiter><frame><delimiter> ... <delimiter><hints>
```

+ Frame is a group of UTF-8 characters rendered at a time.
+ Empty frames are not valid.
+ The delimiter may be any single-byte character that does not appear in either a frame or the hints string. Choose it accordingly.
+ Indicator appearance can be tweaked using hints, which follow the Fontconfig pattern-string format. Hints are optional.
+ All used character glyphs must be available in the same font. Otherwise, some characters may fail to render.
+ Atleast one properly delimited frame must be present for Indicator string to be valid.

### Examples
```
|💡|
$-$/$|$\$
.[-].[\].[|].[/].
|OK|monospace:size=8
|||||||:style=Bold
|🌍|🌎|🌏|:size=10
|▱▱▱|▰▱▱|▰▰▱|▰▰▰|▰▰▱|▰▱▱|▱▱▱|
```

### Named indicator string

```
<delimiter><delimiter><name><delimiter><delimiter>
```

+ Use this format to use indicators saved in the config file.
+ Name cannot be empty.

### Examples
```
||bulb||
||ascii||
..bar..
$$globe$$
```

