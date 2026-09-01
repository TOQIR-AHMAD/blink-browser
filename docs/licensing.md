# Licensing — decision pending

PLAN.md §39 lists a `LICENSE` file at the repository root. It has deliberately
**not** been created yet, because the choice belongs to you and an incorrect
licence is worse than a missing one.

## The constraint that drives the choice

The browser links against Qt 6 and Qt WebEngine. The open-source editions of
both are offered under **LGPL v3** (Qt WebEngine additionally carries Chromium's
BSD-style licences for its own sources). Under LGPL v3, distributing a binary
that links Qt requires, at minimum:

- dynamic linking to Qt, or another mechanism that lets a user relink against a
  modified Qt,
- publishing any modifications you make to Qt itself,
- shipping the Qt licence texts and attribution with the application.

That is compatible with the project's own code being under a permissive licence
(MIT, Apache-2.0, BSD) *or* a copyleft one (MPL-2.0, GPL-3.0). It is not
compatible with a proprietary closed-source release unless a commercial Qt
licence is purchased.

## Reasonable options

| Licence | Fits when |
| --- | --- |
| **MPL-2.0** | You want per-file copyleft on your code while allowing broad reuse. Common for browsers. |
| **GPL-3.0** | You want the whole application to stay open source. |
| **MIT / Apache-2.0** | You want maximum reuse of your code; Qt's LGPL obligations still apply to the distributed binary. |

Tell me which one you want and I will add `LICENSE` plus the third-party
attribution file that a release needs.
