# Native Reader scene check

`native_reader_scene_check.py` is the repeatable native verification command for
the DMC3 stage corpus. It uses the installed
`C:\Users\ADA\AppData\Local\DMC-Rengine\Native-Reader-CLI\dmc-rengine.exe`
and writes only evidence JSON on `F:`.

## Run

```powershell
python F:\Reverse-Core-Corpus\dmc3-stage-reverse\tools\native_reader_scene_check.py `
  --source 'C:\Users\ADA\Desktop\Moding DMC\Мод проєкти\Progect\Extracted - copia' `
  --cli 'C:\Users\ADA\AppData\Local\DMC-Rengine\Native-Reader-CLI\dmc-rengine.exe' `
  --alias 'F:\Reverse-Core-Corpus\stage_source_ascii' `
  --output 'F:\Reverse-Core-Corpus\dmc3-stage-reverse\evidence\native_reader_scene_validation.json' `
  --workers 4 --sample-scm 2 --low-res
```

The alias is a directory junction. The source files are read-only and are not
copied or rewritten. The CLI is invoked with `inspect-workspace ... --stage
--menu`, so each row contains the native reader's Info model: parser identity,
coverage, regions, fields, unknown ranges, conflicts, and diagnostics. A small
number of `inspect-scm ... --json` samples per stage is also included; use
`--full-scm` only when a large report is acceptable.

## PTX and viewport limits

The CLI can structurally inspect `PTX` bundles and records them in the report.
It does not expose the desktop viewport's texture overlay or low-resolution
render toggle. The report therefore marks `texture_overlay_cli_support` and
`low_resolution_cli_support` as false instead of claiming a visual render.
Use the browser Stage Workspace for that visual step and keep this JSON as the
native structural evidence.

## Known reader issue

The installed `verify-scm-corpus` command exits with `0xC0000409` on this
corpus. The script avoids that batch command and isolates each
`inspect-workspace` process, preserving failures per file while continuing the
run.
