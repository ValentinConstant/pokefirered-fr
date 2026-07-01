#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Editeur de textes pour pokefirered / pokeleafgreen.

Application web 100% bibliotheque standard Python (aucun "pip install").
Permet d'editer :
  - les noms (Pokemon, attaques, talents, natures, classes de dresseurs)
  - les descriptions (talents, attaques, objets)
  - les objets (nom + description, depuis items.json)

Lancement :
    python edit_text.py
puis ouvrez http://localhost:8765 (le navigateur s'ouvre automatiquement).

Transportable : copiez simplement le depot sur un autre PC ayant Python 3.
Le script retrouve tout seul la racine du projet.

Securite : avant chaque sauvegarde, une copie des fichiers modifies est
placee dans tools/text_editor/backups/<horodatage>/ pour pouvoir revenir
en arriere en cas de probleme.
"""

import http.server
import socketserver
import json
import re
import os
import sys
import shutil
import datetime
import threading
import webbrowser
import urllib.parse
import base64
import zlib
from pathlib import Path

PORT = 8765

# --- Regex de parsing ------------------------------------------------------
# Forme tableau :   [SPECIES_BULBASAUR] = _("BULBASAUR"),
ARRAY_RE = re.compile(r'^\s*(\[[A-Za-z0-9_]+\])\s*=\s*_\("((?:\\.|[^"\\])*)"\)')
# Forme variable :  const u8 sHardyNatureName[] = _("HARDY");
VAR_RE = re.compile(r'^\s*(?:static\s+)?const u8\s+(\w+)\s*\[\]\s*=\s*_\("((?:\\.|[^"\\])*)"\)')
# Forme champ de struct :  .trainerName = _("GIOVANNI"),
FIELD_RE = re.compile(r'^\s*\.(\w+)\s*=\s*_\("((?:\\.|[^"\\])*)"\)')
# Dialogues (.inc) :  Label::  puis  .string "...."
LABEL_RE = re.compile(r'^(\w+)::\s*$')
STRING_RE = re.compile(r'^(\s*)\.string\s+"((?:\\.|[^"\\])*)"\s*$')
# Champ de struct generique :  .baseHP = 45,   /  .types = {TYPE_GRASS, TYPE_POISON},
STRUCT_FIELD_RE = re.compile(r'^(\s*\.(\w+)\s*=\s*)(.+?)(\s*,?\s*)$')
# Code C : toute chaine _("....") (menus, combat, Pokedex...)
ANY_RE = re.compile(r'_\("((?:\\.|[^"\\])*)"\)')
_KEY_RES = [
    re.compile(r'\.(\w+)\s*=\s*_\("'),                  # .champ = _("
    re.compile(r'\[\s*([A-Za-z0-9_]+)\s*\]\s*=\s*_\("'),  # [CONST] = _("
    re.compile(r'(\w+)\s*\[\s*\]\s*=\s*_\("'),           # var[] = _("
    re.compile(r'(\w+)\s*=\s*_\("'),                     # var = _("
]

# --- Configuration des categories -----------------------------------------
# kind: "array" | "var" | "json"
CATEGORIES = [
    {"id": "moves",          "label": "Attaques (nom + desc.)",  "kind": "paired2",
        "name_file": "src/data/text/move_names.h", "desc_file": "src/move_descriptions.c",
        "desc_prefix": "gMoveDescription_", "name_strip": "MOVE_", "max": 12, "keycol": "Attaque"},
    {"id": "abilities",      "label": "Talents (nom + desc.)",   "file": "src/data/text/abilities.h",           "kind": "paired", "name_kind": "array", "desc_kind": "var", "max": 12},
    {"id": "natures",        "label": "Noms de natures",         "file": "src/data/text/nature_names.h",         "kind": "var",   "max": None},
    {"id": "trainer_class",  "label": "Classes de dresseurs",    "file": "src/data/text/trainer_class_names.h",  "kind": "array", "max": 12},
    {"id": "items",          "label": "Objets (nom + desc.)",    "file": "src/data/items.json",                  "kind": "json",
        "list_key": "items", "key_field": "itemId",
        "fields": [["name", "english"], ["desc", "description_english"]], "ascii": True, "keycol": "Objet"},
    {"id": "locations",      "label": "Noms de lieux",           "file": "src/data/region_map/region_map_sections.json", "kind": "json",
        "list_key": "map_sections", "key_field": "id", "require": "name",
        "fields": [["value", "name"]], "ascii": False, "keycol": "Lieu"},
    {"id": "map_dialogues",  "label": "Dialogues des cartes",    "kind": "groups", "subkind": "strinc",
        "glob": "data/maps/*/text.inc", "group_from": "parent", "path": "data/maps/{group}/text.inc", "keycol": "Dialogue"},
    {"id": "global_text",    "label": "Textes globaux",          "kind": "groups", "subkind": "strinc",
        "glob": "data/text/*.inc", "group_from": "stem", "path": "data/text/{group}.inc",
        "filter_empty": True, "keycol": "Texte"},
    {"id": "script_text",    "label": "Textes de scripts",       "kind": "groups", "subkind": "strinc",
        "glob": "data/scripts/*.inc", "group_from": "stem", "path": "data/scripts/{group}.inc",
        "filter_empty": True, "keycol": "Texte"},
    {"id": "trainer_party",  "label": "Equipes de dresseurs", "kind": "trainers_party",
        "file": "src/data/trainer_parties.h", "file2": "src/data/trainers.h", "keycol": "Equipe"},
    {"id": "shops",          "label": "Magasins (Poke Marts + Centre commercial)", "kind": "shops", "keycol": "Magasin"},
    {"id": "graphics",       "label": "Pokemon (sprites, nom, description, cri, icone)", "kind": "graphics", "keycol": "Pokemon"},
    {"id": "code_text",      "label": "Textes du code (menus, combat, Pokedex...)", "kind": "groups", "subkind": "csource",
        "glob": ["src/**/*.c", "src/**/*.h"], "group_from": "relpath_src", "path": "src/{group}",
        "filter_empty": True, "keycol": "Variable",
        "exclude": ["species_names.h", "move_names.h", "abilities.h", "nature_names.h",
                    "trainer_class_names.h", "move_descriptions.c", "trainers.h", "items.h"]},
]


GROUPED_KINDS = ("groups", "struct", "evos", "learnset", "trainers_party")


def find_repo_root():
    """Retrouve la racine du depot en cherchant un fichier repere."""
    here = Path(__file__).resolve()
    candidates = [here.parent] + list(here.parents) + [Path.cwd()] + list(Path.cwd().parents)
    seen = set()
    for d in candidates:
        if d in seen:
            continue
        seen.add(d)
        if (d / "src" / "data" / "text" / "species_names.h").exists():
            return d
    return None


ROOT = find_repo_root()
if ROOT is None:
    print("ERREUR : impossible de trouver la racine du projet pokefirered.")
    print("Placez ce script dans le depot (ex. tools/text_editor/) ou lancez-le depuis le depot.")
    sys.exit(1)


def read_text(path):
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def write_text(path, text):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


# --- Lecture des donnees ---------------------------------------------------
def parse_source_category(cat):
    """Lit une categorie de type 'array' ou 'var' depuis un fichier .h/.c."""
    path = ROOT / cat["file"]
    entries = []
    if not path.exists():
        return entries
    rx = ARRAY_RE if cat["kind"] == "array" else VAR_RE
    for idx, line in enumerate(read_text(path).splitlines()):
        m = rx.match(line)
        if not m:
            continue
        key = m.group(1)
        value = m.group(2)
        display_key = key[1:-1] if key.startswith("[") else key
        entries.append({
            "i": idx,
            "key": display_key,
            "value": value,
            "_line": line,   # ligne d'origine, pour la sauvegarde
        })
    return entries


def parse_json(cat):
    """Categorie 'json' generique : liste cat['list_key'], colonnes cat['fields']
    ([col, json_key]). Filtre eventuel cat['require'] (cle a presente)."""
    path = ROOT / cat["file"]
    entries = []
    if not path.exists():
        return entries
    data = json.loads(read_text(path))
    require = cat.get("require")
    idx = 0
    for obj in data.get(cat["list_key"], []):
        if require and require not in obj:
            continue
        e = {"i": idx, "key": obj.get(cat["key_field"], "?")}
        for col, jk in cat["fields"]:
            e[col] = obj.get(jk, "")
        entries.append(e)
        idx += 1
    return entries


def parse_field(cat):
    """Categorie 'field' : un champ de struct (ex. .trainerName) dont la cle
    est le dernier bloc englobant (ex. [TRAINER_X]). Ignore les valeurs vides."""
    path = ROOT / cat["file"]
    entries = []
    if not path.exists():
        return entries
    enclosing_re = re.compile(cat["enclosing"])
    field_name = cat["field"]
    cur_key = "?"
    for n, line in enumerate(read_text(path).splitlines()):
        mk = enclosing_re.search(line)
        if mk:
            cur_key = mk.group(1)
        m = FIELD_RE.match(line)
        if m and m.group(1) == field_name and m.group(2) != "":
            entries.append({"i": n, "key": cur_key, "value": m.group(2), "_line": line})
    return entries


def parse_paired(cat):
    """Categorie 'paired' : associe nom (name_kind) et description (desc_kind)
    issus du meme fichier, apparies par ordre d'apparition."""
    names = parse_source_category({"file": cat["file"], "kind": cat["name_kind"]})
    descs = parse_source_category({"file": cat["file"], "kind": cat["desc_kind"]})
    entries = []
    for i in range(min(len(names), len(descs))):
        entries.append({
            "i": i,
            "key": names[i]["key"],
            "name": names[i]["value"],
            "desc": descs[i]["value"],
        })
    return entries


def _camel(suffix):
    """MOVE-friendly : 'KARATE_CHOP' -> 'KarateChop'."""
    return "".join(p.capitalize() for p in suffix.split("_"))


def parse_paired2(cat):
    """Categorie 'paired2' : nom (tableau [KEY]) et description (var gPrefix_X)
    issus de DEUX fichiers, apparies par identite (CamelCase de la cle)."""
    names = parse_source_category({"file": cat["name_file"], "kind": "array"})
    descs = parse_source_category({"file": cat["desc_file"], "kind": "var"})
    prefix = cat["desc_prefix"]
    strip = cat.get("name_strip", "")
    dmap = {d["key"][len(prefix):]: d for d in descs if d["key"].startswith(prefix)}
    entries, idx = [], 0
    for n in names:
        key = n["key"]
        suf = key[len(strip):] if strip and key.startswith(strip) else key
        if suf == "NONE":
            continue
        d = dmap.get(_camel(suf))
        entries.append({
            "i": idx,
            "key": key,
            "name": n["value"],
            "desc": d["value"] if d else "",
            "_nl": n["i"],
            "_dl": d["i"] if d else None,
        })
        idx += 1
    return entries


# --- Dialogues (.inc, blocs .string) groupes par fichier ------------------
def parse_strinc_lines(lines):
    """Decoupe une liste de lignes (.inc) en entrees : un label suivi de
    lignes .string. value = payloads joints par retour ligne (separateur
    visuel du source). Renvoie start/end/indent pour reecriture ciblee."""
    entries = []
    i, n = 0, len(lines)
    while i < n:
        if not LABEL_RE.match(lines[i]):
            i += 1
            continue
        labels = []
        while i < n and LABEL_RE.match(lines[i]):
            labels.append(LABEL_RE.match(lines[i]).group(1))
            i += 1
        start = i
        payloads = []
        indent = "\t"
        while i < n:
            sm = STRING_RE.match(lines[i])
            if not sm:
                break
            if not payloads:
                indent = sm.group(1)
            payloads.append(sm.group(2))
            i += 1
        if payloads:
            entries.append({
                "id": labels[0],
                "labels": labels,
                "value": "\n".join(payloads),
                "start": start,
                "end": i,
                "indent": indent,
            })
    return entries


def display_label(label):
    """PalletTown_Text_OakDontGoOut -> OakDontGoOut (sinon label brut)."""
    if "_Text_" in label:
        return label.split("_Text_", 1)[1]
    return label


def csource_key(line, lineno):
    for rx in _KEY_RES:
        m = rx.search(line)
        if m:
            return m.group(1)
    return "L%d" % (lineno + 1)


def parse_csource_lines(lines):
    """Toute chaine _("...") non vide d'un fichier C. i = numero de ligne."""
    entries = []
    for n, l in enumerate(lines):
        m = ANY_RE.search(l)
        if not m or m.group(1) == "":
            continue
        entries.append({"i": n, "key": csource_key(l, n), "value": m.group(1)})
    return entries


def _iter_glob_files(cat):
    pats = cat["glob"] if isinstance(cat["glob"], list) else [cat["glob"]]
    seen, files = set(), []
    for pat in pats:
        for p in ROOT.glob(pat):
            if p not in seen:
                seen.add(p)
                files.append(p)
    return files


def _group_name(cat, p):
    gf = cat.get("group_from", "stem")
    if gf == "parent":
        return p.parent.name
    if gf == "relpath_src":
        return p.relative_to(ROOT / "src").as_posix()
    return p.stem  # "stem"


def group_path(cat, group):
    return ROOT / cat["path"].format(group=group)


def parse_group_path(cat, path):
    if not path.exists():
        return []
    if cat.get("subkind") == "csource":
        return parse_csource_lines(read_text(path).splitlines())
    raw = parse_strinc_lines(read_text(path).splitlines())
    return [{"i": e["id"], "key": display_label(e["id"]), "value": e["value"]} for e in raw]


def list_struct_groups(cat):
    """Ancres [SPECIES_X] d'un fichier ; ne garde que les blocs ayant des champs
    inline editables (ignore SPECIES_NONE / macros). Ordre du fichier (= Pokedex)."""
    if cat["id"] in _groups_cache:
        return _groups_cache[cat["id"]]
    anchor = re.compile(cat["anchor"])
    lines = read_text(ROOT / cat["file"]).splitlines()
    groups, cur, hasfield = [], None, False
    for l in lines:
        m = anchor.search(l)
        if m:
            if cur and hasfield:
                groups.append(cur)
            cur, hasfield = m.group(1), False
        elif cur and STRUCT_FIELD_RE.match(l):
            hasfield = True
    if cur and hasfield:
        groups.append(cur)
    _groups_cache[cat["id"]] = groups
    return groups


def parse_struct_group(cat, group):
    anchor = re.compile(cat["anchor"])
    lines = read_text(ROOT / cat["file"]).splitlines()
    start = None
    for n, l in enumerate(lines):
        m = anchor.search(l)
        if m and m.group(1) == group:
            start = n
            break
    if start is None:
        return []
    entries = []
    for n in range(start + 1, len(lines)):
        if anchor.search(lines[n]):
            break
        m = STRUCT_FIELD_RE.match(lines[n])
        if m:
            entries.append({"i": n, "key": m.group(2), "value": m.group(3)})
    return entries


def list_all_species():
    """Toutes les especes (ancres de species_info.h ayant des champs), ordre Pokedex.
    Sert a l'editeur d'evolutions (pour pouvoir AJOUTER une evo a un mon qui n'en a pas)."""
    if "_all_species" in _groups_cache:
        return _groups_cache["_all_species"]
    anchor = re.compile(r'\[(SPECIES_\w+)\]\s*=')
    lines = read_text(ROOT / "src/data/pokemon/species_info.h").splitlines()
    names, cur, hasfield = [], None, False
    for l in lines:
        m = anchor.search(l)
        if m:
            if cur and hasfield:
                names.append(cur)
            cur, hasfield = m.group(1), False
        elif cur and STRUCT_FIELD_RE.match(l):
            hasfield = True
    if cur and hasfield:
        names.append(cur)
    _groups_cache["_all_species"] = names
    return names


