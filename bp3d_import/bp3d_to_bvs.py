#!/usr/bin/env python3
"""
BodyParts3D (OBJ) -> BloodVoyagerS-CSV

Ausgabe pro Zeile:  ID,Kind,x_start,y_start,z_start,x_end,y_end,z_end   (cm)

Vorgehen pro Gefäß-OBJ:
  1. Mesh laden (mm)
  2. Mittellinie schätzen: Hauptachse (PCA) in Scheiben schneiden, je Scheibe
     Schwerpunkt = Linienpunkt, mittlerer Abstand der Vertices = Radius
  3. Polylinie mit Douglas-Peucker vereinfachen -> Segmente
  4. Koordinaten nach cm, Ursprung ins Herz, Achsen auf BVS-Konvention
  5. Fließrichtung per Heuristik (Arterie: weg vom Herz, Vene: hin zum Herz)
  6. Endpunkte nahe beieinander werden zu EINEM Punkt verschmolzen (Konnektivität)

Aufruf:
  python bp3d_to_bvs.py --obj-dir isa_BP3D_4.0_obj_99 \
      --parts-list isa_parts_list_e.txt --out vessels.csv
"""
import argparse, csv, glob, os, re
import numpy as np
import trimesh

# ---- Anpassen -------------------------------------------------------------
KIND = {"artery": 0, "vein": 1, "organ": 2}   # TODO: mit deiner Kind-Codierung abgleichen

# Zuordnung BP3D-Achsen -> BVS-Achsen: (Achsenindex, Vorzeichen)
# BVS: x = links/rechts, y = oben/unten (Kopf = +), z = vorne/hinten (vorne = +)
# Bitte einmal visuell in BVS-Vis prüfen und ggf. Vorzeichen drehen!
AXIS_MAP = {"x": (0, +1), "y": (2, +1), "z": (1, -1)}

HEART_KEYWORDS = ("heart",)
ARTERY_KEYWORDS = ("artery", "aorta", "arteria", "pulmonary trunk", "brachiocephalic trunk",
                   "coeliac trunk", "celiac trunk", "thyrocervical trunk", "costocervical trunk")
VEIN_KEYWORDS = ("vein", "vena", "venous", "sagittal sinus", "transverse sinus", "sigmoid sinus",
                 "cavernous sinus", "petrosal sinus", "straight sinus", "coronary sinus",
                 "confluence of sinuses", "occipital sinus")
# ---------------------------------------------------------------------------


def parse_parts_list(path):
    """Liest isa_parts_list_e.txt tolerant: sucht pro Zeile eine Datei-ID (FJ1234)
    und nimmt den Rest der Zeile als Namen. Spalten bitte kurz gegenprüfen."""
    names = {}
    with open(path, encoding="utf-8", errors="ignore") as f:
        for line in f:
            m = re.search(r"\b(FJ\d+|BP\d+)\b", line)
            if m:
                names[m.group(1)] = line.strip().lower()
    return names


def parse_element_parts(path):
    """isa_element_parts.txt (tab-getrennt): concept id, Name des zusammengesetzten
    Organs, element file id (FJ....). Liefert (Namen je FJ-Datei, Compound -> FJ-Liste)."""
    names, compounds = {}, {}
    with open(path, encoding="utf-8", errors="ignore") as f:
        for line in f:
            m = re.search(r"\b(FJ\d+)\b", line)
            if not m:
                continue
            cols = [c.strip() for c in line.rstrip("\n").split("\t")]
            cname = next((c.lower() for c in cols
                          if re.search(r"[a-zA-Z]{3}", c) and not re.fullmatch(r"(FMA|FJ|BP)\d+", c)), "")
            fid = m.group(1)
            names[fid] = names.get(fid, "") + " | " + cname
            compounds.setdefault(cname, []).append(fid)
    return names, compounds


