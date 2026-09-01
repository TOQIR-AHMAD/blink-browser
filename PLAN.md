# Privacy-First Browser — Master Development Plan

## 1. Project Vision

Build a modern, privacy-first desktop web browser with:

- Chromium-based web rendering
- Apple-inspired glass/frosted UI
- Extremely minimal interface
- RAM-only browsing sessions
- No user accounts
- No telemetry
- No analytics
- No browsing data sent to our servers
- No persistent browsing history
- No persistent cookies
- No persistent cache
- No saved passwords
- No autofill
- No cloud sync
- No advertising SDKs
- No tracking SDKs
- Built-in ad/tracker blocking
- Strong permission controls
- Automatic cleanup when the browser closes

The browser should feel premium, fast, simple, and privacy-first.

---

# 2. Core Privacy Principle

The browser must follow this rule:

> If data is not required for the browser to function, do not collect it.

The application must not secretly transmit user browsing information to any developer-controlled server.

There should be no:

- Analytics
- Telemetry
- User tracking
- Advertising IDs
- Browser IDs
- Fingerprinting systems created by us
- Remote browsing-history storage
- Cloud profiles
- User behavior tracking
- URL collection
- Search-query collection
- Crash-report uploads containing browsing information

Any optional network feature must be explicitly documented and disabled by default if it could transmit user-related information.

---

# 3. Technology Stack

## Browser Engine

Use:

- Chromium
- Chromium Content API where appropriate

Do NOT implement an HTML/CSS/JavaScript rendering engine from scratch.

Chromium should handle:

- HTML
- CSS
- JavaScript
- Web APIs
- HTTP/HTTPS
- TLS
- WebSockets
- Service workers
- Rendering
- Site isolation
- Sandboxing

---

# 4. Programming Languages

Primary:

- C++

UI:

- QML
- Qt Quick

Supporting:

- Python only for development/build automation where useful
- GN/Ninja for Chromium builds
- CMake for our own native components where practical

Do not introduce unnecessary programming languages.

---

# 5. UI Technology

Use:

- Qt 6
- Qt Quick
- QML

The UI should be custom-built rather than looking like a traditional desktop application.

The visual design should be inspired by modern Apple-style glass interfaces:

- Frosted glass
- Transparency
- Blur
- Rounded corners
- Soft shadows
- Thin borders
- Subtle gradients
- Smooth animations
- Spacious layout
- Minimal controls
- Elegant typography
- Light and dark modes

Do NOT copy Apple's exact proprietary UI assets.

Create an original visual identity.

---

# 6. UI Design Language

Create a reusable design system.

## Glass Surface

Every major floating UI component should support:

- Background transparency
- Backdrop blur
- Rounded corners
- Thin border
- Soft shadow
- Optional gradient
- Hover state
- Pressed state

Example conceptual component:

```text
GlassSurface
├── Background
├── Blur
├── Border
├── Shadow
└── Content
```

---

# 7. Main Browser Window

The browser should contain:

```text
┌──────────────────────────────────────────────────────┐
│                                                      │
│   Tabs                                               │
│  ┌─────────────┐ ┌─────────────┐ ┌─────┐             │
│  │  Example    │ │  Wikipedia  │ │  +  │             │
│  └─────────────┘ └─────────────┘ └─────┘             │
│                                                      │
│  ◀  ▶  ↻    ┌──────────────────────────────────┐     │
│             │ 🔒 example.com                   │     │
│             └──────────────────────────────────┘     │
│                                           ⋯          │
│                                                      │
├──────────────────────────────────────────────────────┤
│                                                      │
│                    WEB CONTENT                       │
│                                                      │
│                                                      │
└──────────────────────────────────────────────────────┘
```

The exact design can evolve.

---

# 8. Browser Features

## Tabs

Implement:

- New tab
- Close tab
- Switch tab
- Duplicate tab
- Reload tab
- Drag tabs
- Reorder tabs
- New tab button
- Middle-click close where supported
- Keyboard shortcuts
- Active-tab indicator
- Loading indicator
- Favicon
- Tab title
- Tab audio indicator
- Tab crash handling

---

# 9. Navigation

