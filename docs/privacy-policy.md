# Privacy policy

This describes what the software does, not what we would like it to do. Where
something has not been verified by running it, the document says so.

Last reviewed against the code at version 0.1.0.

## The short version

The browser has no backend. There is no account, no sync, no telemetry, no
crash uploader and no update check — not disabled, not implemented. The people
who wrote this software receive nothing from it, because there is nowhere for
it to send anything.

## What the browser stores

| Data | Where | How long |
| --- | --- | --- |
| History of pages you visit | Memory | Until you close the browser |
| Cookies | Memory (off-the-record Chromium profile) | Until you close the browser |
| HTTP cache | Memory | Until you close the browser |
| Local storage, IndexedDB, service workers | Memory | Until you close the browser |
| Answers to permission prompts | Memory | Until you close the browser |
| Blocking counters shown on the dashboard | Memory | Until you close the browser |
| GPU shader cache, Chromium temporary files, crash database | A randomly named folder under your system temporary directory | Deleted when the browser closes |
| Files you download | Wherever you chose to save them | Yours — the browser does not delete them |
| Settings | Nowhere, unless you turn on "Remember settings"; then a single JSON file in your user configuration folder | Until you turn the option off, which deletes the file |

Qt itself would otherwise keep two caches of its own - compiled QML, and a
graphics pipeline cache - in the application's configuration directory, between
runs. Neither contains browsing data, but they are not on the list above, so
the browser turns both off at start-up. Running it and then checking that no
configuration directory appears is part of `scripts/verify-privacy.ps1`.

The browser stores no passwords, no autofill data, no bookmarks, no form
history and no per-installation identifier.

## What the browser does not do

- No telemetry, analytics or usage measurement.
- No crash reports are uploaded. If Chromium writes a crash dump it goes in the
  session folder and is deleted with it.
- No account, no cloud, no sync.
- No advertising identifier, no installation identifier, no machine
  fingerprinting of you by us.
- No update check, so the browser never announces its existence to anyone.
- No search-query collection: what you search for goes to the search provider
  you chose, and nowhere else.

## What network requests the browser makes

Only these, and every one of them is caused by something you did:

1. **The pages you visit**, and the resources those pages ask for — minus what
   the blocker refuses.
2. **A search**, when you type something that is not an address. It goes to the
   provider selected in Settings › Search (DuckDuckGo unless you change it).
3. **Search suggestions**, *only* if you turn them on. They are off by default
   precisely because they send what you type before you press Enter.
4. **A filter-list update**, only for a list you added yourself from a URL, and
   only when you press the update button (or turn on the start-up check). The
   request carries no cookies and no identifiers; the list's host sees your IP
   address and the time, as it would for any HTTPS request.
5. **DNS**, through your operating system's resolver by default, or through the
   DNS-over-HTTPS provider you configure.

The two filter lists that ship with the browser are inside the executable and
are never fetched.

## What other parties can still see

A privacy-first browser does not make you invisible. Specifically:

- **Websites** see your IP address, the contents of your requests, and whatever
  you type into them. They can still recognise you if you log in. They can
  still fingerprint the browser to some degree; the Standard and Strict
  fingerprinting settings reduce the entropy available, they do not remove it.
- **Your DNS provider** (your ISP by default) sees every hostname you look up.
  Turning on DNS-over-HTTPS moves that visibility to the resolver you chose —
  it does not remove it.
- **Your network operator and ISP** see which servers you connect to and how
  much data flows, even over HTTPS.
- **A proxy** you configure sees everything your browser sends through it. A
  proxy is not anonymity.
- **Your search provider** sees your searches, and can associate them with your
  IP address.
- **Anyone with access to your computer** can see files you downloaded, and, if
  you turned it on, your settings file.

## Verification status

- The blocking engine, URL handling, permission policy, settings model, session
  cleanup and the privacy-safe logging are covered by unit tests that run on
  every build.
- `tests/privacy/audit_sources.py` fails the build if the source ever grows an
  analytics SDK, a hard-coded endpoint that is not on its allowlist, a
  plain-http endpoint, a sandbox-disabling switch, a reference to Chromium's
  disk-backed default profile, or a persistent-cookie setting.
- `tests/privacy/tst_persistence.cpp` checks at runtime that the profile is
  off-the-record, that the session folder is created under the system temporary
  directory and removed on exit, and that no settings file appears unless you
  asked for one.
- `scripts/verify-privacy.ps1` compares your profile folders before and after a
  real browsing session and reports anything left behind.
- `tests/ui/check_qml_bindings.py` checks that every C++ member the interface
  reads actually exists, so a renamed property cannot silently blank out a
  privacy indicator.

**What has actually been run.** The unit tests, the integration tests, the QML
tests, the source audit and the binding check all pass, and the application has
been built and started: it creates its session directory under the system
temporary directory and, on a normal close, exits having removed it, leaving
nothing behind.

**What has not.** That build was made without Qt WebEngine (see
`docs/build-windows.md`), so every claim on this page that depends on Chromium
— the off-the-record profile, the in-memory cookie jar and cache, request
blocking, HTTPS-first, permission prompts and downloads — describes what the
code is written to do and has not yet been measured. `tst_persistence` and
`scripts/verify-privacy.ps1` are the checks that will measure it, and they need
the MSVC build. The claims about what is *not implemented* — no telemetry, no
accounts, no update check — are verified by the source audit, which does run.

## Changes

Any change to this document should accompany the code change that made it
necessary, in the same commit.
