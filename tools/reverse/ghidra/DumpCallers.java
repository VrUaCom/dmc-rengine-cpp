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
import java.util.LinkedHashSet;
import java.util.Set;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;

/** List functions that directly reference the supplied EXE entry points. */
public class DumpCallers extends GhidraScript {
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
        if (args.length < 2) { println("Usage: DumpCallers <output-json> <entry-hex> [entry-hex ...]"); return; }
        File out = new File(args[0]);
        out.getParentFile().mkdirs();
        PrintWriter w = new PrintWriter(out, "UTF-8");
        w.println("{");
        w.println("  \"program\": " + json(currentProgram.getName()) + ",");
        w.println("  \"targets\": [");
        boolean firstTarget = true;
        for (int i = 1; i < args.length; i++) {
            Address target = toAddr(args[i]);
            if (!firstTarget) w.println(",");
            firstTarget = false;
            w.println("    {\"target\": " + json(args[i]) + ", \"callers\": [");
            Set<String> callers = new LinkedHashSet<>();
            for (Reference ref : getReferencesTo(target)) {
                Function f = getFunctionContaining(ref.getFromAddress());
                callers.add((f == null ? "<no-function>" : f.getName()) + "@" + ref.getFromAddress());
            }
            boolean first = true;
            for (String caller : callers) {
                if (!first) w.println(",");
                first = false;
                w.print("      " + json(caller));
            }
            w.println();
            w.print("    ]}");
        }
        w.println();
        w.println("  ]");
        w.println("}");
        w.close();
        println("Wrote callers to " + out.getAbsolutePath());
    }
}