Implement:

- Back
- Forward
- Reload
- Stop loading
- Address bar
- URL navigation
- Search from address bar
- Keyboard shortcuts
- Paste and go
- Copy URL
- Open link in new tab
- Open link in new window

---

# 10. Address Bar

The address bar must:

- Accept URLs
- Accept search queries
- Display current URL
- Display security state
- Support keyboard focus
- Select all on shortcut
- Support suggestions without sending unnecessary data to our servers

IMPORTANT:

Do not create a custom remote autocomplete service that collects user queries.

If search suggestions are implemented, make them optional and clearly explain their privacy implications.

Default behavior should prioritize privacy.

---

# 11. New Tab Page

Create a beautiful minimal new-tab experience.

Include:

- Search/address field
- Optional clock
- Optional wallpaper/background
- Privacy status
- Minimal shortcuts

Do not create a server-controlled personalized feed.

No tracking.

No advertising.

---

# 12. Privacy Architecture

Create a dedicated privacy subsystem.

Suggested structure:

```text
src/
├── privacy/
│   ├── PrivacyManager
│   ├── CookieManager
│   ├── StorageManager
│   ├── CacheManager
│   ├── PermissionManager
│   ├── TrackerBlocker
│   ├── FingerprintProtection
│   └── SessionCleaner
```

The privacy system must be separated from the UI.

---

# 13. RAM-Only Browsing

The default browser session must not persist browsing data.

At minimum, investigate and configure:

- Cookies
- HTTP cache
- Local Storage
- IndexedDB
- Service worker storage
- Session storage
- WebSQL where applicable
- File-system APIs
- History
- Download metadata
- Form data
- Password data

Use temporary/in-memory storage where Chromium allows it.

Where Chromium requires disk-backed behavior for a feature, explicitly identify it and implement the safest supported temporary-storage strategy.

Do NOT falsely claim "nothing is ever written to disk" unless this has been verified.

---

# 14. Session Cleanup

When the browser closes:

1. Close all tabs.
2. Destroy browser contexts.
3. Delete temporary profile data.
4. Delete temporary cache.
5. Delete temporary cookies.
6. Delete temporary storage.
7. Delete temporary download metadata.
8. Clear application-generated temporary files.
9. Shut down Chromium cleanly.
10. Verify cleanup in automated tests.

The cleanup system must be idempotent.

If cleanup fails, log only technical information that cannot identify the user's browsing activity.

---

# 15. No Browser-Owned Cloud

Do not create a backend for:

- History
- Bookmarks
- Passwords
- Tabs
- User profiles
- Search queries
- Browsing analytics

The browser should work without logging into anything.

---

# 16. Bookmarks

Default behavior:

Bookmarks are NOT persisted.

If bookmarks are eventually added, provide:

- Temporary session bookmarks
- Optional explicit export
- Optional encrypted local export

Never silently upload bookmarks.

---

# 17. Passwords

Do not implement cloud password storage.

Default:

- No persistent password manager
- No password upload
- No password analytics

If password saving is added later, it must be:

- Explicit
- Local
- Encrypted
- Disabled by default unless the user enables it

---

# 18. History

Default:

```text
History = RAM only
```

History disappears when the browser closes.

There must be no remote history database.

---

# 19. Cookies

Default:

- Session-only cookies
- Clear persistent cookies when session ends
- User-controlled cookie permissions

Provide settings such as:

- Allow cookies
- Block third-party cookies
- Block all cookies
- Clear cookies on exit

---

# 20. Tracking Protection

Build a tracker-blocking system.

Support filter-list based blocking.

Architecture:

```text
Network Request
      │
      ▼
TrackerBlocker
      │
 ┌────┴────┐
 │         │
Block    Allow
 │         │
 X         ▼
       Chromium
```

Potential filter-list compatibility should be investigated rather than reinventing a complete filtering language unnecessarily.

Do not download filter lists from a developer-controlled tracking service.

Provide transparent information about where filter lists come from and how updates work.

---

# 21. Ad Blocking

Implement built-in ad/tracker blocking.

Requirements:

