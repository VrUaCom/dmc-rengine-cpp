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
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;

/** Export references from EXE-defined strings that name DMC3 resource families. */
public class DumpFormatEvidence extends GhidraScript {

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

    private static String text(Data data) {
        Object v = data.getValue();
        if (v != null) return String.valueOf(v);
        return data.getDefaultValueRepresentation();
    }

    private static String family(String value) {
        String lower = value.toLowerCase().trim();
        if (lower.length() > 160) return null;
        String[] exts = {".cam", ".sef", ".efm", ".dds", ".ptx", ".scm", ".mot", ".lig", ".pos", ".eve", ".mod", ".shw"};
        for (String ext : exts) {
            // Keep extension-table entries and short resource paths, but drop
            // shader/source text that merely contains words such as
            // "oPosition".
            if (lower.equals(ext) || lower.endsWith(ext)) return ext.substring(1);
        }
        return null;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length == 0) {
            println("Usage: DumpFormatEvidence <output-json>");
            return;
        }
        File out = new File(args[0]);
        out.getParentFile().mkdirs();
        DataIterator it = currentProgram.getListing().getDefinedData(true);
        PrintWriter w = new PrintWriter(out, "UTF-8");
        w.println("{");
        w.println("  \"program\": " + json(currentProgram.getName()) + ",");
        w.println("  \"image_base\": " + json(currentProgram.getImageBase().toString()) + ",");
        w.println("  \"matches\": [");
        boolean first = true;
        int count = 0;
        while (it.hasNext() && !monitor.isCancelled()) {
            Data d = it.next();
            String value = text(d);
            String fam = family(value);
            if (fam == null) continue;
            Set<String> funcs = new LinkedHashSet<>();
            Reference[] refs = getReferencesTo(d.getAddress());
            for (Reference ref : refs) {
                Function f = getFunctionContaining(ref.getFromAddress());
                funcs.add((f == null ? "<no-function>" : f.getName()) + "@" + ref.getFromAddress());
            }
            if (!first) w.println(",");
            first = false;
            w.print("    {\"family\": " + json(fam));
            w.print(", \"address\": " + json(d.getAddress().toString()));
            w.print(", \"value\": " + json(value));
            w.print(", \"references\": [");
            boolean firstRef = true;
            for (String f : funcs) {
                if (!firstRef) w.print(", ");
                firstRef = false;
                w.print(json(f));
            }
            w.print("]}");
            count++;
        }
        w.println();
        w.println("  ],");
        w.println("  \"byte_tokens\": [");
        String[] tokens = {".ptx", ".PTX", ".cam", ".CAM", ".mot", ".MOT", ".sef", ".SEF", ".efm", ".EFM", "SCM ", "SEF\\0", "EFM ", "MOD ", "SHW", "MRP", "HITS", "ITM", "STE", "DCA", "EST"};
        boolean firstToken = true;
        for (String token : tokens) {
            byte[] needle = token.getBytes("US-ASCII");
            Address cursor = currentProgram.getMinAddress();
            int seen = 0;
            while (cursor != null && seen < 128 && !monitor.isCancelled()) {
                Address hit = currentProgram.getMemory().findBytes(cursor, needle, null, true, monitor);
                if (hit == null) break;
                if (!firstToken) w.println(",");
                firstToken = false;
                Set<String> funcs = new LinkedHashSet<>();
                for (Reference ref : getReferencesTo(hit)) {
                    Function f = getFunctionContaining(ref.getFromAddress());
                    funcs.add((f == null ? "<no-function>" : f.getName()) + "@" + ref.getFromAddress());
                }
                String blockName = currentProgram.getMemory().getBlock(hit) == null ? "<none>" : currentProgram.getMemory().getBlock(hit).getName();
                Instruction code = currentProgram.getListing().getInstructionContaining(hit);
                Function containing = getFunctionContaining(hit);
                w.print("    {\"token\": " + json(token) + ", \"address\": " + json(hit.toString()) + ", \"block\": " + json(blockName));
                w.print(", \"function\": " + json(containing == null ? "<no-function>" : containing.getName()));
                w.print(", \"instruction\": " + json(code == null ? "" : code.toString()) + ", \"references\": [");
                boolean firstRef = true;
                for (String f : funcs) {
                    if (!firstRef) w.print(", ");
                    firstRef = false;
                    w.print(json(f));
                }
                w.print("]}");
                seen++;
                cursor = hit.add(needle.length);
            }
        }
        w.println();
        w.println("  ],");
        w.println("  \"match_count\": " + count);
        w.println("}");
        w.close();
        println("Wrote " + count + " format string matches to " + out.getAbsolutePath());
    }
}
