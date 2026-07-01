# 🎮 Éditeur de ROM décompilée — pokefirered

**Un outil tout-en-un, transportable et sans installation, pour modifier une ROM
Pokémon décompilée (base [pret/pokefirered](https://github.com/pret/pokefirered)).**

Éditez facilement, depuis une interface web locale, **tout ce qui est modifiable
dans le jeu** — textes, dialogues, données des Pokémon, équipes de dresseurs,
magasins, graphismes — sans jamais toucher au code source à la main.

> L'outil vit dans [`tools/text_editor/`](tools/text_editor/). C'est une application
> **web 100 % Python (bibliothèque standard)** : aucune dépendance à installer,
> aucun `pip install`. Copiez le dépôt sur n'importe quel PC avec **Python 3** et
> lancez.

---

## ✨ Ce que l'on peut éditer

| Domaine | Contenu |
|---------|---------|
| **Textes / noms** | Pokémon, attaques, talents, natures, classes de dresseurs, lieux |
| **Descriptions** | attaques (nom + desc. sur la même page), talents, objets, Pokédex |
| **Objets** | nom + description + **prix** (`items.json`) |
| **Dialogues** | dialogues des cartes (groupés par carte), textes globaux, scripts |
| **Textes du code** | menus, combat, Pokédex… (`src/**`, groupés par fichier) |
| **Données Pokémon** | stats de base, **évolutions** (jusqu'à **6** par espèce), attaques par niveau, cri, mini-icône |
| **Dresseurs** | éditeur d'équipes (niveau, espèce, IV, objet, 4 attaques) avec autocomplétion |
| **Magasins** | objets en vente des Poké Marts / Centre commercial |
| **Graphismes** | upload sprites **front/back** (PNG indexé), aperçu, palettes (réutiliser / importer un `.pal`, **prévisu recolorée**) |

Toutes les modifications sont **ciblées ligne par ligne** → diffs Git minimaux, et une
**sauvegarde automatique** est faite avant chaque écriture.

---

## 🚀 Démarrage

```sh
cd tools/text_editor
python edit_text.py
```

…ou double-cliquez **`Lancer_editeur.bat`** (Windows).

Le navigateur s'ouvre sur <http://localhost:8765>. `Ctrl+C` dans la console pour quitter.

1. Choisissez une **catégorie** (et, pour les dialogues/données, une carte / espèce).
2. **Recherchez** par nom de constante ou par texte.
3. Modifiez, puis cliquez **Enregistrer** — les fichiers source sont réécrits proprement.

> ⚠️ Préservez les **codes de contrôle** dans les textes : `\n` (retour ligne),
> `\p` (page suivante), `\l` (défilement), `{PLAYER}` / `{RIVAL}` / `{PKMN}`,
> `{STR_VAR_1}`… Voir [`tools/text_editor/README.md`](tools/text_editor/README.md)
> pour le détail.

---

## 🔧 Compiler la ROM

Cet éditeur modifie le **code source décompilé**. Pour générer le `.gba`, il faut
**recompiler** le projet (chaîne d'outils pret classique : WSL2 + agbcc/binutils).

👉 Guide d'installation et de compilation pas à pas (en français) :
**[`GUIDE_COMPILATION_FR.md`](GUIDE_COMPILATION_FR.md)**.

En résumé, dans un terminal Linux/WSL, à la racine du dépôt :

```sh
make -j$(nproc)
```

→ produit `pokefirered.gba`, à ouvrir dans un émulateur GBA (ex. **mGBA**).

---

## 🙏 Crédits

- Base de décompilation : **[pret/pokefirered](https://github.com/pret/pokefirered)**
  (et l'écosystème pret). L'installation d'origine est décrite dans
  [`INSTALL.md`](INSTALL.md). L'outil d'édition ajouté ici (`tools/text_editor/`)
  est indépendant et n'affecte pas la compilation.
- Voir les licences originales du projet pokefirered pour le code décompilé.
