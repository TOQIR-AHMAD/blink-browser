# Architecture

All ten phases of `PLAN.md` are implemented. What has and has not been *built
and run* is a separate question, answered under [Build status](#build-status).

## Layers

```text
┌────────────────────────────────────────────────────────────┐
│  ui/            QML: window shell, glass components, pages │
├────────────────────────────────────────────────────────────┤
│  src/app/       BrowserApplication - owns the object graph  │
├───────────────┬──────────────┬─────────────┬───────────────┤
│ src/browser/  │ src/tabs/    │ src/privacy/│ src/settings/ │
│ windows,      │ tabs and the │ manager,    │ model and the │
│ history       │ tab model    │ cleanup     │ opt-in store  │
├───────────────┴──────────────┴─────────────┴───────────────┤
│  src/network/   interception, filter engine, audit          │
│  src/permissions/  policy and the prompt bridge             │
│  src/downloads/    download manager                         │
├────────────────────────────────────────────────────────────┤
│  src/chromium/  Qt WebEngine profile, switches, user agent  │
├────────────────────────────────────────────────────────────┤
│  Chromium, through Qt WebEngine                             │
└────────────────────────────────────────────────────────────┘
```

Rules that hold everywhere:

- **The privacy subsystem does not depend on the UI.** The UI observes it
  (PLAN.md §12). `PrivacyManager` has no idea a window exists.
- **Anything that can be Qt-free is Qt-free.** Every subsystem keeps its
  decisions in a `core/` subdirectory built into `pb_core`, which has no Qt
  dependency and is unit-tested on its own. The Qt classes above them are
  adapters: they translate, they do not decide. This is why the blocking
  engine, the permission policy, the settings model, the omnibox classifier and
  the cleanup sequence all have real tests.
- **Platform-specific code stays isolated** so macOS and Linux remain reachable
  (PLAN.md §38). Nothing outside the build system names a Windows-only API.

## Chromium integration decision

**Decision: Qt WebEngine, which embeds Chromium — not CEF, not the Content API,
not a Chromium fork.**

- PLAN.md §3 requires Chromium for rendering, networking, TLS, site isolation
  and sandboxing; §5 requires Qt 6 and QML for the interface. Qt WebEngine is
  the integration that satisfies both without a second toolkit or a custom
  compositor between them.
- It exposes exactly the hooks the privacy architecture needs: off-the-record
  profiles, a URL request interceptor, per-site permission requests,
  certificate-error signals and download requests.
- Building Chromium from source would cost tens of gigabytes and hours per
  build for capabilities Qt WebEngine already provides. If a future requirement
  needs a Chromium patch, `src/chromium/` is the only layer that changes.

Consequences that are handled rather than assumed:

- Qt WebEngine ships without Google API keys, so Safe Browsing and the other
  Google-backed services are inactive. The network audit exists to verify that
  on a real build rather than take it on faith.
- Chromium writes to disk even for an off-the-record profile (GPU shader cache,
  crash database, temporary files). All of it is redirected into a per-session
  temporary directory that is deleted on exit, and PLAN.md §13's prohibition on
  claiming "nothing is written to disk" is respected in the documentation.
- Qt WebEngine's *default* profile is disk-backed. The browser creates its own
  off-the-record profile in C++ and binds it into QML; the source audit fails
  the build if `defaultProfile` is ever named in `src/` or `ui/`.

## Object graph

`BrowserApplication` builds everything once, in one place, so shutdown order is
explicit:

```text
BrowserApplication
├── SessionPaths            random temp directory for this run
├── BlockingStats           atomic counters
├── SettingsController      settings; writes only if asked
├── WebProfile              the single off-the-record QWebEngineProfile
├── FilterService           lists → FilterEngine (behind a read/write lock)
├── RequestInterceptor      HTTPS-first, blocking, counting  [network thread]
├── PrivacyManager          stats, filters, CleanupSequence
├── PermissionManager       policy bridge for the QML prompt
├── DownloadManager         in-memory download list
└── BrowserController       windows → WindowController → TabModel → Tab
    └── SessionHistory      RAM-only, non-private windows only
```

Shutdown, in order: windows and tabs, downloads, permission answers, detach the
interceptor, then `PrivacyManager::runCleanup()` — a `CleanupSequence` whose
steps are individually retryable, report failures without stopping the ones
after them, and are safe to run twice (PLAN.md §14).

## Threading

Only two threads matter.

- **The UI thread** owns every QObject above.
- **Chromium's network thread** calls `RequestInterceptor::interceptRequest()`
  for every request, and the cookie filter in `WebProfile`. Both touch only
  thread-safe state: `FilterService::match()` takes a read lock, `BlockingStats`
  is atomic, and the HTTPS-upgrade set and the audit are behind a mutex. Signals
  emitted from there reach the UI thread as queued calls, and the dashboard
  coalesces them on a 250 ms timer instead of redrawing per blocked request.

## QML module layout

Four modules, so each type's URI matches where the file lives:

| Module | Directory | Contents |
| --- | --- | --- |
| `PrivacyBrowser.Core` | `src/` | the C++ types and their enums |
| `PrivacyBrowser.Ui` | `ui/` | `Main.qml`, `BrowserWindow.qml` |
| `PrivacyBrowser.Ui.Theme` | `ui/theme/` | colour, type, spacing, radius, shadow singletons |
| `PrivacyBrowser.Ui.Components` | `ui/components/` | the glass components |
| `PrivacyBrowser.Ui.Pages` | `ui/pages/` | new tab, settings, dashboard, first run |

`Main.qml` is not a window: it binds `Theme` to the settings and instantiates
one `BrowserWindow` per window the C++ side reports, so window lifetime lives
in C++ and the QML follows.

## Directory map

| Path | Contents |
| --- | --- |
| `src/app/` | `main.cpp`, `BrowserApplication` |
| `src/utils/` | privacy-safe logging, small string helpers |
| `src/chromium/` | `WebProfile`; `core/` builds the switch list and the reduced user agent |
| `src/browser/` | browser and window controllers; `core/SessionHistory` |
| `src/tabs/` | `Tab`, `TabModel` |
| `src/permissions/` | `PermissionManager`; `core/PermissionPolicy` |
| `src/downloads/` | `DownloadManager`, `DownloadItem` |
| `src/privacy/` | `PrivacyManager`; `core/` session paths, cleanup sequence, stats, shared enums |
| `src/network/` | `FilterService`, `RequestInterceptor`; `core/` filter parser and engine, URL parsing, HTTPS upgrade, audit |
| `src/settings/` | `SettingsController`; `core/` settings model, search providers, omnibox classifier |
| `resources/` | the two built-in filter lists, the fingerprint-protection script |
| `tests/unit/` | 17 Qt-free test binaries |
| `tests/integration/` | tabs, windows, history, omnibox (Qt Test) |
| `tests/privacy/` | the source audit (Python) and the runtime persistence checks (Qt Test) |
| `tests/ui/` | QML tests for the design system |
| `packaging/windows/` | version resource, icon, Inno Setup script |
| `scripts/` | environment check, icon generator, packaging, privacy verification |

## Logging

`src/utils/logging.h` is the only logging entry point, because PLAN.md §31
forbids logs containing URLs, queries, form values or page content.

- Release builds redact a URL to its scheme: `https://<redacted>`.
- `-DPB_DEV_LOGGING=ON` allows the host as well: `https://example.com/<redacted>`.
- Credentials, path, query and fragment are dropped at every level, and
  anything unparsable becomes `<redacted>` rather than passing through.

## Build status

| Target | State |
| --- | --- |
| `pb_core` and the 17 unit tests | **Built and passing**, with MSVC 2017 via the `core-only` preset |
| `tests/privacy/audit_sources.py` | **Passing** — runs in every configuration |
| `pb_browser`, `PrivacyBrowser`, the QML modules | **Not yet compiled**: Qt 6 is not installed on the development machine, and its MSVC 2017 toolset is below Qt 6's floor |
| `tests/integration`, `tests/privacy/tst_persistence`, `tests/ui` | Written; run when Qt is available |

`docs/build-windows.md` lists what to install. Until that build has been run,
the Qt-facing code should be treated as reviewed but unverified — the runtime
API calls against Qt WebEngine in particular.

## Dependencies

Qt 6.8 (Core, Gui, Network, Quick, Quick Controls, WebEngine, Test, QuickTest)
and nothing else. Tests use plain executables plus CTest for the core, and Qt
Test where Qt is needed, so no test framework is vendored. PLAN.md §45 applies
before anything is added.
