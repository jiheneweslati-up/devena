# Devena — Module Évaluation (Qt / C++ / Oracle)

Module réalisé d'après l'**Atelier 3 : CRUD (architecture Modèle-Vue)**.

## Architecture Modèle-Vue (3 sources + 3 en-têtes)
| Fichier | Contenu | Rôle |
|---|---|---|
| `connexion.h / connexion.cpp` | classe `Connection` (Singleton) + base de démonstration | Une seule connexion à la BD (QODBC → Oracle), secours SQLite si Oracle est indisponible |
| `evaluation.h / evaluation.cpp` | classe `Evaluation` + classe `EvaluationModel` | **Modèle** : toutes les requêtes SQL (préparées, `bindValue`) et la mise en forme du tableau |
| `header.h` | thème, widget `ChartView`, déclaration de `MainWindow` | **Vue** (déclarations), code découpé en zones `// === MODULE XXX ===` |
| `main.cpp` | `main()` + implémentation de `MainWindow` | Lancement de l'appli et **Vue** (aucune requête SQL) |
| `mainwindow.ui` | **un seul .ui** pour tous les modules | stackedWidget principal : une page par module |

## Entité gérée : EVALUATION (base commune `database/devena.sql`, schéma DEVENA)
`EVALUATION (ID_EVALUATION, NOTE, REMARQUE, DATE_EVALUATION, NOM, TYPE)`
`EQUIPES (..., #ID_EVALUATION, #ID_PROJET)` · `GERER (#ID_EMPLOYE, #ID_EVALUATION, ID_GERER)` ·
`EMPLOYES (ID_EMPLOYE, NOM, PRENOM, MOT_DE_PASSE, POSTE)` · `PROJETS` · `PARTICIPANTS (..., #ID_EQUIPE)`

| CRUD | Méthode (classe Evaluation) | Requête |
|---|---|---|
| Create | `bool ajouter()` | `INSERT INTO EVALUATION … VALUES (:id, :note, :remarque, :date, :nom, :type)` (id EV05, EV06… généré) |
| Read | `QSqlQueryModel* afficher(recherche, tri, période)` | `SELECT … , (SELECT COUNT(*) FROM EQUIPES …) FROM EVALUATION` |
| Update | `bool modifier()` | `UPDATE EVALUATION SET … WHERE ID_EVALUATION = :id` |
| Delete | `bool supprimer(QString id)` | `DELETE FROM EVALUATION WHERE ID_EVALUATION = :id` (refusé si des équipes ou des employés via GERER y sont liés) |

Après chaque ajout / modification / suppression réussi, la vue appelle `actualiserEvaluations()`
qui ré-exécute `afficher()` (actualisation de l'affichage, cf. atelier).

## Interface (sans fenêtres de dialogue, avec un QStackedWidget)
1. **Liste** : tableau (QTableView), recherche, tri, filtre par période, Modifier / Supprimer / Fiche PDF
2. **Ajouter / Modifier** : formulaire intégré (nom, type, note /20, date, remarque)
3. **Classement & alertes** : podium des équipes (note de leur évaluation) + évaluations avec note < 10/20

## Contrôles de saisie (C++) et contraintes (BD)
- Nom et type obligatoires (20 caractères max) · note entre 0 et 20 · date valide et pas dans le futur
- Remarque de 3 à 200 caractères avec au moins une lettre
- Règles métier : pas deux évaluations avec même nom + type + date ; suppression interdite si l'évaluation est utilisée
- Côté Oracle : clés primaires / étrangères, `CHECK (NOTE BETWEEN 0 AND 20)`, `NOT NULL`…

## Fonctionnalités avancées
Recherche multicritère + tri + période · Export PDF (liste, fiche, alertes) ·
Statistiques (cartes, moyenne par équipe, répartition) · Classement des équipes · Alertes

## Mise en place
1. **SQL Developer** : connecté en `Devena` (mot de passe `esprit2006`), exécuter tout
   `database/devena.sql` (F5 / « Exécuter le script »).
2. **ODBC 64 bits** : source utilisateur `Devena` (pilote Oracle, TNS `localhost:1521/XE`,
   User ID `Devena`) → *Test Connection*.
3. Qt Creator → `Devena.pro` → kit MinGW 64-bit → **Ctrl + R** → « Connexion réussie ».
   Connexion dans `connexion.cpp` : source `Devena`, utilisateur `Devena`, mot de passe `esprit2006`.

Si la connexion échoue, l'appli démarre sur la **base de démonstration** (indicateur jaune
en haut) : tout fonctionne mais rien n'est enregistré dans Oracle.

## Thème et dimensions (communs à l'équipe)
- Fenêtre 1400 × 900, barre latérale de 220 px, menu : Tableau de bord, Employés, Participants,
  Équipe, Projet, Évaluation, Paramètre.
- Palette : fond `#050B14`, barre latérale `#030914`, cartes `#091522`, accent `#18D9EF`
  (définie une seule fois dans `header.h`, `Theme::feuilleDeStyle()`).

## Intégration (guide d'intégration collaborative)
- Un seul `mainwindow.ui` : chaque module a sa page dans `stackedPages`.
- Navigation (barre latérale) dans la zone « SLOTS GÉNÉRAUX » de `main.cpp`.
- Chaque étudiant ajoute **sa classe métier** (`equipe.h/.cpp`, `projet.h/.cpp`…) et écrit
  **uniquement dans sa zone** `// === MODULE XXX ===` de `header.h` et `main.cpp`.
- Même configuration de connexion pour toute l'équipe (`connexion.cpp`) : même source ODBC `Devena`,
  même utilisateur `Devena`, même mot de passe `esprit2006`.

## GitHub (jamais d'« upload »)
```
git pull origin main          # toujours avant de commencer
git add -A
git commit -m "Module Evaluation : CRUD + métiers"
git pull origin main          # récupérer le travail des autres avant de pousser
git push origin main
```
