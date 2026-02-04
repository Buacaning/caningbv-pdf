# Caning PDF (CBVPDF)

Fast, lightweight PDF viewer optimized for electronic schematic diagrams.

![License](https://img.shields.io/badge/license-AGPL--3.0-blue)
![Platform](https://img.shields.io/badge/platform-macos-lightgrey)

## Overview

Caning PDF (CBVPDF) is a specialized PDF viewer designed for viewing electronic schematic diagrams. It is optimized for speed and efficiency when rendering complex technical drawings.

## Project Lineage

CBVPDF is part of a lineage of PDF viewers built on the MuPDF rendering engine:

```
MuPDF (Artifex Software)
    │
    ├──► FlexBV / OpenBoardView PDF integration
    │       (by Inflex / Paul Daniels)
    │
    └──► fbvpdf (by Inflex)
            │
            └──► Caning PDF (CBVPDF) - This Repository
                    (Buacaning fork with modifications)
```

### Fork History

| Project | Author | Description | License |
|---------|--------|-------------|---------|
| **MuPDF** | Artifex Software | Core PDF rendering engine | AGPL-3.0 |
| **fbvpdf** | [inflex](https://github.com/inflex) | Fast PDF viewer for schematics | AGPL-3.0 |
| **CBVPDF** | Buacaning | Modified fbvpdf with DDI protocol support | AGPL-3.0 |

## Features

- ⚡ Fast PDF rendering using MuPDF engine
- 🔍 Optimized for schematic diagrams
- 🔗 DDI (Dynamic Data Interface) protocol for external communication
- 🖥️ macOS native application
- ⌨️ Keyboard shortcuts for quick navigation
- 📄 Multi-page document support

## Building

### Prerequisites

- macOS 11.0+
- Xcode Command Line Tools
- Homebrew packages:
  ```bash
  brew install pkg-config
  ```

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/Buacaning/caningbv-pdf.git
cd caningbv-pdf

# Build for macOS
./makemacos.sh

# Output
# Build products will be in build/ directory
# Application bundle: fbvpdf.app
```

## Usage

### Basic Usage

```bash
# Open a PDF file
./fbvpdf.app/Contents/MacOS/fbvpdf schematic.pdf

# With DDI protocol (for external integration)
./fbvpdf.app/Contents/MacOS/fbvpdf -i schematic.pdf -p /tmp/ddi_pipe
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-i <file>` | Input PDF file |
| `-p <path>` | DDI protocol pipe path |
| `-h` | Show help |

## DDI Protocol

CBVPDF supports the Dynamic Data Interface (DDI) protocol for bidirectional communication with external applications.

### Protocol Features

- Component selection events
- Page navigation commands
- Search functionality
- Bidirectional sync capability

### Example DDI Communication

```bash
# Create DDI pipe
mkfifo /tmp/cbvpdf_ddi

# Launch CBVPDF with DDI
./fbvpdf.app/Contents/MacOS/fbvpdf -i schematic.pdf -p /tmp/cbvpdf_ddi &

# Send commands to CBVPDF
echo "GOTO_PAGE:5" > /tmp/cbvpdf_ddi

# Read events from CBVPDF
cat /tmp/cbvpdf_ddi
```

## License

This program is free software: you can redistribute it and/or modify it under the terms of the **GNU Affero General Public License** as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

### Third-Party Licenses

- **MuPDF**: Copyright (c) 2006-2024 Artifex Software, Inc. (AGPL-3.0)
- **curl**: Copyright (c) 1996-2024, Daniel Stenberg (MIT-style)
- **mujs**: Copyright (c) 2013-2024 Artifex Software, Inc. (ISC)

For commercial licensing inquiries regarding MuPDF, please contact sales@artifex.com.

## Modifications from Upstream

Changes made in this fork:

- Rebranded from fbvpdf to Caning PDF
- Enhanced DDI protocol implementation
- macOS-specific build optimizations

## Acknowledgments

- **Artifex Software** - Original MuPDF authors
- **Paul Daniels (inflex)** - fbvpdf and FlexBV author
- The MuPDF developers community

## Related Projects

- [MuPDF](https://mupdf.com/) - Original PDF rendering engine
- [fbvpdf](https://github.com/inflex/fbvpdf) - Upstream project by Inflex
- [OpenBoardView](https://openboardview.org) - Board viewer software

## Reporting Issues

Please report bugs and issues on the GitHub issue tracker:
https://github.com/Buacaning/caningbv-pdf/issues

When reporting issues with specific PDF files, please include the file as an attachment if possible.

---

**Maintainer:** Buacaning  
**Original MuPDF:** Artifex Software, Inc.  
**Upstream fbvpdf:** Paul Daniels (inflex)