def list_anchor_groups(cat):
    """Groupes = toutes les ancres trouvees (ordre du fichier)."""
    if cat["id"] in _groups_cache:
        return _groups_cache[cat["id"]]
    rx = re.compile(cat["anchor"])
    names = []
    for l in read_text(ROOT / cat["file"]).splitlines():
        m = rx.search(l)
        if m:
            names.append(m.group(1))
    _groups_cache[cat["id"]] = names
    return names


def _find_anchor_line(lines, anchor, group):
    rx = re.compile(anchor)
    for n, l in enumerate(lines):
        m = rx.search(l)
        if m and m.group(1) == group:
            return n
    return None


# --- Evolutions : 1 evolution par ligne ("METHODE, PARAM, ESPECE") ----------
def parse_evo_group(cat, species):
    lines = read_text(ROOT / cat["file"]).splitlines()
    start = _find_anchor_line(lines, cat["anchor"], species)
    if start is None:
        # espece sans evolution : zone vide (on pourra en ajouter une)
        return [{"i": species, "key": "evolutions", "value": ""}]
    text = "\n".join(lines[start:])
    b = text.index("{")
    depth, endi = 0, None
    for j in range(b, len(text)):
        if text[j] == "{":
            depth += 1
        elif text[j] == "}":
            depth -= 1
            if depth == 0:
                endi = j
                break
    inner = text[b + 1:endi]  # {r1},{r2},...
    rows = [" ".join(r.split()) for r in re.findall(r'\{([^{}]*)\}', inner)]
    return [{"i": species, "key": "evolutions", "value": "\n".join(rows)}]


def save_evos(cat, species, value):
    path = ROOT / cat["file"]
    lines = read_text(path).splitlines(keepends=True)
    start = _find_anchor_line(lines, cat["anchor"], species)
    rows = [" ".join(r.split()) for r in value.split("\n") if r.strip()]
    warns = []
    if len(rows) > 6:
        warns.append("%s : %d evolutions (> EVOS_PER_MON=6). Augmentez EVOS_PER_MON dans include/constants/pokemon.h." % (species, len(rows)))
    inner = ", ".join("{%s}" % r for r in rows)

    if start is not None:
        # entree existante : trouver sa fin (comptage d'accolades)
        depth, started, end_line = 0, False, None
        for n in range(start, len(lines)):
            for c in lines[n]:
                if c == "{":
                    depth += 1
                    started = True
                elif c == "}":
                    depth -= 1
            if started and depth == 0:
                end_line = n
                break
        if rows:
            pm = re.match(r'^(.*?=\s*)\{', lines[start])
            prefix = pm.group(1)
            lines[start:end_line + 1] = [prefix + "{" + inner + "},\n"]
        else:
            # plus aucune evolution : on retire l'entree
            del lines[start:end_line + 1]
        write_text(path, "".join(lines))
        return 1, warns

    # pas d'entree existante : on en INSERE une (mon qui n'evoluait pas)
    if not rows:
        return 0, warns
    close = next((n for n, l in enumerate(lines) if l.startswith("};")), None)
    if close is None:
        return 0, warns + ["fin de gEvolutionTable introuvable"]
    lines.insert(close, "    [%s] = {%s},\n" % (species, inner))
    write_text(path, "".join(lines))
    return 1, warns


# --- Learnsets : 1 attaque par ligne ("niveau, MOVE_X") ---------------------
def parse_learnset_group(cat, name):
    lines = read_text(ROOT / cat["file"]).splitlines()
    start = _find_anchor_line(lines, cat["anchor"], name)
    if start is None:
        return []
    rows = []
    for n in range(start + 1, len(lines)):
        s = lines[n].strip()
        if s.startswith("};"):
            break
        m = re.match(r'LEVEL_UP_MOVE\((.*)\)', s)
        if m:
            rows.append(" ".join(m.group(1).split()))
    return [{"i": name, "key": "learnset", "value": "\n".join(rows)}]


def save_learnset(cat, name, value):
    path = ROOT / cat["file"]
    lines = read_text(path).splitlines(keepends=True)
    start = _find_anchor_line(lines, cat["anchor"], name)
    if start is None:
        return 0, ["%s introuvable" % name]
    end = None
    for n in range(start + 1, len(lines)):
        if lines[n].strip().startswith("};"):
            end = n
            break
    if end is None:
        return 0, ["%s : fin de bloc introuvable" % name]
    rows = [" ".join(r.split()) for r in value.split("\n") if r.strip()]
    new_rows = ["    LEVEL_UP_MOVE(%s),\n" % r for r in rows]
    new_block = [lines[start]] + new_rows + ["    LEVEL_UP_END\n", lines[end]]
    lines[start:end + 1] = new_block
    write_text(path, "".join(lines))
    return 1, []


# --- Equipes de dresseurs : 1 Pokemon par ligne -----------------------------
# Mapping TRAINER_X -> (symbole sParty_Y, macro courant) lu dans trainers.h.
# Format d'une ligne : "niveau, SPECIES_X[, iv=N][, item=ITEM_X][, moves=MOVE_A/MOVE_B/...]"
# Niveau + espece obligatoires ; iv / item / moves optionnels (ordre libre).
# Le type de struct et le macro sont recalcules automatiquement a l'ecriture.

_trainer_map_cache = {}


def trainer_party_map(cat):
    """Retourne {TRAINER_X: {"sym": "sParty_Y", "macro": "NO_ITEM_...", "name": "BEN"}}."""
    if "map" in _trainer_map_cache:
        return _trainer_map_cache["map"]
    text = read_text(ROOT / cat["file2"])
    mp = {}
    cur, names = None, {}
    rx_tr = re.compile(r"\[(TRAINER_\w+)\]\s*=")
    rx_name = re.compile(r'\.trainerName\s*=\s*_\("((?:\\.|[^"\\])*)"\)')
    rx_party = re.compile(r"\.party\s*=\s*(\w+)\((\w+)\)")
    for line in text.splitlines():
        m = rx_tr.search(line)
        if m:
            cur = m.group(1)
            continue
        if cur:
            mn = rx_name.search(line)
            if mn:
                names[cur] = mn.group(1)
                continue
        m = rx_party.search(line)
        if m and cur:
            mp[cur] = {"sym": m.group(2), "macro": m.group(1), "name": names.get(cur, "")}
            cur = None
    _trainer_map_cache["map"] = mp
    return mp


def save_trainer_name(cat, trainer, name):
    """Reecrit '.trainerName = _("...")' du bloc [TRAINER_X] dans trainers.h."""
    path = ROOT / cat["file2"]
    lines = read_text(path).splitlines(keepends=True)
    rx_tr = re.compile(r"\[(TRAINER_\w+)\]\s*=")
    rx_name = re.compile(r'(\.trainerName\s*=\s*_\(")((?:\\.|[^"\\])*)("\))')
    in_block = False
    for n, l in enumerate(lines):
        m = rx_tr.search(l)
        if m:
            in_block = (m.group(1) == trainer)
            continue
        if in_block:
            mm = rx_name.search(l)
            if mm:
                if mm.group(2) == name:
                    return 0, []
                lines[n] = rx_name.sub(lambda x: x.group(1) + name + x.group(3), l, count=1)
                write_text(path, "".join(lines))
                _trainer_map_cache.pop("map", None)
                return 1, []
    return 0, ["%s : trainerName introuvable" % trainer]


def list_trainer_groups(cat):
    """Groupes = tous les TRAINER_X ayant une equipe (ordre du fichier)."""
    if cat["id"] in _groups_cache:
        return _groups_cache[cat["id"]]
    names = [t for t in trainer_party_map(cat) if t != "TRAINER_NONE"]
    _groups_cache[cat["id"]] = names
    return names


def _find_party_block(lines, sym):
    """Retourne (start, end, stype) du tableau sParty_<sym> dans trainer_parties.h.
    start = ligne de la declaration, end = ligne du '};' fermant (inclus)."""
    rx = re.compile(r"\bstruct\s+(\w+)\s+" + re.escape(sym) + r"\[\]\s*=\s*\{")
    start = None
    for n, l in enumerate(lines):
        if rx.search(l):
            start = n
            break
    if start is None:
        return None, None, None
    m = rx.search(lines[start])
    stype = m.group(1)
    # Tableau sur une seule ligne (ex. placeholders RS "= {DUMMY_TRAINER_MON};")
    if "};" in lines[start][m.end():]:
        return start, start, stype
    for n in range(start + 1, len(lines)):
        if lines[n].lstrip().startswith("};"):
            return start, n, stype
    return start, None, stype


def _parse_party_mons(lines, start, end):
    """Extrait les Pokemon (dicts) entre les lignes start..end d'un tableau."""
    body = "".join(lines[start + 1:end])
    # Decoupe les Pokemon en suivant la profondeur d'accolades (gere .moves = {})
    blocks, depth, buf = [], 0, []
    for ch in body:
        if ch == "{":
            depth += 1
            if depth == 1:
                buf = []
                continue
        elif ch == "}":
            depth -= 1
            if depth == 0:
                blocks.append("".join(buf))
                continue
        if depth >= 1:
            buf.append(ch)
    mons = []
    for blk in blocks:
        iv = re.search(r"\.iv\s*=\s*(\d+)", blk)
        lvl = re.search(r"\.lvl\s*=\s*(\d+)", blk)
        sp = re.search(r"\.species\s*=\s*(\w+)", blk)
        item = re.search(r"\.heldItem\s*=\s*(\w+)", blk)
        mv = re.search(r"\.moves\s*=\s*\{([^}]*)\}", blk)
        if not sp:
            continue
        moves = []
        if mv:
            moves = [x.strip() for x in mv.group(1).split(",") if x.strip()]
        mons.append({
            "iv": int(iv.group(1)) if iv else 0,
            "lvl": int(lvl.group(1)) if lvl else 0,
            "species": sp.group(1),
            "item": item.group(1) if item else None,
            "moves": [x for x in moves if x != "MOVE_NONE"],
        })
    return mons


def _mon_to_line(mon):
    parts = ["%d, %s" % (mon["lvl"], mon["species"])]
    if mon["iv"]:
        parts.append("iv=%d" % mon["iv"])
    if mon["item"] and mon["item"] != "ITEM_NONE":
        parts.append("item=%s" % mon["item"])
    if mon["moves"]:
        parts.append("moves=%s" % "/".join(mon["moves"]))
    return ", ".join(parts)


def parse_trainer_party(cat, trainer):
    info = trainer_party_map(cat).get(trainer)
    if not info:
        return []
    lines = read_text(ROOT / cat["file"]).splitlines(keepends=True)
    start, end, _ = _find_party_block(lines, info["sym"])
    if start is None or end is None:
        return [{"i": trainer, "key": info["sym"], "value": ""}]
    mons = _parse_party_mons(lines, start, end)
    rows = [_mon_to_line(m) for m in mons]
    return [{"i": trainer, "key": info["sym"], "value": "\n".join(rows)}]


def _parse_party_line(line):
    """Parse 'niveau, SPECIES[, iv=N][, item=X][, moves=A/B]' -> (dict, err)."""
    toks = [t.strip() for t in line.split(",") if t.strip()]
    if len(toks) < 2:
        return None, "ligne incomplete (niveau + espece requis) : '%s'" % line.strip()
    try:
        lvl = int(toks[0])
    except ValueError:
        return None, "niveau invalide : '%s'" % toks[0]
    mon = {"iv": 0, "lvl": lvl, "species": toks[1], "item": None, "moves": []}
    for t in toks[2:]:
        if "=" not in t:
            return None, "champ non reconnu (attendu cle=valeur) : '%s'" % t
        k, v = t.split("=", 1)
        k, v = k.strip().lower(), v.strip()
        if k == "iv":
            try:
                mon["iv"] = int(v)
            except ValueError:
                return None, "iv invalide : '%s'" % v
        elif k == "item":
            mon["item"] = v
        elif k == "moves":
            mon["moves"] = [x.strip() for x in v.split("/") if x.strip()]
        else:
            return None, "cle inconnue : '%s' (attendu iv/item/moves)" % k
    return mon, None


def save_trainer_party(cat, trainer, value):
    warns = []
    info = trainer_party_map(cat).get(trainer)
    if not info:
        return 0, ["%s introuvable dans trainers.h" % trainer]
    sym = info["sym"]

    mons, errs = [], []
    for raw in value.split("\n"):
        if not raw.strip():
            continue
        mon, err = _parse_party_line(raw)
        if err:
            errs.append(err)
        else:
            mons.append(mon)
    if errs:
        return 0, ["%s : %s" % (trainer, e) for e in errs]
    if not mons:
        return 0, ["%s : une equipe doit contenir au moins un Pokemon" % trainer]
    if len(mons) > 6:
        warns.append("%s : %d Pokemon (> 6, le jeu n'en utilise que 6)" % (trainer, len(mons)))

    has_item = any(m["item"] and m["item"] != "ITEM_NONE" for m in mons)
    has_moves = any(m["moves"] for m in mons)
    if has_moves:
        for m in mons:
            if not m["moves"]:
                warns.append("%s : %s sans attaque alors que l'equipe utilise des attaques perso (il sera sans attaque)" % (trainer, m["species"]))
    type_macro = {
        (False, False): ("TrainerMonNoItemDefaultMoves", "NO_ITEM_DEFAULT_MOVES"),
        (False, True):  ("TrainerMonNoItemCustomMoves",  "NO_ITEM_CUSTOM_MOVES"),
        (True, False):  ("TrainerMonItemDefaultMoves",   "ITEM_DEFAULT_MOVES"),
        (True, True):   ("TrainerMonItemCustomMoves",    "ITEM_CUSTOM_MOVES"),
    }
    stype, macro = type_macro[(has_item, has_moves)]

    # 1) Reecrire le tableau dans trainer_parties.h
    path = ROOT / cat["file"]
    lines = read_text(path).splitlines(keepends=True)
    start, end, _ = _find_party_block(lines, sym)
    if start is None or end is None:
        return 0, ["%s : tableau %s introuvable" % (trainer, sym)]
    block = ["static const struct %s %s[] = {\n" % (stype, sym)]
    for m in mons:
        block.append("    {\n")
        block.append("        .iv = %d,\n" % m["iv"])
        block.append("        .lvl = %d,\n" % m["lvl"])
        block.append("        .species = %s,\n" % m["species"])
        if has_item:
            block.append("        .heldItem = %s,\n" % (m["item"] or "ITEM_NONE"))
        if has_moves:
            mv = (m["moves"] + ["MOVE_NONE"] * 4)[:4]
            block.append("        .moves = {%s},\n" % ", ".join(mv))
        block.append("    },\n")
    block.append("};\n")
    lines[start:end + 1] = block
    write_text(path, "".join(lines))

    # 2) Mettre le macro a jour dans trainers.h pour TOUTES les references a sym
    path2 = ROOT / cat["file2"]
    text2 = read_text(path2)
    new_text2 = re.sub(
        r"(\.party\s*=\s*)\w+\(" + re.escape(sym) + r"\)",
        r"\g<1>%s(%s)" % (macro, sym),
        text2,
    )
    if new_text2 != text2:
        write_text(path2, new_text2)
        _trainer_map_cache.pop("map", None)  # le macro courant a change
    return 1, warns


def placeholder_trainer_set(cat):
    """Dresseurs RS placeholder (inutiles en FireRed) : ceux dont le tableau sParty_*
    est declare AVANT le marqueur '// Start of actual trainer data'."""
    if "ph_tr" not in _trainer_map_cache:
        txt = read_text(ROOT / cat["file"])
        marker = txt.find("// Start of actual trainer data")
        head = txt[:marker] if marker != -1 else ""
        syms = set(re.findall(r"(sParty_\w+)\[\]", head))
        mp = trainer_party_map(cat)
        _trainer_map_cache["ph_tr"] = {t for t, info in mp.items() if info["sym"] in syms}
    return _trainer_map_cache["ph_tr"]


