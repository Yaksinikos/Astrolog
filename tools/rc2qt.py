#!/usr/bin/env python3
"""
Astrolog (Version 8.00) File: tools/rc2qt.py

Generate rcqt.inc for the Qt version of Astrolog (#define QT) from the
Windows resource script astrolog.rc and resource.h, so the Windows resources
stay the single source of truth for the Qt user interface.

Usage:  python3 tools/rc2qt.py [astrolog.rc] [resource.h] [rcqt.inc]

The output is plain C data (no Qt types), included by wqt.cpp.

Currently emitted:
  - rgQtAccel[]: the keyboard accelerator table (ACCELERATORS resource).
  - rgQtMenu[]: the main menu bar and all context menus (MENU resources).
  - rgQtDlg[]: all dialog templates (DIALOG and DIALOGEX resources).
"""

import re
import sys

VK_NAMES = {
    "VK_BACK": 0x08, "VK_TAB": 0x09, "VK_RETURN": 0x0D, "VK_PAUSE": 0x13,
    "VK_ESCAPE": 0x1B, "VK_PRIOR": 0x21, "VK_NEXT": 0x22, "VK_END": 0x23,
    "VK_HOME": 0x24, "VK_LEFT": 0x25, "VK_UP": 0x26, "VK_RIGHT": 0x27,
    "VK_DOWN": 0x28, "VK_OEM_PLUS": 0xBB, "VK_OEM_COMMA": 0xBC,
    "VK_OEM_MINUS": 0xBD, "VK_OEM_PERIOD": 0xBE, "VK_OEM_3": 0xC0,
    "VK_OEM_4": 0xDB, "VK_OEM_5": 0xDC, "VK_OEM_6": 0xDD,
}
for _i in range(1, 25):
    VK_NAMES["VK_F%d" % _i] = 0x6F + _i

# Flag bits, mirrored by the QTACCEL_* constants in wqt.cpp.
F_VIRTKEY, F_SHIFT, F_CONTROL, F_ALT = 1, 2, 4, 8


