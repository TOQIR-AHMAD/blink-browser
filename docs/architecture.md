# Architecture

Status: Phase 0 (architecture and build system). Only the parts marked
*implemented* exist today; everything else describes where later phases will put
their code.

## Layers

```text
┌──────────────────────────────────────────────┐
│  ui/            QML shell, glass design system│
├──────────────────────────────────────────────┤
│  src/browser/   controllers, tabs, windows    │
│  src/privacy/   profile, cookies, cleanup     │
│  src/network/   request interception, filters │
├──────────────────────────────────────────────┤
│  src/chromium/  Qt WebEngine integration      │
├──────────────────────────────────────────────┤
│  Chromium (via Qt WebEngine)                  │
└──────────────────────────────────────────────┘
```

Rules that hold across all phases:

- The privacy subsystem never depends on the UI. The UI observes it; it does not
  own it (PLAN.md §12).
- Platform-specific code stays isolated so macOS and Linux remain reachable
  (PLAN.md §38). Nothing in `src/` outside a platform directory may call a
  Windows-only API.
- The core libraries under `src/` that do not need Qt stay Qt-free, so they can
  be built and tested with `PB_BUILD_UI=OFF`.

## Chromium integration decision

**Decision: use Qt WebEngine, which embeds Chromium, rather than embedding
Chromium directly (CEF or the Content API) or building Chromium from source.**

Why:

- PLAN.md §3 requires Chromium for rendering, networking, TLS, site isolation
  and sandboxing, and forbids writing a rendering engine. §5 requires Qt 6 /
  QML for the UI. Qt WebEngine is the integration that satisfies both without a
  second UI toolkit or a custom compositor between Chromium and QML.
- It exposes the pieces the privacy architecture needs:
  - off-the-record (in-memory) profiles for RAM-only browsing (Phase 5),
  - a URL request interceptor for tracker/ad blocking (Phase 6),
  - per-site permission requests with an explicit "ask" default (Phase 4/7),
  - certificate-error and security-state signals (Phase 7).
- Building Chromium from source with GN/Ninja would cost tens of gigabytes and
  hours per build for capabilities Qt WebEngine already provides. If a future
  requirement genuinely needs a Chromium patch, the `src/chromium/` layer is the
  only place that would change.

Privacy consequences to handle in later phases (each is a task, not a claim):

- Qt WebEngine ships without Google API keys, so Safe Browsing, translate and
  the other Google-backed services are inactive by default. This must be
  verified with the network audit in Phase 8, not assumed.
- Chromium writes to disk even for off-the-record profiles (GPU shader cache,
  crashpad database, temporary files). Phase 5 identifies each path, points it
  at a per-session temporary directory, and Phase 8 tests that the directory is
  gone after exit. PLAN.md §13 forbids claiming "nothing is ever written to
  disk" until that is verified.
- Qt WebEngine's default profile is disk-backed. The browser must never use the
  default profile; Phase 5 creates the off-the-record profile explicitly.

## Directory map

| Path | Phase | Contents |
| --- | --- | --- |
| `src/app/` | 0 *implemented* | `main.cpp`, application entry point |
| `src/utils/` | 0 *implemented* | privacy-safe logging, shared helpers |
| `src/chromium/` | 1 | Qt WebEngine profile and web-view integration |
| `src/browser/` | 4 | browser, window and navigation controllers |
| `src/tabs/` | 4 | tab model and tab manager |
| `src/permissions/` | 4 | permission manager (default: ask) |
| `src/downloads/` | 4 | download manager |
| `src/privacy/` | 5 | `PrivacyManager`, cookie/storage/cache/session cleanup |
| `src/network/` | 6 | request interception, filter-list engine |
| `src/settings/` | 9 | settings model |
| `ui/` | 0 *implemented* | `Main.qml`; components, pages and theme follow in 2-3 |
| `tests/unit/` | 0 *implemented* | core unit tests |
| `tests/integration/` | 4 | browser lifecycle and navigation |
| `tests/privacy/` | 8 | disk-persistence, cookie, history, cleanup checks |
| `tests/ui/` | 9 | QML tests |

## Logging

`src/utils/logging.h` is the only logging entry point. It exists because
PLAN.md §31 forbids logs that contain full URLs, queries, form values or page
content.

- Release builds redact a URL to its scheme (`https://<redacted>`).
- Developer builds configured with `-DPB_DEV_LOGGING=ON` may keep the host
  (`https://example.com/<redacted>`). Credentials, path, query and fragment are
  dropped at every level, and an unrecognised value becomes `<redacted>` rather
  than being passed through.

## Dependencies

Current: Qt 6.5+ (Core, Gui, Quick) and, from Phase 1, Qt WebEngine. Nothing
else. Tests use plain executables plus CTest so no test framework is needed.
PLAN.md §45 applies before anything is added.

## Network behaviour

As of Phase 0 the application makes no network requests of any kind: there is no
web engine, no update check, no telemetry and no analytics. Phase 8 adds the
audit that keeps this honest as web content arrives.