def all_party_texts(cat):
    """Lit trainer_parties.h EN UNE PASSE -> {sParty_X: texte (1 Pokemon/ligne)}."""
    lines = read_text(ROOT / cat["file"]).splitlines(keepends=True)
    rx = re.compile(r"\bstruct\s+\w+\s+(sParty_\w+)\[\]\s*=\s*\{")
    out, n = {}, 0
    while n < len(lines):
        m = rx.search(lines[n])
        if not m:
            n += 1
            continue
        sym, start = m.group(1), n
        if "};" in lines[start][m.end():]:
            end = start
        else:
            end = start
            for k in range(start + 1, len(lines)):
                if lines[k].lstrip().startswith("};"):
                    end = k
                    break
        mons = _parse_party_mons(lines, start, end)
        out[sym] = "\n".join(_mon_to_line(x) for x in mons)
        n = end + 1
    return out


def all_trainer_parties(cat):
    """Tous les dresseurs : reels d'abord, puis placeholders RS. Lecture groupee."""
    mp = trainer_party_map(cat)
    texts = all_party_texts(cat)
    ph = placeholder_trainer_set(cat)
    reals, phs = [], []
    for t in list_trainer_groups(cat):
        e = {"key": t, "value": texts.get(mp[t]["sym"], ""), "name": mp[t].get("name", ""),
             "placeholder": t in ph}
        (phs if t in ph else reals).append(e)
    return reals + phs


def option_lists():
    """Listes d'options pour l'editeur structure des dresseurs :
    {species, moves, items} -> [{"v": CONSTANTE, "l": libelle}]."""
    if "opts" not in _trainer_map_cache:
        species = [{"v": e["key"], "l": e["value"]}
                   for e in parse_source_category({"file": "src/data/text/species_names.h", "kind": "array"})
                   if e["key"] != "SPECIES_NONE"]
        moves = [{"v": "MOVE_NONE", "l": "(aucune)"}]
        moves += [{"v": e["key"], "l": e["value"]}
                  for e in parse_source_category({"file": "src/data/text/move_names.h", "kind": "array"})
                  if e["key"] != "MOVE_NONE"]
        items = [{"v": "ITEM_NONE", "l": "(aucun)"}]
        try:
            data = json.loads(read_text(ROOT / "src/data/items.json"))
            for o in data.get("items", []):
                iid = o.get("itemId")
                if iid and iid != "ITEM_NONE":
                    items.append({"v": iid, "l": o.get("english", iid)})
        except Exception:
            pass
        _trainer_map_cache["opts"] = {"species": species, "moves": moves, "items": items}
    return _trainer_map_cache["opts"]


# --- Magasins (Poke Marts + Centre commercial) ------------------------------
# Un magasin = un label `<X>::` (reference par `pokemart <X>`) suivi de lignes
# `.2byte ITEM_Y` terminees par `.2byte ITEM_NONE`, dans data/maps/*/scripts.inc.
# Le prix est celui de l'objet (champ "price" de items.json) -> global.
_shop_cache = {}


def _parse_shop_block(lines, label):
    start = None
    for n, l in enumerate(lines):
        if l.strip() == label + "::":
            start = n
            break
    if start is None:
        return None
    items = []
    for n in range(start + 1, len(lines)):
        s = lines[n].strip()
        if not s.startswith(".2byte"):
            break
        m = re.match(r"\.2byte\s+(\w+)", s)
        if m and m.group(1) != "ITEM_NONE":
            items.append(m.group(1))
    return items


def discover_shops():
    """[{key(label), map(dossier), file, items:[ITEM_X]}], tries par carte."""
    if "shops" not in _shop_cache:
        shops = []
        for p in sorted(ROOT.glob("data/maps/*/scripts.inc")):
            text = read_text(p)
            labels = []
            for m in re.finditer(r"pokemart\s+(\w+)", text):
                if m.group(1) not in labels:
                    labels.append(m.group(1))
            if not labels:
                continue
            lines = text.splitlines(keepends=True)
            for label in labels:
                items = _parse_shop_block(lines, label)
                if items is None:
                    continue
                shops.append({"key": label, "map": p.parent.name,
                              "file": p.relative_to(ROOT).as_posix(), "items": items})
        _shop_cache["shops"] = shops
    return _shop_cache["shops"]


def shop_by_label(label):
    return next((s for s in discover_shops() if s["key"] == label), None)


def item_prices():
    if "prices" not in _shop_cache:
        pr = {}
        try:
            data = json.loads(read_text(ROOT / "src/data/items.json"))
            for o in data.get("items", []):
                if o.get("itemId"):
                    pr[o["itemId"]] = o.get("price", 0)
        except Exception:
            pass
        _shop_cache["prices"] = pr
    return _shop_cache["prices"]


def save_shop(file_rel, label, items):
    """Regenere le bloc `.2byte` du magasin (items sans le terminateur ITEM_NONE)."""
    path = ROOT / file_rel
    lines = read_text(path).splitlines(keepends=True)
    start = None
    for n, l in enumerate(lines):
        if l.strip() == label + "::":
            start = n
            break
    if start is None:
        return 0, ["%s introuvable" % label]
    end = start + 1
    while end < len(lines) and lines[end].strip().startswith(".2byte"):
        end += 1
    new = ["\t.2byte %s\n" % it for it in items] + ["\t.2byte ITEM_NONE\n"]
    lines[start + 1:end] = new
    write_text(path, "".join(lines))
    return 1, []


def save_item_price(item_id, price):
    """Edition CIBLEE du champ 'price' de l'objet dans items.json (encodage preserve)."""
    path = ROOT / "src/data/items.json"
    lines = read_text(path).splitlines(keepends=True)
    cur = None
    for n, l in enumerate(lines):
        m = re.search(r'"itemId":\s*"(ITEM_\w+)"', l)
        if m:
            cur = m.group(1)
            continue
        if cur == item_id:
            pm = re.match(r'(\s*"price":\s*)(\d+)(.*)$', l.rstrip("\n"))
            if pm:
                if pm.group(2) == str(price):
                    return 0
                lines[n] = pm.group(1) + str(price) + pm.group(3) + "\n"
                write_text(path, "".join(lines))
                return 1
    return 0


def parse_group(cat, group):
    k = cat["kind"]
    if k == "struct":
        return parse_struct_group(cat, group)
    if k == "trainers_party":
        return parse_trainer_party(cat, group)
    if k == "evos":
        return parse_evo_group(cat, group)
    if k == "learnset":
        return parse_learnset_group(cat, group)
    return parse_group_path(cat, group_path(cat, group))


_groups_cache = {}


def list_groups(cat):
    if cat["id"] in _groups_cache:
        return _groups_cache[cat["id"]]
    exclude = set(cat.get("exclude", []))
    names, seen = [], set()
    for p in _iter_glob_files(cat):
        rel = p.relative_to(ROOT).as_posix()
        if p.name in exclude or rel in exclude:
            continue
        g = _group_name(cat, p)
        if g in seen:
            continue
        if cat.get("filter_empty") and not parse_group_path(cat, p):
            continue
        seen.add(g)
        names.append(g)
    names.sort()
    _groups_cache[cat["id"]] = names
    return names


def save_strinc_file(cat, group, edits):
    """Reecrit UNIQUEMENT les blocs .string edites (par label). edits = list
    (label, new_value). value separee par retours ligne -> une .string/ligne."""
    path = group_path(cat, group)
    if not path.exists():
        return 0, ["%s introuvable" % group]
    # keepends=True : on conserve les fins de ligne exactes des lignes
    # non touchees (aucune normalisation parasite).
    lines = read_text(path).splitlines(keepends=True)
    by_label = {}
    for e in parse_strinc_lines(lines):
        for lab in e["labels"]:
            by_label[lab] = e
    warns = []
    target = {}  # start -> (entry, value)  (dedoublonne par bloc)
    for lab, val in edits:
        e = by_label.get(lab)
        if not e:
            warns.append("%s : label %s introuvable" % (group, lab))
            continue
        target[e["start"]] = (e, val)
    applied = 0
    for start in sorted(target, reverse=True):  # decroissant : indices stables
        e, val = target[start]
        if e["value"] == val:
            continue
        new_lines = [e["indent"] + '.string "' + p + '"\n' for p in val.split("\n")]
        # preserver l'absence eventuelle de saut de ligne final du bloc
        if not lines[e["end"] - 1].endswith("\n"):
            new_lines[-1] = new_lines[-1].rstrip("\n")
        lines[e["start"]:e["end"]] = new_lines
        applied += 1
    write_text(path, "".join(lines))
    return applied, warns


def save_csource_file(cat, group, edits):
    """Remplace _("ancien") -> _("nouveau") sur la ligne ciblee (i = numero)."""
    path = group_path(cat, group)
    if not path.exists():
        return 0, ["%s introuvable" % group]
    lines = read_text(path).splitlines(keepends=True)
    warns = []
    target = {}  # lineno -> value
    for i, val in edits:
        target[int(i)] = val
    applied = 0
    for ln, val in target.items():
        if ln < 0 or ln >= len(lines):
            warns.append("%s ligne %d introuvable" % (group, ln))
            continue
        line = lines[ln]
        m = ANY_RE.search(line)
        if not m:
            warns.append("%s ligne %d : format inattendu" % (group, ln))
            continue
        old = m.group(1)
        if old == val:
            continue
        old_tok, new_tok = '_("%s")' % old, '_("%s")' % val
        if line.count(old_tok) != 1:
            warns.append("%s ligne %d : remplacement ambigu, ignore" % (group, ln))
            continue
        lines[ln] = line.replace(old_tok, new_tok, 1)
        applied += 1
    write_text(path, "".join(lines))
    return applied, warns


# --- Graphismes Pokemon ----------------------------------------------------
def species_folder_map():
    """SPECIES_X -> dossier graphics/pokemon/<dossier> (via front_pic_table + pokemon.h)."""
    if "_sp_folder" in _groups_cache:
        return _groups_cache["_sp_folder"]
    sym2folder = {}
    pk = ROOT / "src/data/graphics/pokemon.h"
    if pk.exists():
        for l in read_text(pk).splitlines():
            m = re.search(r'(gMonFrontPic_\w+)\[\]\s*=\s*INCBIN_U32\("graphics/pokemon/([^/"]+)/front', l)
            if m:
                sym2folder[m.group(1)] = m.group(2)
    sp2folder = {}
    ft = ROOT / "src/data/pokemon_graphics/front_pic_table.h"
    if ft.exists():
        for l in read_text(ft).splitlines():
            m = re.search(r'SPECIES_SPRITE\((\w+),\s*(gMonFrontPic_\w+)\)', l)
            if m and m.group(1) != "NONE":
                f = sym2folder.get(m.group(2))
                if f:
                    sp2folder["SPECIES_" + m.group(1)] = f
    _groups_cache["_sp_folder"] = sp2folder
    return sp2folder


def gfx_dir(species):
    folder = species_folder_map().get(species)
    return (ROOT / "graphics" / "pokemon" / folder) if folder else None


def read_jasc(path):
    """[[r,g,b], ...] depuis un .pal JASC (ignore l'entete de 3 lignes)."""
    if not path or not path.exists():
        return []
    colors = []
    for l in read_text(path).splitlines()[3:]:
        p = l.split()
        if len(p) >= 3:
            try:
                colors.append([int(p[0]), int(p[1]), int(p[2])])
            except ValueError:
                pass
    return colors


PNG_SIG = b"\x89PNG\r\n\x1a\n"


def png_chunks(data):
    chunks, i = [], 8
    while i + 8 <= len(data):
        ln = int.from_bytes(data[i:i + 4], "big")
        ctype = data[i + 4:i + 8]
        cdata = data[i + 8:i + 8 + ln]
        chunks.append([ctype, cdata])
        i += 12 + ln
    return chunks


def png_build(chunks):
    out = bytearray(PNG_SIG)
    for ctype, cdata in chunks:
        out += len(cdata).to_bytes(4, "big") + ctype + cdata
        out += zlib.crc32(ctype + cdata).to_bytes(4, "big")
    return bytes(out)


def png_info(data):
    """(colortype, w, h, bitdepth) ou None."""
    if data[:8] != PNG_SIG:
        return None
    for ctype, cdata in png_chunks(data):
        if ctype == b"IHDR":
            return cdata[9], int.from_bytes(cdata[0:4], "big"), int.from_bytes(cdata[4:8], "big"), cdata[8]
    return None


