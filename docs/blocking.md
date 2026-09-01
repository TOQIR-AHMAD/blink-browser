# Tracker and ad blocking

## How a request is decided

```text
Chromium is about to make a request
              │
              ▼
   RequestInterceptor (network thread)
              │
      ┌───────┴────────┐
      │  HTTPS-first?  │  top-level http:// → redirect to https://
      └───────┬────────┘
              ▼
     FilterService::match()      (read lock; engine may be swapped)
              │
      ┌───────┴────────┐
      │  FilterEngine  │  token index → candidate rules → pattern match
      └───────┬────────┘
              │
   ┌──────────┴───────────┐
   │                      │
$important block      block matched?
   │                      │
   │              ┌───────┴────────┐
   │              │ exception rule? │
   │              └───────┬────────┘
   ▼                      ▼
 BLOCK              BLOCK or ALLOW
   │
   ▼
counter += 1  (category only: no URL, no host, no time)
```

The decision happens before the connection is made. Nothing about a blocked
request is stored: the only trace is a number on the dashboard.

## Supported filter syntax

The engine implements the network-filtering subset of the Adblock Plus syntax:

| Form | Meaning |
| --- | --- |
| `||example.com^` | domain anchor: matches the host or any subdomain |
| `|https://example.com` | anchored at the start of the URL |
| `/banner.gif|` | anchored at the end |
| `/ads/` | plain substring |
| `*` | wildcard |
| `^` | separator: anything that is not a letter, digit, `_`, `-`, `.` or `%` |
| `@@` | exception (allow) rule |
| `$script,image,stylesheet,font,media,xmlhttprequest,subdocument,document,websocket,ping,other` | resource types, and `~type` to exclude |
| `$third-party` / `$first-party` | party restriction |
| `$domain=a.com|~b.a.com` | only on (or never on) these document domains |
| `$important` | beats exception rules |
| `$match-case` | case-sensitive matching |

**Not supported, and counted rather than ignored:**

- Cosmetic rules (`##`, `#@#`, `#?#`, `#$#`). This browser blocks requests; it
  does not hide page elements. An ad slot may therefore leave a gap.
- Regular-expression rules (`/pattern/`), whose match cost is unbounded.
- Any `$option` the parser does not know. Such a rule is dropped, because
  applying it without its option would block more than the rule's author meant.

The settings page shows, per list, how many rules loaded and how many were
skipped, so a list is never presented as fully applied when it is not.

## Performance

Every rule is indexed under its longest alphanumeric token (three characters or
more). Matching a request extracts the same kind of tokens from its URL and
checks only the rules sharing one, plus the small set of rules that have no
usable token. This is what makes the check cheap enough to run inline on
Chromium's network thread for every request.

## Where lists come from

Two small lists ship inside the executable — `resources/filters/trackers.txt`
and `resources/filters/ads.txt`. They were written for this project rather than
copied from a maintained list, so that a default installation blocks something
useful without redistributing someone else's list under an incompatible licence
and, more importantly, **without fetching anything from a server controlled by
this project** (PLAN.md §20, §21). They are deliberately conservative: only
hosts whose whole purpose is measurement or advertising, and mostly only as
third-party requests.

For real coverage a user adds a maintained list (EasyList, EasyPrivacy or
similar) from a local file or a URL they type. Then:

- The list is fetched only when the user asks, or at start-up if they turn that
  on. Nothing is fetched on a timer.
- The request carries no cookies, no cache and no identifier beyond the browser
  name and version. The list's host learns an IP address and a time.
- A list URL must be `https`, so it cannot be tampered with in transit.
- There is no mirror, no proxy and no "recommended lists" service belonging to
  this project. The user's list is between the user and its author.

## Counting

`BlockingStats` holds seven counters: trackers, ads, other blocked requests,
blocked cookies, permissions granted, permissions denied, HTTPS upgrades. They
are atomic (the interceptor increments from the network thread, the dashboard
reads from the UI thread), they are never written to disk, and
`PrivacyManager::runCleanup()` resets them on exit.

The blocker knows nothing else. There is no per-site breakdown, because
building one would mean keeping a record of the sites visited — exactly what
the rest of the browser goes to some trouble not to do.
