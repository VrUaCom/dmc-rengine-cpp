#!/usr/bin/env python3
import argparse,json,re
from collections import Counter
from pathlib import Path

def main():
 ap=argparse.ArgumentParser(); ap.add_argument('--source',required=True,type=Path); ap.add_argument('--output',required=True,type=Path); a=ap.parse_args(); rows=[]; ext_re=re.compile(r'\.([A-Za-z0-9_]+)(?:$|\s)')
 for p in sorted(a.source.rglob('*.index')):
  b=p.read_bytes(); text=b.decode('utf-8-sig',errors='replace'); lines=[x for x in text.splitlines() if x.strip()]; first=lines[0].strip() if lines else '<empty>'; refs=Counter(); folders=0; malformed=0
  for x in lines[1:] if lines else []:
   if x.endswith(' folder'): folders+=1; continue
   m=ext_re.search(x.strip());
   if m: refs[m.group(1).lower()]+=1
   else: malformed+=1
  rows.append({'path':p.relative_to(a.source).as_posix(),'size':len(b),'first_line':first,'line_count':len(lines),'folder_entries':folders,'extension_entries':dict(sorted(refs.items())),'untyped_entries':malformed,'bom':b.startswith(b'\xef\xbb\xbf'),'ascii_ratio':round(sum(32<=c<127 or c in (9,10,13) for c in b)/len(b),4) if b else 0})
 c=Counter(r['first_line'] for r in rows); routes=Counter();
 for r in rows:
  routes.update(r['extension_entries'])
 out={'schema_version':1,'source':str(a.source),'files':len(rows),'first_line_counts':dict(c.most_common()),'referenced_extension_counts':dict(routes.most_common()),'rows':rows}; a.output.parent.mkdir(parents=True,exist_ok=True); a.output.write_text(json.dumps(out,indent=2,ensure_ascii=False)+'\n',encoding='utf-8'); print(json.dumps({'files':len(rows),'first_lines':dict(c.most_common()),'refs':dict(routes.most_common())},ensure_ascii=False))
if __name__=='__main__':main()
