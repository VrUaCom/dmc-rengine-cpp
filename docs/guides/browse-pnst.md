# How to Browse DMC3 PNST Containers

PNST is another slot-oriented container surface in Devil May Cry 3 HD Collection. DMC Rengine keeps PNST slot identity, empty entries, aliases and nested resources explicit so browsing does not erase the structure needed for later analysis or reintegration.

## Practical browsing path

A PNST workflow in GDSpaces follows the resource graph instead of pretending the container is a normal directory:

```text
PNST
  -> slot table
  -> slot identity
  -> nested container or resource
  -> typed inspection when a canonical reader exists
```

The useful question is not only "what bytes are inside?" but also "which physical slot produced them, what parent container did they come from, and what parser is authoritative for the child?"

## Native Reader handoff

When a materialized PNST child matches a canonical Native Reader family such as DDS, PTX, SCM, MOD, SHW, HITS, DCA, LIG/LIG2, Stage TXT or PE/EXE, downstream inspection can use that typed reader instead of staying at raw-byte level.

Recognition is still distinct from edit or writer authority. A successfully opened child resource does not imply that every format supports modification or production reintegration.

## Evidence boundary

The canonical container layer supports recursive PAC/PNST expansion and guarded slot-path authoring, but original-game acceptance remains a separate runtime proof requirement. Use `docs/status/current.md` for the exact current boundary.