def classify(name):
    a = any(k in name for k in ARTERY_KEYWORDS)
    v = any(k in name for k in VEIN_KEYWORDS)
    if a and not v:
        return "artery"
    if v and not a:
        return "vein"
    if a and v:
        return "ambiguous"
    return None


def centerline(mesh, n_slices=40):
    """Grobe Mittellinie + Radius. Gut für überwiegend röhrenförmige Segmente.
    Stark verzweigte Meshes (z.B. kompletter Aortenbogen mit Abgängen) besser
    mit VMTK / skeletor behandeln."""
    v = np.asarray(mesh.vertices, dtype=float)
    c = v.mean(axis=0)
    _, _, vt = np.linalg.svd(v - c, full_matrices=False)
    axis = vt[0]
    t = (v - c) @ axis
    edges = np.linspace(t.min(), t.max(), n_slices + 1)
    pts, radii = [], []
    for a, b in zip(edges[:-1], edges[1:]):
        sel = v[(t >= a) & (t <= b)]
        if len(sel) < 3:
            continue
        p = sel.mean(axis=0)
        pts.append(p)
        radii.append(np.linalg.norm(sel - p, axis=1).mean())
    return np.array(pts), float(np.median(radii)) if radii else 0.0


def douglas_peucker(pts, tol):
    if len(pts) < 3:
        return pts
    start, end = pts[0], pts[-1]
    d = end - start
    nd = np.linalg.norm(d)
    if nd == 0:
        dist = np.linalg.norm(pts - start, axis=1)
    else:
        dist = np.linalg.norm(np.cross(pts - start, d / nd), axis=1)
    i = int(np.argmax(dist))
    if dist[i] > tol:
        left = douglas_peucker(pts[: i + 1], tol)
        right = douglas_peucker(pts[i:], tol)
        return np.vstack([left[:-1], right])
    return np.vstack([start, end])


def to_bvs(p_mm, origin_mm):
    """mm (BP3D) -> cm (BVS), Ursprung ins Herz, Achsen umordnen."""
    p = (np.asarray(p_mm) - origin_mm) / 10.0
    out = np.zeros_like(p)
    for k, name in enumerate("xyz"):
        idx, sign = AXIS_MAP[name]
        out[..., k] = sign * p[..., idx]
    return out