- Efficient
- Low CPU usage
- Low memory overhead
- Easy to update
- Privacy-preserving updates
- No user browsing information sent during filter updates

The blocking engine must not itself become a tracking system.

---

# 22. Fingerprinting Protection

Investigate protections for:

- Canvas fingerprinting
- WebGL fingerprinting
- Audio fingerprinting
- Font enumeration
- Screen information
- User-agent entropy
- Hardware information
- Device memory
- CPU information
- Timezone
- Language information

IMPORTANT:

Do not randomly break APIs without testing.

Privacy protections should prioritize:

1. Preventing unnecessary entropy
2. Maintaining website compatibility
3. Avoiding unique browser identifiers

Use Chromium's supported privacy mechanisms whenever possible.

---

# 23. Permissions

Create a permission manager for:

- Camera
- Microphone
- Location
- Notifications
- Clipboard
- MIDI
- USB
- Bluetooth
- Screen capture
- File access

Default to:

```text
Ask user
```

Never silently grant powerful permissions.

---

# 24. HTTPS / Security

The browser should:

- Prefer HTTPS
- Display certificate/security status
- Respect Chromium TLS validation
- Warn about invalid certificates
- Avoid insecure downgrade behavior where Chromium supports appropriate controls

Do not create your own cryptography.

Use established Chromium/OS cryptographic infrastructure.

---

# 25. DNS

Do not create a proprietary DNS logging service.

Investigate support for:

- System DNS
- Secure DNS / DoH
- User-selected DNS provider

Make provider choice transparent.

If a remote DNS provider is enabled, clearly explain that DNS queries are visible to that provider.

---

# 26. Proxy Support

Support standard:

- HTTP proxy
- HTTPS proxy
- SOCKS proxy

Potential future support:

- User-configured privacy networks

Do not claim that a proxy provides anonymity.

---

# 27. Private Window

Provide an explicit private-window mode.

Private mode should:

- Use temporary browser context
- Avoid persistent cookies
- Avoid persistent history
- Avoid persistent cache
- Clear session storage
- Close temporary data when window closes

---

# 28. Downloads

Downloads are inherently written to disk when the user chooses to save a file.

Therefore:

- Do not pretend downloads are RAM-only.
- Ask where appropriate.
- Do not maintain unnecessary download history.
- Do not upload downloaded files.
- Do not scan/upload filenames to our servers.
- Do not create a cloud download database.

Temporary downloads should be deleted when appropriate.

---

# 29. Telemetry

ABSOLUTELY NO:

```text
Google Analytics
Sentry
Mixpanel
PostHog
Amplitude
Segment
Custom analytics
Custom telemetry
Tracking pixels
User-behavior monitoring
```

Unless explicitly added later with a separate privacy review and explicit user consent.

Default:

```text
Telemetry = OFF
```

Prefer:

```text
Telemetry system = not included
```

---

# 30. Crash Reporting

Do not automatically upload crash reports.

Provide:

```text
Crash occurred.
[Close]
[View technical details]
[Export report]
```

If users export a report, warn them that technical information may contain environment details.

Never automatically upload browsing URLs or page content.

---

# 31. Logging

Application logs must not contain:

- Full URLs
- Search queries
- Cookies
- POST bodies
- Form values
- Passwords
- Page contents
- Personal information

Development logs can be more verbose.

Production logs should be privacy-safe.

---

# 32. Settings

Create a modern glass settings interface.

Categories:

```text
General
Privacy
Security
Appearance
Search
Network
Permissions
Downloads
Advanced
About
```

Privacy page should clearly show:

```text
Tracking protection       ON
Ad blocking               ON
Telemetry                 OFF
History persistence       OFF
Cookie persistence        OFF
Cloud sync                OFF
Crash reporting           OFF
```

---

# 33. Privacy Dashboard

Create a beautiful privacy dashboard.

Show:

- Trackers blocked
- Ads blocked
- Third-party requests blocked
- Cookies blocked
- Permissions granted

IMPORTANT:

These statistics should be stored only in RAM by default.

Do not upload them.

Example:

```text
Privacy Protection

        1,284
    trackers blocked

         842
      ads blocked

          76
    cookies blocked

    Everything stays
       on this device.
```

