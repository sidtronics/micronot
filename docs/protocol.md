# Micronot Protocol Specification

This document defines a lightweight, line-based text protocol used to communicate with the unotd over a unix domain socket.
All messages consist of lines of UTF-8 text and are terminated by a blank line (two consecutive newline characters: `\n\n`).

---

## Command Framing

* Each command ends with a blank line (`\n\n`).
* Fields are newline-separated.

---

## Indicator String Format


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
| `typ` | Indicator type (msg or spn)|
| `nme` | Name of Indicator (from config) |
| `tfn` | Message font (Fontconfig pattern string) |
| `ifg` | Indicator foreground color (`RRGGBB` hex) |
| `tfg` | Message foreground color (`RRGGBB` hex) |
| `tim` | Timeout in seconds (integer) | 

---

## Command Format

```
COMMAND\n
field1:value\n
field2:value\n
.
.
.
fieldN:value\n
\n
```

## Command: NTF

Open a notification. Will return a <nid> if spinner notification.

### Format

```
NTF
<txt>
<ind> | <nme>
<typ>
[tfn]
[ifg]
[tfg]
[tim]

```

### Response

```
OK
[nid]

```

Or:

```
ERROR

```

---

## Command: RET

Close the spinner notification with window id <nid>. 0 indicates success. 1 indicates fail.

### Format

```
RET
<nid>
<ret>

```

### Response

```
OK

```

Or:

```
ERROR

```
