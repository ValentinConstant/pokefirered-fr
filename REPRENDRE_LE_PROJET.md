# 📌 Reprise du projet — pokefirered

> Document pour reprendre le travail après une pause. Dernière mise à jour : 2026-06-30 (données Pokémon + graphismes terminés).

---

## 🎯 Où on en est (résumé)

| Sujet | État |
|-------|------|
| Éditeur maison (texte + données + graphismes) | ✅ **fait et fonctionnel** (16 catégories) |
| Chaîne de compilation (WSL2 + agbcc) | ✅ **installée et fonctionnelle** |
| Première compilation du jeu | ✅ **réussie** → `pokefirered.gba` (16 Mo) existe |
| Test d'édition (BULBASAUR → BULBIZARRE) | ✅ enregistré dans `src/data/text/species_names.h` |
| Phase 1 — **dialogues des cartes** | ✅ fait (groupé par carte, chargement à la demande) |
| Phase 2 — **textes globaux + scripts + code** (menus/combat/Pokédex) | ✅ fait (groupé par fichier) |
| **Bloc Données Pokémon** : stats, évolutions, attaques/niveau | ✅ fait (par espèce) |
| **Graphismes** : upload sprites front/back + palettes (réutiliser/importer .pal + prévisu recolorée) | ✅ fait |
| **Bouton « Compiler »** dans l'éditeur | ⏳ pas encore fait (à ajouter) |

⚠️ **À noter** : le `.gba` actuel date d'**avant** les modifs récentes (BULBIZARRE,
`EVOS_PER_MON=6`, etc.). Pour les voir en jeu, il faut **recompiler** (voir plus bas).

---

## 🛠️ L'éditeur de textes

Dossier : `tools/text_editor/`
- `edit_text.py` — l'appli (serveur web Python pur, port 8765, zéro dépendance)
- `Lancer_editeur.bat` — double-clic pour démarrer
- `README.md` — mode d'emploi
- `backups/` — sauvegardes auto avant chaque écriture (non versionné)

**Lancer** : double-cliquer `Lancer_editeur.bat` → ouvre `http://localhost:8765`.

**Catégories éditables (13)** :
- **Noms** : talents, natures, classes de dresseurs, lieux.
  (Le **nom d'un dresseur** s'édite désormais dans la page Équipes de dresseurs.)
  (Les **noms de Pokémon** sont désormais sur la page **Pokémon**, et les **noms d'attaques**
  sont fusionnés avec leurs descriptions, voir ci-dessous.)
- **Attaques (nom + desc.)** : une seule page à deux colonnes (`move_names.h` + `move_descriptions.c`,
  appariés par identité CamelCase).
- **Descriptions** : talents, objets.
- **Objets** : nom + description (items.json).
- **Dialogues des cartes** : groupés par carte (2ᵉ menu déroulant), chargement à la demande.
- **Textes globaux** (`data/text/*.inc`), **Textes de scripts** (`data/scripts/*.inc`),
  **Textes du code** (`src/**` : menus, combat, Pokédex…), groupés par fichier.