---

# 34. Design System

Create reusable QML components:

```text
ui/
├── components/
│   ├── GlassSurface.qml
│   ├── GlassButton.qml
│   ├── GlassTextField.qml
│   ├── GlassTab.qml
│   ├── GlassMenu.qml
│   ├── GlassDialog.qml
│   ├── GlassToggle.qml
│   ├── GlassSlider.qml
│   ├── GlassCard.qml
│   ├── AddressBar.qml
│   ├── TabBar.qml
│   ├── NavigationBar.qml
│   └── PrivacyBadge.qml
│
├── pages/
│   ├── NewTab.qml
│   ├── Settings.qml
│   ├── Privacy.qml
│   ├── Security.qml
│   └── About.qml
│
└── theme/
    ├── Theme.qml
    ├── Colors.qml
    ├── Typography.qml
    ├── Spacing.qml
    ├── Radius.qml
    └── Shadows.qml
```

---

# 35. Animation Guidelines

Animations should be subtle.

Use:

- Fade
- Scale
- Slide
- Spring-like transitions
- Blur transitions where practical

Avoid:

- Excessive animation
- Long transitions
- Distracting effects

Target a responsive 60 FPS+ UI where practical.

---

# 36. Keyboard Shortcuts

Implement standard shortcuts.

Examples:

```text
Ctrl/Cmd + L       Focus address bar
Ctrl/Cmd + T       New tab
Ctrl/Cmd + W       Close tab
Ctrl/Cmd + Shift+T Reopen tab
Ctrl/Cmd + R       Reload
Alt + Left         Back
Alt + Right        Forward
Ctrl/Cmd + Shift+P Private window
Ctrl/Cmd + Plus    Zoom in
Ctrl/Cmd + Minus   Zoom out
Ctrl/Cmd + 0       Reset zoom
```

Follow platform conventions.

---

# 37. Accessibility

Support:

- Keyboard navigation
- Screen readers where practical
- High contrast
- Reduced motion
- Adjustable text scaling
- Focus indicators
- Accessible labels

Respect OS accessibility preferences.

---

# 38. Platform Strategy

Initial target:

```text
Windows 11
```

Architecture should allow future:

```text
macOS
Linux
```

Do not tightly couple core browser logic to Windows-only APIs.

Platform-specific code must be isolated.

---

# 39. Project Structure

Use a structure similar to:

```text
privacy-browser/
│
├── PLAN.md
├── README.md
├── LICENSE
├── .gitignore
│
├── src/
│   ├── app/
│   ├── browser/
│   ├── chromium/
│   ├── privacy/
│   ├── network/
│   ├── permissions/
│   ├── downloads/
│   ├── tabs/
│   ├── settings/
│   └── utils/
│
├── ui/
│   ├── components/
│   ├── pages/
│   ├── theme/
│   └── assets/
│
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── privacy/
│   └── ui/
│
├── scripts/
│
├── third_party/
│
├── build/
│
└── docs/
```

---

# 40. Git Strategy

Use Git from the beginning.

Create commits after every meaningful milestone.

Example:

```text
feat: create browser shell
feat: add chromium integration
feat: add tab management
feat: add glass navigation UI
feat: implement temporary browser profile
feat: add privacy manager
feat: add tracker blocking
test: add privacy persistence tests
fix: prevent cookie persistence
```

Never make giant unreviewable commits.

---

# 41. Testing Strategy

Every major feature must have tests.

Test:

- Browser startup
- Browser shutdown
- Tab creation
- Tab destruction
- Navigation
- Back/forward
- Cookies
- Cache
- History
- Local storage
- IndexedDB
- Permissions
- Privacy settings
- Tracker blocking
- Session cleanup
- Crash handling

---

# 42. Privacy Verification

Create automated tests that verify:

```text
After browser closes:

History files = none
Persistent cookies = none
Cache = none
Browser profile = none
Telemetry requests = none
Browsing URLs uploaded = none
User IDs created = none
```

Also inspect network traffic during testing.

The browser must not make unexpected developer-controlled network requests.

---

# 43. Network Audit

Create a development tool/test that records outbound network connections.

