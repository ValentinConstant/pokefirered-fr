# Guide complet — Compiler pokefirered (Windows 11 + WSL2)

Du téléchargement du dépôt jusqu'au fichier **pokefirered.gba** jouable.

> Légende des terminaux :
> - 🟦 **PowerShell (Admin)** : clic droit menu Démarrer → *Terminal (Admin)*
> - 🟦 **PowerShell** : menu Démarrer → *PowerShell* (sans admin)
> - 🐧 **Ubuntu** : menu Démarrer → *Ubuntu*

---

## Étape 0 — Récupérer le dépôt pokefirered

Si ce n'est pas déjà fait, dans 🟦 **PowerShell** :
```powershell
cd C:\Users\const\documents\github
git clone https://github.com/pret/pokefirered
```
(Ici, le dépôt est déjà présent dans `C:\Users\const\documents\github\pokefirered`.)

---

## Étape 1 — Installer WSL2 + Ubuntu

Dans 🟦 **PowerShell (Admin)** :
```powershell
wsl --install -d Ubuntu
```
- **Redémarrer** le PC si demandé.
- Au premier lancement d'Ubuntu : créer un **nom d'utilisateur** et un **mot de passe** Linux
  (le mot de passe est invisible pendant la frappe, c'est normal).

> Ce PC ne supporte pas WSL1 → WSL2 est utilisé automatiquement. WSL2 compile très bien.

---

## Étape 2 — Autoriser les permissions de fichiers sur le disque Windows

Sans ça, `git clone` échoue avec `could not set 'core.filemode'`.

Dans 🐧 **Ubuntu** :
```bash
sudo tee /etc/wsl.conf > /dev/null <<'EOF'
[automount]
options = "metadata,noatime"
EOF
```

Puis redémarrer WSL depuis 🟦 **PowerShell** :
```powershell
wsl --shutdown
```
⚠️ `wsl --shutdown` est une commande **Windows** : elle ne marche PAS dans Ubuntu.

Rouvrir 🐧 **Ubuntu** et vérifier :
```bash
mount | grep /mnt/c
```
→ les options doivent contenir **`metadata`** (et `uid=1000;gid=1000`).

---

## Étape 3 — Installer les paquets de compilation

Dans 🐧 **Ubuntu** :
```bash
sudo apt update
sudo apt install build-essential binutils-arm-none-eabi libpng-dev git -y
```
Vérifier :
```bash
make --version
```

---

## Étape 4 — Installer agbcc (le compilateur ARM du jeu)

Dans 🐧 **Ubuntu** :
```bash
cd /mnt/c/Users/const/documents/github
git clone https://github.com/pret/agbcc
cd agbcc
./build.sh
./install.sh ../pokefirered
```
> `./build.sh` est **long** et affiche beaucoup de texte : c'est normal.

---

## Étape 5 — Compiler le jeu

Dans 🐧 **Ubuntu** :
```bash
cd /mnt/c/Users/const/documents/github/pokefirered
make -j$(nproc)
```
- `-j$(nproc)` = compilation en parallèle (plus rapide).
- La **première** compilation construit aussi les outils de `tools/` → c'est long.
  Les suivantes sont rapides (incrémentales).

En cas de succès, le fichier **`pokefirered.gba`** apparaît à la racine du dépôt.

(Facultatif) Vérifier que la ROM est identique à l'originale :
```bash
make compare
```
→ `pokefirered.gba: OK` si conforme.

---

## Étape 6 — Lancer le jeu 🎮

Le fichier est visible depuis l'Explorateur Windows :
`C:\Users\const\documents\github\pokefirered\pokefirered.gba`

Ouvrez-le avec un émulateur GBA (ex. **mGBA**).

---

## Les fois suivantes (résumé express)

1. Ouvrir 🐧 **Ubuntu**
2. ```bash
   cd /mnt/c/Users/const/documents/github/pokefirered
   make -j$(nproc)
   ```
3. Lancer le `.gba` mis à jour dans l'émulateur.

---

## Problèmes rencontrés & solutions

| Symptôme | Cause | Solution |
|----------|-------|----------|
| `could not set 'core.filemode'` au `git clone` | `/mnt/c` sans `metadata` | Étape 2 (wsl.conf + `wsl --shutdown`) |
| `wsl: command not found` dans Ubuntu | commande Windows lancée dans Linux | la lancer dans PowerShell |
| `make: not found` au `./build.sh` | paquets non installés | Étape 3 (`build-essential`) |