- **Données Pokémon** : **stats, évolutions, attaques par niveau** sont désormais **toutes sur la
  page Pokémon** (cartes dédiées) — plus de catégories séparées. (`EVOS_PER_MON` = **6**,
  `include/constants/pokemon.h` → jusqu'à 6 évos/espèce, concept « Pokémon Evolved ».)
- **Équipes de dresseurs** — **éditeur structuré en accordéon** (endpoint `/api/trainers`, chargé
  en une passe ; listes d'options via `/api/lists`). Clic sur un dresseur → déplie le **nom du
  dresseur** (`.trainerName` dans `trainers.h`, action via champ `name`) + jusqu'à 6 lignes,
  chaque ligne = `niveau` (n°), `espèce`, `IV` (n°), `objet`, `4 attaques`, en **champs à
  autocomplétion** (`<datalist>` partagés : 1 seule liste espèces/objets/attaques pour toute la page,
  sinon des millions d'`<option>`). Boutons ＋/✕ (max 6). Construit **à la volée au clic** (lazy) pour
  rester fluide avec 654 dresseurs. Le nom apparaît aussi dans l'en-tête de chaque dresseur.
  - Sérialisation vers le format `niveau, SPECIES_X[, iv=N][, item=X][, moves=A/B/C/D]` → réutilise
    le **même chemin de sauvegarde** (régénère `sParty_*` dans `trainer_parties.h` + recalcule type de
    struct & macro `.party=` dans `trainers.h`). Mapping `TRAINER_X → sParty_*` lu dans `trainers.h`.
  - **88 dresseurs RS placeholders** (`AQUA_LEADER → ELITE_FOUR_DRAKE`, inutilisés en FireRed) en bas,
    section repliable. Détection : `sParty_*` déclarés **avant** `// Start of actual trainer data`.
  - Édition par lot (bouton « Enregistrer » global), recherche par nom/contenu, résumé par dresseur.
- **Magasins (Poké Marts + Centre commercial)** — accordéon (endpoint `/api/shops`). 23 magasins
  découverts en scannant `data/maps/*/scripts.inc` (références `pokemart <Label>` → bloc `<Label>::`
  de `.2byte ITEM_X` terminé par `ITEM_NONE`). Clic → liste des objets en vente : **＋ ajouter /
  ✕ retirer** (objet en champ à autocomplétion, datalist partagée) + **prix** (champ `price` de
  `items.json`, **global à tout le jeu**). Sauvegarde : `save_shop` régénère le bloc `.2byte`,
  `save_item_price` édite `items.json` de façon ciblée (encodage préservé). Catégorie `kind="shops"`,
  vue custom `renderShops` (comme dresseurs).

Les éditeurs de **blocs** (dialogues, évolutions, learnsets) régénèrent le bloc édité :
ajouter/supprimer une ligne dans la zone de texte = ajouter/retirer une entrée.
- **Pokémon (sprites, nom, description, stats, cri, icône)** (par espèce) — page unifiée :
  - **Nom** (`species_names.h`) + **description Pokédex** (`pokedex_text_fr.h`, bloc multi-lignes
    régénéré ; mapping espèce→symbole via `pokedex_entries.h`). Actions `set_name` / `set_pokedex`.
  - **Stats** (`species_info.h`) — grille compacte des champs `.baseHP`/types/talents/etc. ;
    réécriture ciblée des lignes `.champ = valeur` du bloc `[SPECIES_X]`. Action `set_stats`.
  - **Évolutions** (`evolution.h`, action `set_evos`) — 1 évo/ligne `METHODE, PARAM, ESPECE` ;
    zone vide = aucune évo ; max 6.
  - **Attaques par niveau** (`level_up_learnsets.h`, action `set_learnset`) — 1 attaque/ligne
    `niveau, MOVE_X` ; mapping espèce→symbole via `level_up_learnset_pointers.h`.
  - aperçu front/back, **upload** PNG indexé front/back (sauvegarde immédiate + backup),
  aperçu des palettes normale/shiny, **réutiliser**
  la palette d'une autre espèce (avec **prévisu recolorée**) ou **importer** un `.pal`.
  Mapping espèce↔dossier via `front_pic_table.h` + `pokemon.h`. Endpoints `/api/gfx`,
  `/api/sprite` (recolor par swap du chunk PLTE), `/api/gfx_action`.
  - **Cri** : remplacer le cri d'une espèce par celui d'une autre (remappe `gCryTable` +
    `gCryTable_Reverse` dans `sound/cry_tables.inc`, via le cry id calculé par reproduction
    de `SpeciesToCryId` + table Hoenn `cry_ids.h`/`hoenn_cries.h`). Action `set_cry`.
  - **Mini-icône** (équipe/boîte PC) : remplacer l'icône par celle d'une autre espèce (copie
    `gMonIconTable` **et** `gMonIconPaletteIndices` dans `src/pokemon_icon.c`, prévisu via
    `which=icon`). Action `set_icon`.
  Note : les `.pal` de l'arbre de travail sont en CRLF (artefact de checkout, sans incidence build).

Le code C exclut les tables déjà couvertes et `items.h` (généré). Les libellés/variables
servent de repère ; la **recherche** filtre dans la carte/fichier sélectionné.

**Important** :
- Édition **ciblée ligne par ligne** → diffs Git minimaux.
- ⚠️ Si le navigateur affiche une page mais que « Enregistrer » échoue → vérifier
  que le **serveur Python est bien lancé** (le `.bat` doit tourner). Une alerte
  rouge s'affiche désormais en cas de serveur injoignable.
- Serveur **multi-thread** (`ThreadingTCPServer`) : un onglet/une requête lente ne
  bloque plus les autres (corrige les anciens « gels » du serveur mono-thread).

---

## ⚙️ Compiler le jeu (recompiler après une modif)

Dans le terminal **Ubuntu** (WSL2) :
```bash
cd /mnt/c/Users/const/documents/github/pokefirered
make -j$(nproc)
```
→ régénère `pokefirered.gba` à la racine du dépôt.
La compilation incrémentale est **rapide** (seuls les fichiers modifiés sont recompilés).

Lancer la ROM : ouvrir `C:\Users\const\documents\github\pokefirered\pokefirered.gba`
dans un émulateur GBA (**mGBA**).

> Guide d'installation complet de la chaîne d'outils : voir **`GUIDE_COMPILATION_FR.md`**.

---

## 🚀 Prochaines étapes (TODO)

1. **Recompiler** pour voir BULBIZARRE en jeu (`make -j$(nproc)`).
2. **Bouton « Compiler »** dans l'interface web : lancera la build via WSL
   (`wsl make -j`) et affichera la progression/erreurs dans la page.
   (Maintenant que la compilation marche, c'est faisable.)
3. (Optionnel) **Classement des dialogues par PNJ** (plutôt que par carte) : nécessite
   de croiser `map.json` (object events) + `scripts.inc` (`msgbox`). Approximatif, lourd.
4. (Optionnel) **Accélérer la build** : compiler depuis le système de fichiers Linux
   (`~/pokefirered`) plutôt que `/mnt/c` → 5 à 10× plus rapide. Compromis : l'éditeur
   Windows ne verrait plus directement ces fichiers (adaptable).

---

## ⚠️ Particularités de l'environnement (à savoir)

- **`git` n'est PAS dans le PATH Windows** (et pas de `git.exe` installé), mais le
  dépôt `.git` existe (objets *packés*). Côté **WSL/Ubuntu**, `git` fonctionne.
- **`src/data/items.json`** a un encodage d'accents **incohérent** (noms `é`,
  descriptions `é` littéral). → **ne jamais** le reformater via `json.dumps` complet,
  toujours édition ciblée (déjà géré par l'outil).
- Fichiers source en **LF**, **SAUF les `.pal`** de l'arbre de travail qui sont en **CRLF**
  (artefact de checkout ; Git=LF) — sans incidence sur le build.
- WSL : `/mnt/c` configuré avec l'option `metadata` (via `/etc/wsl.conf`) pour les
  permissions de fichiers.

---

## 🧠 Comment redémarrer une session avec Claude

Phrase d'amorçage suggérée (à coller après un `/clear`) :
> « On reprend le projet pokefirered. Lis `REPRENDRE_LE_PROJET.md` et `tools/text_editor/edit_text.py`. L'éditeur (17 catégories : textes, données Pokémon, graphismes) est terminé. Aujourd'hui je veux : [recompiler / ajouter le bouton Compiler / autre]. »

ℹ️ **État technique** : l'outil `tools/text_editor/edit_text.py` est un serveur web Python pur
(stdlib) ; relancer via `Lancer_editeur.bat`. Pour comparer/restaurer un fichier vs Git sans
`git.exe`, lire le packfile en Python (cf. méthode déjà utilisée). Édition **ciblée** partout
(diffs minimaux) ; chaque sauvegarde fait un backup dans `tools/text_editor/backups/`.
