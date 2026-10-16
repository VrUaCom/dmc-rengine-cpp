/* ###
 * IP: GHIDRA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 */
// @category DMC3.Reverse

import java.io.File;
import java.io.PrintWriter;
import java.util.LinkedHashMap;
import java.util.Map;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;

/** Find instructions that compare/load four-byte DMC3 format magic values. */
public class FindMagicInstructionRefs extends GhidraScript {
    private static String json(String s) {
        if (s == null) return "null";
        StringBuilder b = new StringBuilder("\"");
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '\\': b.append("\\\\"); break;
                case '"': b.append("\\\""); break;
                case '\n': b.append("\\n"); break;
                case '\r': b.append("\\r"); break;
                case '\t': b.append("\\t"); break;
                default: if (c < 0x20) b.append(String.format("\\u%04x", (int)c)); else b.append(c);
            }
        }
        return b.append('"').toString();
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) { println("Usage: FindMagicInstructionRefs <output-json>"); return; }
        Map<Long, String> magic = new LinkedHashMap<>();
        magic.put(0x204d4645L, "EFM ");
        magic.put(0x00464553L, "SEF\\0");
        magic.put(0x004d4143L, "CAM\\0");
        magic.put(0x204d4353L, "SCM ");
        magic.put(0x53544948L, "HITS");
        magic.put(0x004d5449L, "ITM\\0");
        magic.put(0x00455453L, "STE\\0");
        magic.put(0x00414344L, "DCA\\0");
        magic.put(0x00545345L, "EST\\0");
        magic.put(0x20444f4dL, "MOD ");
        magic.put(0x00574853L, "SHW\\0");
        magic.put(0x0050524dL, "MRP\\0");
        File out = new File(args[0]);
        out.getParentFile().mkdirs();
        PrintWriter w = new PrintWriter(out, "UTF-8");
        w.println("{");
        w.println("  \"program\": " + json(currentProgram.getName()) + ",");
        w.println("  \"matches\": [");
        boolean first = true;
        int count = 0;
        InstructionIterator it = currentProgram.getListing().getInstructions(true);
        while (it.hasNext() && !monitor.isCancelled()) {
            Instruction ins = it.next();
            for (int op = 0; op < ins.getNumOperands(); op++) {
                Object[] objs = ins.getOpObjects(op);
                for (Object obj : objs) {
                    if (!(obj instanceof Scalar)) continue;
                    long value = ((Scalar)obj).getUnsignedValue();
                    for (Map.Entry<Long, String> e : magic.entrySet()) {
                        if ((value & 0xffffffffL) != e.getKey()) continue;
                        Function f = getFunctionContaining(ins.getAddress());
                        if (!first) w.println(",");
                        first = false;
                        w.print("    {\"magic\": " + json(e.getValue()) + ", \"value\": " + json(String.format("0x%08x", value & 0xffffffffL)));
                        w.print(", \"address\": " + json(ins.getAddress().toString()));
                        w.print(", \"function\": " + json(f == null ? "<no-function>" : f.getName()));
                        w.print(", \"instruction\": " + json(ins.toString()) + "}");
                        count++;
                    }
                }
            }
        }
        w.println();
        w.println("  ],");
        w.println("  \"match_count\": " + count);
        w.println("}");
        w.close();
        println("Wrote " + count + " magic instruction matches to " + out.getAbsolutePath());
    }
}
