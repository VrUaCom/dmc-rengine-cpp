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

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

/** Decompile a bounded list of EXE functions for format provenance notes. */
public class DumpFunctionDecomp extends GhidraScript {
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
                default:
                    if (c < 0x20) b.append(String.format("\\u%04x", (int)c));
                    else b.append(c);
            }
        }
        return b.append('"').toString();
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) {
            println("Usage: DumpFunctionDecomp <output-json> <entry-hex> [entry-hex ...]");
            return;
        }
        File out = new File(args[0]);
        out.getParentFile().mkdirs();
        DecompInterface ifc = new DecompInterface();
        ifc.openProgram(currentProgram);
        PrintWriter w = new PrintWriter(out, "UTF-8");
        w.println("{");
        w.println("  \"program\": " + json(currentProgram.getName()) + ",");
        w.println("  \"functions\": [");
        boolean first = true;
        for (int i = 1; i < args.length && !monitor.isCancelled(); i++) {
            Address addr = toAddr(args[i]);
            Function f = getFunctionContaining(addr);
            if (f == null) f = getFunctionAt(addr);
            if (!first) w.println(",");
            first = false;
            String c = "";
            String sig = "";
            String name = "<no-function>";
            if (f != null) {
                name = f.getName();
                sig = f.getSignature().getPrototypeString();
                DecompileResults d = ifc.decompileFunction(f, 120, monitor);
                if (d != null && d.decompileCompleted() && d.getDecompiledFunction() != null) {
                    c = d.getDecompiledFunction().getC();
                }
            }
            w.print("    {\"requested\": " + json(args[i]) + ", \"entry\": " + json(f == null ? addr.toString() : f.getEntryPoint().toString()));
            w.print(", \"name\": " + json(name) + ", \"signature\": " + json(sig) + ", \"decompilation\": " + json(c) + "}");
        }
        w.println();
        w.println("  ]");
        w.println("}");
        w.close();
        ifc.dispose();
        println("Wrote function decompilations to " + out.getAbsolutePath());
    }
}