def read_text(path):
    with open(path, "rb") as f:
        data = f.read()
    for enc in ("utf-8", "cp1252", "latin-1"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            continue
    return data.decode("latin-1")


def load_ids(path):
    ids = {"IDOK": 1, "IDCANCEL": 2, "IDC_STATIC": -1}
    for line in read_text(path).splitlines():
        m = re.match(r"\s*#define\s+(\w+)\s+(-?\w+)", line)
        if m:
            try:
                ids[m.group(1)] = int(m.group(2), 0)
            except ValueError:
                pass
    return ids


def resolve(tok, ids):
    tok = tok.strip()
    if tok in ids:
        return ids[tok]
    return int(tok, 0)


def split_fields(line):
    """Split a resource statement on commas, honoring quoted strings."""
    out, cur, inq, i = [], "", False, 0
    while i < len(line):
        c = line[i]
        if inq:
            cur += c
            if c == '"':
                if i + 1 < len(line) and line[i + 1] == '"':
                    cur += '"'
                    i += 1
                else:
                    inq = False
        elif c == '"':
            inq = True
            cur += c
        elif c == ",":
            out.append(cur.strip())
            cur = ""
        else:
            cur += c
        i += 1
    if cur.strip():
        out.append(cur.strip())
    return out


def strip_comment(line):
    """Remove a // comment from a line, ignoring // within strings."""
    inq = False
    for i, c in enumerate(line):
        if c == '"':
            inq = not inq
        elif c == "/" and not inq and line[i + 1:i + 2] == "/":
            return line[:i]
    return line


def unquote(s):
    s = s.strip()
    assert s[0] == '"' and s[-1] == '"', s
    s = s[1:-1].replace('""', '"')
    return s.replace("\\\\", "\\").replace('\\"', '"')


def parse_accelerators(lines, ids):
    start = None
    for i, line in enumerate(lines):
        if re.match(r"\s*accelerator\s+ACCELERATORS\b", line):
            start = i
            break
    if start is None:
        raise SystemExit("ACCELERATORS resource not found")
    accels = []
    i = start + 1
    while not re.match(r"\s*BEGIN\b", lines[i]):
        i += 1
    i += 1
    while not re.match(r"\s*END\b", lines[i]):
        line = strip_comment(lines[i]).strip()
        i += 1
        if not line:
            continue
        f = split_fields(line)
        key, cmd, opts = f[0], f[1], [o.strip().upper() for o in f[2:]]
        flags = 0
        if "VIRTKEY" in opts:
            flags |= F_VIRTKEY
        if "SHIFT" in opts:
            flags |= F_SHIFT
        if "CONTROL" in opts:
            flags |= F_CONTROL
        if "ALT" in opts:
            flags |= F_ALT
        if key.startswith('"'):
            s = unquote(key)
            if s.startswith("^") and len(s) == 2:
                code, flags = ord(s[1].upper()) - 64, flags  # Ctrl+letter.
            else:
                assert len(s) == 1, key
                code = ord(s)
                if flags & F_VIRTKEY:
                    code = ord(s.upper())
        elif key in VK_NAMES:
            code = VK_NAMES[key]
        else:
            code = int(key, 0)
        accels.append((code, flags, resolve(cmd, ids), cmd))
    return accels


def decode_rc_string(tok):
    """Decode a quoted resource script string to a Python string."""
    tok = tok.strip()
    assert tok[0] == '"' and tok[-1] == '"', tok
    s, out, i = tok[1:-1], "", 0
    while i < len(s):
        c = s[i]
        if c == '"' and i + 1 < len(s) and s[i + 1] == '"':
            out += '"'
            i += 2
        elif c == "\\" and i + 1 < len(s):
            n = s[i + 1]
            out += {"t": "\t", "n": "\n", "\\": "\\", '"': '"'}.get(n, n)
            i += 2
        else:
            out += c
            i += 1
    return out


def c_string(s):
    """Encode a Python string as a C string literal holding UTF-8."""
    out = '"'
    for b in s.encode("utf-8"):
        c = chr(b)
        if c == "\\":
            out += "\\\\"
        elif c == '"':
            out += '\\"'
        elif c == "\t":
            out += "\\t"
        elif c == "\n":
            out += "\\n"
        elif 32 <= b < 127:
            out += c
        else:
            out += "\\%03o" % b
    return out + '"'


# Menu entry kinds, mirrored by the QTMENU_* constants in wqt.cpp.
M_ITEM, M_SEP, M_POPUP, M_END = 0, 1, 2, 3


def parse_menus(lines, ids):
    """Return a list of (menu id name, menu id, [(kind, cmd, cmdname, text)])
    for every MENU resource, in file order."""
    menus = []
    i = 0
    while i < len(lines):
        m = re.match(r"\s*(\w+)\s+MENU\b", lines[i])
        if not m:
            i += 1
            continue
        name = m.group(1)
        i += 1
        while not re.match(r"\s*BEGIN\b", lines[i]):
            i += 1
        i += 1
        depth, items = 1, []
        while depth > 0:
            line = lines[i].strip()
            i += 1
            if not line or line.startswith("//"):
                continue
            if re.match(r"BEGIN\b", line):
                depth += 1
            elif re.match(r"END\b", line):
                depth -= 1
                if depth > 0:
                    items.append((M_END, 0, "", ""))
            elif re.match(r"MENUITEM\s+SEPARATOR\b", line):
                items.append((M_SEP, 0, "", ""))
            elif line.startswith("POPUP"):
                f = split_fields(line[len("POPUP"):])
                items.append((M_POPUP, 0, "", decode_rc_string(f[0])))
            elif line.startswith("MENUITEM"):
                f = split_fields(line[len("MENUITEM"):])
                items.append((M_ITEM, resolve(f[1], ids), f[1].strip(),
                              decode_rc_string(f[0])))
            else:
                raise SystemExit("Unexpected menu line: %s" % line)
        menus.append((name, resolve(name, ids), items))
    return menus


# Dialog entry kinds and flags, mirrored by QTDLG_* constants in wqt.cpp.
D_KINDS = ["DIALOG", "LTEXT", "RTEXT", "CTEXT", "EDIT", "COMBO", "LIST",
           "BUTTON", "DEFBUTTON", "GROUPBOX", "ICON", "CHECK", "RADIO",
           "STATIC"]
DF_GROUP, DF_TABSTOP, DF_SORT, DF_DISABLED, DF_HIDDEN, DF_DROPLIST = (
    1, 2, 4, 8, 16, 32)
DF_READONLY, DF_MULTILINE, DF_NUMBER, DF_PASSWORD = 64, 128, 256, 512

CONTROL_KEYWORDS = ("CONTROL", "LTEXT", "RTEXT", "CTEXT", "EDITTEXT",
                    "COMBOBOX", "LISTBOX", "PUSHBUTTON", "DEFPUSHBUTTON",
                    "GROUPBOX", "ICON", "CHECKBOX", "AUTOCHECKBOX",
                    "RADIOBUTTON", "AUTORADIOBUTTON", "END")


def parse_style(s):
    """Return the set of style names in a style expression, and the set of
    names removed with NOT."""
    on, off = set(), set()
    for m in re.finditer(r"(NOT\s+)?(\w+)", s or ""):
        (off if m.group(1) else on).add(m.group(2))
    return on, off


def parse_icons(lines):
    icons = {}
    for line in lines:
        m = re.match(r'\s*(\w+)\s+ICON\s+(?:\w+\s+)*"([^"]+)"', line)
        if m:
            icons[m.group(1)] = m.group(2)
    return icons


def parse_dialogs(lines, ids, icons):
    """Return a list of (name, id, caption, cx, cy, [controls]), where each
    control is (kind, id, idname, x, y, cx, cy, flags, text)."""
    dlgs = []
    i = 0
    while i < len(lines):
        m = re.match(r"\s*(\w+)\s+DIALOG(?:EX)?\s+(.*)", lines[i])
        if not m:
            i += 1
            continue
        name = m.group(1)
        hdr = [int(v) for v in split_fields(m.group(2))[:4]]
        caption = ""
        i += 1
        while not re.match(r"\s*BEGIN\b", lines[i]):
            mc = re.match(r'\s*CAPTION\s+(".*")', lines[i])
            if mc:
                caption = decode_rc_string(mc.group(1))
            i += 1
        i += 1

        # Join statements continued onto following lines.
        stmts = []
        while True:
            line = strip_comment(lines[i]).strip()
            i += 1
            if not line:
                continue
            word = line.split()[0].rstrip(",")
            if word in CONTROL_KEYWORDS or not stmts:
                stmts.append(line)
            else:
                stmts[-1] += " " + line
            if word == "END":
                break

        ctls = []
        for st in stmts:
            word = st.split()[0]
            if word == "END":
                break
            f = split_fields(st[len(word):])
            text, style = "", ""
            if word == "CONTROL":
                text, idn, cls, style = f[0], f[1], f[2], f[3]
                x, y, cx, cy = f[4:8]
                on, off = parse_style(style)
                cls = decode_rc_string(cls).upper()
                if cls == "BUTTON":
                    if "BS_AUTOCHECKBOX" in on or "BS_CHECKBOX" in on:
                        kind = "CHECK"
                    elif "BS_AUTORADIOBUTTON" in on or "BS_RADIOBUTTON" in on:
                        kind = "RADIO"
                    elif "BS_GROUPBOX" in on:
                        kind = "GROUPBOX"
                    elif "BS_DEFPUSHBUTTON" in on:
                        kind = "DEFBUTTON"
                    else:
                        kind = "BUTTON"
                elif cls == "STATIC":
                    kind = "STATIC"
                elif cls == "EDIT":
                    kind = "EDIT"
                elif cls == "COMBOBOX":
                    kind = "COMBO"
                elif cls == "LISTBOX":
                    kind = "LIST"
                else:
                    raise SystemExit("Unknown control class %s in %s" %
                                     (cls, name))
                text = decode_rc_string(text) if text.startswith('"') else ""
            elif word in ("EDITTEXT", "COMBOBOX", "LISTBOX"):
                idn, x, y, cx, cy = f[0:5]
                style = f[5] if len(f) > 5 else ""
                kind = {"EDITTEXT": "EDIT", "COMBOBOX": "COMBO",
                        "LISTBOX": "LIST"}[word]
            elif word == "ICON":
                text = icons.get(f[0].strip(), "")
                idn, x, y = f[1:4]
                cx, cy = (f[4], f[5]) if len(f) > 5 else ("0", "0")
                kind = "ICON"
            else:
                text = decode_rc_string(f[0])
                idn, x, y, cx, cy = f[1:6]
                style = f[6] if len(f) > 6 else ""
                kind = {"LTEXT": "LTEXT", "RTEXT": "RTEXT", "CTEXT": "CTEXT",
                        "PUSHBUTTON": "BUTTON", "DEFPUSHBUTTON": "DEFBUTTON",
                        "GROUPBOX": "GROUPBOX", "CHECKBOX": "CHECK",
                        "AUTOCHECKBOX": "CHECK", "RADIOBUTTON": "RADIO",
                        "AUTORADIOBUTTON": "RADIO"}[word]
            on, off = parse_style(style)
            flags = 0
            if ("WS_GROUP" in on or kind in ("LTEXT", "RTEXT", "CTEXT")) \
                    and "WS_GROUP" not in off:
                flags |= DF_GROUP
            if "WS_TABSTOP" in on:
                flags |= DF_TABSTOP
            if "CBS_SORT" in on or "LBS_SORT" in on:
                flags |= DF_SORT
            if "WS_DISABLED" in on:
                flags |= DF_DISABLED
            if "WS_VISIBLE" in off:
                flags |= DF_HIDDEN
            if "CBS_DROPDOWNLIST" in on:
                flags |= DF_DROPLIST
            if "ES_READONLY" in on:
                flags |= DF_READONLY
            if "ES_MULTILINE" in on:
                flags |= DF_MULTILINE
            if "ES_NUMBER" in on:
                flags |= DF_NUMBER
            if "ES_PASSWORD" in on:
                flags |= DF_PASSWORD
            idv = resolve(idn, ids)
            ctls.append((kind, idv, idn.strip(), int(x), int(y), int(cx),
                         int(cy), flags, text))
        dlgs.append((name, resolve(name, ids), caption, hdr[2], hdr[3],
                     ctls))
    return dlgs


def emit(out, accels, menus, dlgs):
    w = out.write
    w("// Generated by tools/rc2qt.py from astrolog.rc and resource.h.\n")
    w("// Do not edit by hand: change astrolog.rc and rerun 'make rcqt'.\n\n")
    w("static const QtAccel rgQtAccel[] = {\n")
    for code, flags, cmd, name in accels:
        w("  {0x%02X, %d, %d},  // %s\n" % (code, flags, cmd, name))
    w("  {0, 0, 0}};\n\n")
    w("// Menus: {menu id, kind, command id, text}. Each menu's entries end\n")
    w("// with a QTMENU_END entry for each POPUP, and the table with id 0.\n\n")
    w("static const QtMenuEntry rgQtMenu[] = {\n")
    for name, mid, items in menus:
        w("  // %s\n" % name)
        for kind, cmd, cname, text in items:
            if kind == M_ITEM:
                w("  {%d, QTMENU_ITEM, %d, %s},  // %s\n" %
                  (mid, cmd, c_string(text), cname))
            elif kind == M_SEP:
                w("  {%d, QTMENU_SEP, 0, NULL},\n" % mid)
            elif kind == M_POPUP:
                w("  {%d, QTMENU_POPUP, 0, %s},\n" % (mid, c_string(text)))
            else:
                w("  {%d, QTMENU_END, 0, NULL},\n" % mid)
    w("  {0, 0, 0, NULL}};\n\n")
    w("// Dialogs: {dialog id, kind, control id, x, y, cx, cy, flags, text},\n")
    w("// in dialog units. Each dialog starts with a QTDLG_DIALOG entry with\n")
    w("// its size and caption, followed by its controls in tab order. The\n")
    w("// table ends with id 0. For icons, the text is the .ico file name.\n\n")
    w("static const QtDlgEntry rgQtDlg[] = {\n")
    for name, did, caption, cx, cy, ctls in dlgs:
        w("  // %s\n" % name)
        w("  {%d, QTDLG_DIALOG, 0, 0, 0, %d, %d, 0, %s},\n" %
          (did, cx, cy, c_string(caption)))
        for kind, cid, cname, x, y, ccx, ccy, flags, text in ctls:
            w("  {%d, QTDLG_%s, %d, %d, %d, %d, %d, %d, %s},  // %s\n" %
              (did, kind, cid, x, y, ccx, ccy, flags, c_string(text), cname))
    w("  {0, 0, 0, 0, 0, 0, 0, 0, NULL}};\n")


def main(argv):
    rc = argv[1] if len(argv) > 1 else "astrolog.rc"
    rh = argv[2] if len(argv) > 2 else "resource.h"
    dst = argv[3] if len(argv) > 3 else "rcqt.inc"
    ids = load_ids(rh)
    lines = read_text(rc).splitlines()
    accels = parse_accelerators(lines, ids)
    menus = parse_menus(lines, ids)
    dlgs = parse_dialogs(lines, ids, parse_icons(lines))
    with open(dst, "w", newline="\r\n") as out:  # CRLF like the sources.
        emit(out, accels, menus, dlgs)
    print("%s: %d accelerators, %d menus, %d menu entries, %d dialogs, "
          "%d controls" % (dst, len(accels), len(menus),
                           sum(len(m[2]) for m in menus), len(dlgs),
                           sum(len(d[5]) for d in dlgs)))


if __name__ == "__main__":
    main(sys.argv)
