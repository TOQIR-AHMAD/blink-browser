#!/usr/bin/env python3
"""Source-level privacy audit (PLAN.md 29, 43, 44).

This is the check that does not need a running browser: it reads the source of
src/ and ui/ and fails if the code has grown something the privacy architecture
forbids. It runs as part of the normal test suite, including on a machine
without Qt.

What it enforces:

1. No analytics, telemetry or crash-reporting SDK is referenced anywhere.
2. Every network endpoint written into the source is on the allowlist below,
   with a reason. A new endpoint fails this test until somebody adds it here,
   which is the point: adding one should be a deliberate, reviewed act.
3. No plain-http endpoint is hard-coded.
4. The switches that would disable Chromium's sandbox, site isolation or
   certificate validation appear only where they are being refused.
5. Qt WebEngine's disk-backed default profile is never referenced, and no code
   asks for persistent cookies.
6. No QSettings: the only thing this browser may write is the settings file it
   writes itself, deliberately, when asked.

Usage: python tests/privacy/audit_sources.py [repository root]
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

# Endpoints allowed to appear in the source, and why. Anything else fails.
ALLOWED_HOSTS = {
    # Search providers the user can choose between (PLAN.md 50). None of them
    # is contacted unless the user searches.
    "duckduckgo.com": "search provider, user-selectable",
    "www.startpage.com": "search provider, user-selectable",
    "search.brave.com": "search provider, user-selectable",
    "www.google.com": "search provider, user-selectable",
    "suggestqueries.google.com": "search suggestions, opt-in only",
    "www.bing.com": "search provider, user-selectable",
    "api.bing.com": "search suggestions, opt-in only",
    # Placeholder text and examples shown in the UI; never requested.
    "example.com": "placeholder in help text",
    "resolver.example": "placeholder in the DNS settings field",
    "my.example": "placeholder in the custom search field",
}

FORBIDDEN_SDKS = [
    "google-analytics.com/analytics.js",
    "googletagmanager.com/gtag",
    "sentry-sdk",
    "sentry.init",
    "@sentry/",
    "mixpanel.init",
    "posthog.init",
    "amplitude.getInstance",
    "analytics.track(",
    "firebase",
    "crashlytics",
    "appcenter",
    "bugsnag.start",
    "datadogRum",
]

FORBIDDEN_SWITCHES = [
    "--no-sandbox",
    "--disable-web-security",
    "--ignore-certificate-errors",
    "--disable-gpu-sandbox",
    "--disable-setuid-sandbox",
    "--disable-site-isolation-trials",
]

FORBIDDEN_API = [
    ("defaultProfile", "Qt WebEngine's default profile is disk-backed"),
    ("AllowPersistentCookies", "cookies must not survive the session"),
    ("ForcePersistentCookies", "cookies must not survive the session"),
    ("QSettings", "settings are written by SettingsController alone, when asked"),
    ("DiskHttpCache", "the HTTP cache must stay in memory"),
]

# Files allowed to name the forbidden switches, because refusing them is their
# job.
SWITCH_EXEMPT = {
    Path("src/chromium/core/chromium_flags.cpp"),
    Path("src/chromium/core/chromium_flags.h"),
}

SCANNED_DIRECTORIES = ("src", "ui")
SCANNED_SUFFIXES = {".cpp", ".h", ".qml", ".js"}

URL_PATTERN = re.compile(r"(https?)://([A-Za-z0-9._-]+)")


def strip_comments(text: str) -> str:
    """Removes // and /* */ comments.

    The audit is about what the code does, and a comment that explains why the
    browser refuses to use something (Qt WebEngine's default profile, say)
    should not read as using it.
    """
    out: list[str] = []
    i = 0
    end = len(text)
    while i < end:
        char = text[i]
        if char == "/" and i + 1 < end and text[i + 1] == "/":
            newline = text.find("\n", i)
            i = end if newline < 0 else newline
        elif char == "/" and i + 1 < end and text[i + 1] == "*":
            close = text.find("*/", i + 2)
            i = end if close < 0 else close + 2
        else:
            out.append(char)
            i += 1
    return "".join(out)


def source_files(root: Path) -> list[Path]:
    files: list[Path] = []
    for directory in SCANNED_DIRECTORIES:
        for path in sorted((root / directory).rglob("*")):
            if path.suffix in SCANNED_SUFFIXES and path.is_file():
                files.append(path)
    return files


def main() -> int:
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[2]
    failures: list[str] = []
    scanned = 0

    for path in source_files(root):
        scanned += 1
        relative = path.relative_to(root)
        text = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
        lowered = text.lower()

        for marker in FORBIDDEN_SDKS:
            if marker.lower() in lowered:
                failures.append(f"{relative}: references '{marker}' (PLAN.md 29)")

        if relative not in SWITCH_EXEMPT:
            for switch in FORBIDDEN_SWITCHES:
                if switch in text:
                    failures.append(f"{relative}: names '{switch}' (PLAN.md 44)")

        for symbol, reason in FORBIDDEN_API:
            if symbol in text:
                failures.append(f"{relative}: uses '{symbol}' - {reason}")

        for scheme, host in URL_PATTERN.findall(text):
            if scheme == "http" and host not in ("localhost", "127.0.0.1"):
                failures.append(f"{relative}: hard-codes a plain-http endpoint '{host}'")
            elif scheme == "https" and host not in ALLOWED_HOSTS:
                failures.append(
                    f"{relative}: endpoint '{host}' is not on the allowlist in "
                    f"tests/privacy/audit_sources.py"
                )

    # The environment variable that carries Chromium's switches must be set in
    # exactly one place, from buildFlags().
    application = (root / "src/app/application.cpp").read_text(encoding="utf-8")
    if application.count("QTWEBENGINE_CHROMIUM_FLAGS") != 1:
        failures.append(
            "src/app/application.cpp: Chromium's switch list must be assembled in one place"
        )
    if "buildFlags(" not in application:
        failures.append("src/app/application.cpp: switches must come from buildFlags()")

    print(f"privacy source audit: {scanned} files scanned")
    if failures:
        for failure in failures:
            print(f"FAIL {failure}")
        print(f"{len(failures)} problem(s) found")
        return 1

    print("no forbidden endpoint, SDK or switch found")
    return 0


if __name__ == "__main__":
    sys.exit(main())
