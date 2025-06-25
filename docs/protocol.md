
## Definitions:

delimiter: <d>
spinner: <d><frame-1><d><frame-2><d><frame-3><d>....<frame-n><d><d><success><d><fail><d><d><hints>
icon: <d><icon><d><d><hints>
nme: <spinner-name> (spinner name defined in config)
mfn: <message-font> (fontconfig pattern string)
ifg: <RRGGBB> (icon fg color)
mfg: <RRGGBB> (message fg color)
tmo: <seconds> (timeout)

```
MSG\n
<message>\n
[icon]\n
[mfn:]\n
[ifg:]\n
[mfg:]\n
[tmo:]\n
\n

returns:
OK/ERROR\n
```

```
SPN\n
<message>\n
<[nme:]|[spinner]>\n
[mfn:]\n
[ifg:]\n
[mfg:]\n
[tmo:]\n
\n

returns:
<wid>\n
OK/ERROR\n
```

```
RET\n
<wid>\n
0/1\n
\n

returns:
OK/ERROR\n
```
