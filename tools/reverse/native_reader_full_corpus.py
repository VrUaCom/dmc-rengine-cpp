#!/usr/bin/env python3
"""Run Native Reader inspect-workspace over a recursive corpus subset.

This is intentionally bounded and evidence-oriented: each file gets one row,
including timeout/invalid-json failures, and no file is modified.
"""
from __future__ import annotations
import argparse,json,subprocess,time
from collections import Counter
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path

def inspect(cli:Path,p:Path,timeout:float,source:Path):
    t=time.perf_counter(); rel=p.relative_to(source).as_posix()
    try:
        q=subprocess.run([str(cli),'inspect-workspace',str(p),'--stage','--menu'],capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=timeout)
    except subprocess.TimeoutExpired:
        return {'ok':False,'error':'timeout','path':rel,'ms':round((time.perf_counter()-t)*1000)}
    raw=q.stdout.strip(); first=min((i for i in (raw.find('{'),raw.find('[')) if i>=0),default=-1); last=max(raw.rfind('}'),raw.rfind(']'))
    if first<0 or last<first:
        return {'ok':False,'error':'invalid-json','exit_code':q.returncode,'stderr':q.stderr[-400:],'path':rel,'ms':round((time.perf_counter()-t)*1000)}
    try: o=json.loads(raw[first:last+1])
    except Exception: return {'ok':False,'error':'invalid-json','exit_code':q.returncode,'path':rel,'ms':round((time.perf_counter()-t)*1000)}
    integ=o.get('format_integration') or {}; diag=o.get('diagnostics') or []; res=o.get('resource') or {}
    return {'ok':q.returncode==0,'path':rel,'extension':p.suffix.lower() or '<none>','format':res.get('format'),'parser_id':integ.get('parser_id',''),'maturity':integ.get('maturity',''),'diagnostic_codes':sorted({d.get('code','') for d in diag if isinstance(d,dict)}),'size':res.get('size'),'ms':round((time.perf_counter()-t)*1000)}

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--source',required=True,type=Path); ap.add_argument('--cli',required=True,type=Path); ap.add_argument('--output',required=True,type=Path); ap.add_argument('--extensions',nargs='+',required=True); ap.add_argument('--workers',type=int,default=12); ap.add_argument('--timeout',type=float,default=8.0); a=ap.parse_args(); src=a.source; wanted={x.lower() if x.startswith('.') else '.'+x.lower() for x in a.extensions}; files=sorted(p for p in src.rglob('*') if p.is_file() and (p.suffix.lower() or '<none>') in wanted)
    rows=[]
    with ThreadPoolExecutor(max_workers=max(1,a.workers)) as pool:
        fs=[pool.submit(inspect,a.cli,p,a.timeout,src) for p in files]
        for f in as_completed(fs): rows.append(f.result())
    rows.sort(key=lambda x:x['path']); summary=Counter((str(r.get('extension')),str(r.get('format') or '<none>'),str(r.get('parser_id') or '<none>'),str(r.get('maturity') or '<none>'),str(r.get('ok'))) for r in rows)
    report={'schema_version':1,'source':str(src),'cli':str(a.cli),'extensions':sorted(wanted),'files':len(rows),'ok':sum(bool(r.get('ok')) for r in rows),'failed':sum(not r.get('ok') for r in rows),'summary':{'|'.join(k):v for k,v in sorted(summary.items())},'rows':rows}; a.output.parent.mkdir(parents=True,exist_ok=True); a.output.write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8'); print(json.dumps({k:report[k] for k in ('files','ok','failed','summary')},ensure_ascii=False)); return 0 if report['failed']==0 else 2
if __name__=='__main__': raise SystemExit(main())
