#!/usr/bin/env python3
"""Checks that every C++ member the QML uses actually exists.

QML resolves properties at run time, so a renamed C++ property does not break
the build - it breaks the browser, quietly, in whichever window happens to use
it. This script closes that gap for the object chains whose type is known
statically (App and everything hanging off it, the window controller, the tab
model and a tab), by reading Q_PROPERTY, Q_INVOKABLE, Q_SIGNALS and public
methods out of the headers and comparing them with what the QML asks for.

It is not a QML type checker. It is the specific check that catches the
specific mistake: a member that no longer exists.

Usage: python tests/ui/check_qml_bindings.py [repository root]
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

# QML expression prefix -> the C++ header and class it resolves to.
CHAINS = [
    ("App.settings.", "src/settings/settings_controller.h", "SettingsController"),
    ("App.privacy.filters.", "src/network/filter_service.h", "FilterService"),
    ("App.privacy.", "src/privacy/privacy_manager.h", "PrivacyManager"),
    ("App.permissions.", "src/permissions/permission_manager.h", "PermissionManager"),
    ("App.downloads.", "src/downloads/download_manager.h", "DownloadManager"),
    ("App.browser.", "src/browser/browser_controller.h", "BrowserController"),
    ("App.", "src/app/application.h", "BrowserApplication"),
]

# Identifiers whose type is known from how the QML is wired, and the header
# that defines them.
TYPED_IDENTIFIERS = [
    (r"(?:root\.|browserWindow\.)?controller\.", "src/browser/window_controller.h",
     "WindowController"),
    (r"(?:root\.|browserWindow\.|window\.)?tabs\.", "src/tabs/tab_model.h", "TabModel"),
    (r"(?:view\.tab|tabDelegate\.tab|control\.tabData|root\.tab|browserWindow\.currentTab|"
     r"currentTab)\.", "src/tabs/tab.h", "Tab"),
    (r"row\.download\.", "src/downloads/download_manager.h", "DownloadItem"),
]

# Members every QObject has, plus what QML adds to any object.
UNIVERSAL = {
    "objectName", "destroy", "toString", "parent", "children", "data", "count",
    "length", "valueOf", "hasOwnProperty",
}

# The name is whatever sits just before READ or MEMBER, after any pointer or
# reference sigil - "QWebEngineProfile *profile READ ..." included.
PROPERTY_RE = re.compile(r"Q_PROPERTY\s*\([^()]*?[\s*&](\w+)\s+(?:READ|MEMBER)")
INVOKABLE_RE = re.compile(r"Q_INVOKABLE[^;{]*?[\s*&](\w+)\s*\(")
METHOD_RE = re.compile(r"^\s*(?:virtual\s+)?[\w:<>*&\s]+?[\s*&](\w+)\s*\([^;]*\)\s*(?:const\s*)?;",
                       re.M)
ENUM_VALUE_RE = re.compile(r"^\s*(\w+)\s*(?:=\s*[^,]+)?,?\s*$", re.M)
SIGNAL_BLOCK_RE = re.compile(r"Q_SIGNALS:(.*?)(?:\n\s*(?:public|private|protected)[\s:]|\Z)",
                             re.S)


def members_of(header_text: str, class_name: str) -> set[str]:
    """Every member name QML could legitimately use on this class."""
    start = header_text.find("class " + class_name)
    if start < 0:
        return set()
    # Stop at the next class declaration in the same header.
    following = header_text.find("\nclass ", start + 1)
    body = header_text[start:] if following < 0 else header_text[start:following]

    names: set[str] = set()
    names.update(PROPERTY_RE.findall(body))
    names.update(INVOKABLE_RE.findall(body))
    names.update(METHOD_RE.findall(body))

    # Signals become onSignalName handlers, and are also callable.
    for block in SIGNAL_BLOCK_RE.findall(body):
        for signal in METHOD_RE.findall(block):
            names.add(signal)
            names.add("on" + signal[0].upper() + signal[1:])

    # Enum values, which QML reaches as Type.Value.
    for enum_body in re.findall(r"enum\s+\w+\s*\{(.*?)\}", body, re.S):
        names.update(ENUM_VALUE_RE.findall(enum_body))

    # A property called "x" also gets an "xChanged" signal handler.
    for name in list(names):
        names.add(name + "Changed")
        names.add("on" + name[0].upper() + name[1:] + "Changed")

    return names


def strip_comments(text: str) -> str:
    out: list[str] = []
    i = 0
    end = len(text)
    while i < end:
        if text[i] == "/" and i + 1 < end and text[i + 1] == "/":
            newline = text.find("\n", i)
            i = end if newline < 0 else newline
        elif text[i] == "/" and i + 1 < end and text[i + 1] == "*":
            close = text.find("*/", i + 2)
            i = end if close < 0 else close + 2
        else:
            out.append(text[i])
            i += 1
    return "".join(out)


def main() -> int:
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[2]
    failures: list[str] = []
    checked = 0

    cache: dict[str, set[str]] = {}

    def members(header: str, class_name: str) -> set[str]:
        key = header + "::" + class_name
        if key not in cache:
            path = root / header
            if not path.exists():
                failures.append(f"missing header {header}")
                cache[key] = set()
            else:
                found = members_of(path.read_text(encoding="utf-8"), class_name)
                if not found:
                    failures.append(f"could not read members of {class_name} in {header}")
                cache[key] = found
        return cache[key]

    for qml in sorted((root / "ui").rglob("*.qml")):
        text = strip_comments(qml.read_text(encoding="utf-8"))
        relative = qml.relative_to(root)

        for prefix, header, class_name in CHAINS:
            for match in re.finditer(re.escape(prefix) + r"(\w+)", text):
                member = match.group(1)
                checked += 1
                if member in UNIVERSAL:
                    continue
                if member not in members(header, class_name):
                    failures.append(
                        f"{relative}: {prefix}{member} - {class_name} has no such member")

        for pattern, header, class_name in TYPED_IDENTIFIERS:
            for match in re.finditer(pattern + r"(\w+)", text):
                member = match.group(1)
                checked += 1
                if member in UNIVERSAL:
                    continue
                if member not in members(header, class_name):
                    failures.append(
                        f"{relative}: {class_name}.{member} - no such member")

    print(f"qml/C++ binding check: {checked} references checked")
    if failures:
        for failure in sorted(set(failures)):
            print(f"FAIL {failure}")
        print(f"{len(set(failures))} problem(s) found")
        return 1

    print("every referenced C++ member exists")
    return 0


if __name__ == "__main__":
    sys.exit(main())
