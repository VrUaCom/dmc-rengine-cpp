#!/usr/bin/env python3
"""Census DMC3 effect text witnesses for EXE-confirmed HTH/RDN tags."""
import argparse, hashlib, json, zipfile

def parse_clips(raw: bytes):
    text = raw.decode("latin1", "ignore")
    clips, cur = [], None
    for line in text.splitlines():
        s=line.strip()
        if s=="# Clip":
            if cur: clips.append(cur)
            cur={"fields":{}}
            continue
        if cur is None or not s or s.startswith(";") or s=="Param":
            continue
        parts=s.split()
        if len(parts)>=2:
            key=parts[0]
            val=" ".join(parts[1:])
            if key=="Id": cur["id"]=val
            else: cur["fields"][key]=val
    if cur: clips.append(cur)
    return clips

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("archive")
    ap.add_argument("-o","--output")
    a=ap.parse_args()
    out={"archive":a.archive,"witnesses":{"HTH":[],"RDN":[]}}
    with zipfile.ZipFile(a.archive) as z:
        for info in z.infolist():
            if info.is_dir() or not info.filename.lower().endswith(".txt"):
                continue
            raw=z.read(info)
            for clip in parse_clips(raw):
                tag=clip.get("id")
                if tag not in out["witnesses"]: continue
                item={"path":info.filename,"sha256":hashlib.sha256(raw).hexdigest(),"fields":clip["fields"]}
                if tag=="HTH":
                    try:
                        dx=int(float(clip["fields"]["DetailH"]))
                        dy=int(float(clip["fields"]["DetailV"]))
                        w,h=512,256
                        sx,sy=1<<dx,1<<dy
                        rows=h//sy+2
                        cols=w//sx+3
                        item["startup512x256"]={"detailH":dx,"detailV":dy,"sx":sx,"sy":sy,
                          "rows":rows,"recordsPerRow":cols,"records":rows*cols,"ringBytes":40*rows*cols}
                    except (KeyError,ValueError,OverflowError):
                        pass
                out["witnesses"][tag].append(item)
    text=json.dumps(out,indent=2,ensure_ascii=False)+"\n"
    if a.output: open(a.output,"w",encoding="utf-8").write(text)
    else: print(text,end="")
if __name__=="__main__": main()