Classify connections:

```text
Website request
Browser infrastructure
Filter-list update
DNS
User-selected service
Unknown
```

The browser must never silently contact our own servers with browsing information.

---

# 44. Security Rules for Claude

Claude MUST NOT:

- Invent cryptography
- Implement custom TLS
- Disable Chromium sandboxing without a documented reason
- Disable certificate validation
- Add analytics
- Add telemetry
- Add remote logging
- Add hidden network requests
- Add cloud storage
- Store browsing history
- Store cookies persistently
- Store passwords
- Create hidden browser identifiers
- Collect URLs
- Collect search queries
- Upload crash reports automatically

If a feature conflicts with privacy requirements, STOP and explain the conflict before implementing it.

---

# 45. Dependency Rules

Keep dependencies minimal.

Before adding a dependency:

1. Explain why it is needed.
2. Check its license.
3. Check whether it performs network communication.
4. Check whether it collects telemetry.
5. Check whether it has unnecessary permissions.
6. Prefer mature and well-maintained dependencies.

Do not add random UI libraries when Qt/QML can implement the feature.

---

# 46. Build Requirements

The project must provide documented build instructions.

Create:

```text
docs/build-windows.md
docs/build-macos.md
docs/build-linux.md
```

Initially prioritize Windows.

Document:

- Required SDK
- Compiler
- Qt version
- Chromium version
- Build tools
- Environment variables
- Build commands
- Debug build
- Release build
- Packaging

---

# 47. Release Packaging

Create a proper application package.

Windows target:

```text
PrivacyBrowser.exe
```

Eventually:

```text
Installer
Portable version
```

Do not include unnecessary background services.

The browser should not install a permanent tracking service.

---

# 48. Update System

Do NOT build a custom update service that tracks users.

Future update architecture should support:

- Signed releases
- HTTPS
- Integrity verification
- Version checking without user identification where possible
- Transparent update behavior

Document what information an update check reveals to the update server.

---

# 49. Browser Identity

Do not generate a unique installation ID.

Avoid:

```text
UUID per installation
Machine ID
Advertising ID
Persistent browser ID
```

If Chromium requires identifiers for specific functionality, investigate whether they can be disabled or made ephemeral.

---

# 50. Search Engine

Do not force a developer-controlled search engine.

Allow users to select their preferred search provider.

Possible configuration:

```text
Search Engine

Google
Bing
DuckDuckGo
Brave Search
Startpage
Custom
```

Do not hard-code external services as mandatory.

Search suggestions should be opt-in because search providers can receive queries.

---

# 51. First-Run Experience

First launch should explain:

```text
Welcome.

This browser is designed to keep your
browsing private.

By default:

• No browsing history is saved
• No cookies persist
• No telemetry
• No cloud account
• No advertising ID
• No browsing data is sent to us

You control your browser.
```

Do not manipulate users into enabling tracking.

---

# 52. Privacy Policy

Create a simple privacy policy that accurately describes the implementation.

Do not make claims that the software does not technically guarantee.

Especially avoid absolute claims like:

> "Nobody can ever track you."

Instead describe exactly:

- What the browser stores
- What it does not store
- What network requests it makes
- What websites can still see
- What DNS providers can see
- What proxy providers can see
- What search engines can see

---

# 53. Threat Model

Document protection against:

### Browser-level tracking

Goal:

Strong protection.

### Developer telemetry

Goal:

None by default.

### Persistent local browsing history

Goal:

None by default.

### Third-party trackers

Goal:

Block as much as practical.

### Fingerprinting

Goal:

Reduce fingerprint uniqueness.

### Malicious websites

Goal:

Rely on Chromium sandbox/security architecture.

### Network observers

Goal:

HTTPS protects content, but do not claim complete anonymity.

### ISP/DNS visibility

Goal:

Explain limitations and provide configurable secure DNS/proxy options.

---

# 54. Development Milestones

## Phase 0 — Architecture

Create:

- Repository
- Build system
- Documentation
- Directory structure
- Development environment

Deliverable:

```text
Empty application builds successfully.
```

---

## Phase 1 — Chromium Shell

Implement:

