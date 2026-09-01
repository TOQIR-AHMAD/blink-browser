# Threat model

What this browser defends against, what it does not, and why (PLAN.md §53).

## Developer telemetry

**Goal: none, by construction.**

There is no analytics, no telemetry, no crash upload and no update check, and
no server belonging to this project for any of them to talk to. This is
enforced three ways: the settings model has no key for any of it, the network
audit's list of developer-controlled hosts is empty and a unit test fails if it
is not, and the source audit fails the build if an SDK or an unlisted endpoint
appears anywhere in `src/` or `ui/`.

**Residual risk:** a dependency could contact something on its own. Qt WebEngine
ships without Google API keys, so Safe Browsing and the other Google services
are inactive, and the Chromium command line turns off background networking,
the component updater and domain reliability. The network audit
(`-DPB_NETWORK_AUDIT=ON`) exists to check this on a real build rather than
assume it.

## Persistent local browsing data

**Goal: none by default.**

Cookies, cache, storage and history live in an off-the-record Chromium profile,
which keeps them in memory. History is a bounded in-process list. Everything
Chromium insists on writing goes into a randomly named session folder that is
deleted on exit, and cleanup is idempotent and retried.

**Residual risk:** an operating system can page memory to disk, and a crash can
leave the session folder behind until the next cleanup. The browser does not
encrypt memory and does not claim that nothing ever touches a disk block.

## Third-party trackers and advertising

**Goal: block as much as is practical without breaking the web.**

Requests are matched against Adblock-Plus-syntax filter lists before the
connection is made, third-party cookies are blocked by default, and hyperlink
auditing and DNS prefetching are off.

**Residual risk:** the two lists that ship with the browser are small and
hand-written. Real coverage needs a maintained list, which the user adds. The
engine implements network filtering only — no cosmetic filtering — so an ad
slot may leave an empty space. First-party tracking, and tracking done by a
server the page already talks to, is not addressed by request blocking at all.

## Fingerprinting

**Goal: reduce uniqueness where it is cheap; be honest that it is partial.**

Standard removes the QtWebEngine token from the user agent (which would
otherwise be a strong signal on its own) and stops WebRTC from revealing local
IP addresses. Strict additionally blocks canvas readback, masks the GPU vendor
and renderer, rounds audio measurements and sends a fixed `Accept-Language`.

**Residual risk:** screen dimensions, installed fonts, timezone, and dozens of
subtler signals remain. Randomising values per load was rejected deliberately:
it is itself detectable and makes a browser *more* distinctive. This browser
does not attempt to be indistinguishable.

## Malicious websites

**Goal: rely on Chromium, and never weaken it.**

Site isolation, the process sandbox and certificate validation are Chromium's,
and the switch builder refuses to emit anything that would disable them — even
if a user types such a switch into the advanced settings. A unit test asserts
that refusal.

**Residual risk:** a Chromium sandbox escape affects this browser exactly as it
affects any other Chromium. There is no automatic updater, so the user is
responsible for installing a newer build after a Chromium security release.
That is a real trade-off, made because an updater is also a beacon.

## Network observers (ISP, Wi-Fi, transit)

**Goal: HTTPS for content; no claim beyond that.**

Top-level http:// navigations are retried over https:// first, and a host that
genuinely has no HTTPS is remembered for the session rather than retried
forever. Certificate errors show a full-page interstitial that defaults to
going back.

**Residual risk:** an observer still sees which servers you connect to, when,
and how much. HTTPS does not hide that, and neither does this browser.

## DNS visibility

**Goal: make the choice explicit.**

The system resolver is the default, which usually means the ISP sees every
hostname. DNS-over-HTTPS moves that visibility to a resolver the user names.
The settings page says so in those words rather than implying DoH is privacy.

## Someone with access to the computer

**Goal: leave as little as possible; do not pretend to be a security boundary.**

After a normal exit there is no history, no cookie jar, no cache and no session
folder. Downloads and — if enabled — the settings file remain, because the user
asked for them.

**Residual risk:** this is not protection against a person or process that can
read the machine's memory, install software, or watch the browser while it
runs. There is no master password, no encrypted profile, and nothing here
resists forensic recovery of deleted files.

## Us, the developers

**Goal: make trust unnecessary where possible.**

The value of a privacy claim is what a reader can verify. The blocking engine,
URL parsing, permission policy, settings model and cleanup sequence are all
Qt-free and unit-tested; the source audit enforces the negative claims; and the
network audit shows what the running browser actually contacts. The code is the
document — this file only explains it.