def snap_endpoints(segments, tol):
    """Endpunkte auf gemeinsame Knoten setzen. NICHT verkettend: ein Punkt wird nur
    einem Knoten zugeordnet, der selbst hoechstens tol entfernt ist. So kollabieren
    dichte Bereiche (Kopf, Thorax) nicht zu einem einzigen Punkt."""
    pts = np.array([p for s in segments for p in (s["start"], s["end"])])
    centers, grid = [], {}
    assign = np.empty(len(pts), dtype=int)
    for i, p in enumerate(pts):
        key = tuple((p // tol).astype(int))
        best, found = tol, -1
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                for dz in (-1, 0, 1):
                    for ci in grid.get((key[0] + dx, key[1] + dy, key[2] + dz), []):
                        d = np.linalg.norm(centers[ci] - p)
                        if d < best:
                            best, found = d, ci
        if found < 0:
            centers.append(p.copy())
            found = len(centers) - 1
            grid.setdefault(key, []).append(found)
        assign[i] = found
    centers = np.array(centers)
    for c in range(len(centers)):          # Knoten = Mittelwert seiner Mitglieder
        centers[c] = pts[assign == c].mean(axis=0)
    for k, s in enumerate(segments):
        s["start"], s["end"] = centers[assign[2 * k]], centers[assign[2 * k + 1]]
    return segments


DEFAULT_ORGANS = ["brain", "heart", "liver", "spleen", "stomach", "pancreas", "gallbladder",
                  "urinary bladder", "right lung", "left lung", "right kidney", "left kidney",
                  "esophagus", "trachea", "small intestine", "large intestine", "thyroid gland",
                  "prostate", "uterus", "diaphragm"]


def export_organs(compounds, files, origin, wanted, out_base, n_points):
    """Pro Organ: Schwerpunkt + Bounding-Box + Punktwolke der Oberflaeche (BVS-Koordinaten, cm)."""
    rows, pts = [], []
    rng = np.random.default_rng(0)
    for want in wanted:
        key = want.lower().strip()
        fids = [f for f in dict.fromkeys(compounds.get(key, [])) if f in files]
        if not fids:
            word = key.split()[-1]
            sugg = [c for c in compounds if word in c][:5]
            print(f"  Organ nicht gefunden: '{want}'  (aehnliche Namen: {sugg})")
            continue
        verts = np.vstack([trimesh.load(files[f], force="mesh").vertices for f in fids])
        v = to_bvs(verts, origin)
        rows.append([want, *np.round(v.mean(axis=0), 1), *np.round(v.min(axis=0), 1), *np.round(v.max(axis=0), 1)])
        for q in v[rng.choice(len(v), min(n_points, len(v)), replace=False)]:
            pts.append([want, *np.round(q, 1)])
    with open(out_base + "_organs.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["name", "cx", "cy", "cz", "minx", "miny", "minz", "maxx", "maxy", "maxz"])
        w.writerows(rows)
    with open(out_base + "_organ_points.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["name", "x", "y", "z"])
        w.writerows(pts)
    print(f"{len(rows)} Organe -> {out_base}_organs.csv und _organ_points.csv")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--obj-dir", required=True)
    ap.add_argument("--parts-list", required=True)
    ap.add_argument("--element-parts", default=None,
                    help="isa_element_parts.txt (ordnet BP-Organen ihre FJ-Mesh-Dateien zu)")
    ap.add_argument("--out", default="vessels.csv")
    ap.add_argument("--simplify-cm", type=float, default=0.5, help="Douglas-Peucker Toleranz")
    ap.add_argument("--snap-cm", type=float, default=0.8, help="Endpunkte verschmelzen")
    ap.add_argument("--min-len-cm", type=float, default=0.5)
    ap.add_argument("--heart-xyz-mm", type=float, nargs=3, default=None,
                    help="Herzmittelpunkt in BP3D-mm; sonst aus Herz-Mesh")
    ap.add_argument("--organs", default="default",
                    help='Komma-Liste exakter Organnamen, "default" oder "none"')
    ap.add_argument("--organ-points", type=int, default=300, help="Punkte pro Organ")
    ap.add_argument("--find", default=None,
                    help='Namen durchsuchen, z.B. --find "sinus,cerebral vein" (braucht --element-parts)')
    args = ap.parse_args()

    names = parse_parts_list(args.parts_list)
    compounds = {}
    if args.element_parts:
        en, compounds = parse_element_parts(args.element_parts)
        for k, v in en.items():
            names[k] = names.get(k, "") + v
    files = {os.path.splitext(os.path.basename(p))[0]: p
             for p in glob.glob(os.path.join(args.obj_dir, "**", "*.obj"), recursive=True)}

    if compounds:
        with open(args.out.replace(".csv", "_compound_names.txt"), "w") as f:
            for c, fs in sorted(compounds.items()):
                f.write(f"{c}\t{len(set(fs))}\t{classify(c) or '-'}\n")
    if args.find:
        if not compounds:
            raise SystemExit("--find braucht --element-parts")
        for kw in [k.strip().lower() for k in args.find.split(",") if k.strip()]:
            hits = [(c, set(fs)) for c, fs in compounds.items() if kw in c]
            print(f"--- '{kw}': {len(hits)} Treffer")
            for c, fs in sorted(hits)[:40]:
                have = sum(1 for x in fs if x in files)
                print(f"  {c}: {len(fs)} Elemente, {have} mit OBJ, Klasse={classify(c) or '-'}")
        return

    # Herz bestimmen
    if args.heart_xyz_mm:
        origin = np.array(args.heart_xyz_mm)
    else:
        heart = [i for i in compounds.get("heart", []) if i in files]
        if not heart:
            heart = [i for i, n in names.items() if any(k in n for k in HEART_KEYWORDS) and i in files]
        if not heart:
            raise SystemExit("Kein Herz-Mesh gefunden, bitte --heart-xyz-mm angeben.")
        origin = np.mean([trimesh.load(files[i], force="mesh").centroid for i in heart], axis=0)
    print("Ursprung (BP3D mm):", origin)

    cls = [classify(names.get(f, "")) for f in files]
    print(f"{len(files)} OBJ-Dateien, davon Arterien: {cls.count('artery')}, "
          f"Venen: {cls.count('vein')}, mehrdeutig (uebersprungen): {cls.count('ambiguous')}")
    segments, report = [], []
    for fid, path in sorted(files.items()):
        kind = classify(names.get(fid, ""))
        if kind is None:
            continue
        nm = names.get(fid, "").strip(" |")[:70]
        mesh = trimesh.load(path, force="mesh")
        cen = to_bvs(mesh.centroid, origin) if len(mesh.vertices) else np.zeros(3)
        row = [fid, kind, nm, *np.round(cen, 1)]
        if kind == "ambiguous":
            report.append(row + ["uebersprungen: mehrdeutiger Name"])
            continue
        if len(mesh.vertices) < 10:
            report.append(row + ["uebersprungen: Mesh zu klein"])
            continue
        line, radius_mm = centerline(mesh)
        if len(line) < 2:
            report.append(row + ["uebersprungen: keine Mittellinie"])
            continue
        line = to_bvs(line, origin)
        line = douglas_peucker(line, args.simplify_cm)
        # Richtung: Arterie weg vom Herz (Ursprung), Vene hin zum Herz
        d0, d1 = np.linalg.norm(line[0]), np.linalg.norm(line[-1])
        if (kind == "artery" and d0 > d1) or (kind == "vein" and d0 < d1):
            line = line[::-1]
        n_before = len(segments)
        for a, b in zip(line[:-1], line[1:]):
            if np.linalg.norm(b - a) >= args.min_len_cm:
                segments.append({"fid": fid, "kind": KIND[kind], "start": a, "end": b,
                                 "diam_cm": 2 * radius_mm / 10.0})
        report.append(row + ["ok" if len(segments) > n_before else "uebersprungen: Segmente zu kurz"])

    n_raw = len(segments)
    segments = snap_endpoints(segments, args.snap_cm)
    # Nach dem Verschmelzen können Segmente die Länge 0 haben -> entfernen
    segments = [s for s in segments if np.linalg.norm(s["end"] - s["start"]) > 1e-6]
    print(f"Verschmelzen: {n_raw} -> {len(segments)} Segmente ({n_raw - len(segments)} Laenge 0 entfernt)")

    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        for i, s in enumerate(segments, 1):
            w.writerow([i, s["kind"], *np.round(s["start"], 2), *np.round(s["end"], 2)])
    # Zusatzdatei mit Durchmesser + Herkunft (BVS liest sie nicht, aber nützlich)
    with open(args.out.replace(".csv", "_meta.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["ID", "bp3d_file", "diameter_cm"])
        for i, s in enumerate(segments, 1):
            w.writerow([i, s["fid"], round(s["diam_cm"], 2)])
    with open(args.out.replace(".csv", "_report.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["bp3d_file", "klasse", "name", "x_cm", "y_cm", "z_cm", "status"])
        w.writerows(report)
    print(f"{len(segments)} Segmente -> {args.out} (+ _meta.csv, _report.csv)")
    if args.organs != "none" and compounds:
        wanted = DEFAULT_ORGANS if args.organs == "default" else [o for o in args.organs.split(",") if o.strip()]
        export_organs(compounds, files, origin, wanted, args.out.replace(".csv", ""), args.organ_points)
    elif args.organs != "none":
        print("Organe uebersprungen: --element-parts fehlt")


if __name__ == "__main__":
    main()