def png_recolor(data, colors):
    """Remplace le chunk PLTE par 'colors' ([r,g,b]) pour une prevu recoloree."""
    chunks = png_chunks(data)
    for ch in chunks:
        if ch[0] == b"PLTE":
            pal = bytearray(ch[1])
            for idx in range(min(len(pal) // 3, len(colors))):
                pal[idx * 3:idx * 3 + 3] = bytes(colors[idx][:3])
            ch[1] = bytes(pal)
            break
    return png_build(chunks)


def validate_png(data, ref_path=None):
    warns = []
    info = png_info(data)
    if info is None:
        return ["Ce n'est pas un fichier PNG valide."]
    colortype, w, h, bd = info
    if colortype != 3:
        warns.append("PNG non indexe (colortype=%d) : le build attend une image a palette (<=16 couleurs)." % colortype)
    if ref_path and ref_path.exists():
        ri = png_info(open(ref_path, "rb").read())
        if ri and (ri[1], ri[2]) != (w, h):
            warns.append("Dimensions %dx%d != original %dx%d." % (w, h, ri[1], ri[2]))
    return warns


def palette_for(species, which):
    """which = 'normal' | 'shiny' | 'SPECIES_Y' (normal de Y)."""
    if which in ("normal", "shiny"):
        return read_jasc(gfx_dir(species) / ("%s.pal" % which))
    d = gfx_dir(which)  # which = SPECIES_Y
    return read_jasc(d / "normal.pal") if d else []


# --- Cris et mini-icones : remappage vers une autre espece ------------------
# "Le cri/icone de A devient celui de B" = on copie l'entree de B dans celle de A.
# Cris : gCryTable / gCryTable_Reverse (sound/cry_tables.inc), indexes par cry id.
# Icones : gMonIconTable + gMonIconPaletteIndices (src/pokemon_icon.c), par SPECIES_.

_cry_cache = {}
CRY_FILE = "sound/cry_tables.inc"
ICON_FILE = "src/pokemon_icon.c"


def _species_values():
    if "vals" not in _cry_cache:
        vals = {}
        for l in read_text(ROOT / "include/constants/species.h").splitlines():
            m = re.match(r"\s*#define\s+(SPECIES_\w+)\s+(\d+)", l)
            if m:
                vals[m.group(1)] = int(m.group(2))
        _cry_cache["vals"] = vals
    return _cry_cache["vals"]


def _hoenn_cry_values():
    if "hcv" not in _cry_cache:
        hcv = {}
        for l in read_text(ROOT / "include/constants/hoenn_cries.h").splitlines():
            m = re.match(r"\s*(CRY_\w+)\s*=\s*(\d+)", l)
            if m:
                hcv[m.group(1)] = int(m.group(2))
        _cry_cache["hcv"] = hcv
    return _cry_cache["hcv"]


def _species_to_cryname():
    if "s2c" not in _cry_cache:
        s2c = {}
        for l in read_text(ROOT / "src/data/pokemon/cry_ids.h").splitlines():
            m = re.search(r"\[(SPECIES_\w+)\s*-\s*HOENN_MON_SPECIES_START\]\s*=\s*(CRY_\w+)", l)
            if m:
                s2c[m.group(1)] = m.group(2)
        _cry_cache["s2c"] = s2c
    return _cry_cache["s2c"]


def cry_id_for(species):
    """Reproduit SpeciesToCryId(species - 1) -> index dans gCryTable."""
    vals = _species_values()
    if species not in vals:
        return None
    s = vals[species] - 1
    if s < 251:           # SPECIES_OLD_UNOWN_B - 1
        return s
    if s <= 275:          # formes Unown -> cri d'Unown
        return 200
    cn = _species_to_cryname().get(species)
    return _hoenn_cry_values().get(cn) if cn else None


def _cry_table_rows(lines):
    """Indices de lignes des entrees de gCryTable et gCryTable_Reverse."""
    i1 = i2 = None
    for n, l in enumerate(lines):
        s = l.strip()
        if s.startswith("gCryTable::"):
            i1 = n
        elif s.startswith("gCryTable_Reverse::"):
            i2 = n
    rows1, rows2 = [], []
    if i1 is not None:
        end = i2 if i2 is not None else len(lines)
        rows1 = [n for n in range(i1 + 1, end) if re.match(r"\s*cry\s+\S", lines[n])]
    if i2 is not None:
        rows2 = [n for n in range(i2 + 1, len(lines)) if re.match(r"\s*cry_reverse\s+\S", lines[n])]
    return rows1, rows2


def current_cry_label(species):
    cid = cry_id_for(species)
    if cid is None:
        return None
    lines = read_text(ROOT / CRY_FILE).splitlines()
    rows1, _ = _cry_table_rows(lines)
    if cid >= len(rows1):
        return None
    m = re.match(r"\s*cry\s+(\S+)", lines[rows1[cid]])
    return m.group(1) if m else None


def set_cry(species, source):
    ct, cd = cry_id_for(species), cry_id_for(source)
    if ct is None or cd is None:
        return {"error": "espece sans cri identifiable"}
    lines = read_text(ROOT / CRY_FILE).splitlines(keepends=True)
    rows1, rows2 = _cry_table_rows(lines)
    for rows in (rows1, rows2):
        if ct >= len(rows) or cd >= len(rows):
            return {"error": "index de cri hors limites"}
    backup_files([CRY_FILE])
    for rows, macro in ((rows1, "cry"), (rows2, "cry_reverse")):
        donor = re.match(r"\s*" + macro + r"\s+(\S+)", lines[rows[cd]]).group(1)
        m = re.match(r"(\s*" + macro + r"\s+)(\S+)(.*)", lines[rows[ct]], re.S)
        lines[rows[ct]] = m.group(1) + donor + m.group(3)
    write_text(ROOT / CRY_FILE, "".join(lines))
    return {"applied": 1, "warnings": []}


def _icon_array(lines, name):
    """{SPECIES_X: (lineidx, prefixe, valeur)} pour le tableau 'name' de pokemon_icon.c."""
    start = None
    for n, l in enumerate(lines):
        if name + "[]" in l and "{" in l:
            start = n
            break
    res = {}
    if start is None:
        return res
    for n in range(start + 1, len(lines)):
        if lines[n].lstrip().startswith("};"):
            break
        m = re.match(r"(\s*\[(SPECIES_\w+)\]\s*=\s*)(.+?),?\s*$", lines[n].rstrip("\n"))
        if m:
            res[m.group(2)] = (n, m.group(1), m.group(3).strip())
    return res


def current_icon(species):
    lines = read_text(ROOT / ICON_FILE).splitlines()
    tbl = _icon_array(lines, "gMonIconTable")
    pal = _icon_array(lines, "gMonIconPaletteIndices")
    return {
        "icon": tbl[species][2] if species in tbl else None,
        "icon_pal": pal[species][2] if species in pal else None,
    }


def set_icon(species, source):
    lines = read_text(ROOT / ICON_FILE).splitlines(keepends=True)
    plan = []
    for name in ("gMonIconTable", "gMonIconPaletteIndices"):
        arr = _icon_array(lines, name)
        if species not in arr or source not in arr:
            return {"error": "espece absente de %s" % name}
        tline, prefix, _ = arr[species]
        plan.append((tline, prefix, arr[source][2]))
    backup_files([ICON_FILE])
    for tline, prefix, sval in plan:
        lines[tline] = prefix + sval + ",\n"
    write_text(ROOT / ICON_FILE, "".join(lines))
    return {"applied": 1, "warnings": []}


# --- Nom + description Pokedex (edites depuis la page Pokemon/graphismes) ----
SPECIES_NAMES_FILE = "src/data/text/species_names.h"
POKEDEX_FILE = "src/data/pokemon/pokedex_text_fr.h"   # FIRERED -> *_fr.h


def species_name_line(species):
    """(lineidx, valeur) de [SPECIES_X] dans species_names.h, ou (None, None)."""
    for e in parse_source_category({"file": SPECIES_NAMES_FILE, "kind": "array"}):
        if e["key"] == species:
            return e["i"], e["value"]
    return None, None


def set_species_name(species, value):
    li, old = species_name_line(species)
    if li is None:
        return {"error": "espece absente de species_names.h"}
    if old == value:
        return {"applied": 0, "warnings": []}
    path = ROOT / SPECIES_NAMES_FILE
    lines = read_text(path).splitlines(keepends=True)
    old_token, new_token = '_("%s")' % old, '_("%s")' % value
    if lines[li].count(old_token) != 1:
        return {"error": "remplacement du nom ambigu"}
    backup_files([SPECIES_NAMES_FILE])
    lines[li] = lines[li].replace(old_token, new_token, 1)
    write_text(path, "".join(lines))
    return {"applied": 1, "warnings": []}


def pokedex_symbol_map():
    """SPECIES_X -> symbole gXxxPokedexText (via pokedex_entries.h)."""
    if "pdx" not in _cry_cache:
        m, cur = {}, None
        for l in read_text(ROOT / "src/data/pokemon/pokedex_entries.h").splitlines():
            mk = re.search(r"\[NATIONAL_DEX_(\w+)\]", l)
            if mk:
                cur = mk.group(1)
            md = re.search(r"\.description\s*=\s*(g\w+PokedexText)\b", l)
            if md and cur:
                m["SPECIES_" + cur] = md.group(1)
                cur = None
        _cry_cache["pdx"] = m
    return _cry_cache["pdx"]


def _pokedex_find(lines, sym):
    rx = re.compile(r"\bconst\s+u8\s+" + re.escape(sym) + r"\[\]\s*=\s*_\(")
    start = None
    for n, l in enumerate(lines):
        if rx.search(l):
            start = n
            break
    if start is None:
        return None, None
    for n in range(start, len(lines)):
        if ");" in lines[n]:
            return start, n
    return start, None


def pokedex_get(species):
    """Texte du Pokedex (segments concatenes, '\\n' litteraux conserves)."""
    sym = pokedex_symbol_map().get(species)
    if not sym:
        return None
    lines = read_text(ROOT / POKEDEX_FILE).splitlines()
    s, e = _pokedex_find(lines, sym)
    if s is None or e is None:
        return ""
    segs = []
    for n in range(s, e + 1):
        segs += re.findall(r'"((?:[^"\\]|\\.)*)"', lines[n])
    return "".join(segs)


def set_pokedex(species, text):
    sym = pokedex_symbol_map().get(species)
    if not sym:
        return {"error": "espece sans entree Pokedex"}
    path = ROOT / POKEDEX_FILE
    lines = read_text(path).splitlines(keepends=True)
    s, e = _pokedex_find(lines, sym)
    if s is None or e is None:
        return {"error": "bloc Pokedex introuvable"}
    indent = re.match(r"\s*", lines[s]).group(0)
    body = indent + "    "
    segs = text.replace('"', "").split("\\n")   # pas de guillemet brut ; coupe sur \n
    backup_files([POKEDEX_FILE])
    block = ["%sconst u8 %s[] = _(\n" % (indent, sym)]
    for i, seg in enumerate(segs):
        if i == len(segs) - 1:
            block.append('%s"%s");\n' % (body, seg))
        else:
            block.append('%s"%s\\n"\n' % (body, seg))
    lines[s:e + 1] = block
    write_text(path, "".join(lines))
    return {"applied": 1, "warnings": []}


# --- Stats (species_info.h), editees depuis la page Pokemon ------------------
STATS_FILE = "src/data/pokemon/species_info.h"
STATS_CAT = {"file": STATS_FILE, "anchor": r"\[(SPECIES_\w+)\]\s*=", "kind": "struct"}


def stats_fields(species):
    """[{key, value}] des champs inline du bloc [SPECIES_X] de species_info.h."""
    return [{"key": e["key"], "value": e["value"]} for e in parse_struct_group(STATS_CAT, species)]


def set_stats(species, fields):
    """Reecrit les champs '.cle = valeur' du bloc de l'espece (dict fields)."""
    if not isinstance(fields, dict) or not fields:
        return {"applied": 0, "warnings": []}
    path = ROOT / STATS_FILE
    lines = read_text(path).splitlines(keepends=True)
    entries = parse_struct_group(STATS_CAT, species)
    if not entries:
        return {"error": "aucune stat inline pour cette espece"}
    by_key = {e["key"]: e["i"] for e in entries}
    backup_files([STATS_FILE])
    applied, warns = 0, []
    for key, val in fields.items():
        if key not in by_key:
            continue
        idx = by_key[key]
        m = STRUCT_FIELD_RE.match(lines[idx])
        if not m:
            warns.append("%s : champ %s, format inattendu" % (species, key))
            continue
        if m.group(3) == val:
            continue
        lines[idx] = m.group(1) + val + m.group(4)
        applied += 1
    write_text(path, "".join(lines))
    return {"applied": applied, "warnings": warns}


# --- Evolutions + Attaques par niveau, editees depuis la page Pokemon --------
EVOS_CAT = {"file": "src/data/pokemon/evolution.h", "anchor": r"\[(SPECIES_\w+)\]\s*=", "kind": "evos"}
LEARN_CAT = {"file": "src/data/pokemon/level_up_learnsets.h",
             "anchor": r"(\w+LevelUpLearnset)\[\]\s*=\s*\{", "kind": "learnset"}


def learnset_symbol_map():
    """SPECIES_X -> symbole sXLevelUpLearnset (via level_up_learnset_pointers.h)."""
    if "lsm" not in _cry_cache:
        m = {}
        for l in read_text(ROOT / "src/data/pokemon/level_up_learnset_pointers.h").splitlines():
            mm = re.search(r"\[(SPECIES_\w+)\]\s*=\s*(\w+LevelUpLearnset)", l)
            if mm:
                m[mm.group(1)] = mm.group(2)
        _cry_cache["lsm"] = m
    return _cry_cache["lsm"]


def evos_get(species):
    g = parse_evo_group(EVOS_CAT, species)
    return g[0]["value"] if g else ""


def learnset_get(species):
    sym = learnset_symbol_map().get(species)
    if not sym:
        return None
    g = parse_learnset_group(LEARN_CAT, sym)
    return g[0]["value"] if g else ""


def gfx_action(p):
    """Actions graphismes (immediates, avec sauvegarde de securite)."""
    action = p.get("action")
    species = p.get("species")
    folder = species_folder_map().get(species)
    d = gfx_dir(species)
    if d is None:
        return {"error": "espece inconnue"}
    rel = lambda name: "graphics/pokemon/%s/%s" % (folder, name)

    if action == "upload_sprite":
        which = p.get("which")
        if which not in ("front", "back"):
            return {"error": "type de sprite invalide"}
        raw = base64.b64decode(p.get("png_b64", ""))
        warns = validate_png(raw, d / ("%s.png" % which))
        backup_files([rel("%s.png" % which)])
        with open(d / ("%s.png" % which), "wb") as f:
            f.write(raw)
        return {"applied": 1, "warnings": warns}

    if action == "reuse_palette":
        sd = gfx_dir(p.get("source"))
        if sd is None:
            return {"error": "espece source inconnue"}
        backup_files([rel("normal.pal"), rel("shiny.pal")])
        for pal in ("normal.pal", "shiny.pal"):
            if (sd / pal).exists():
                shutil.copy2(sd / pal, d / pal)
        return {"applied": 1, "warnings": []}

    if action == "import_palette":
        which = p.get("which", "normal")
        text = (p.get("pal_text", "") or "").replace("\r\n", "\n").replace("\r", "\n")
        if not text.startswith("JASC-PAL"):
            return {"error": "Fichier .pal invalide (entete 'JASC-PAL' attendu)."}
        targets = ["normal.pal", "shiny.pal"] if which == "both" else [which + ".pal"]
        backup_files([rel(t) for t in targets])
        for t in targets:
            write_text(d / t, text if text.endswith("\n") else text + "\n")
        return {"applied": 1, "warnings": []}

    if action == "set_cry":
        src = p.get("source")
        if src not in species_folder_map():
            return {"error": "espece source inconnue"}
        return set_cry(species, src)

    if action == "set_icon":
        src = p.get("source")
        if src not in species_folder_map():
            return {"error": "espece source inconnue"}
        return set_icon(species, src)

    if action == "set_name":
        return set_species_name(species, p.get("value", ""))

    if action == "set_pokedex":
        return set_pokedex(species, p.get("value", ""))

    if action == "set_stats":
        return set_stats(species, p.get("fields", {}))

    if action == "set_evos":
        a, w = save_evos(EVOS_CAT, species, p.get("value", ""))
        return {"applied": a, "warnings": w}

    if action == "set_learnset":
        sym = learnset_symbol_map().get(species)
        if not sym:
            return {"error": "aucun learnset pour cette espece"}
        a, w = save_learnset(LEARN_CAT, sym, p.get("value", ""))
        return {"applied": a, "warnings": w}

    return {"error": "action inconnue"}


def build_payload():
    cats = []
    for cat in CATEGORIES:
        if cat["kind"] == "shops":
            cats.append({
                "id": cat["id"], "label": cat["label"], "kind": "shops",
                "grouped": False, "twofields": False, "keycol": cat.get("keycol", "Magasin"),
                "count": len(discover_shops()), "entries": [],
            })
            continue
        if cat["kind"] == "graphics":
            species = list(species_folder_map().keys())
            cats.append({
                "id": cat["id"], "label": cat["label"], "kind": "graphics",
                "grouped": True, "graphics": True, "groups": species,
                "twofields": False, "keycol": cat.get("keycol", "Pokemon"),
                "count": len(species), "entries": [],
            })
            continue
        if cat["kind"] in GROUPED_KINDS:
            if cat["kind"] == "struct":
                groups = list_struct_groups(cat)
            elif cat["kind"] == "evos":
                groups = list_all_species()
            elif cat["kind"] == "learnset":
                groups = list_anchor_groups(cat)
            elif cat["kind"] == "trainers_party":
                groups = list_trainer_groups(cat)
            else:
                groups = list_groups(cat)
            cats.append({
                "id": cat["id"],
                "label": cat["label"],
                "kind": cat["kind"],
                "grouped": True,
                "groups": groups,
                "twofields": False,
                "keycol": cat.get("keycol", "Texte"),
                "count": len(groups),
                "entries": [],
            })
            continue
        if cat["kind"] == "json":
            entries = parse_json(cat)
            twofields = len(cat["fields"]) > 1
            keycol = cat.get("keycol", "Cle")
        elif cat["kind"] == "paired":
            entries = parse_paired(cat)
            twofields = True
            keycol = "Talent"
        elif cat["kind"] == "paired2":
            entries = parse_paired2(cat)
            twofields = True
            keycol = cat.get("keycol", "Cle")
        elif cat["kind"] == "field":
            entries = parse_field(cat)
            twofields = False
            keycol = cat.get("keycol", "Cle")
        else:
            entries = parse_source_category(cat)
            twofields = False
            keycol = "Cle"
        cats.append({
            "id": cat["id"],
            "label": cat["label"],
            "kind": cat["kind"],
            "max": cat.get("max"),
            "twofields": twofields,
            "keycol": keycol,
            "count": len(entries),
            "entries": entries,
        })
    return {"root": str(ROOT), "categories": cats}


# --- Sauvegarde ------------------------------------------------------------
def backup_files(files):
    if not files:
        return None
    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    dest = Path(__file__).resolve().parent / "backups" / stamp
    dest.mkdir(parents=True, exist_ok=True)
    for rel in files:
        src = ROOT / rel
        if src.exists():
            safe = rel.replace("/", "__").replace("\\", "__")
            shutil.copy2(src, dest / safe)
    return str(dest)


def save_changes(changes):
    """
    changes = [{category, i, field, value}, ...]
    field vaut "value" (sources) ou "name"/"desc" (items.json)
    """
    cat_by_id = {c["id"]: c for c in CATEGORIES}

    # Regrouper par fichier
    source_edits = {}   # file_rel -> list of (line_idx, new_value, cat)
    json_edits = {}     # file_rel -> list of (cat, i, field, value)
    strinc_edits = {}   # (cat_id, group) -> list of (label, new_value)
    struct_edits = {}   # file_rel -> list of (line_idx, new_value)
    blocklist_edits = {}  # (cat_id, group) -> value (1 zone par groupe)
    trainer_name_edits = {}  # TRAINER_X -> nom (trainerName dans trainers.h)
    shop_item_edits = {}  # label magasin -> liste d'items (texte, 1/ligne)
    shop_price_edits = {}  # ITEM_X -> prix (items.json)
    touched_files = set()
    warnings = []

    for ch in changes:
        cat = cat_by_id.get(ch["category"])
        if not cat:
            continue
        if cat["kind"] == "shops":
            if ch.get("field") == "price":
                shop_price_edits[ch["i"]] = ch["value"]
                touched_files.add("src/data/items.json")
            else:
                label = ch.get("group", "")
                shop_item_edits[label] = ch["value"]
                s = shop_by_label(label)
                if s:
                    touched_files.add(s["file"])
            continue
        if cat["kind"] == "trainers_party" and ch.get("field") == "name":
            trainer_name_edits[ch.get("group", "")] = ch["value"]
            touched_files.add(cat["file2"])
            continue
        if cat["kind"] in ("evos", "learnset", "trainers_party"):
            blocklist_edits[(cat["id"], ch.get("group", ""))] = ch["value"]
            touched_files.add(cat["file"])
            if cat.get("file2"):
                touched_files.add(cat["file2"])
            continue
        if cat["kind"] == "struct":
            struct_edits.setdefault(cat["file"], []).append((int(ch["i"]), ch["value"]))
            touched_files.add(cat["file"])
            continue
        if cat["kind"] == "groups":
            grp = ch.get("group", "")
            strinc_edits.setdefault((cat["id"], grp), []).append((ch["i"], ch["value"]))
            touched_files.add(cat["path"].format(group=grp))
            continue
        if cat["kind"] == "json":
            json_edits.setdefault(cat["file"], []).append((cat, int(ch["i"]), ch["field"], ch["value"]))
            touched_files.add(cat["file"])
        elif cat["kind"] == "field":
            source_edits.setdefault(cat["file"], []).append((int(ch["i"]), ch["value"], {"kind": "field"}))
            touched_files.add(cat["file"])
        elif cat["kind"] == "paired2":
            # nom et description dans DEUX fichiers, apparies par identite
            parsed = parse_paired2(cat)
            i = int(ch["i"])
            if i < 0 or i >= len(parsed):
                warnings.append("%s entree %d introuvable" % (cat["id"], i))
                continue
            e = parsed[i]
            if ch["field"] == "name":
                source_edits.setdefault(cat["name_file"], []).append((e["_nl"], ch["value"], {"kind": "array"}))
                touched_files.add(cat["name_file"])
            elif e["_dl"] is None:
                warnings.append("%s : pas de description a editer" % e["key"])
            else:
                source_edits.setdefault(cat["desc_file"], []).append((e["_dl"], ch["value"], {"kind": "var"}))
                touched_files.add(cat["desc_file"])
        elif cat["kind"] == "paired":
            # field "name" -> sous-source name_kind ; "desc" -> desc_kind
            sub_kind = cat["name_kind"] if ch["field"] == "name" else cat["desc_kind"]
            parsed = parse_source_category({"file": cat["file"], "kind": sub_kind})
            i = int(ch["i"])
            if i < 0 or i >= len(parsed):
                warnings.append("%s entree %d introuvable" % (cat["file"], i))
                continue
            line_idx = parsed[i]["i"]
            source_edits.setdefault(cat["file"], []).append((line_idx, ch["value"], {"kind": sub_kind}))
            touched_files.add(cat["file"])
        else:
            source_edits.setdefault(cat["file"], []).append((int(ch["i"]), ch["value"], cat))
            touched_files.add(cat["file"])

    # Sauvegarde de securite
    backup_dir = backup_files(sorted(touched_files))

    applied = 0

    # Editions des fichiers sources (.h/.c) : on reconstruit par numero de ligne
    for file_rel, edits in source_edits.items():
        path = ROOT / file_rel
        lines = read_text(path).splitlines(keepends=True)
        rx = None
        for idx, new_value, cat in edits:
            k = cat["kind"]
            rx = ARRAY_RE if k == "array" else (FIELD_RE if k == "field" else VAR_RE)
            if idx < 0 or idx >= len(lines):
                warnings.append("%s ligne %d introuvable" % (file_rel, idx))
                continue
            line = lines[idx]
            m = rx.match(line)
            if not m:
                warnings.append("%s ligne %d : format inattendu, ignoree" % (file_rel, idx))
                continue
            old_value = m.group(2)
            if old_value == new_value:
                continue
            old_token = '_("%s")' % old_value
            new_token = '_("%s")' % new_value
            if line.count(old_token) != 1:
                warnings.append("%s ligne %d : remplacement ambigu, ignoree" % (file_rel, idx))
                continue
            lines[idx] = line.replace(old_token, new_token, 1)
            applied += 1
        write_text(path, "".join(lines))

    # Edition des fichiers JSON : remplacement CIBLE ligne par ligne pour
    # preserver exactement le formatage d'origine (encodage des accents
    # parfois irregulier selon le fichier ; on ne touche que les valeurs editees).
    # On apparie : Nieme valeur affichee <-> Nieme ligne "<json_key>": du fichier.
    for file_rel, edits in json_edits.items():
        path = ROOT / file_rel
        lines = read_text(path).splitlines(keepends=True)
        cache = {}  # json_key -> liste des numeros de lignes
        for cat, i, field, value in edits:
            jk = dict((c, j) for c, j in cat["fields"]).get(field)
            if not jk:
                continue
            if jk not in cache:
                cache[jk] = [n for n, l in enumerate(lines)
                             if re.match(r'^\s*"' + re.escape(jk) + r'"\s*:', l)]
            line_list = cache[jk]
            if i < 0 or i >= len(line_list):
                warnings.append("%s %s #%d introuvable" % (file_rel, jk, i))
                continue
            ln = line_list[i]
            line = lines[ln]
            pat = re.compile(r'("' + re.escape(jk) + r'"\s*:\s*)"((?:\\.|[^"\\])*)"')
            m = pat.search(line)
            if not m:
                warnings.append("%s ligne %d : format inattendu" % (file_rel, ln))
                continue
            old_raw = m.group(2)
            new_raw = json.dumps(value, ensure_ascii=cat.get("ascii", True))[1:-1]
            if old_raw == new_raw:
                continue
            lines[ln] = pat.sub(lambda mm: mm.group(1) + '"' + new_raw + '"', line, count=1)
            applied += 1
        write_text(path, "".join(lines))

    # Champs de struct (stats Pokemon) : remplacement cible de la valeur
    for file_rel, edits in struct_edits.items():
        path = ROOT / file_rel
        lines = read_text(path).splitlines(keepends=True)
        target = {}
        for i, val in edits:
            target[int(i)] = val
        for ln, val in target.items():
            if ln < 0 or ln >= len(lines):
                warnings.append("%s ligne %d introuvable" % (file_rel, ln))
                continue
            m = STRUCT_FIELD_RE.match(lines[ln])
            if not m:
                warnings.append("%s ligne %d : format inattendu" % (file_rel, ln))
                continue
            if m.group(3) == val:
                continue
            lines[ln] = m.group(1) + val + m.group(4)
            applied += 1
        write_text(path, "".join(lines))

    # Blocs par espece (evolutions, learnsets) : regeneration du bloc
    for (cat_id, grp), value in blocklist_edits.items():
        cat = cat_by_id[cat_id]
        if cat["kind"] == "evos":
            a, w = save_evos(cat, grp, value)
        elif cat["kind"] == "trainers_party":
            a, w = save_trainer_party(cat, grp, value)
        else:
            a, w = save_learnset(cat, grp, value)
        applied += a
        warnings.extend(w)

    # Noms de dresseurs (.trainerName dans trainers.h)
    for trainer, name in trainer_name_edits.items():
        a, w = save_trainer_name(cat_by_id["trainer_party"], trainer, name)
        applied += a
        warnings.extend(w)

    # Magasins : listes d'items (.2byte dans scripts.inc) + prix (items.json)
    for label, text in shop_item_edits.items():
        s = shop_by_label(label)
        if not s:
            warnings.append("magasin %s introuvable" % label)
            continue
        items = [x.strip() for x in text.split("\n") if x.strip()]
        a, w = save_shop(s["file"], label, items)
        applied += a
        warnings.extend(w)
    for item_id, price in shop_price_edits.items():
        try:
            applied += save_item_price(item_id, int(price))
        except ValueError:
            warnings.append("prix invalide pour %s : %s" % (item_id, price))
    if shop_item_edits or shop_price_edits:
        _shop_cache.clear()  # invalider le cache des magasins/prix

    # Categories groupees (dialogues .inc + code C), dispatch par subkind
    for (cat_id, grp), edits in strinc_edits.items():
        cat = cat_by_id[cat_id]
        if cat.get("subkind") == "csource":
            a, w = save_csource_file(cat, grp, edits)
        else:
            a, w = save_strinc_file(cat, grp, edits)
        applied += a
        warnings.extend(w)

    return {"applied": applied, "warnings": warnings, "backup": backup_dir}


# --- Serveur HTTP ----------------------------------------------------------
class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, *args):
        pass  # silencieux

    def _send(self, code, body, ctype="application/json; charset=utf-8"):
        data = body.encode("utf-8") if isinstance(body, str) else body
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            self._send(200, HTML, "text/html; charset=utf-8")
        elif self.path == "/api/data":
            self._send(200, json.dumps(build_payload(), ensure_ascii=False))
        elif self.path.startswith("/api/group"):
            q = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(q.query)
            cat_id = params.get("cat", [""])[0]
            group = params.get("group", [""])[0]
            cat = next((c for c in CATEGORIES if c["id"] == cat_id), None)
            if cat is None or cat["kind"] not in GROUPED_KINDS:
                self._send(404, json.dumps({"error": "categorie inconnue"}))
                return
            try:
                entries = parse_group(cat, group)
                self._send(200, json.dumps({"entries": entries, "keycol": cat.get("keycol", "Texte")}, ensure_ascii=False))
            except Exception as e:
                self._send(500, json.dumps({"error": str(e)}))
        elif self.path == "/api/lists":
            self._send(200, json.dumps(option_lists(), ensure_ascii=False))
        elif self.path == "/api/shops":
            try:
                self._send(200, json.dumps({"shops": discover_shops(), "prices": item_prices()}, ensure_ascii=False))
            except Exception as e:
                self._send(500, json.dumps({"error": str(e)}))
        elif self.path == "/api/trainers":
            cat = next((c for c in CATEGORIES if c["id"] == "trainer_party"), None)
            if cat is None:
                self._send(404, json.dumps({"error": "categorie absente"}))
                return
            try:
                self._send(200, json.dumps(all_trainer_parties(cat), ensure_ascii=False))
            except Exception as e:
                self._send(500, json.dumps({"error": str(e)}))
        elif self.path.startswith("/api/gfx"):
            params = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query)
            species = params.get("species", [""])[0]
            d = gfx_dir(species)
            if d is None:
                self._send(404, json.dumps({"error": "espece inconnue"}))
                return
            payload = {
                "species": species,
                "front": (d / "front.png").exists(),
                "back": (d / "back.png").exists(),
                "icon_png": (d / "icon.png").exists(),
                "normal": read_jasc(d / "normal.pal"),
                "shiny": read_jasc(d / "shiny.pal"),
                "cry_label": current_cry_label(species),
                "name": species_name_line(species)[1],
                "pokedex": pokedex_get(species),
                "pokedex_ok": species in pokedex_symbol_map(),
                "stats": stats_fields(species),
                "evos": evos_get(species),
                "learnset": learnset_get(species),
                "learnset_ok": species in learnset_symbol_map(),
            }
            payload.update(current_icon(species))
            self._send(200, json.dumps(payload))
        elif self.path.startswith("/api/sprite"):
            params = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query)
            species = params.get("species", [""])[0]
            which = params.get("which", ["front"])[0]
            recolor = params.get("recolor", [""])[0]
            d = gfx_dir(species)
            png = d / ("%s.png" % which) if d else None
            if png is None or not png.exists():
                self._send(404, json.dumps({"error": "sprite absent"}))
                return
            data = open(png, "rb").read()
            if recolor:
                try:
                    data = png_recolor(data, palette_for(species, recolor))
                except Exception:
                    pass
            self._send(200, data, "image/png")
        else:
            self._send(404, json.dumps({"error": "not found"}))

    def do_POST(self):
        if self.path not in ("/api/save", "/api/gfx_action"):
            self._send(404, json.dumps({"error": "not found"}))
            return
        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length).decode("utf-8")
        try:
            body = json.loads(raw)
            if self.path == "/api/save":
                result = save_changes(body.get("changes", []))
            else:
                result = gfx_action(body)
            self._send(200, json.dumps(result, ensure_ascii=False))
        except Exception as e:
            self._send(500, json.dumps({"error": str(e)}))


