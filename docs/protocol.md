# Micronot Protocol Specification

This document defines a lightweight, line-based text protocol used to communicate with the unotd over a unix domain socket.
All messages consist of lines of UTF-8 text and are terminated by a blank line (two consecutive newline characters: `\n\n`).

---

## Message Framing

* Each command ends with a blank line (`\n\n`).
* Fields are newline-separated.
* No escaping is used.

---

## Indicator Format

| Term             | Format                                                                   | Example                                       |
| ---------------- | ------------------------------------------------------------------------ | --------------------------------------------- |
| `delimiter`      | `<d>`                                                                    | `\|`, `$`, `.`, `#`, `:`                      |
| `UTF8 character` | `<chr>`                                                                  | `A`, `🌍`, ``, `▰`                           |
| `hints`          | `<hints>`                                                                | `:style=Bold:size=10`                         |
| `frame`          | `<chr><chr>...<chr>`                                                     | `▱▱▱`, `▰▱▱`, `▰▰▱`                           |
| `success`        | `<chr><chr>...<chr>`                                                     | ``, ``, ``, `OK`                           |
| `fail`           | `<chr><chr>...<chr>`                                                     | ``, ``, ``, `ERR`                          |
| `icon`           | `<d><chr><chr>...<chr><d><d>[hints]`                                     | ``$▰▱▱$$:size=8``                             |
| `spinner`        | `<d><frame><d><frame>...<d><frame><d><d><success><d><fail><d><d>[hints]` | ``\|\|\|\|\|\|\|\|\|\|\|:style=Bold`` |

---

## Field Definitions


| Field | Description |                                                                                            
|-------|-------------|
| `txt` | Notification text |
| `ind` | Indicator string |
| `nme` | Name of spinner (from config) |
| `mfn` | Message font (Fontconfig pattern string) |
| `ifg` | Icon foreground color (`RRGGBB` hex) |
| `mfg` | Message foreground color (`RRGGBB` hex) |
| `tmo` | Timeout in seconds (integer) | 

---

## Command: MSG

Open a notification with icon.

### Format

```
MSG
<txt>
<ind>
[mfn]
[ifg]
[mfg]
[tmo]

```

### Response

```
OK

```

Or:

```
ERROR

```

---

## Command: SPN

Open a notification with spinner. Returns the window id of opened notification. Must be closed with RET command.

### Format

```
SPN
<txt>
<ind>|<nme> 
[mfn]
[ifg]
[mfg]
[tmo]

```

### Response

```
OK
wid:<number>

```

Or:

```
ERROR

```

---

## Command: RET

Close the spinner notification with window id <wid>. 0 indicates success. 1 indicates fail.

### Format

```
RET
wid
0|1

```

### Response

```
OK

```

Or:

```
ERROR

```

---

## Notes

* Communication is UTF-8 encoded.
* Messages end with a blank line.
* More commands may be added later.
