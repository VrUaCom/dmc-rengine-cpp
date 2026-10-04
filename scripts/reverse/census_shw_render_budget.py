#!/usr/bin/env python3
"""Census DMC3 SHW topology and derive EXE-backed shadow render budgets.

Usage:
  python scripts/reverse/census_shw_render_budget.py "DMC 3 RENGINE (6).zip" output.json
"""
from __future__ import annotations
import hashlib, json, struct, sys, zipfile
from pathlib import Path

def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def components(adjacency):
    seen=set(); count=0
    for root in range(len(adjacency)):
        if root in seen: continue
        count += 1; stack=[root]; seen.add(root)
        while stack:
            i=stack.pop()
            for n in adjacency[i]:
                if n not in seen:
                    seen.add(n); stack.append(n)
    return count

def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: census_shw_render_budget.py archive.zip output.json")
    archive=Path(sys.argv[1]); out_path=Path(sys.argv[2])
    archive_bytes=archive.read_bytes()
    files=[]
    with zipfile.ZipFile(archive) as zf:
        infos=[i for i in zf.infolist() if i.filename.lower().endswith(".shw")]
        for info in infos:
            data=zf.read(info)
            if data[:4] != b"SHW ":
                raise ValueError(f"{info.filename}: bad SHW magic")
            version=struct.unpack_from("<f",data,4)[0]
            hull_count=data[0x10]
            hulls=[]
            for hi in range(hull_count):
                off=0x20+hi*0x40
                v,t=struct.unpack_from("<HH",data,off)
                tri_ptr,adj_ptr=struct.unpack_from("<QQ",data,off+0x10)
                edge_map={}
                tris=[]
                for ti in range(t):
                    a,b,c,z=struct.unpack_from("<IIII",data,tri_ptr+ti*0x10)
                    if z != 0 or max(a,b,c) >= v:
                        raise ValueError(f"{info.filename}: invalid triangle record")
                    tris.append((a,b,c))
                    for x,y in ((a,b),(b,c),(c,a)):
                        edge_map.setdefault(tuple(sorted((x,y))),[]).append(ti)
                if any(len(vv) != 2 for vv in edge_map.values()):
                    raise ValueError(f"{info.filename}: open/nonmanifold triangle edge")
                adjacency=[]
                for ti in range(t):
                    n0,n1,n2,z=struct.unpack_from("<HHHH",data,adj_ptr+ti*8)
                    if z != 0 or max(n0,n1,n2) >= t:
                        raise ValueError(f"{info.filename}: invalid adjacency record")
                    adjacency.append((n0,n1,n2))
                    expected=set()
                    a,b,c=tris[ti]
                    for x,y in ((a,b),(b,c),(c,a)):
                        expected.update(q for q in edge_map[tuple(sorted((x,y)))] if q != ti)
                    if set(adjacency[-1]) != expected or len(expected) != 3:
                        raise ValueError(f"{info.filename}: adjacency mismatch")
                comp=components(adjacency)
                if t != 2*v - 4*comp:
                    raise ValueError(f"{info.filename}: topology identity failed")
                hulls.append({"index":hi,"vertices":v,"triangles":t,"components":comp})
            V=sum(h["vertices"] for h in hulls); T=sum(h["triangles"] for h in hulls)
            H=len(hulls); C=sum(h["components"] for h in hulls)
            planned=0x280 + 0xA0*H + 0x20*(V+T+6*H)
            files.append({
                "path":info.filename,"sha256":sha256(data),"bytes":len(data),"version":version,
                "hulls":H,"vertices":V,"triangles":T,"components":C,
                "maxHullVertices":max((h["vertices"] for h in hulls),default=0),
                "maxHullTriangles":max((h["triangles"] for h in hulls),default=0),
                "plannedRuntimeBytes":planned,
                "fileWorstCaseGroupsUpper":2*T,
                "fileWorstCaseRingBytesUpper":144*T,
                "hullsDetail":hulls,
            })
    result={
        "schema":"dmc-rengine.dmc3-shw-retail-render-budget.v1",
        "archive":{"name":archive.name,"sha256":sha256(archive_bytes),"bytes":len(archive_bytes)},
        "population":{
            "files":len(files),"uniqueSha256":len({f["sha256"] for f in files}),
            "hulls":sum(f["hulls"] for f in files),"vertices":sum(f["vertices"] for f in files),
            "triangles":sum(f["triangles"] for f in files),"connectedComponents":sum(f["components"] for f in files),
        },
        "bound":{
            "exeFormula":"groups=A+E; ringBytes=72*(A+E)",
            "closedTopologyUpper":"A+E<=2*T; ringBytes<=144*T",
            "reason":"E<=3*A and E<=3*(T-A) for closed triangular adjacency",
        },
        "files":files,
    }
    out_path.write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")

if __name__ == "__main__":
    main()
