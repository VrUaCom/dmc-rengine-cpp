#!/usr/bin/env python3
"""Compatibility entry point for the full EventTbl00-21 runtime-family importer.

The earlier census-only importer has been superseded. Keeping this filename avoids
breaking local workflows, but all work is now delegated to
`import_eventtbl_runtime_family.py`, which models all 22 runtime slots and expands
every available payload into commands and raw arguments.
"""

from import_eventtbl_runtime_family import main


if __name__ == "__main__":
    raise SystemExit(main())