- Chromium integration
- Main window
- Web contents
- Basic navigation

Deliverable:

```text
Can open a website.
```

---

## Phase 2 — Browser UI

Implement:

- Qt/QML shell
- Address bar
- Back
- Forward
- Reload
- Tabs
- New tab

Deliverable:

```text
Usable basic browser.
```

---

## Phase 3 — Glass UI

Implement:

- Glass surfaces
- Blur
- Rounded corners
- Shadows
- Animations
- Dark mode
- Light mode

Deliverable:

```text
Premium glass browser interface.
```

---

## Phase 4 — Browser Architecture

Implement:

- Tab manager
- Window manager
- Navigation controller
- Browser controller
- Download manager
- Permission manager

Deliverable:

```text
Stable browser architecture.
```

---

## Phase 5 — Privacy

Implement:

- RAM-only profile
- Temporary cookies
- Temporary cache
- No history persistence
- Session cleanup
- Permission controls

Deliverable:

```text
Browser leaves no normal browsing profile behind.
```

---

## Phase 6 — Blocking

Implement:

- Tracker blocker
- Ad blocker
- Filter list management
- Blocking dashboard

Deliverable:

```text
Built-in privacy protection.
```

---

## Phase 7 — Security

Implement:

- HTTPS indicators
- Permission controls
- Security UI
- Safe defaults
- Security tests

Deliverable:

```text
Security-focused browser.
```

---

## Phase 8 — Privacy Verification

Implement:

- Network audit
- Disk persistence tests
- Cookie persistence tests
- History tests
- Storage tests
- Telemetry verification

Deliverable:

```text
Automated proof that browser behavior matches privacy requirements.
```

---

## Phase 9 — Polish

Implement:

- Animations
- Accessibility
- Keyboard shortcuts
- Error pages
- Loading states
- Empty states
- Settings
- About page
- Privacy dashboard

Deliverable:

```text
Production-quality browser UI.
```

---

## Phase 10 — Packaging

Implement:

- Release build
- Windows installer
- Portable build
- Version information
- Code signing preparation
- Release documentation

Deliverable:

```text
Installable browser.
```

---

# 55. Claude Execution Rules

Claude should work sequentially.

DO NOT attempt all phases simultaneously.

For each phase:

1. Read PLAN.md.
2. Inspect the current repository.
3. Identify the current phase.
4. Implement only that phase.
5. Build the project.
6. Run tests.
7. Fix build errors.
8. Fix test failures.
9. Review privacy implications.
10. Update documentation.
11. Create a Git commit.
12. Report what was completed.
13. Move to the next phase only when the current phase is stable.

---

# 56. Definition of Done

A feature is NOT complete just because code exists.

A feature is complete only when:

```text
Code exists
+
Build succeeds
+
Tests pass
+
UI works
+
Privacy implications reviewed
+
Documentation updated
+
No unnecessary dependencies
+
No unexpected network activity
```

---

# 57. Critical Rule

Never sacrifice privacy for convenience without explicitly telling the developer.

If Chromium, Qt, an external dependency, or any service requires behavior that conflicts with the privacy architecture:

STOP.

Explain:

1. What the conflict is.
2. What data could be exposed.
3. What alternatives exist.
4. Which option is safest.

Then wait for approval before weakening the privacy model.

---

# 58. Final Product Goal

The finished browser should feel like:

```text
             PREMIUM UI
                  +
              CHROMIUM
                  +
           STRONG PRIVACY
                  +
          RAM-FIRST STORAGE
                  +
          TRACKER BLOCKING
                  +
          NO DEVELOPER TRACKING
```

The user should feel:

> "This browser belongs to me. It isn't watching me."

---

# 59. Start Here

Claude should NOT immediately implement everything.

First perform:

```text
PHASE 0 — ARCHITECTURE
```

Then:

- inspect the development machine
- determine OS
- verify compiler/toolchain
- verify Qt availability
- determine Chromium integration strategy
- create repository structure
- create build system
- create minimal executable
- build it successfully
- document setup

After Phase 0 is complete, proceed to Phase 1.

Before making major architectural changes, explain the decision and its privacy implications.

# END OF PLAN