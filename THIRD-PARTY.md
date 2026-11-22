# Third-Party Notices and Credits

## Libraries Fetched at Build Time

| Library | Version / Commit | Used By | License |
| :--- | :--- | :--- | :--- |
| [KDE md4qt](https://github.com/KDE/md4qt) | `1a878810adeeb534de4de395f4951edd54ab6072` | `apps/markdown_viewer/` (when `QMLTUI_BUILD_MARKDOWN_VIEWER=ON`) | MIT |
| [QmlTui.TVision](https://github.com/qmltui/tvision) | `main` | `apps/turbo_hex/` (when `QMLTUI_BUILD_TURBO_HEX=ON`) | BSD-3-Clause |

QmlTui itself (`LGPL-3.0-only`) and its optional transitive build dependencies (`utf8proc` and `libvterm`, both MIT) retain their upstream licenses; see [`qmltui/qmltui`](https://github.com/qmltui/qmltui/blob/main/LICENSE.md). Qt 6 is licensed separately by The Qt Company (<https://www.qt.io/licensing>).

### md4qt

SPDX-FileCopyrightText: 2026 Igor Mironchik
SPDX-License-Identifier: MIT

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Visual & Layout Inspiration

- **`apps/qtop`**: Visual dashboard layout inspired by [`aristocratos/btop`](https://github.com/aristocratos/btop) (Apache-2.0); implemented from scratch in C++17 and QmlTui QML without derived source code.
- **`apps/turbo_hex`** (and [`qmltui/tvision`](https://github.com/qmltui/tvision)): Classic MDI desktop chrome and multi-window binary inspector workflow inspired by Borland Turbo Vision / [`magiblot/tvision`](https://github.com/magiblot/tvision) and IDA `idat`; implemented from scratch in C++17 and QmlTui QML without derived source code.
