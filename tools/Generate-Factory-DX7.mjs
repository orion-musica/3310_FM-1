// SPDX-License-Identifier: GPL-3.0-only WITH GPL-3.0-linking-exception
// Reproduce the immutable V15 VMEM bank. No network or device access.
import fs from 'node:fs';
import crypto from 'node:crypto';
import assert from 'node:assert/strict';
import '../sdk/WebUpdate.js';
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const pkg=fs.readFileSync(new URL('../sdk/firmware/FM-1_015.fwsc',import.meta.url));
assert.equal(sha(pkg),'db1642b2b6fa5c2cccb11ffd13878068bb28601678d3644049f99dc40e7edb8a');
const raw=FM1Web.inspectPackage(new Uint8Array(pkg),true).factory;
assert.equal(sha(raw),'ecad0b8620f9d784d46c8da1e3c5de02c40214649ed8dec218026431046aad25');
// VMEM literals are seven-bit. A high bit starts a match of 3..66 bytes:
// 128..191 take a one/two-byte base-128 backward distance, 192..255 take
// one byte k for a distance of 128*(k+1), the same field one or more voices
// back. Optimal parse (shortest stream), decoded without a heap.
const n=raw.length,INF=1e9,heads=new Map(),matches=[];
for(let i=0;i<n;i++){
 matches.push([]);
 if(i+3>n)continue;
 const key=raw[i]|raw[i+1]<<8|raw[i+2]<<16,list=heads.get(key)||[],best={};
 for(let c=list.length-1,seen=0;c>=0&&seen<600;c--,seen++){
  const j=list[c],d=i-j;if(d>=16384)break;
  let L=0;while(L<66&&i+L<n&&raw[j+L]===raw[i+L])L++;
  if(L<3)continue;
  const k=d%128===0&&d/128<=128?'a':d<128?'s':'l';
  if(!best[k]||L>best[k][0])best[k]=[L,d];
 }
 for(const k in best)matches[i].push([best[k][0],best[k][1],k==='l'?3:2,k]);
 list.push(i);heads.set(key,list);
}
const cost=new Array(n+1).fill(INF),pick=new Array(n+1).fill(null);cost[n]=0;
for(let i=n-1;i>=0;i--){
 cost[i]=cost[i+1]+1;
 for(const [L,d,c,k] of matches[i])for(let l=3;l<=L;l++)if(c+cost[i+l]<cost[i]){cost[i]=c+cost[i+l];pick[i]=[l,d,k];}
}
const packed=[];
for(let i=0;i<n;){
 if(cost[i]===cost[i+1]+1){assert(raw[i]<128);packed.push(raw[i++]);continue;}
 const [l,d,k]=pick[i];
 if(k==='a')packed.push(192+l-3,d/128-1);
 else if(k==='s')packed.push(128+l-3,d);
 else packed.push(128+l-3,(d&127)|128,d>>7);
 i+=l;
}
const decoded=[];
for(let i=0;i<packed.length;){const token=packed[i++];if(token<128)decoded.push(token);else{
 const n=(token&63)+3,lo=packed[i++],distance=token>=192?(lo+1)*128:(lo&127)|(lo>=128?packed[i++]<<7:0);
 assert(distance>0&&distance<=decoded.length);
 for(let k=0;k<n;k++)decoded.push(decoded[decoded.length-distance]);
}}
assert.deepEqual(Buffer.from(decoded),Buffer.from(raw));
const lines=[];for(let i=0;i<packed.length;i+=24)lines.push('    '+packed.slice(i,i+24).join(',')+',');
const content='/* SPDX-License-Identifier: Apache-2.0\n'+
 ' * M-VAVE original V15 factory DX7 bank; lossless encoding, 2026-10-19 (3310: optimal parse, voice-aligned matches).\n'+
 ' * Regenerate: node tools/Generate-Factory-DX7.mjs\n'+
 ' * VMEM SHA-256: '+sha(raw)+'\n'+
 ' * Source and license: THIRD_PARTY.md (FM-1_015.fwsc). */\n'+
 'static const uint8_t factory_packed[] = {\n'+lines.join('\n')+'\n};\n';
const target=new URL('../app/fm_factory.inc',import.meta.url);
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8'),content);
else fs.writeFileSync(target,content);
console.log(`V15 factory bank verified: 128 voices, ${raw.length} VMEM bytes, ${packed.length} encoded bytes.`);