HTML = r"""<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="utf-8">
<title>Editeur de textes - pokefirered</title>
<style>
  :root { --bg:#1e1f26; --panel:#2a2c36; --accent:#e3350d; --txt:#eceff4; --muted:#9aa0b0; --ok:#37b24d; --warn:#f59f00; }
  * { box-sizing: border-box; }
  body { margin:0; font-family: "Segoe UI", system-ui, sans-serif; background:var(--bg); color:var(--txt); }
  header { background:var(--panel); padding:10px 16px; display:flex; align-items:center; gap:14px; position:sticky; top:0; z-index:10; box-shadow:0 2px 6px rgba(0,0,0,.4); flex-wrap:wrap; }
  header h1 { font-size:16px; margin:0; color:var(--accent); }
  select, input[type=text] { background:#1b1c22; color:var(--txt); border:1px solid #3a3d4a; border-radius:6px; padding:6px 9px; font-size:14px; }
  #search { width:240px; }
  button { background:var(--accent); color:#fff; border:0; border-radius:6px; padding:8px 14px; font-size:14px; cursor:pointer; }
  button:disabled { background:#555; cursor:default; }
  button.secondary { background:#3a3d4a; }
  #status { margin-left:auto; font-size:13px; color:var(--muted); }
  #count { color:var(--muted); font-size:13px; }
  .wrap { padding:12px 16px 80px; }
  table { border-collapse:collapse; width:100%; }
  th { text-align:left; padding:6px 8px; color:var(--muted); font-weight:600; font-size:12px; text-transform:uppercase; position:sticky; top:54px; background:var(--bg); }
  td { padding:4px 8px; border-bottom:1px solid #2f3140; vertical-align:top; }
  td.key { font-family:Consolas,monospace; font-size:12px; color:var(--muted); white-space:nowrap; padding-top:9px; }
  input.val, textarea.val { width:100%; background:#1b1c22; color:var(--txt); border:1px solid #3a3d4a; border-radius:5px; padding:6px 8px; font-size:14px; font-family:Consolas,monospace; }
  textarea.val { resize:vertical; min-height:34px; line-height:1.35; }
  tr.changed input.val, tr.changed textarea.val { border-color:var(--warn); background:#2a2615; }
  .len { font-size:11px; color:var(--muted); }
  .len.over { color:var(--accent); font-weight:bold; }
  .legend { font-size:12px; color:var(--muted); margin:6px 0 12px; line-height:1.5; }
  code { background:#1b1c22; padding:1px 5px; border-radius:4px; color:#ffd43b; }
  .changed-badge { background:var(--warn); color:#000; border-radius:10px; padding:1px 8px; font-size:12px; }
  /* Disposition en colonnes (masonry) : empile les cartes de hauteurs variees
     sans laisser de trous, et remplit la largeur de l'ecran. */
  .gfx { column-width:330px; column-gap:16px; }
  .card { background:var(--panel); border:1px solid #3a3d4a; border-radius:8px; padding:14px;
          margin:0 0 16px; break-inside:avoid; -webkit-column-break-inside:avoid; }
  .card h3 { margin:0 0 10px; color:var(--accent); font-size:15px; display:flex; align-items:center; gap:8px; }
  .card h3::before { content:""; width:4px; height:15px; background:var(--accent); border-radius:2px; }
  .row2 { display:flex; gap:12px; flex-wrap:wrap; }
  .row2 > div { min-width:0; }
  .spr { image-rendering:pixelated; width:128px; height:128px; background:#11131a; border:1px solid #3a3d4a; border-radius:4px; display:inline-block; }
  .spr.none { color:var(--muted); font-size:12px; text-align:center; line-height:128px; }
  .sw { display:inline-block; width:22px; height:22px; border:1px solid #000; border-radius:3px; margin:1px; vertical-align:middle; }
  .lbl { font-size:12px; color:var(--muted); margin-bottom:4px; }
  .hint { font-size:11px; color:var(--muted); margin:8px 0 0; }
  .card hr { border:0; border-top:1px solid #3a3d4a; margin:12px 0; }
  .card input[type=file] { font-size:12px; margin-top:6px; color:var(--txt); max-width:100%; }
  .card { overflow:hidden; }
  /* Editeur structure des equipes de dresseurs (accordeon) */
  .trnlist { display:flex; flex-direction:column; gap:4px; max-width:1180px; }
  .trn { background:var(--panel); border:1px solid #3a3d4a; border-radius:6px; overflow:hidden; }
  .trn.changed { border-color:var(--warn); }
  .trn-head { display:flex; align-items:center; gap:12px; padding:7px 10px; cursor:pointer; user-select:none; }
  .trn-head:hover { background:#30323d; }
  .trn-name { font-family:Consolas,monospace; font-size:12px; color:var(--accent); flex:0 0 270px;
              white-space:nowrap; overflow:hidden; text-overflow:ellipsis; }
  .trn-tname { flex:0 0 96px; font-size:12px; color:var(--ok); font-weight:600;
               white-space:nowrap; overflow:hidden; text-overflow:ellipsis; }
  .trn-sum { flex:1; font-size:12px; color:var(--muted); white-space:nowrap; overflow:hidden; text-overflow:ellipsis; }
  .tnamerow { display:flex; align-items:center; gap:8px; margin-bottom:11px; }
  .tnamerow label { font-size:12px; color:var(--muted); }
  .tnamerow .tname { background:#1b1c22; color:var(--txt); border:1px solid #3a3d4a; border-radius:5px;
                     padding:5px 8px; font-size:13px; font-family:Consolas,monospace; width:220px; }
  .trn-caret { color:var(--muted); font-size:11px; }
  .trn-body { padding:8px 10px 12px; border-top:1px solid #3a3d4a; }
  .monhead { display:grid; grid-template-columns:54px 1.3fr 54px 1.3fr 2.4fr 24px; gap:8px;
             font-size:11px; color:var(--muted); padding-bottom:5px; }
  .monlist { display:flex; flex-direction:column; gap:6px; }
  .mon { display:grid; grid-template-columns:54px 1.3fr 54px 1.3fr 2.4fr 24px; gap:8px; align-items:center; }
  .mon input { background:#1b1c22; color:var(--txt); border:1px solid #3a3d4a; border-radius:5px;
               padding:5px 6px; font-size:12px; font-family:Consolas,monospace; width:100%; min-width:0; }
  .mmv-wrap { display:grid; grid-template-columns:1fr 1fr 1fr 1fr; gap:4px; }
  .mdel { background:#4a3030 !important; color:#ff8787 !important; padding:4px 0 !important; font-size:12px !important; }
  .maddbtn { margin-top:9px; background:#2f7d40; padding:6px 12px; font-size:13px; }
  .trnsep { display:flex; align-items:center; gap:10px; color:var(--muted); font-size:13px; margin:20px 0 10px; }
  .trnsep::before, .trnsep::after { content:""; flex:1; height:1px; background:#3a3d4a; }
  details summary { cursor:pointer; color:var(--muted); font-size:13px; margin:6px 0; }
  /* Magasins */
  .shophead { display:grid; grid-template-columns:1fr 110px 24px; gap:8px; font-size:11px; color:var(--muted); padding-bottom:5px; }
  .shoplist { display:flex; flex-direction:column; gap:6px; }
  .shoprow { display:grid; grid-template-columns:1fr 110px 24px; gap:8px; align-items:center; }
  .shoprow input { background:#1b1c22; color:var(--txt); border:1px solid #3a3d4a; border-radius:5px;
                   padding:5px 6px; font-size:12px; font-family:Consolas,monospace; width:100%; min-width:0; }
  .card button { margin-left:6px; padding:5px 10px; }
</style>
</head>
<body>
<header>
  <h1>Editeur de textes</h1>
  <select id="cat"></select>
  <select id="group" style="display:none; max-width:240px"></select>
  <input type="text" id="search" placeholder="Rechercher (cle ou texte)...">
  <span id="count"></span>
  <button id="save" disabled>Enregistrer (<span id="nchanged">0</span>)</button>
  <button id="reload" class="secondary">Recharger</button>
  <span id="status"></span>
</header>
<div class="wrap">
  <div class="legend">
    Codes de controle a <b>preserver</b> : <code>\n</code> retour ligne &middot;
    <code>\p</code> nouvelle page &middot; <code>\l</code> defilement &middot;
    <code>{PLAYER}</code>, <code>{RIVAL}</code>, <code>{PKMN}</code>, <code>{STR_VAR_1}</code> variables.
    N'utilisez pas de guillemet droit <code>"</code> brut dans le texte.
    <br><b>Dialogues</b> : choisissez d'abord une <b>carte</b>. Un retour a la ligne (touche Entree)
    ne fait que scinder la ligne source (sans effet en jeu) ; les vrais sauts en jeu sont <code>\n</code> / <code>\p</code> / <code>\l</code>.
    <br><b>Evolutions</b> : une evolution par ligne, format <code>METHODE, PARAM, ESPECE</code>
    (ex. <code>EVO_LEVEL, 16, SPECIES_IVYSAUR</code>). Fonctionne sur <b>toutes</b> les especes :
    si le Pokemon n'evoluait pas, l'entree est creee ; zone videe = evolution retiree. Max 6 par espece (EVOS_PER_MON=6).
    <br><b>Attaques par niveau</b> : une par ligne, format <code>niveau, MOVE_X</code> (ex. <code>7, MOVE_LEECH_SEED</code>).
    <br><b>Equipes de dresseurs</b> : <b>cliquez un dresseur</b> pour deplier son editeur (jusqu'a 6 Pokemon).
    Chaque ligne = niveau, espece, IV, objet, 4 attaques, via des champs a autocompletion (tapez quelques
    lettres : <code>char</code> &rarr; Charmander). Attaques laissees vides = attaques par defaut du Pokemon.
    Boutons <b>＋ ajouter</b> / <b>✕ retirer</b>. Le type de struct et le macro sont recalcules automatiquement.
    Les 88 dresseurs RS (inutilises en FireRed) sont en bas, dans une section repliable.
    <br><b>Magasins</b> : cliquez un magasin (Poke Mart ou etage du Centre commercial) pour gerer
    les objets en vente (＋ ajouter / ✕ retirer, autocompletion). Le <b>prix</b> affiche est celui de
    l'objet (items.json) &mdash; le modifier change son prix dans <b>tout le jeu</b>.
    <br><b>Pokemon</b> : page unique regroupant nom, description (Pokedex), stats, sprites, palette,
    cri et mini-icone. Les champs nom/description/stats s'enregistrent immediatement (bouton dedie, backup auto).
    <br><b>Noms d'attaques</b> : 12 caracteres max (limite du jeu, pas d'indicateur visuel sur la page fusionnee).
    Dans les descriptions (attaques, Pokedex), gardez les <code>\n</code> <b>litteraux</b> (n'appuyez pas sur Entree).
  </div>
  <div id="tablewrap"></div>
</div>
<script>
let DATA = null;
let current = null;          // id de categorie
let currentGroup = null;     // groupe (carte) pour les categories groupees
const groupCache = {};       // "cat|group" -> entries
const changes = new Map();   // ckey -> {category, group, i, field, value}
let trainerData = null;      // cache de la vue empilee des dresseurs

function ckey(cat, group, i, field) { return cat + "|" + group + "|" + i + "|" + field; }

async function load() {
  trainerData = null;   // refetch des dresseurs apres rechargement/sauvegarde
  shopData = null;
  const r = await fetch("/api/data");
  DATA = await r.json();
  const sel = document.getElementById("cat");
  sel.innerHTML = "";
  DATA.categories.forEach(c => {
    const o = document.createElement("option");
    o.value = c.id; o.textContent = c.label + " (" + c.count + (c.grouped ? " cartes" : "") + ")";
    sel.appendChild(o);
  });
  if (!current) current = DATA.categories[0].id;
  sel.value = current;
  await selectCategory();
}

function getCat(id) { return DATA.categories.find(c => c.id === id); }

async function selectCategory() {
  const c = getCat(current);
  const gsel = document.getElementById("group");
  if (c.kind === "trainers_party") {
    gsel.style.display = "none";
    currentGroup = "";
    await renderTrainers();
    return;
  }
  if (c.kind === "shops") {
    gsel.style.display = "none";
    currentGroup = "";
    await renderShops();
    return;
  }
  if (c.grouped) {
    gsel.style.display = "";
    gsel.innerHTML = "";
    c.groups.forEach(g => { const o = document.createElement("option"); o.value = g; o.textContent = g; gsel.appendChild(o); });
    if (!currentGroup || c.groups.indexOf(currentGroup) === -1) currentGroup = c.groups[0] || "";
    gsel.value = currentGroup;
    if (c.kind !== "graphics") await ensureGroup();
  } else {
    gsel.style.display = "none";
    currentGroup = "";
  }
  render();
}

async function ensureGroup() {
  const key = current + "|" + currentGroup;
  if (!groupCache[key] && currentGroup) {
    setStatus("Chargement de " + currentGroup + "...");
    const r = await fetch("/api/group?cat=" + encodeURIComponent(current) + "&group=" + encodeURIComponent(currentGroup));
    const d = await r.json();
    groupCache[key] = d.entries || [];
    setStatus("");
  }
}

function getEntries() {
  const c = getCat(current);
  if (c.grouped) return groupCache[current + "|" + currentGroup] || [];
  return c.entries;
}

function render() {
  const c = getCat(current);
  if (c.kind === "graphics") { renderGraphics(); return; }
  if (c.kind === "trainers_party") { renderTrainers(); return; }
  if (c.kind === "shops") { renderShops(); return; }
  const entries = getEntries();
  const filter = document.getElementById("search").value.toLowerCase();
  const rows = [];
  let shown = 0;
  entries.forEach(e => {
    const hay = (e.key + " " + (e.value||"") + " " + (e.name||"") + " " + (e.desc||"")).toLowerCase();
    if (filter && hay.indexOf(filter) === -1) return;
    shown++;
    rows.push(c.twofields ? rowTwo(c, e) : rowOne(c, e));
  });
  const head = c.twofields
    ? "<tr><th style='width:170px'>" + (c.keycol || "Cle") + "</th><th style='width:30%'>Nom</th><th>Description</th></tr>"
    : "<tr><th style='width:260px'>" + (c.keycol || "Cle") + "</th><th>Texte</th></tr>";
  document.getElementById("tablewrap").innerHTML = "<table>" + head + rows.join("") + "</table>";
  document.getElementById("count").textContent = shown + " / " + entries.length + " affiches";
}

function esc(s) { return (s||"").replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;").replace(/"/g,"&quot;"); }

function dataAttrs(c, e, field) {
  return `data-cat="${c.id}" data-group="${esc(currentGroup||"")}" data-i="${esc(String(e.i))}" data-field="${field}"`;
}

function rowOne(c, e) {
  const k = ckey(c.id, currentGroup||"", e.i, "value");
  const cur = changes.has(k) ? changes.get(k).value : e.value;
  const changed = changes.has(k) ? " changed" : "";
  const blockEditor = (c.kind === "evos" || c.kind === "learnset" || c.kind === "trainers_party" || c.kind === "groups");
  const multiline = blockEditor || (cur && cur.length > 40) || c.id.indexOf("desc") !== -1;
  const a = dataAttrs(c, e, "value");
  const nLines = ((cur||"").match(/\n/g) || []).length + 1;
  const rows = Math.min(Math.max(nLines + 1, 3), 30);
  const field = multiline
    ? `<textarea class="val" rows="${rows}" ${a} oninput="onEdit(this)">${esc(cur)}</textarea>`
    : `<input class="val" type="text" ${a} value="${esc(cur)}" oninput="onEdit(this)">`;
  const lenInfo = c.max ? `<div class="len ${cur.length>c.max?'over':''}" data-len="${esc(k)}">${cur.length}/${c.max}</div>` : "";
  return `<tr class="row${changed}"><td class="key">${esc(e.key)}</td><td>${field}${lenInfo}</td></tr>`;
}

function rowTwo(c, e) {
  const kn = ckey(c.id, "", e.i, "name");
  const kd = ckey(c.id, "", e.i, "desc");
  const cn = changes.has(kn) ? changes.get(kn).value : e.name;
  const cd = changes.has(kd) ? changes.get(kd).value : e.desc;
  const an = dataAttrs(c, e, "name"), ad = dataAttrs(c, e, "desc");
  return `<tr data-row2><td class="key">${esc(e.key)}</td>`
    + `<td class="${changes.has(kn)?'changed':''}"><input class="val" type="text" ${an} value="${esc(cn)}" oninput="onEdit(this)"></td>`
    + `<td class="${changes.has(kd)?'changed':''}"><textarea class="val" ${ad} oninput="onEdit(this)">${esc(cd)}</textarea></td></tr>`;
}

function origValue(cat, group, i, field) {
  const c = getCat(cat);
  const entries = c.grouped ? (groupCache[cat + "|" + group] || []) : c.entries;
  const e = entries.find(x => String(x.i) === String(i));
  if (!e) return "";
  return field === "name" ? e.name : field === "desc" ? e.desc : e.value;
}

function onEdit(el) {
  const cat = el.dataset.cat, group = el.dataset.group, i = el.dataset.i, field = el.dataset.field;
  const k = ckey(cat, group, i, field);
  const v = el.value;
  if (v === origValue(cat, group, i, field)) changes.delete(k);
  else changes.set(k, {category:cat, group:group, i:i, field:field, value:v});
  const cell = el.closest("td"), row = el.closest("tr");
  if (row) {
    if (row.hasAttribute("data-row2")) cell.className = changes.has(k) ? "changed" : (cell.classList.contains("key")?"key":"");
    else row.classList.toggle("changed", changes.has(k));
  } else {
    const box = el.closest(".trn");
    if (box) box.classList.toggle("changed", changes.has(k));
  }
  const lenDiv = document.querySelector('[data-len="' + (window.CSS && CSS.escape ? CSS.escape(k) : k) + '"]');
  if (lenDiv) { const c = getCat(cat); lenDiv.textContent = v.length + "/" + c.max; lenDiv.classList.toggle("over", v.length > c.max); }
  refreshSave();
}

function refreshSave() {
  document.getElementById("nchanged").textContent = changes.size;
  document.getElementById("save").disabled = changes.size === 0;
}

async function save() {
  const btn = document.getElementById("save");
  btn.disabled = true;
  setStatus("Enregistrement...");
  const payload = {changes: Array.from(changes.values())};
  let res;
  try {
    const r = await fetch("/api/save", {method:"POST", headers:{"Content-Type":"application/json"}, body: JSON.stringify(payload)});
    res = await r.json();
  } catch (e) {
    const m = "ECHEC : impossible de contacter le serveur. Le programme Python est-il toujours lance ? (relancez Lancer_editeur.bat)";
    setStatus(m, true); alert(m); btn.disabled = false; return;
  }
  if (res.error) { setStatus("ERREUR : " + res.error, true); alert("ERREUR : " + res.error); btn.disabled=false; return; }
  let msg = res.applied + " modification(s) enregistree(s).";
  if (res.warnings && res.warnings.length) { msg += " Avertissements : " + res.warnings.join("; "); alert(msg); }
  setStatus(msg);
  changes.clear();
  for (const key in groupCache) delete groupCache[key];  // valeurs sur disque modifiees
  refreshSave();
  await load();
}

function setStatus(msg, isErr) {
  const s = document.getElementById("status");
  s.textContent = msg;
  s.style.color = isErr ? "#e3350d" : "#37b24d";
  if (!isErr && msg) setTimeout(()=>{ if (s.textContent === msg) s.textContent=""; }, 6000);
}

// --- Editeur structure des dresseurs (datalists partagees + accordeon) -----
let optLists = null;
const speciesLabel = {};
const itemLabel = {};
const trainerNameOrig = {};   // TRAINER_X -> nom d'origine (pour detection de revert)
let shopData = null;
const shopPrices = {};        // ITEM_X -> prix d'origine

async function ensureLists() {
  if (optLists) return;
  optLists = await (await fetch("/api/lists")).json();
  optLists.species.forEach(o => speciesLabel[o.v] = o.l);
  optLists.items.forEach(o => itemLabel[o.v] = o.l);
}
function datalistHtml() {
  // Option = constante seule (1 ligne). Pas de libelle -> evite l'affichage 2 lignes.
  const dl = (id, arr) => `<datalist id="${id}">`
    + arr.map(o => `<option value="${esc(o.v)}"></option>`).join("") + `</datalist>`;
  return dl("dl-species", optLists.species) + dl("dl-items", optLists.items) + dl("dl-moves", optLists.moves);
}
function parseMonLine(line) {
  const toks = line.split(",").map(s => s.trim()).filter(s => s);
  if (toks.length < 2) return null;
  const lvl = parseInt(toks[0]); if (isNaN(lvl)) return null;
  const m = {lvl: lvl, species: toks[1], iv: 0, item: "", moves: []};
  for (let j = 2; j < toks.length; j++) {
    const eq = toks[j].indexOf("="); if (eq < 0) continue;
    const k = toks[j].slice(0, eq).trim().toLowerCase(), v = toks[j].slice(eq + 1).trim();
    if (k === "iv") m.iv = parseInt(v) || 0;
    else if (k === "item") m.item = v;
    else if (k === "moves") m.moves = v.split("/").map(x => x.trim()).filter(x => x);
  }
  return m;
}
function parseParty(text) {
  return (text || "").split("\n").map(l => l.trim()).filter(l => l).map(parseMonLine).filter(m => m);
}
function monToLine(m) {
  const parts = [m.lvl + ", " + m.species];
  if (m.iv && m.iv != 0) parts.push("iv=" + m.iv);
  if (m.item && m.item !== "ITEM_NONE") parts.push("item=" + m.item);
  const mv = (m.moves || []).filter(x => x && x !== "MOVE_NONE");
  if (mv.length) parts.push("moves=" + mv.join("/"));
  return parts.join(", ");
}
function trnSummaryText(text) {
  const mons = parseParty(text);
  if (!mons.length) return "(equipe vide)";
  return mons.map(m => "Lv" + m.lvl + " " + (speciesLabel[m.species] || m.species.replace("SPECIES_", ""))).join(", ");
}
function trnOrig(key) { return ((groupCache["trainer_party|" + key] || [])[0] || {}).value || ""; }
function trnValue(key) {
  const k = ckey("trainer_party", key, key, "value");
  return changes.has(k) ? changes.get(k).value : trnOrig(key);
}
function monRow(m) {
  const mv = m.moves || [];
  return `<div class="mon">
    <input type="number" class="mlvl" min="1" max="100" value="${m.lvl || 5}" title="Niveau" oninput="trnChange(this)">
    <input list="dl-species" class="mspc" value="${esc(m.species || '')}" placeholder="SPECIES_..." oninput="trnChange(this)">
    <input type="number" class="miv" min="0" max="255" value="${m.iv || 0}" title="IV (0-255)" oninput="trnChange(this)">
    <input list="dl-items" class="mitm" value="${esc(m.item || '')}" placeholder="(aucun)" oninput="trnChange(this)">
    <div class="mmv-wrap">
      <input list="dl-moves" class="mmv" value="${esc(mv[0] || '')}" placeholder="att1" oninput="trnChange(this)">
      <input list="dl-moves" class="mmv" value="${esc(mv[1] || '')}" placeholder="att2" oninput="trnChange(this)">
      <input list="dl-moves" class="mmv" value="${esc(mv[2] || '')}" placeholder="att3" oninput="trnChange(this)">
      <input list="dl-moves" class="mmv" value="${esc(mv[3] || '')}" placeholder="att4" oninput="trnChange(this)">
    </div>
    <button class="mdel" title="retirer ce Pokemon" onclick="trnDelMon(this)">&times;</button>
  </div>`;
}
function trnNameVal(key) {
  const k = ckey("trainer_party", key, key, "name");
  return changes.has(k) ? changes.get(k).value : (trainerNameOrig[key] || "");
}
function buildTrnEditor(key) {
  const body = document.getElementById("body-" + key);
  const mons = parseParty(trnValue(key));
  body.innerHTML =
    `<div class="tnamerow"><label>Nom du dresseur</label>`
    + `<input class="tname" value="${esc(trnNameVal(key))}" placeholder="(sans nom)" oninput="trnNameChange('${key}', this)"></div>`
    + `<div class="monhead"><span>Niv</span><span>Espece</span><span>IV</span><span>Objet</span><span>Attaques (vide = attaques par defaut)</span><span></span></div>`
    + `<div class="monlist" data-key="${esc(key)}">${mons.map(monRow).join("")}</div>`
    + `<button class="maddbtn" onclick="trnAddMon('${key}')">+ ajouter un Pokemon</button>`;
}
function updateTrnChanged(key) {
  const vk = ckey("trainer_party", key, key, "value");
  const nk = ckey("trainer_party", key, key, "name");
  const trn = document.querySelector('.trn[data-key="' + (window.CSS && CSS.escape ? CSS.escape(key) : key) + '"]');
  if (trn) trn.classList.toggle("changed", changes.has(vk) || changes.has(nk));
}
function trnNameChange(key, el) {
  const v = el.value;
  const k = ckey("trainer_party", key, key, "name");
  if (v === (trainerNameOrig[key] || "")) changes.delete(k);
  else changes.set(k, {category:"trainer_party", group:key, i:key, field:"name", value:v});
  const tn = document.getElementById("tname-" + key);
  if (tn) tn.textContent = v;
  updateTrnChanged(key);
  refreshSave();
}
function toggleTrn(key) {
  const body = document.getElementById("body-" + key);
  if (body.dataset.built !== "1") { buildTrnEditor(key); body.dataset.built = "1"; }
  const open = body.style.display === "none";
  body.style.display = open ? "" : "none";
  const trn = body.closest(".trn");
  const caret = trn && trn.querySelector(".trn-caret");
  if (caret) caret.textContent = open ? "▾" : "▸";
}
function trnSerialize(list) {
  const lines = [];
  list.querySelectorAll(".mon").forEach(r => {
    const spc = r.querySelector(".mspc").value.trim();
    if (!spc) return;
    lines.push(monToLine({
      lvl: parseInt(r.querySelector(".mlvl").value) || 5,
      species: spc,
      iv: parseInt(r.querySelector(".miv").value) || 0,
      item: r.querySelector(".mitm").value.trim(),
      moves: [...r.querySelectorAll(".mmv")].map(x => x.value.trim()),
    }));
  });
  return lines.join("\n");
}
function trnApply(key, list) {
  const text = trnSerialize(list);
  const k = ckey("trainer_party", key, key, "value");
  if (text === trnOrig(key)) changes.delete(k);
  else changes.set(k, {category:"trainer_party", group:key, i:key, field:"value", value:text});
  updateTrnChanged(key);
  const sum = document.getElementById("sum-" + key);
  if (sum) sum.textContent = trnSummaryText(text);
  refreshSave();
}
function trnChange(el) { trnApply(el.closest(".monlist").dataset.key, el.closest(".monlist")); }
function trnAddMon(key) {
  const list = document.querySelector('.monlist[data-key="' + (window.CSS && CSS.escape ? CSS.escape(key) : key) + '"]');
  if (list.querySelectorAll(".mon").length >= 6) { alert("6 Pokemon maximum par dresseur."); return; }
  list.insertAdjacentHTML("beforeend", monRow({lvl:5, species:"", iv:0, item:"", moves:[]}));
}
function trnDelMon(btn) {
  const list = btn.closest(".monlist");
  btn.closest(".mon").remove();
  trnApply(list.dataset.key, list);
}

async function renderTrainers() {
  const wrap = document.getElementById("tablewrap");
  await ensureLists();
  if (!trainerData) {
    setStatus("Chargement des dresseurs...");
    try { trainerData = await (await fetch("/api/trainers")).json(); }
    catch (e) { wrap.innerHTML = "Erreur de chargement des dresseurs."; return; }
    trainerData.forEach(t => {
      groupCache["trainer_party|" + t.key] = [{i:t.key, value:t.value}];
      trainerNameOrig[t.key] = t.name || "";
    });
    setStatus("");
  }
  const filter = document.getElementById("search").value.toLowerCase();
  let shown = 0;
  const headFor = (t) => {
    const hay = (t.key + " " + (t.value||"")).toLowerCase();
    if (filter && hay.indexOf(filter) === -1) return "";
    shown++;
    const k = ckey("trainer_party", t.key, t.key, "value");
    const cur = changes.has(k) ? changes.get(k).value : t.value;
    const changed = changes.has(k) ? " changed" : "";
    const nk = ckey("trainer_party", t.key, t.key, "name");
    const nm = changes.has(nk) ? changes.get(nk).value : (t.name || "");
    return `<div class="trn${changed}" data-key="${esc(t.key)}">
      <div class="trn-head" onclick="toggleTrn('${t.key}')">
        <span class="trn-name">${esc(t.key)}</span>
        <span class="trn-tname" id="tname-${esc(t.key)}">${esc(nm)}</span>
        <span class="trn-sum" id="sum-${esc(t.key)}">${esc(trnSummaryText(cur))}</span>
        <span class="trn-caret">▸</span>
      </div>
      <div class="trn-body" id="body-${esc(t.key)}" style="display:none"></div>
    </div>`;
  };
  const reals = trainerData.filter(t => !t.placeholder);
  const phs = trainerData.filter(t => t.placeholder);
  wrap.innerHTML = datalistHtml()
    + `<div class="trnlist">${reals.map(headFor).join("") || '<i>aucun dresseur ne correspond au filtre</i>'}</div>`
    + `<div class="trnsep">Dresseurs RS (placeholders, inutilises en FireRed)</div>`
    + `<details${filter ? ' open' : ''}><summary>Afficher / masquer les ${phs.length} dresseurs RS</summary>`
    + `<div class="trnlist" style="margin-top:10px">${phs.map(headFor).join("")}</div></details>`;
  document.getElementById("count").textContent =
    shown + " affiches (" + reals.length + " reels + " + phs.length + " RS)";
}

// --- Magasins (Poke Marts + Centre commercial) -----------------------------
function humanize(s) { return (s || "").replace(/_/g, " ").replace(/([a-z])([A-Z])/g, "$1 $2"); }
function shopTitle(s) {
  let tag = s.key;
  if (s.key.indexOf(s.map) === 0) tag = s.key.slice(s.map.length).replace(/^_/, "");
  return humanize(s.map) + (tag ? " · " + humanize(tag) : "");
}
function shopSummary(items) {
  if (!items.length) return "(magasin vide)";
  const names = items.slice(0, 5).map(it => itemLabel[it] || it.replace("ITEM_", ""));
  return items.length + " objets : " + names.join(", ") + (items.length > 5 ? "…" : "");
}
function shopValue(key) {
  const k = ckey("shops", key, key, "items");
  if (changes.has(k)) return changes.get(k).value;
  const e = (groupCache["shops|" + key] || [])[0];
  return e ? e.value : "";
}
function shopCurPrice(it) {
  const k = ckey("shops", "__price__", it, "price");
  if (changes.has(k)) return changes.get(k).value;
  return shopPrices[it] != null ? shopPrices[it] : 0;
}
function shopRow(it) {
  return `<div class="shoprow">
    <input list="dl-items" class="sitm" value="${esc(it)}" placeholder="ITEM_..." oninput="shopItemChange(this)">
    <input type="number" class="sprice" min="0" value="${shopCurPrice(it)}" title="Prix (global, items.json)" oninput="shopPriceChange(this)">
    <button class="mdel" title="retirer cet objet" onclick="shopDelItem(this)">&times;</button>
  </div>`;
}
function buildShopEditor(key) {
  const body = document.getElementById("sbody-" + key);
  const items = shopValue(key).split("\n").filter(x => x);
  body.innerHTML =
    `<div class="shophead"><span>Objet en vente</span><span>Prix (global)</span><span></span></div>`
    + `<div class="shoplist" data-key="${esc(key)}">${items.map(shopRow).join("")}</div>`
    + `<button class="maddbtn" onclick="shopAddItem('${key}')">+ ajouter un objet</button>`
    + `<p class="hint">Le prix est celui de l'objet (items.json) : le modifier change son prix dans <b>tout le jeu</b>.</p>`;
}
function toggleShop(key) {
  const body = document.getElementById("sbody-" + key);
  if (body.dataset.built !== "1") { buildShopEditor(key); body.dataset.built = "1"; }
  const open = body.style.display === "none";
  body.style.display = open ? "" : "none";
  const trn = body.closest(".trn");
  const caret = trn && trn.querySelector(".trn-caret");
  if (caret) caret.textContent = open ? "▾" : "▸";
}
function shopSerialize(list) {
  const items = [];
  list.querySelectorAll(".sitm").forEach(inp => { const v = inp.value.trim(); if (v) items.push(v); });
  return items.join("\n");
}
function shopApplyItems(key, list) {
  const text = shopSerialize(list);
  const k = ckey("shops", key, key, "items");
  const orig = ((groupCache["shops|" + key] || [])[0] || {}).value || "";
  if (text === orig) changes.delete(k);
  else changes.set(k, {category:"shops", group:key, i:key, field:"items", value:text});
  const trn = list.closest(".trn");
  if (trn) trn.classList.toggle("changed", changes.has(k));
  const sum = document.getElementById("ssum-" + key);
  if (sum) sum.textContent = shopSummary(text.split("\n").filter(x => x));
  refreshSave();
}
function shopItemChange(el) {
  const row = el.closest(".shoprow");
  row.querySelector(".sprice").value = shopCurPrice(el.value.trim());
  const list = el.closest(".shoplist");
  shopApplyItems(list.dataset.key, list);
}
function shopPriceChange(el) {
  const it = el.closest(".shoprow").querySelector(".sitm").value.trim();
  if (!it) return;
  const k = ckey("shops", "__price__", it, "price");
  const orig = shopPrices[it] != null ? String(shopPrices[it]) : "0";
  if (el.value === orig || el.value === "") changes.delete(k);
  else changes.set(k, {category:"shops", group:"__price__", i:it, field:"price", value:el.value});
  refreshSave();
}
function shopAddItem(key) {
  const list = document.querySelector('.shoplist[data-key="' + (window.CSS && CSS.escape ? CSS.escape(key) : key) + '"]');
  list.insertAdjacentHTML("beforeend", shopRow(""));
}
function shopDelItem(btn) {
  const list = btn.closest(".shoplist");
  btn.closest(".shoprow").remove();
  shopApplyItems(list.dataset.key, list);
}
async function renderShops() {
  const wrap = document.getElementById("tablewrap");
  await ensureLists();
  if (!shopData) {
    setStatus("Chargement des magasins...");
    try { const r = await (await fetch("/api/shops")).json(); shopData = r.shops; Object.assign(shopPrices, r.prices); }
    catch (e) { wrap.innerHTML = "Erreur de chargement des magasins."; return; }
    shopData.forEach(s => { groupCache["shops|" + s.key] = [{i:s.key, value:s.items.join("\n")}]; });
    setStatus("");
  }
  const filter = document.getElementById("search").value.toLowerCase();
  let shown = 0;
  const head = (s) => {
    const hay = (s.key + " " + s.map + " " + s.items.join(" ")).toLowerCase();
    if (filter && hay.indexOf(filter) === -1) return "";
    shown++;
    const k = ckey("shops", s.key, s.key, "items");
    const items = changes.has(k) ? changes.get(k).value.split("\n").filter(x => x) : s.items;
    return `<div class="trn${changes.has(k) ? ' changed' : ''}" data-key="${esc(s.key)}">
      <div class="trn-head" onclick="toggleShop('${s.key}')">
        <span class="trn-name" style="flex:0 0 320px">${esc(shopTitle(s))}</span>
        <span class="trn-sum" id="ssum-${esc(s.key)}">${esc(shopSummary(items))}</span>
        <span class="trn-caret">▸</span>
      </div>
      <div class="trn-body" id="sbody-${esc(s.key)}" style="display:none"></div>
    </div>`;
  };
  wrap.innerHTML = datalistHtml()
    + `<div class="trnlist">${shopData.map(head).join("") || '<i>aucun magasin ne correspond au filtre</i>'}</div>`;
  document.getElementById("count").textContent = shown + " / " + shopData.length + " magasins";
}

async function renderGraphics() {
  const sp = currentGroup;
  const wrap = document.getElementById("tablewrap");
  if (!sp) { wrap.innerHTML = ""; return; }
  setStatus("Chargement " + sp + "...");
  let g;
  try { g = await (await fetch("/api/gfx?species=" + encodeURIComponent(sp))).json(); }
  catch (e) { wrap.innerHTML = "Erreur de chargement."; return; }
  setStatus("");
  const t = Date.now();
  const img = (w) => g[w]
    ? `<img src="/api/sprite?species=${sp}&which=${w}&t=${t}" class="spr">`
    : `<div class="spr none">aucun ${w}</div>`;
  const sw = (arr) => ((arr || []).map(c => `<span class="sw" title="${c.join(', ')}" style="background:rgb(${c[0]},${c[1]},${c[2]})"></span>`).join("") || "<i>—</i>");
  const opts = getCat(current).groups.map(s => `<option value="${s}"${s === sp ? ' selected' : ''}>${s}</option>`).join("");
  document.getElementById("count").textContent = sp;
  wrap.innerHTML = `
  <div class="gfx">
    <div class="card">
      <h3>Nom &amp; description</h3>
      <div class="lbl">Nom</div>
      <input class="val" type="text" id="spname" value="${esc(g.name || '')}">
      <button onclick="applyName()" style="margin-top:6px">Enregistrer le nom</button>
      <hr>
      <div class="lbl">Description (Pokedex) ${g.pokedex_ok ? '' : '<i>— aucune entree Pokedex pour cette espece</i>'}</div>
      <textarea class="val" id="spdex" rows="5" ${g.pokedex_ok ? '' : 'disabled'}>${esc(g.pokedex || '')}</textarea>
      <button onclick="applyPokedex()" ${g.pokedex_ok ? '' : 'disabled'} style="margin-top:6px">Enregistrer la description</button>
      <p class="hint">Gardez les <code>\\n</code> (sauts de ligne en jeu). 3 lignes max par page d'affichage.</p>
    </div>
    <div class="card">
      <h3>Stats</h3>
      ${(g.stats && g.stats.length) ? `
      <div style="display:grid;grid-template-columns:1fr 1fr;gap:6px 10px">
        ${g.stats.map(s => `<div style="display:flex;flex-direction:column;min-width:0">
          <span class="lbl" style="margin:0;white-space:nowrap;overflow:hidden;text-overflow:ellipsis" title="${esc(s.key)}">${esc(s.key)}</span>
          <input class="val" id="st_${esc(s.key)}" value="${esc(s.value)}" style="font-size:12px;padding:3px 5px">
        </div>`).join("")}
      </div>
      <button onclick="applyStats()" style="margin-top:8px">Enregistrer les stats</button>
      <p class="hint">Valeurs brutes (ex. <code>TYPE_GRASS</code>, <code>ABILITY_OVERGROW</code>, <code>{TYPE_GRASS, TYPE_POISON}</code>, <code>PERCENT_FEMALE(12.5)</code>).</p>
      ` : '<div class="lbl">Aucune stat inline (espece basee sur une macro).</div>'}
    </div>
    <div class="card">
      <h3>Evolutions</h3>
      <textarea class="val" id="spevos" rows="4" placeholder="(aucune)">${esc(g.evos || '')}</textarea>
      <button onclick="applyEvos()" style="margin-top:6px">Enregistrer les evolutions</button>
      <p class="hint">1 evolution/ligne : <code>METHODE, PARAM, ESPECE</code> (ex. <code>EVO_LEVEL, 16, SPECIES_IVYSAUR</code>). Zone vide = aucune evolution. Max 6.</p>
    </div>
    <div class="card">
      <h3>Attaques (par niveau)</h3>
      <textarea class="val" id="splearn" rows="9" ${g.learnset_ok ? '' : 'disabled'}>${esc(g.learnset || '')}</textarea>
      <button onclick="applyLearnset()" ${g.learnset_ok ? '' : 'disabled'} style="margin-top:6px">Enregistrer les attaques</button>
      <p class="hint">1 attaque/ligne : <code>niveau, MOVE_X</code> (ex. <code>7, MOVE_LEECH_SEED</code>).</p>
    </div>
    <div class="card">
      <h3>Sprites</h3>
      <div class="row2">
        <div><div class="lbl">Front</div>${img('front')}<br><input type="file" accept="image/png" onchange="uploadSprite(this,'front')"></div>
        <div><div class="lbl">Back</div>${img('back')}<br><input type="file" accept="image/png" onchange="uploadSprite(this,'back')"></div>
      </div>
      <p class="hint">PNG indexe (palette), memes dimensions que l'original. Sauvegarde immediate (backup auto).</p>
    </div>
    <div class="card">
      <h3>Palette</h3>
      <div class="lbl">Normale</div><div>${sw(g.normal)}</div>
      <div class="lbl" style="margin-top:8px">Shiny</div><div>${sw(g.shiny)}</div>
      <hr>
      <div class="lbl">Reutiliser la palette d'un autre Pokemon</div>
      <select id="reusepal">${opts}</select>
      <button class="secondary" onclick="previewReuse()">Previsualiser</button>
      <button onclick="applyReuse()">Appliquer</button>
      <div id="reusepreview" style="margin-top:8px"></div>
      <hr>
      <div class="lbl">Importer un fichier .pal (JASC)</div>
      <select id="importwhich"><option value="normal">normale</option><option value="shiny">shiny</option><option value="both">les deux</option></select>
      <input type="file" accept=".pal" onchange="importPalette(this)">
    </div>
    <div class="card">
      <h3>Cri</h3>
      <div class="lbl">Cri actuel : <b>${esc(g.cry_label || '?')}</b></div>
      <div class="lbl" style="margin-top:8px">Utiliser le cri d'un autre Pokemon</div>
      <select id="crysrc">${opts}</select>
      <button onclick="applyCry()">Appliquer</button>
      <p class="hint">Remplace l'entree dans gCryTable + gCryTable_Reverse (cri normal et inverse).</p>
    </div>
    <div class="card">
      <h3>Mini-icone (equipe / boite PC)</h3>
      <div class="row2">
        <div><div class="lbl">Actuelle</div>${g.icon_png ? `<img src="/api/sprite?species=${sp}&which=icon&t=${t}" class="spr">` : '<div class="spr none">aucune</div>'}</div>
        <div><div class="lbl">Apercu source</div><div id="iconpreview"><div class="spr none">—</div></div></div>
      </div>
      <div class="lbl" style="margin-top:8px">Graphisme : <b>${esc(g.icon || '?')}</b> &middot; palette : <b>${esc(String(g.icon_pal))}</b></div>
      <div class="lbl" style="margin-top:8px">Utiliser l'icone d'un autre Pokemon</div>
      <select id="iconsrc" onchange="previewIcon()">${opts}</select>
      <button class="secondary" onclick="previewIcon()">Previsualiser</button>
      <button onclick="applyIcon()">Appliquer</button>
      <p class="hint">Copie le graphisme ET l'index de palette (gMonIconTable + gMonIconPaletteIndices).</p>
    </div>
  </div>`;
  previewIcon();
}

function fileToBase64(file, cb) { const r = new FileReader(); r.onload = () => cb(r.result.split(',')[1]); r.readAsDataURL(file); }
async function gfxPost(body) {
  try { return await (await fetch("/api/gfx_action", {method:"POST", headers:{"Content-Type":"application/json"}, body: JSON.stringify(body)})).json(); }
  catch (e) { return {error: "serveur injoignable (relancez Lancer_editeur.bat)"}; }
}
function handleGfx(res, okmsg) {
  if (res.error) { alert("ERREUR : " + res.error); return; }
  let m = okmsg;
  if (res.warnings && res.warnings.length) { m += " | Avertissements : " + res.warnings.join("; "); alert(m); }
  setStatus(m);
  renderGraphics();
}
function uploadSprite(input, which) {
  const f = input.files[0]; if (!f) return;
  fileToBase64(f, async b64 => handleGfx(await gfxPost({action:"upload_sprite", species:currentGroup, which:which, png_b64:b64}), "Sprite " + which + " remplace."));
}
function importPalette(input) {
  const f = input.files[0]; if (!f) return;
  const which = document.getElementById("importwhich").value;
  const r = new FileReader();
  r.onload = async () => handleGfx(await gfxPost({action:"import_palette", species:currentGroup, which:which, pal_text:r.result}), "Palette importee (" + which + ").");
  r.readAsText(f);
}
function previewReuse() {
  const src = document.getElementById("reusepal").value;
  document.getElementById("reusepreview").innerHTML =
    `<div class="lbl">Apercu de ${currentGroup} avec la palette de ${src} :</div>`
    + `<img src="/api/sprite?species=${currentGroup}&which=front&recolor=${src}&t=${Date.now()}" class="spr">`;
}
async function applyReuse() {
  const src = document.getElementById("reusepal").value;
  if (!confirm("Remplacer la palette (normale + shiny) de " + currentGroup + " par celle de " + src + " ?")) return;
  handleGfx(await gfxPost({action:"reuse_palette", species:currentGroup, source:src}), "Palette de " + src + " appliquee.");
}
async function applyName() {
  const v = document.getElementById("spname").value;
  handleGfx(await gfxPost({action:"set_name", species:currentGroup, value:v}), "Nom enregistre.");
}
async function applyStats() {
  const fields = {};
  document.querySelectorAll('#tablewrap [id^="st_"]').forEach(el => { fields[el.id.slice(3)] = el.value; });
  handleGfx(await gfxPost({action:"set_stats", species:currentGroup, fields:fields}), "Stats enregistrees.");
}
async function applyEvos() {
  const v = document.getElementById("spevos").value;
  handleGfx(await gfxPost({action:"set_evos", species:currentGroup, value:v}), "Evolutions enregistrees.");
}
async function applyLearnset() {
  const v = document.getElementById("splearn").value;
  handleGfx(await gfxPost({action:"set_learnset", species:currentGroup, value:v}), "Attaques enregistrees.");
}
async function applyPokedex() {
  const v = document.getElementById("spdex").value;
  handleGfx(await gfxPost({action:"set_pokedex", species:currentGroup, value:v}), "Description enregistree.");
}
async function applyCry() {
  const src = document.getElementById("crysrc").value;
  if (!confirm("Remplacer le cri de " + currentGroup + " par celui de " + src + " ?")) return;
  handleGfx(await gfxPost({action:"set_cry", species:currentGroup, source:src}), "Cri de " + src + " applique.");
}
function previewIcon() {
  const src = document.getElementById("iconsrc").value;
  document.getElementById("iconpreview").innerHTML =
    `<img src="/api/sprite?species=${src}&which=icon&t=${Date.now()}" class="spr">`;
}
async function applyIcon() {
  const src = document.getElementById("iconsrc").value;
  if (!confirm("Remplacer la mini-icone de " + currentGroup + " par celle de " + src + " ?")) return;
  handleGfx(await gfxPost({action:"set_icon", species:currentGroup, source:src}), "Icone de " + src + " appliquee.");
}

document.getElementById("cat").addEventListener("change", async e => { current = e.target.value; await selectCategory(); });
document.getElementById("group").addEventListener("change", async e => { currentGroup = e.target.value; if (getCat(current).kind !== "graphics") await ensureGroup(); render(); });
document.getElementById("search").addEventListener("input", render);
document.getElementById("save").addEventListener("click", save);
document.getElementById("reload").addEventListener("click", () => {
  if (changes.size && !confirm("Des modifications non enregistrees seront perdues. Continuer ?")) return;
  changes.clear(); for (const key in groupCache) delete groupCache[key]; refreshSave(); load();
});
load();
</script>
</body>
</html>"""


def main():
    url = "http://localhost:%d" % PORT
    print("=" * 60)
    print(" Editeur de textes pokefirered")
    print(" Racine du projet : %s" % ROOT)
    print(" Ouvrez : %s" % url)
    print(" (Ctrl+C pour arreter)")
    print("=" * 60)
    # Multi-thread : une requete lente (ou un onglet navigateur qui garde la
    # connexion ouverte) ne bloque plus les autres requetes.
    class ThreadingServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
        daemon_threads = True
        allow_reuse_address = True

    try:
        httpd = ThreadingServer(("127.0.0.1", PORT), Handler)
    except OSError as e:
        print("Impossible de demarrer le serveur sur le port %d : %s" % (PORT, e))
        print("Le port est peut-etre deja utilise.")
        sys.exit(1)
    threading.Timer(0.8, lambda: webbrowser.open(url)).start()
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nArret.")
        httpd.shutdown()


if __name__ == "__main__":
    main()
