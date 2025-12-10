## Indicator String Format


| Type | Format | Example |
|------|--------|---------|
| Icon | i<delimiter><frame><delimiter><hints> | "i/💡/:style=bold" |
| Spinner | s<delimiter><frame><delimiter><frame> ... <delimiter><frame><delimiter><hints> | "s.▱▱▱.▰▱▱.▰▰▱.▰▰▰.:size=12" |



+ The delimiter may be any single-byte character that does not appear in either a frame or the hints string. Choose it accordingly.
+ Indicator appearance can be tweaked using hints, which follow the Fontconfig pattern-string format. Hints are optional.
+ All used character glyphs must be available in the same font. Otherwise, some characters may fail to render.
