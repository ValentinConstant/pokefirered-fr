# Éditeur de textes — pokefirered / pokeleafgreen

Petite application **web locale en Python pur** (aucune dépendance à installer)
pour éditer facilement les textes du jeu :

- **Noms** : Pokémon, attaques, talents, natures, classes de dresseurs, lieux, dresseurs
- **Descriptions** : talents, attaques, objets
- **Objets** : nom + description (depuis `src/data/items.json`)
- **Dialogues des cartes** : groupés par carte (2ᵉ menu déroulant), chargés à la demande
- **Textes globaux / scripts / code** : `data/text/`, `data/scripts/`, `src/**`
  (menus, combat, Pokédex…), groupés par fichier

Pour les catégories groupées, choisissez d'abord la **carte / le fichier** dans le
2ᵉ menu déroulant, puis cherchez/éditez. Le code C exclut les tables déjà couvertes
et `items.h` (généré au build).

## Lancement

Trois façons de lancer l'éditeur (Windows) :

- Double-cliquez **`Editeur_textes.exe`** (aucun Python requis, mais doit être
  regénéré après une modif de `edit_text.py` — voir plus bas).
- Double-cliquez **`Lancer_editeur.bat`** (nécessite Python).
- En ligne de commande : `python edit_text.py`

Le navigateur s'ouvre automatiquement sur <http://localhost:8765>.
Fermez avec `Ctrl+C` dans la console (ou fermez la fenêtre pour l'exe).

### Regénérer l'exe

`Editeur_textes.exe` n'est pas versionné (binaire, dépend de la machine).
Après toute modif de `edit_text.py`, double-cliquez **`build_exe.bat`** pour le
recompiler (installe PyInstaller si besoin, puis régénère l'exe dans ce dossier).

## Utilisation

1. Choisissez une **catégorie** dans le menu déroulant.
2. **Recherchez** par nom de constante ou par texte.
3. Modifiez les champs (les cellules modifiées passent en orange).
4. Cliquez **Enregistrer** : les fichiers source sont réécrits proprement.

### ⚠️ Codes de contrôle à préserver

Dans les textes, certains symboles sont **spéciaux** et doivent rester intacts :

| Code | Effet |
|------|-------|
| `\n` | retour à la ligne |
| `\p` | nouvelle page (boîte de dialogue suivante) |
| `\l` | défilement de ligne |
| `{PLAYER}`, `{RIVAL}`, `{PKMN}` | variables (nom du joueur, etc.) |
| `{STR_VAR_1}` … | valeurs insérées par le jeu |

N'utilisez pas de guillemet droit `"` brut dans le texte (cela casse la
syntaxe C/JSON).

Les compteurs de longueur (ex. `9/10`) signalent en rouge un nom **trop long**
pour le jeu — à éviter pour ne pas tronquer/déborder l'affichage.

## Sécurité / sauvegardes

Avant **chaque** enregistrement, une copie des fichiers modifiés est placée dans
`tools/text_editor/backups/<horodatage>/`. Pour revenir en arrière, recopiez le
fichier sauvegardé par-dessus l'original. (Avec Git, `git diff` montre aussi
toutes les modifications.)

## Transportable

Aucune installation : copiez le dépôt sur un autre PC disposant de **Python 3**
et relancez. Le script retrouve automatiquement la racine du projet.

## Ajouter d'autres catégories plus tard

Les dialogues des cartes (`data/maps/*/text.inc`) et textes globaux
(`data/text/*.inc`) ne sont pas encore inclus. Ils pourront être ajoutés dans
`CATEGORIES` (en haut de `edit_text.py`) une fois cette première version validée.
```
