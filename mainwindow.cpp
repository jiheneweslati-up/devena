#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>

#include <QApplication>
#include <QGuiApplication>
#include <QHash>
#include <QScreen>
#include <QLocale>
#include <QMap>
#include <QProgressBar>
#include <QSet>
#include <QSignalBlocker>
#include <QButtonGroup>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QEvent>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPainterPath>
#include <QPdfWriter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QShortcut>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextDocument>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QVector>
#include <QBrush>
#include <QDateEdit>
#include <QDate>
#include <QDebug>
#include <QScrollBar>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDateTime>
#include <QEasingCurve>
#include <QDesktopServices>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QStackedWidget>
#include <QStyle>
#include <QFontMetricsF>
#include <QFileInfo>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSettings>
#include <QUrl>
#include <QGraphicsDropShadowEffect>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPointer>
#include <QPropertyAnimation>
#include <QVariantAnimation>
// Index des colonnes du tableau
enum Colonne {
    ColId, ColTitre, ColDescription, ColThematique, ColStatut,
    ColCreation, ColLimite, ColActions, NbColonnes
};

static QString ouTiret(const QString &s)
{
    return s.trimmed().isEmpty() ? QStringLiteral("-") : s;
}

static QString dateOuTiret(const QDate &d)
{
    return d.isValid() ? d.toString("dd/MM/yyyy") : QStringLiteral("-");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1400 x 900 comme le module Equipes, mais jamais plus grand que l'ecran
    // (sinon la colonne Actions sort de l'ecran sur un portable ou avec le zoom Windows)
    if (QScreen *ecran = QGuiApplication::primaryScreen()) {
        const QRect zone = ecran->availableGeometry();
        resize(qMin(1400, zone.width() - 20), qMin(900, zone.height() - 40));
    }

    // Marges un peu plus serrees et sous titre sur deux lignes si besoin :
    // toute la page tient dans la fenetre, meme etroite (pas de defilement horizontal)
    ui->layoutContenu->setContentsMargins(22, 18, 18, 18);
    ui->layoutEntete->setSpacing(6);
    ui->labelSousTitrePage->setWordWrap(true);

    construireMenu();
    configurerTableau();
    construireKanban();   // vue Kanban + boutons Tableau / Kanban

    connect(ui->boutonAjouter,     &QPushButton::clicked, this, &MainWindow::ajouterProjet);
    connect(ui->boutonExporterPdf, &QPushButton::clicked, this, &MainWindow::exporterPdf);
    connect(ui->boutonGagnant,     &QPushButton::clicked, this, &MainWindow::afficherProjetGagnant);
    connect(ui->boutonTheme,       &QPushButton::clicked, this, &MainWindow::basculerTheme);
    connect(ui->boutonAnnuler,     &QPushButton::clicked, this, &MainWindow::annulerDerniereAction);
    connect(ui->lineEditRecherche, &QLineEdit::textChanged, this, &MainWindow::rafraichir);

    auto *raccourci = new QShortcut(QKeySequence::Undo, this);
    connect(raccourci, &QShortcut::activated, this, &MainWindow::annulerDerniereAction);
    ui->boutonAnnuler->setEnabled(false);

    creerCentreNotifications();
    chargerNotifications();

    appliquerTheme();
    rafraichir();
    animerPage(ui->pageProjets);
    animerBoutonAjouter();
    // Les lignes arrivent une fois la fenetre affichee (leur position est alors connue)
    QTimer::singleShot(80, this, &MainWindow::animerEntreeLignes);

    // Alertes d'echeances : au demarrage (apres les animations), puis toutes les 30 minutes
    QTimer::singleShot(1400, this, &MainWindow::verifierEcheances);
    auto *minuteur = new QTimer(this);
    connect(minuteur, &QTimer::timeout, this, &MainWindow::verifierEcheances);
    minuteur->start(30 * 60 * 1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Menu de gauche : seule la page Projets est developpee ici,
// les autres modules ont une page d'attente a remplacer par ceux de l'equipe.
void MainWindow::construireMenu()
{
    auto *groupe = new QButtonGroup(this);
    groupe->setExclusive(true);

    const QList<QPushButton *> boutons = {
        ui->boutonTableauBord, ui->boutonEmployee, ui->boutonParticipants,
        ui->boutonEquipes, ui->boutonProjets, ui->boutonEvaluations, ui->boutonParametres};

    for (QPushButton *b : boutons) {
        groupe->addButton(b);
        b->setCursor(Qt::PointingHandCursor);

        QWidget *page = ui->pageProjets;
        if (b != ui->boutonProjets) {
            page = new QWidget;
            auto *l = new QVBoxLayout(page);
            auto *texte = new QLabel(tr("Module « %1 » à intégrer.")
                                         .arg(b->text().section(' ', -1)), page);
            texte->setProperty("role", "sousTitre");
            texte->setAlignment(Qt::AlignCenter);
            l->addWidget(texte);
            ui->stackedWidget->addWidget(page);
        }
        connect(b, &QPushButton::clicked, this, [this, page] {
            ui->stackedWidget->setCurrentWidget(page);
            animerPage(page);
            if (page == ui->pageProjets)
                animerEntreeLignes();
        });
    }
    ui->stackedWidget->setCurrentWidget(ui->pageProjets);
}

void MainWindow::configurerTableau()
{
    QTableWidget *t = ui->tableProjets;
    t->setColumnCount(NbColonnes);
    mettreAJourEntetes();   // libelles + fleche de tri
    t->verticalHeader()->setVisible(false);
    t->verticalHeader()->setDefaultSectionSize(48);
    t->setShowGrid(false);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setFocusPolicy(Qt::NoFocus);
    t->setWordWrap(false);
    t->setTextElideMode(Qt::ElideRight);

    QHeaderView *h = t->horizontalHeader();
    h->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    h->setHighlightSections(false);
    h->setSectionResizeMode(QHeaderView::Fixed);
    h->setMinimumSectionSize(40);
    // Les largeurs sont recalculees a chaque redimensionnement (ajusterColonnes) :
    // le tableau tient toujours dans sa zone et la colonne Actions reste visible
    t->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    t->viewport()->installEventFilter(this);
    ajusterColonnes();

    // Tri : clic sur un en tete (un second clic inverse l'ordre)
    h->setSectionsClickable(true);
    h->setCursor(Qt::PointingHandCursor);
    h->setToolTip(tr("Cliquez sur une colonne pour trier"));
    connect(h, &QHeaderView::sectionClicked, this, [this](int colonne) {
        if (colonne == ColActions)
            return;
        m_triCroissant = (colonne == m_triColonne) ? !m_triCroissant : true;
        m_triColonne = colonne;
        mettreAJourEntetes();
        rafraichir();
        animerEntreeLignes();
    });

    // Menu "Trier par" + sens du tri, a droite de la recherche
    auto *libelleTri = new QLabel(tr("Trier par"), this);
    libelleTri->setProperty("role", "sousTitre");
    auto *menuTri = new QComboBox(this);
    menuTri->setObjectName("comboTri");
    menuTri->addItem(tr("ID Projet"), int(ColId));
    menuTri->addItem(tr("Titre"), int(ColTitre));
    menuTri->addItem(tr("Thématique"), int(ColThematique));
    menuTri->addItem(tr("Statut"), int(ColStatut));
    menuTri->addItem(tr("Date de création"), int(ColCreation));
    menuTri->addItem(tr("Date limite"), int(ColLimite));
    menuTri->setMinimumWidth(170);
    auto *sensTri = new QPushButton(this);
    sensTri->setObjectName("boutonSensTri");
    sensTri->setCursor(Qt::PointingHandCursor);
    sensTri->setMinimumWidth(130);
    const int position = ui->layoutRecherche->indexOf(ui->boutonAnnuler);
    ui->layoutRecherche->insertWidget(position, libelleTri);
    ui->layoutRecherche->insertWidget(position + 1, menuTri);
    ui->layoutRecherche->insertWidget(position + 2, sensTri);
    connect(menuTri, &QComboBox::currentIndexChanged, this, [this, menuTri](int) {
        m_triColonne = menuTri->currentData().toInt();
        m_triCroissant = true;
        mettreAJourEntetes();
        rafraichir();
        animerEntreeLignes();
    });
    connect(sensTri, &QPushButton::clicked, this, [this] {
        m_triCroissant = !m_triCroissant;
        mettreAJourEntetes();
        rafraichir();
        animerEntreeLignes();
    });
    mettreAJourEntetes();

    // Double clic sur une ligne : afficher les informations
    connect(t, &QTableWidget::cellDoubleClicked, this, [this](int ligne, int) {
        if (ligne >= 0 && ligne < m_projetsAffiches.size())
            voirProjet(m_projetsAffiches[ligne].id());
    });
}

// Largeur des colonnes selon la place disponible : la colonne Actions garde toujours
// la place de ses 4 boutons, les autres se partagent le reste en proportion.
void MainWindow::ajusterColonnes()
{
    QTableWidget *t = ui->tableProjets;
    const int actions = 4 * 32 + 3 * 4 + 16;   // 4 boutons + espaces + marges
    const int reste = qMax(560, t->viewport()->width()) - actions;
    //                       ID    Titre Descr Theme Statut Creat Limite
    const double parts[] = {0.10, 0.16, 0.24, 0.11, 0.12, 0.14, 0.13};
    int utilise = 0;
    for (int c = 0; c < ColActions; ++c) {
        const int largeur = (c == ColActions - 1) ? reste - utilise : int(reste * parts[c]);
        t->setColumnWidth(c, largeur);
        utilise += largeur;
    }
    t->setColumnWidth(ColActions, actions);
}

// Petite etiquette arrondie (Thematique, Statut)
static QWidget *badge(const QString &texte)
{
    auto *conteneur = new QWidget;
    auto *l = new QHBoxLayout(conteneur);
    l->setContentsMargins(6, 0, 6, 0);
    auto *lbl = new QLabel(ouTiret(texte), conteneur);
    lbl->setProperty("role", "badge");
    lbl->setToolTip(texte);
    l->addWidget(lbl, 0, Qt::AlignVCenter);
    l->addStretch();
    return conteneur;
}

void MainWindow::remplirTableau(const QList<Projet> &projets)
{
    m_projetsAffiches = projets;
    QTableWidget *t = ui->tableProjets;
    t->clearContents();
    t->setRowCount(projets.size());

    for (int i = 0; i < projets.size(); ++i) {
        const Projet &p = projets[i];

        auto *itemId = new QTableWidgetItem(p.id());
        QFont gras = itemId->font();
        gras.setBold(true);
        itemId->setFont(gras);
        itemId->setForeground(couleurs().accent);
        t->setItem(i, ColId, itemId);

        t->setItem(i, ColTitre, new QTableWidgetItem(p.titre()));
        auto *itemDescr = new QTableWidgetItem(p.description());
        itemDescr->setToolTip(p.description());
        t->setItem(i, ColDescription, itemDescr);

        t->setCellWidget(i, ColThematique, badge(p.thematique()));
        t->setCellWidget(i, ColStatut, badge(p.statut()));

        auto *itemCrea = new QTableWidgetItem(dateOuTiret(p.dateCreation()));
        auto *itemLim  = new QTableWidgetItem(dateOuTiret(p.dateLimite()));
        for (QTableWidgetItem *it : {itemCrea, itemLim})
            it->setTextAlignment(Qt::AlignCenter);
        t->setItem(i, ColCreation, itemCrea);
        t->setItem(i, ColLimite, itemLim);

        // Boutons d'action : voir, modifier, fiche PDF, supprimer
        auto *actions = new QWidget;
        auto *la = new QHBoxLayout(actions);
        la->setContentsMargins(4, 0, 4, 0);
        la->setSpacing(4);
        la->addStretch();

        const QString id = p.id();
        auto creer = [actions, la](const QString &icone, const QString &aide, const char *role) {
            auto *b = new QToolButton(actions);
            b->setText(icone);
            b->setToolTip(aide);
            b->setProperty("role", role);
            b->setCursor(Qt::PointingHandCursor);
            b->setFixedSize(32, 30);
            la->addWidget(b);
            return b;
        };
        connect(creer(QStringLiteral("👁"), tr("Voir"), "action"), &QToolButton::clicked,
                this, [this, id] { voirProjet(id); });
        connect(creer(QStringLiteral("✎"), tr("Modifier"), "action"), &QToolButton::clicked,
                this, [this, id] { modifierProjet(id); });
        connect(creer(QStringLiteral("📄"), tr("Fiche PDF du projet"), "action"), &QToolButton::clicked,
                this, [this, id] { exporterFichePdf(id); });
        connect(creer(QStringLiteral("🗑"), tr("Supprimer"), "danger"), &QToolButton::clicked,
                this, [this, id] { supprimerProjet(id); });

        t->setCellWidget(i, ColActions, actions);
    }
    remplirKanban();
}

void MainWindow::rafraichir()
{
    QString erreur;
    QList<Projet> projets = Projet::lister(ui->lineEditRecherche->text(), &erreur);
    if (!erreur.isEmpty()) {
        QMessageBox::critical(this, tr("Base de données"),
                              tr("Impossible de lire les projets.\n%1").arg(erreur));
        notifier(Notification::Erreur, tr("Lecture impossible"), erreur, QString(), false);
    }
    trierProjets(projets);
    remplirTableau(projets);
    mettreAJourStatistiques();
}

// ===== Formulaire "Ajouter un projet" / "Modifier le projet" =====
// Meme presentation pour les deux : titre en majuscules, puis tous les champs
// alignes sur une ligne. En modification, l'ID (cle du projet) n'est pas modifiable.
// Renvoie true si l'utilisateur a clique Enregistrer avec une saisie valide ;
// p contient alors les valeurs saisies.

static const QStringList THEMATIQUES = {"IA", "Web", "IoT", "Mobile", "Cybersécurité", "Data"};
static const QStringList STATUTS     = {"En attente", "En cours", "Terminé", "Annulé"};

// Selectionne le texte dans la liste, ou l'ajoute s'il vient de la base sans y figurer
static void choisir(QComboBox *combo, const QString &texte)
{
    if (texte.isEmpty())
        return;
    int i = combo->findText(texte);
    if (i < 0) {
        combo->addItem(texte);
        i = combo->count() - 1;
    }
    combo->setCurrentIndex(i);
}

// Equipes rattachees a un projet (table EQUIPES, cle FK_EQUIPE_PROJET), sous la forme "E01 Alpha"
static QStringList equipesDuProjet(const QString &idProjet)
{
    QStringList equipes;
    QSqlQuery q;
    q.prepare("SELECT ID_EQUIPE, NOM_EQUIPE FROM EQUIPES WHERE ID_PROJET = :id ORDER BY ID_EQUIPE");
    q.bindValue(":id", idProjet);
    if (q.exec()) {
        while (q.next())
            equipes << QString("%1 %2").arg(q.value(0).toString(), q.value(1).toString()).trimmed();
    }
    return equipes;
}

// Une seule fenetre pour Ajouter, Modifier et Voir (lectureSeule) : meme format, meme taille
// (1070 x 650, comme les fenetres du module Equipes). En haut l'identifiant et le titre,
// puis thematique, statut et dates sur une ligne, puis la description sur toute la largeur.
// Renvoie true si l'utilisateur a clique Enregistrer avec une saisie valide.
bool MainWindow::ouvrirFormulaire(Projet &p, bool modification, bool lectureSeule)
{
    QDialog dlg(this);
    dlg.setWindowTitle(lectureSeule ? tr("Informations du projet")
                       : modification ? tr("Modifier le projet") : tr("Ajouter un projet"));
    dlg.resize(1070, 650);
    dlg.setMinimumSize(1070, 650);

    // ===== Champs =====
    auto *id = new QLineEdit(p.id(), &dlg);
    id->setMaxLength(30);
    id->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9_]{0,30}"), id));
    id->setReadOnly(modification || lectureSeule);

    auto *titre = new QLineEdit(p.titre(), &dlg);
    titre->setMaxLength(80);
    titre->setPlaceholderText(tr("Nom du projet"));
    titre->setReadOnly(lectureSeule);

    auto *description = new QPlainTextEdit(p.description(), &dlg);
    description->setPlaceholderText(tr("Objectif du projet, technologies, public visé…"));
    description->setReadOnly(lectureSeule);

    auto *thematique = new QComboBox(&dlg);
    thematique->addItems(THEMATIQUES);
    choisir(thematique, p.thematique());

    auto *statut = new QComboBox(&dlg);
    statut->addItems(STATUTS);
    choisir(statut, p.statut());

    auto *dateCreation = new QDateEdit(p.dateCreation().isValid() ? p.dateCreation()
                                                                   : QDate::currentDate(), &dlg);
    auto *dateLimite = new QDateEdit(p.dateLimite().isValid() ? p.dateLimite()
                                                               : QDate::currentDate().addDays(7), &dlg);
    for (QDateEdit *d : {dateCreation, dateLimite}) {
        d->setCalendarPopup(true);
        d->setDisplayFormat("dd/MM/yyyy");
    }

    // En lecture seule, listes et dates ne s'ouvrent pas (mais restent lisibles)
    if (lectureSeule) {
        for (QWidget *w : std::initializer_list<QWidget *>{thematique, statut, dateCreation, dateLimite}) {
            w->setAttribute(Qt::WA_TransparentForMouseEvents);
            w->setFocusPolicy(Qt::NoFocus);
        }
        for (QDateEdit *d : {dateCreation, dateLimite}) {
            d->setReadOnly(true);
            d->setButtonSymbols(QAbstractSpinBox::NoButtons);
        }
    }

    // ===== Titre et sous titre =====
    auto *labelTitre = new QLabel(lectureSeule ? tr("INFORMATIONS DU PROJET")
                                  : modification ? tr("MODIFIER LE PROJET") : tr("AJOUTER UN PROJET"), &dlg);
    labelTitre->setProperty("role", "titreDialogue");
    auto *sousTitre = new QLabel(tr("Informations du projet"), &dlg);
    sousTitre->setProperty("role", "gras");

    // ===== Cadre des champs =====
    auto *cadre = new QFrame(&dlg);
    cadre->setProperty("role", "carte");
    auto *grille = new QGridLayout(cadre);
    grille->setContentsMargins(18, 16, 18, 18);
    grille->setHorizontalSpacing(14);
    grille->setVerticalSpacing(6);

    auto etiquette = [cadre](const QString &texte) {
        auto *l = new QLabel(texte, cadre);
        l->setProperty("role", "etiquette");
        return l;
    };

    // Ligne 1 : identifiant et titre
    grille->addWidget(etiquette(tr("ID Projet")), 0, 0);
    grille->addWidget(etiquette(tr("Titre")), 0, 1, 1, 3);
    grille->addWidget(id, 1, 0);
    grille->addWidget(titre, 1, 1, 1, 3);
    grille->setRowMinimumHeight(2, 10);

    // Ligne 2 : thematique, statut, dates
    grille->addWidget(etiquette(tr("Thématique")), 3, 0);
    grille->addWidget(etiquette(tr("Statut")), 3, 1);
    grille->addWidget(etiquette(tr("Date de création")), 3, 2);
    grille->addWidget(etiquette(tr("Date limite")), 3, 3);
    grille->addWidget(thematique, 4, 0);
    grille->addWidget(statut, 4, 1);
    grille->addWidget(dateCreation, 4, 2);
    grille->addWidget(dateLimite, 4, 3);
    grille->setRowMinimumHeight(5, 10);

    // Ligne 3 : description, avec un compteur de caracteres
    auto *ligneDescr = new QHBoxLayout;
    ligneDescr->addWidget(etiquette(tr("Description")));
    ligneDescr->addStretch();
    auto *compteur = new QLabel(cadre);
    compteur->setProperty("role", "sousTitre");
    ligneDescr->addWidget(compteur);
    grille->addLayout(ligneDescr, 6, 0, 1, 4);
    grille->addWidget(description, 7, 0, 1, 4);
    grille->setRowStretch(7, 1);
    for (int col = 0; col < 4; ++col)
        grille->setColumnStretch(col, 1);

    auto majCompteur = [compteur, description] {
        const int n = description->toPlainText().trimmed().size();
        compteur->setText(tr("%1 / 400 caractères").arg(n));
        compteur->setStyleSheet(n > 400 ? "color: #f87171;" : QString());
    };
    majCompteur();
    connect(description, &QPlainTextEdit::textChanged, &dlg, majCompteur);

    // En lecture seule : equipes liees et note, sous les champs
    auto *infos = new QLabel(&dlg);
    infos->setProperty("role", "sousTitre");
    infos->setWordWrap(true);
    if (lectureSeule) {
        const QStringList equipes = equipesDuProjet(p.id());
        QString note = tr("pas encore évalué");
        for (const ScoreProjet &s : classementProjets())
            if (s.id == p.id())
                note = tr("%1 / 20 (%2 évaluation(s))")
                           .arg(QLocale(QLocale::French).toString(s.moyenne, 'f', 1))
                           .arg(s.nbEvaluations);
        infos->setText(tr("Équipes : %1     ·     Note moyenne : %2")
                           .arg(equipes.isEmpty() ? tr("aucune") : equipes.join(", "), note));
    }
    infos->setVisible(lectureSeule);

    // ===== Boutons =====
    auto *ligneBoutons = new QHBoxLayout;
    ligneBoutons->setSpacing(12);
    auto *enregistrer = new QPushButton(tr("Enregistrer"), &dlg);
    enregistrer->setProperty("role", "primaire");
    auto *annuler = new QPushButton(lectureSeule ? tr("Fermer") : tr("Annuler"), &dlg);
    for (QPushButton *b : {enregistrer, annuler}) {
        b->setMinimumSize(150, 42);
        b->setCursor(Qt::PointingHandCursor);
    }
    if (lectureSeule) {
        auto *fiche = new QPushButton(tr("📄  Fiche PDF"), &dlg);
        auto *modifier = new QPushButton(tr("✎  Modifier"), &dlg);
        for (QPushButton *b : {fiche, modifier}) {
            b->setMinimumSize(150, 42);
            b->setCursor(Qt::PointingHandCursor);
        }
        const QString idProjet = p.id();
        connect(fiche, &QPushButton::clicked, &dlg, [this, idProjet] { exporterFichePdf(idProjet); });
        connect(modifier, &QPushButton::clicked, &dlg, [this, &dlg, idProjet] {
            dlg.reject();
            QTimer::singleShot(0, this, [this, idProjet] { modifierProjet(idProjet); });
        });
        ligneBoutons->addWidget(fiche);
        ligneBoutons->addWidget(modifier);
        enregistrer->hide();
        annuler->setProperty("role", "primaire");
    }
    ligneBoutons->addStretch();
    ligneBoutons->addWidget(enregistrer);
    ligneBoutons->addWidget(annuler);

    auto *racine = new QVBoxLayout(&dlg);
    racine->setContentsMargins(30, 24, 30, 22);
    racine->setSpacing(12);
    racine->addWidget(labelTitre);
    racine->addWidget(sousTitre);
    racine->addWidget(cadre, 1);
    racine->addWidget(infos);
    racine->addLayout(ligneBoutons);

    connect(annuler, &QPushButton::clicked, &dlg, &QDialog::reject);

    // Enregistrer : controle de saisie, la fenetre reste ouverte si un champ est invalide
    connect(enregistrer, &QPushButton::clicked, &dlg, [&] {
        const QString t = dlg.windowTitle();
        const QString valId = id->text().trimmed().toUpper();
        const QString valDescr = description->toPlainText().trimmed();

        if (valId.isEmpty()) {
            QMessageBox::warning(&dlg, t, tr("L'ID du projet est obligatoire."));
            id->setFocus();
            return;
        }
        if (titre->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, t, tr("Le titre est obligatoire."));
            titre->setFocus();
            return;
        }
        if (valDescr.isEmpty()) {
            QMessageBox::warning(&dlg, t, tr("La description est obligatoire."));
            description->setFocus();
            return;
        }
        if (valDescr.size() > 400) {
            QMessageBox::warning(&dlg, t, tr("La description ne doit pas dépasser 400 caractères."));
            description->setFocus();
            return;
        }
        if (dateLimite->date() < dateCreation->date()) {
            QMessageBox::warning(&dlg, t, tr("La date limite doit être après la date de création."));
            dateLimite->setFocus();
            return;
        }
        if (!modification && Projet::existe(valId)) {
            QMessageBox::warning(&dlg, t, tr("Un projet avec l'ID %1 existe déjà.").arg(valId));
            id->setFocus();
            return;
        }
        dlg.accept();
    });

    animerApparition(&dlg);
    if (dlg.exec() != QDialog::Accepted || lectureSeule)
        return false;

    p = Projet(id->text().trimmed().toUpper(),
               titre->text().trimmed(),
               description->toPlainText().trimmed(),
               thematique->currentText(),
               statut->currentText(),
               dateCreation->date(),
               dateLimite->date());
    return true;
}

// ===== Create =====
void MainWindow::ajouterProjet()
{
    Projet p;
    p.setId(prochainId());
    if (!ouvrirFormulaire(p, false))
        return;

    QString erreur;
    if (!p.ajouter(&erreur)) {
        QMessageBox::critical(this, tr("Ajout"), tr("Échec de l'ajout.\n%1").arg(erreur));
        notifier(Notification::Erreur, tr("Échec de l'ajout"), erreur, QString(), false);
        return;
    }
    empiler({Action::Ajout, Projet(), p});
    rafraichir();
    animerLigne(p.id());
    notifier(Notification::Succes, tr("Projet ajouté"),
             tr("%1 « %2 » a été ajouté.").arg(p.id(), p.titre()),
             p.id(), true, tr("Annuler"), actionAnnuler());
    QTimer::singleShot(700, this, &MainWindow::verifierEcheances);   // sa date limite est peut etre proche
}

// ===== Read =====
// Meme fenetre que Ajouter / Modifier, en lecture seule, avec les equipes et la note
void MainWindow::voirProjet(const QString &id)
{
    Projet p;
    QString erreur;
    if (!Projet::charger(id, p, &erreur)) {
        QMessageBox::warning(this, tr("Informations du projet"), erreur);
        return;
    }
    ouvrirFormulaire(p, false, true);
}

// ===== Update =====
void MainWindow::modifierProjet(const QString &id)
{
    Projet avant;
    QString erreur;
    if (!Projet::charger(id, avant, &erreur)) {
        QMessageBox::warning(this, tr("Modifier le projet"), erreur);
        return;
    }

    Projet apres = avant;
    if (!ouvrirFormulaire(apres, true))
        return;

    if (!apres.modifier(&erreur)) {
        QMessageBox::critical(this, tr("Modification"),
                              tr("Échec de la modification.\n%1").arg(erreur));
        notifier(Notification::Erreur, tr("Échec de la modification"), erreur, apres.id(), false);
        return;
    }
    empiler({Action::Modification, avant, apres});
    rafraichir();
    animerLigne(apres.id());
    notifier(Notification::Succes, tr("Projet modifié"),
             tr("%1 « %2 » a été mis à jour.").arg(apres.id(), apres.titre()),
             apres.id(), true, tr("Annuler"), actionAnnuler());
    QTimer::singleShot(700, this, &MainWindow::verifierEcheances);
}

// ===== Delete =====
void MainWindow::supprimerProjet(const QString &id)
{
    Projet avant;
    QString erreur;
    if (!Projet::charger(id, avant, &erreur)) {
        QMessageBox::warning(this, tr("Suppression"), erreur);
        return;
    }

    if (m_suppressionEnCours)   // une suppression est deja en train de s'animer
        return;

    const QString question = tr("Supprimer le projet %1 « %2 » ?").arg(avant.id(), avant.titre());
    if (QMessageBox::question(this, tr("Suppression"), question) != QMessageBox::Yes)
        return;

    // La ligne est balayee en rouge, puis le projet est supprime
    m_suppressionEnCours = true;
    animerSuppression(id, [this, id, avant] {
        m_suppressionEnCours = false;
        QString erreur;
        if (!Projet::supprimer(id, &erreur)) {
            QMessageBox::critical(this, tr("Suppression"),
                                  tr("Échec de la suppression.\n%1").arg(erreur));
            notifier(Notification::Erreur, tr("Échec de la suppression"), erreur, id, false);
            return;
        }
        empiler({Action::Suppression, avant, Projet()});
        rafraichir();
        notifier(Notification::Suppression, tr("Projet supprimé"),
                 tr("%1 « %2 » a été supprimé.").arg(id, avant.titre()),
                 QString(), true, tr("Annuler"), actionAnnuler());
    });
}

// ===== Annuler (Ctrl+Z) =====
void MainWindow::empiler(const Action &a)
{
    m_historique.append(a);
    ui->boutonAnnuler->setEnabled(true);
}

void MainWindow::annulerDerniereAction()
{
    if (m_historique.isEmpty())
        return;

    const Action a = m_historique.takeLast();
    QString erreur;
    bool ok = false;
    QString message;

    switch (a.type) {
    case Action::Ajout:
        ok = Projet::supprimer(a.apres.id(), &erreur);
        message = tr("Ajout du projet %1 annulé.").arg(a.apres.id());
        break;
    case Action::Modification:
        ok = a.avant.modifier(&erreur);
        message = tr("Modification du projet %1 annulée.").arg(a.avant.id());
        break;
    case Action::Suppression:
        ok = a.avant.ajouter(&erreur);
        message = tr("Projet %1 restauré.").arg(a.avant.id());
        break;
    }

    ui->boutonAnnuler->setEnabled(!m_historique.isEmpty());
    if (!ok) {
        QMessageBox::critical(this, tr("Annuler"), tr("Impossible d'annuler.\n%1").arg(erreur));
        notifier(Notification::Erreur, tr("Annulation impossible"), erreur, QString(), false);
        return;
    }
    rafraichir();
    if (a.type != Action::Ajout)
        animerLigne(a.avant.id());   // le projet modifie ou restaure s'illumine
    notifier(Notification::Info, tr("Action annulée"), message,
             a.type == Action::Ajout ? QString() : a.avant.id());
}

// Cherche le plus grand numero parmi les ID de forme Pnnn et propose le suivant
QString MainWindow::prochainId() const
{
    int maxi = 0;
    QSqlQuery q;
    if (q.exec("SELECT ID_PROJET FROM PROJETS")) {
        static const QRegularExpression re("^P(\\d+)$", QRegularExpression::CaseInsensitiveOption);
        while (q.next()) {
            const auto m = re.match(q.value(0).toString().trimmed());
            if (m.hasMatch())
                maxi = qMax(maxi, m.captured(1).toInt());
        }
    }
    return QString("P%1").arg(maxi + 1, 3, 10, QChar('0'));
}

// Dessins faits a la main : le donut des statistiques et les confettis du gagnant
bool MainWindow::eventFilter(QObject *objet, QEvent *evenement)
{
    // Cartes et colonnes du Kanban (glisser deposer)
    if (gererKanban(objet, evenement))
        return true;

    // Le tableau change de taille : colonnes recalculees une fois la mise en page terminee
    // (l'apparition d'une barre de defilement peut encore changer la largeur)
    if (objet == ui->tableProjets->viewport() && evenement->type() == QEvent::Resize)
        QTimer::singleShot(0, this, &MainWindow::ajusterColonnes);

    if (evenement->type() == QEvent::Paint) {
        if (objet->objectName() == "donutStatut") {
            dessinerDonut(static_cast<QWidget *>(objet));
            return true;
        }
        if (objet->objectName() == "confettis") {
            dessinerConfettis(static_cast<QWidget *>(objet));
            return true;
        }
    }

    // Clic sur le fond assombri : on ferme le panneau de notifications
    if (objet == m_voile && evenement->type() == QEvent::MouseButtonPress) {
        fermerPanneau();
        return true;
    }

    auto *cadre = qobject_cast<QFrame *>(objet);
    if (cadre && (cadre->objectName() == "toast" || cadre->objectName() == "carteNotif")) {
        // Survol d'un toast : son compte a rebours se met en pause
        if (cadre->objectName() == "toast") {
            auto *vie = cadre->findChild<QVariantAnimation *>("vieToast");
            if (vie && evenement->type() == QEvent::Enter)
                vie->pause();
            if (vie && evenement->type() == QEvent::Leave && vie->state() == QAbstractAnimation::Paused)
                vie->resume();
        }

        // Clic : on va au projet concerne
        if (evenement->type() == QEvent::MouseButtonRelease
            && static_cast<QMouseEvent *>(evenement)->button() == Qt::LeftButton) {
            const QString id = cadre->property("idProjet").toString();
            if (cadre->objectName() == "carteNotif") {
                const int i = cadre->property("indexNotif").toInt();
                if (i >= 0 && i < m_notifications.size())
                    m_notifications[i].lue = true;
                sauverNotifications();
                mettreAJourBadge();
                if (!id.isEmpty()) {
                    fermerPanneau();
                    allerAuProjet(id);
                } else {
                    remplirPanneau();
                }
            } else if (!id.isEmpty()) {
                fermerToast(cadre);
                allerAuProjet(id);
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(objet, evenement);
}

// ==========================================================
// ===== Animations =====
// ==========================================================
// QPropertyAnimation anime une propriete Qt (opacite, transparence d'une fenetre).
// QVariantAnimation produit une suite de valeurs (nombres, couleurs) qu'on applique nous memes.

// Fondu d'une page du menu : elle apparait progressivement (0 -> 1 d'opacite)
void MainWindow::animerPage(QWidget *page)
{
    // Si une animation de cette page tourne encore, on l'arrete
    for (auto *a : page->findChildren<QPropertyAnimation *>("animationPage", Qt::FindDirectChildrenOnly))
        a->stop();

    auto *effet = new QGraphicsOpacityEffect(page);
    effet->setOpacity(0.0);
    page->setGraphicsEffect(effet);   // remplace l'effet precedent s'il y en avait un

    auto *anim = new QPropertyAnimation(effet, "opacity", page);
    anim->setObjectName("animationPage");
    anim->setDuration(350);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    // A la fin on retire l'effet : la page redevient un widget normal (plus rapide)
    QPointer<QGraphicsOpacityEffect> garde(effet);
    connect(anim, &QPropertyAnimation::finished, page, [page, garde] {
        if (garde && page->graphicsEffect() == garde)
            page->setGraphicsEffect(nullptr);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// Les chiffres des cartes defilent de l'ancienne valeur jusqu'a la nouvelle
void MainWindow::animerCompteur(QLabel *label, int valeur)
{
    for (auto *a : label->findChildren<QVariantAnimation *>(QString(), Qt::FindDirectChildrenOnly))
        a->stop();

    const int depart = label->text().toInt();
    if (depart == valeur) {
        label->setText(QString::number(valeur));
        return;
    }

    auto *anim = new QVariantAnimation(label);
    anim->setDuration(700);
    anim->setStartValue(depart);
    anim->setEndValue(valeur);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QVariantAnimation::valueChanged, label, [label](const QVariant &v) {
        label->setText(QString::number(v.toInt()));
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// Une fenetre (formulaire, informations) apparait en fondu ;
// avec glisser = true elle monte aussi de 60 pixels jusqu'au centre de l'application
void MainWindow::animerApparition(QWidget *fenetre, bool glisser)
{
    fenetre->setWindowOpacity(0.0);
    auto *fondu = new QPropertyAnimation(fenetre, "windowOpacity", fenetre);
    fondu->setDuration(320);
    fondu->setStartValue(0.0);
    fondu->setEndValue(1.0);
    fondu->setEasingCurve(QEasingCurve::OutCubic);
    fondu->start(QAbstractAnimation::DeleteWhenStopped);

    if (!glisser)
        return;

    const QPoint cible = geometry().center() - QPoint(fenetre->width() / 2, fenetre->height() / 2);
    fenetre->move(cible + QPoint(0, 60));
    auto *glisse = new QPropertyAnimation(fenetre, "pos", fenetre);
    glisse->setDuration(380);
    glisse->setStartValue(cible + QPoint(0, 60));
    glisse->setEndValue(cible);
    glisse->setEasingCurve(QEasingCurve::OutBack);   // depasse un peu puis se pose
    glisse->start(QAbstractAnimation::DeleteWhenStopped);
}

// La ligne du projet ajoute ou modifie s'illumine en couleur d'accent puis s'eteint.
// On pose un "voile" colore par dessus la ligne (la feuille de style du tableau
// empeche de changer directement la couleur de fond des cases), puis on le fait disparaitre.
void MainWindow::animerLigne(const QString &id)
{
    QTableWidget *t = ui->tableProjets;

    // Numero de ligne du projet dans le tableau (-1 s'il n'est pas affiche)
    auto ligneDuProjet = [this, id] {
        for (int r = 0; r < m_projetsAffiches.size(); ++r)
            if (m_projetsAffiches[r].id() == id)
                return r;
        return -1;
    };

    const int ligne = ligneDuProjet();
    if (ligne < 0)
        return;
    t->scrollToItem(t->item(ligne, 0));

    auto *voile = new QWidget(t->viewport());
    voile->setAttribute(Qt::WA_TransparentForMouseEvents);   // les clics passent au travers

    // Place le voile sur toute la largeur de la ligne (recalcule a chaque image)
    auto placer = [t, voile, ligneDuProjet] {
        const int r = ligneDuProjet();
        if (r < 0) {
            voile->hide();
            return;
        }
        QRect zone = t->visualRect(t->model()->index(r, 0));
        zone.setLeft(0);
        zone.setRight(t->viewport()->width());
        voile->setGeometry(zone);
    };
    placer();
    voile->show();
    voile->raise();

    const QColor accent = couleurs().accent;
    auto *anim = new QVariantAnimation(voile);
    anim->setDuration(1500);
    anim->setStartValue(110);   // transparence de depart (0 a 255)
    anim->setEndValue(0);
    anim->setEasingCurve(QEasingCurve::InQuad);
    connect(anim, &QVariantAnimation::valueChanged, voile, [voile, placer, accent](const QVariant &v) {
        voile->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4); border: none;")
                                 .arg(accent.red()).arg(accent.green()).arg(accent.blue())
                                 .arg(v.toInt()));
        placer();
    });
    connect(anim, &QVariantAnimation::finished, voile, &QObject::deleteLater);
    anim->start();
}

// Les lignes du tableau apparaissent l'une apres l'autre : chaque ligne est d'abord
// cachee par un cache de la couleur du tableau, avec un trait d'accent a gauche,
// qui glisse vers la droite et la decouvre. Chaque ligne part 80 ms apres la precedente.
void MainWindow::animerEntreeLignes()
{
    QTableWidget *t = ui->tableProjets;
    const Couleurs c = couleurs();

    for (int r = 0; r < t->rowCount(); ++r) {
        QRect zone = t->visualRect(t->model()->index(r, 0));
        if (!zone.isValid() || zone.top() > t->viewport()->height())
            break;   // lignes hors de l'ecran : inutile de les animer
        zone.setLeft(0);
        zone.setRight(t->viewport()->width());

        auto *cache = new QWidget(t->viewport());
        cache->setAttribute(Qt::WA_TransparentForMouseEvents);
        cache->setStyleSheet(QString("background-color: %1; border: none; border-left: 3px solid %2;")
                                 .arg(c.carte.name(), c.accent.name()));
        cache->setGeometry(zone);
        cache->show();
        cache->raise();

        auto *glisse = new QPropertyAnimation(cache, "geometry");
        glisse->setDuration(450);
        glisse->setStartValue(zone);
        glisse->setEndValue(QRect(zone.right(), zone.top(), 0, zone.height()));
        glisse->setEasingCurve(QEasingCurve::InOutCubic);

        auto *groupe = new QSequentialAnimationGroup(cache);
        groupe->addPause(80 * r);
        groupe->addAnimation(glisse);
        connect(groupe, &QAbstractAnimation::finished, cache, &QObject::deleteLater);
        groupe->start();
    }
}

// Avant une suppression, une bande rouge balaie la ligne de gauche a droite ;
// quand elle arrive au bout, on appelle "ensuite" (qui supprime vraiment le projet).
void MainWindow::animerSuppression(const QString &id, const std::function<void()> &ensuite)
{
    QTableWidget *t = ui->tableProjets;
    int ligne = -1;
    for (int r = 0; r < m_projetsAffiches.size(); ++r)
        if (m_projetsAffiches[r].id() == id)
            ligne = r;
    if (ligne < 0) {
        ensuite();
        return;
    }

    QRect zone = t->visualRect(t->model()->index(ligne, 0));
    zone.setLeft(0);
    zone.setRight(t->viewport()->width());

    const QColor rouge = couleurs().danger;
    auto *balayage = new QWidget(t->viewport());
    balayage->setAttribute(Qt::WA_TransparentForMouseEvents);
    balayage->setStyleSheet(QString("background-color: rgba(%1, %2, %3, 150); border: none;")
                                .arg(rouge.red()).arg(rouge.green()).arg(rouge.blue()));
    balayage->setGeometry(QRect(zone.left(), zone.top(), 0, zone.height()));
    balayage->show();
    balayage->raise();

    auto *anim = new QPropertyAnimation(balayage, "geometry", balayage);
    anim->setDuration(450);
    anim->setStartValue(QRect(zone.left(), zone.top(), 0, zone.height()));
    anim->setEndValue(zone);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QAbstractAnimation::finished, this, [balayage, ensuite] {
        balayage->deleteLater();
        ensuite();
    });
    anim->start();
}

// Halo couleur d'accent qui grandit et retrecit en boucle autour de "+ Ajouter un projet"
void MainWindow::animerBoutonAjouter()
{
    auto *halo = new QGraphicsDropShadowEffect(ui->boutonAjouter);
    halo->setOffset(0, 0);
    halo->setColor(couleurs().accent);
    halo->setBlurRadius(0);
    ui->boutonAjouter->setGraphicsEffect(halo);

    auto *pulsation = new QPropertyAnimation(halo, "blurRadius", halo);
    pulsation->setDuration(2000);
    pulsation->setStartValue(0.0);
    pulsation->setKeyValueAt(0.5, 34.0);
    pulsation->setEndValue(0.0);
    pulsation->setEasingCurve(QEasingCurve::InOutSine);
    pulsation->setLoopCount(-1);   // -1 = en boucle, sans fin
    pulsation->start();
}

// ==========================================================
// ===== Tri du tableau =====
// ==========================================================
// Un clic sur l'en tete d'une colonne trie le tableau sur cette colonne ; un second clic
// inverse l'ordre. Le menu "Trier par" a cote de la recherche fait la meme chose.
// Le tri est fait ici, sur la liste lue par Projet::lister() : projet.cpp reste du CRUD pur.

// ID de forme P001, P002... compares par leur numero (P2 avant P10)
static int numeroId(const QString &id)
{
    static const QRegularExpression re("(\\d+)$");
    const auto m = re.match(id);
    return m.hasMatch() ? m.captured(1).toInt() : -1;
}

// Ordre logique des statuts (et non alphabetique)
static int rangStatut(const QString &statut)
{
    const QStringList ordre = {"En cours", "En attente", "Terminé", "Annulé"};
    const int i = ordre.indexOf(statut);
    return i < 0 ? ordre.size() : i;
}

// Compare deux dates ; une date vide va toujours a la fin
static int comparerDates(const QDate &a, const QDate &b)
{
    if (!a.isValid() || !b.isValid())
        return a.isValid() ? -1 : (b.isValid() ? 1 : 0);
    return a < b ? -1 : (a > b ? 1 : 0);
}

void MainWindow::trierProjets(QList<Projet> &projets) const
{
    const int colonne = m_triColonne;
    const bool croissant = m_triCroissant;

    std::stable_sort(projets.begin(), projets.end(), [colonne, croissant](const Projet &a, const Projet &b) {
        int r = 0;
        switch (colonne) {
        case ColTitre:
            r = QString::localeAwareCompare(a.titre(), b.titre());
            break;
        case ColDescription:
            r = QString::localeAwareCompare(a.description(), b.description());
            break;
        case ColThematique:
            r = QString::localeAwareCompare(a.thematique(), b.thematique());
            break;
        case ColStatut:
            r = rangStatut(a.statut()) - rangStatut(b.statut());
            break;
        case ColCreation:
            r = comparerDates(a.dateCreation(), b.dateCreation());
            break;
        case ColLimite:
            r = comparerDates(a.dateLimite(), b.dateLimite());
            break;
        default:   // ColId
            r = numeroId(a.id()) - numeroId(b.id());
            if (r == 0)
                r = QString::compare(a.id(), b.id());
            break;
        }
        if (r == 0)   // a egalite : par ID, pour un ordre stable
            r = numeroId(a.id()) - numeroId(b.id());
        return croissant ? r < 0 : r > 0;
    });
}

// Fleche ▲ / ▼ sur la colonne triee, et menu "Trier par" synchronise
void MainWindow::mettreAJourEntetes()
{
    const QStringList noms = {tr("ID Projet"), tr("Titre"), tr("Description"), tr("Thématique"),
                              tr("Statut"), tr("Date de création"), tr("Date limite"), tr("Actions")};
    for (int c = 0; c < NbColonnes; ++c) {
        QString texte = noms[c];
        if (c == m_triColonne)
            texte += m_triCroissant ? QStringLiteral("  ▲") : QStringLiteral("  ▼");
        if (QTableWidgetItem *entete = ui->tableProjets->horizontalHeaderItem(c))
            entete->setText(texte);
        else
            ui->tableProjets->setHorizontalHeaderItem(c, new QTableWidgetItem(texte));
    }

    if (auto *menu = findChild<QComboBox *>("comboTri")) {
        const QSignalBlocker bloque(menu);
        menu->setCurrentIndex(menu->findData(m_triColonne));
    }
    if (auto *sens = findChild<QPushButton *>("boutonSensTri"))
        sens->setText(m_triCroissant ? tr("▲ Croissant") : tr("▼ Décroissant"));
}

// ==========================================================
// ===== Evaluation : projet gagnant =====
// ==========================================================
// La note d'un projet est la moyenne des notes (EVALUATION.NOTE) des equipes qui travaillent
// dessus : PROJETS <- EQUIPES.ID_PROJET, EQUIPES.ID_EVALUATION -> EVALUATION.
// A moyenne egale, la meilleure note puis le nombre d'evaluations departagent.

QList<MainWindow::ScoreProjet> MainWindow::classementProjets(QString *erreur) const
{
    QList<ScoreProjet> classement;
    QSqlQuery q;
    if (!q.exec("SELECT P.ID_PROJET, P.TITRE, P.THEMATIQUE, "
                "AVG(V.NOTE), MAX(V.NOTE), COUNT(V.NOTE) "
                "FROM PROJETS P "
                "JOIN EQUIPES E ON E.ID_PROJET = P.ID_PROJET "
                "JOIN EVALUATION V ON V.ID_EVALUATION = E.ID_EVALUATION "
                "WHERE V.NOTE IS NOT NULL "
                "GROUP BY P.ID_PROJET, P.TITRE, P.THEMATIQUE "
                "ORDER BY 4 DESC, 5 DESC, 6 DESC")) {
        if (erreur)
            *erreur = q.lastError().text();
        return classement;
    }
    while (q.next()) {
        ScoreProjet s;
        s.id = q.value(0).toString();
        s.titre = q.value(1).toString();
        s.thematique = q.value(2).toString();
        s.moyenne = q.value(3).toDouble();
        s.meilleure = q.value(4).toDouble();
        s.nbEvaluations = q.value(5).toInt();
        classement << s;
    }
    return classement;
}

static QString note1(double v)
{
    return QLocale(QLocale::French).toString(v, 'f', 1);
}

// Fenetre du gagnant : le trophee apparait avec un rebond, des confettis tombent,
// la moyenne defile jusqu'a sa valeur, puis le classement complet s'affiche ligne par ligne.
void MainWindow::afficherProjetGagnant()
{
    QString erreur;
    const QList<ScoreProjet> classement = classementProjets(&erreur);
    if (!erreur.isEmpty()) {
        QMessageBox::warning(this, tr("Évaluation"),
                             tr("Impossible de lire les notes.\n%1").arg(erreur));
        notifier(Notification::Erreur, tr("Classement impossible"), erreur, QString(), false);
        return;
    }
    if (classement.isEmpty()) {
        notifier(Notification::Info, tr("Pas encore de gagnant"),
                 tr("Aucun projet n'a de note : les notes viennent des évaluations de ses équipes."));
        return;
    }

    const Couleurs c = couleurs();
    const ScoreProjet gagnant = classement.first();
    int exAequo = 0;
    for (const ScoreProjet &s : classement)
        if (qFuzzyCompare(s.moyenne, gagnant.moyenne) && qFuzzyCompare(s.meilleure, gagnant.meilleure)
            && s.nbEvaluations == gagnant.nbEvaluations)
            ++exAequo;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Évaluation : projet gagnant"));
    dlg.setFixedSize(1070, 650);

    auto *titre = new QLabel(tr("PROJET GAGNANT"), &dlg);
    titre->setProperty("role", "titreDialogue");
    auto *sousTitre = new QLabel(tr("Classement selon la moyenne des notes des équipes de chaque projet"), &dlg);
    sousTitre->setProperty("role", "sousTitre");

    // ===== Carte du gagnant =====
    auto *carte = new QFrame(&dlg);
    carte->setObjectName("carteGagnant");
    carte->setStyleSheet(QString("QFrame#carteGagnant { background: %1; border: 2px solid %2; border-radius: 14px; }"
                                 "QFrame#carteGagnant QLabel { background: transparent; border: none; }")
                             .arg(c.carte.name(), c.accent.name()));
    carte->setFixedHeight(215);
    auto *lCarte = new QHBoxLayout(carte);
    lCarte->setContentsMargins(26, 16, 30, 16);
    lCarte->setSpacing(24);

    auto *trophee = new QLabel(QStringLiteral("🏆"), carte);
    trophee->setFixedSize(170, 170);
    trophee->setAlignment(Qt::AlignCenter);
    trophee->setStyleSheet("font-size: 1px;");
    lCarte->addWidget(trophee, 0, Qt::AlignVCenter);

    auto *lInfos = new QVBoxLayout;
    lInfos->setSpacing(4);
    auto *labelId = new QLabel(exAequo > 1 ? tr("%1  ·  ex aequo avec %2 autre(s) projet(s)").arg(gagnant.id).arg(exAequo - 1)
                                           : gagnant.id, carte);
    labelId->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: 700;").arg(c.accent.name()));
    auto *labelTitre = new QLabel(gagnant.titre, carte);
    labelTitre->setWordWrap(true);
    labelTitre->setStyleSheet(QString("color: %1; font-size: 30px; font-weight: 800;").arg(c.texte.name()));
    auto *labelTheme = new QLabel(tr("Thématique : %1").arg(ouTiret(gagnant.thematique)), carte);
    labelTheme->setStyleSheet(QString("color: %1; font-size: 13px;").arg(c.discret.name()));
    lInfos->addStretch();
    lInfos->addWidget(labelId);
    lInfos->addWidget(labelTitre);
    lInfos->addWidget(labelTheme);
    lInfos->addStretch();
    lCarte->addLayout(lInfos, 1);

    auto *lNote = new QVBoxLayout;
    lNote->setSpacing(2);
    auto *texteMoyenne = new QLabel(tr("MOYENNE"), carte);
    texteMoyenne->setAlignment(Qt::AlignCenter);
    texteMoyenne->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 700;").arg(c.discret.name()));
    auto *labelNote = new QLabel(QStringLiteral("0,0"), carte);
    labelNote->setAlignment(Qt::AlignCenter);
    labelNote->setStyleSheet(QString("color: %1; font-size: 54px; font-weight: 800;").arg(c.accent.name()));
    auto *surVingt = new QLabel(tr("sur 20"), carte);
    surVingt->setAlignment(Qt::AlignCenter);
    surVingt->setStyleSheet(QString("color: %1; font-size: 13px;").arg(c.discret.name()));
    auto *detail = new QLabel(tr("%1 évaluation(s)  ·  meilleure note %2")
                                  .arg(gagnant.nbEvaluations).arg(note1(gagnant.meilleure)), carte);
    detail->setAlignment(Qt::AlignCenter);
    detail->setStyleSheet(QString("color: %1; font-size: 12px;").arg(c.discret.name()));
    lNote->addStretch();
    lNote->addWidget(texteMoyenne);
    lNote->addWidget(labelNote);
    lNote->addWidget(surVingt);
    lNote->addWidget(detail);
    lNote->addStretch();
    lCarte->addLayout(lNote);

    // ===== Classement =====
    auto *titreClassement = new QLabel(tr("CLASSEMENT"), &dlg);
    titreClassement->setProperty("role", "etiquette");

    auto *defilement = new QScrollArea(&dlg);
    defilement->setWidgetResizable(true);
    defilement->setFrameShape(QFrame::NoFrame);
    defilement->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *contenu = new QWidget;
    contenu->setStyleSheet("background: transparent;");
    auto *lListe = new QVBoxLayout(contenu);
    lListe->setContentsMargins(0, 0, 6, 0);
    lListe->setSpacing(6);

    QList<QFrame *> lignes;
    for (int i = 0; i < classement.size(); ++i) {
        const ScoreProjet &s = classement[i];
        auto *ligne = new QFrame;
        ligne->setObjectName("ligneClassement");
        ligne->setFixedHeight(46);
        ligne->setStyleSheet(QString("QFrame#ligneClassement { background: %1; border: 1px solid %2; border-radius: 8px; }"
                                     "QFrame#ligneClassement QLabel { background: transparent; border: none; }")
                                 .arg(i == 0 ? c.badge.name() : c.carte.name(),
                                      i == 0 ? c.accent.name() : c.bordure.name()));
        auto *h = new QHBoxLayout(ligne);
        h->setContentsMargins(14, 4, 16, 4);
        h->setSpacing(14);

        const QString medailles[] = {QStringLiteral("🥇"), QStringLiteral("🥈"), QStringLiteral("🥉")};
        auto *rang = new QLabel(i < 3 ? medailles[i] : QString("%1.").arg(i + 1), ligne);
        rang->setFixedWidth(34);
        rang->setAlignment(Qt::AlignCenter);
        rang->setStyleSheet(QString("font-size: %1px; font-weight: 700; color: %2;")
                                .arg(i < 3 ? 20 : 14).arg(c.discret.name()));
        auto *nom = new QLabel(QString("<b>%1</b>  <span style='color:%2'>%3</span>")
                                   .arg(s.titre.toHtmlEscaped(), c.discret.name(), s.id.toHtmlEscaped()), ligne);
        nom->setFixedWidth(330);
        auto *evals = new QLabel(tr("%1 évaluation(s)").arg(s.nbEvaluations), ligne);
        evals->setFixedWidth(110);
        evals->setStyleSheet(QString("color: %1; font-size: 12px;").arg(c.discret.name()));
        auto *barre = new QProgressBar(ligne);
        barre->setRange(0, 2000);
        barre->setValue(0);
        barre->setTextVisible(false);
        barre->setFixedHeight(8);
        barre->setProperty("cible", int(s.moyenne * 100));
        barre->setStyleSheet(QString("QProgressBar { background: %1; border: none; border-radius: 4px; }"
                                     "QProgressBar::chunk { background: %2; border-radius: 4px; }")
                                 .arg(c.fond.name(), c.accent.name()));
        auto *valeur = new QLabel(note1(s.moyenne), ligne);
        valeur->setFixedWidth(44);
        valeur->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valeur->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: 800;").arg(c.accent.name()));
        h->addWidget(rang);
        h->addWidget(nom);
        h->addWidget(evals);
        h->addWidget(barre, 1);
        h->addWidget(valeur);
        lListe->addWidget(ligne);
        lignes << ligne;
    }
    lListe->addStretch();
    defilement->setWidget(contenu);

    // ===== Boutons =====
    auto *fiche = new QPushButton(tr("📄  Fiche PDF du gagnant"), &dlg);
    auto *fermer = new QPushButton(tr("Fermer"), &dlg);
    fermer->setProperty("role", "primaire");
    for (QPushButton *b : {fiche, fermer}) {
        b->setMinimumSize(170, 42);
        b->setCursor(Qt::PointingHandCursor);
    }
    const QString idGagnant = gagnant.id;
    connect(fiche, &QPushButton::clicked, &dlg, [this, idGagnant] { exporterFichePdf(idGagnant); });
    connect(fermer, &QPushButton::clicked, &dlg, &QDialog::accept);
    auto *ligneBoutons = new QHBoxLayout;
    ligneBoutons->addWidget(fiche);
    ligneBoutons->addStretch();
    ligneBoutons->addWidget(fermer);

    auto *racine = new QVBoxLayout(&dlg);
    racine->setContentsMargins(30, 24, 30, 22);
    racine->setSpacing(12);
    racine->addWidget(titre);
    racine->addWidget(sousTitre);
    racine->addWidget(carte);
    racine->addWidget(titreClassement);
    racine->addWidget(defilement, 1);
    racine->addLayout(ligneBoutons);

    // ===== Confettis : un calque transparent par dessus toute la fenetre =====
    auto *confettis = new QWidget(&dlg);
    confettis->setObjectName("confettis");
    confettis->setAttribute(Qt::WA_TransparentForMouseEvents);
    confettis->setGeometry(0, 0, 1070, 650);
    confettis->setProperty("t", 0.0);
    confettis->installEventFilter(this);   // dessine par dessinerConfettis()
    confettis->raise();

    // ===== Animations =====
    // 1. Le trophee "pop" : de minuscule a tres grand, avec un rebond
    auto *pop = new QVariantAnimation(&dlg);
    pop->setDuration(1000);
    pop->setStartValue(1);
    pop->setEndValue(120);
    pop->setEasingCurve(QEasingCurve::OutElastic);
    connect(pop, &QVariantAnimation::valueChanged, trophee, [trophee](const QVariant &v) {
        trophee->setStyleSheet(QString("font-size: %1px;").arg(qMax(1, v.toInt())));
    });

    // 2. Halo autour de la carte du gagnant
    auto *halo = new QGraphicsDropShadowEffect(carte);
    halo->setOffset(0, 0);
    halo->setColor(c.accent);
    halo->setBlurRadius(0);
    carte->setGraphicsEffect(halo);
    auto *lueur = new QPropertyAnimation(halo, "blurRadius", &dlg);
    lueur->setDuration(1600);
    lueur->setStartValue(0.0);
    lueur->setKeyValueAt(0.4, 55.0);
    lueur->setEndValue(22.0);

    // 3. La moyenne defile de 0 a sa valeur
    auto *compte = new QVariantAnimation(&dlg);
    compte->setDuration(1500);
    compte->setStartValue(0.0);
    compte->setEndValue(gagnant.moyenne);
    compte->setEasingCurve(QEasingCurve::OutCubic);
    connect(compte, &QVariantAnimation::valueChanged, labelNote, [labelNote](const QVariant &v) {
        labelNote->setText(note1(v.toDouble()));
    });

    // 4. Les lignes du classement apparaissent l'une apres l'autre et leur barre se remplit
    for (int i = 0; i < lignes.size(); ++i) {
        QFrame *ligne = lignes[i];
        auto *effet = new QGraphicsOpacityEffect(ligne);
        effet->setOpacity(0.0);
        ligne->setGraphicsEffect(effet);
        auto *fondu = new QPropertyAnimation(effet, "opacity");
        fondu->setDuration(350);
        fondu->setStartValue(0.0);
        fondu->setEndValue(1.0);
        auto *barre = ligne->findChild<QProgressBar *>();
        auto *remplir = new QPropertyAnimation(barre, "value");
        remplir->setDuration(700);
        remplir->setStartValue(0);
        remplir->setEndValue(barre->property("cible").toInt());
        remplir->setEasingCurve(QEasingCurve::OutCubic);
        auto *groupe = new QSequentialAnimationGroup(ligne);
        groupe->addPause(700 + 110 * qMin(i, 12));
        groupe->addAnimation(fondu);
        groupe->addAnimation(remplir);
        connect(groupe, &QAbstractAnimation::finished, ligne, [ligne] { ligne->setGraphicsEffect(nullptr); });
        groupe->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // 5. Pluie de confettis
    auto *pluie = new QVariantAnimation(&dlg);
    pluie->setDuration(4200);
    pluie->setStartValue(0.0);
    pluie->setEndValue(1.0);
    connect(pluie, &QVariantAnimation::valueChanged, confettis, [confettis](const QVariant &v) {
        confettis->setProperty("t", v.toReal());
        confettis->update();
    });

    // Tout demarre juste apres l'ouverture de la fenetre
    QTimer::singleShot(180, &dlg, [pop, lueur, compte, pluie] {
        pop->start();
        lueur->start();
        compte->start();
        pluie->start();
    });

    notifier(Notification::Succes, tr("Projet gagnant : %1").arg(gagnant.titre),
             tr("%1 avec une moyenne de %2 / 20 (%3 évaluation(s)).")
                 .arg(gagnant.id, note1(gagnant.moyenne)).arg(gagnant.nbEvaluations),
             gagnant.id, false);

    animerApparition(&dlg);
    dlg.exec();
}

// Confettis : leur position ne depend que du temps t (0 a 1) et de leur numero, grace a
// un "hasard" calcule (toujours le meme pour un numero donne) : rien a memoriser entre deux images.
void MainWindow::dessinerConfettis(QWidget *zone)
{
    const qreal t = zone->property("t").toReal();
    if (t <= 0.0 || t >= 1.0)
        return;

    auto hasard = [](int i, int k) {
        const double v = std::sin(i * 12.9898 + k * 78.233) * 43758.5453;
        return v - std::floor(v);   // entre 0 et 1
    };
    const QColor teintes[] = {QColor("#18D9EF"), QColor("#A879F5"), QColor("#F59E0B"),
                              QColor("#22C55E"), QColor("#F472B6"), QColor("#FFFFFF")};

    QPainter p(zone);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setOpacity(t < 0.75 ? 1.0 : (1.0 - t) / 0.25);   // disparition en douceur a la fin

    const qreal W = zone->width();
    const qreal H = zone->height();
    for (int i = 0; i < 150; ++i) {
        const qreal depart = hasard(i, 1) * 0.35;   // chaque confetti part a son moment
        if (t < depart)
            continue;
        const qreal u = (t - depart) / (1.0 - depart);
        const qreal x = hasard(i, 2) * W + std::sin(u * 6.283 * (1 + hasard(i, 3) * 2) + i) * 28;
        const qreal y = -20 + u * (H + 60) * (0.75 + hasard(i, 4) * 0.5);
        const qreal angle = u * 720 * (hasard(i, 5) - 0.5) * 2;
        const qreal largeur = 6 + hasard(i, 6) * 7;
        const qreal hauteur = 3 + hasard(i, 7) * 4;

        p.save();
        p.translate(x, y);
        p.rotate(angle);
        p.setBrush(teintes[i % 6]);
        p.drawRoundedRect(QRectF(-largeur / 2, -hauteur / 2, largeur, hauteur), 1.5, 1.5);
        p.restore();
    }
}

// ==========================================================
// ===== Export PDF : fiches des projets =====
// ==========================================================
// Meme design que les PDF du module Equipes : fond bleu nuit, en tete DEVENA, cartes.
// exporterFichePdf()  : la fiche d'un seul projet (bouton 📄 de chaque ligne)
// exporterPdf()       : la liste des projets affiches, sans les fiches
//                       (dans l'ordre du tri et avec le filtre de recherche en cours)

void MainWindow::exporterFichePdf(const QString &id)
{
    Projet p;
    QString erreur;
    if (!Projet::charger(id, p, &erreur)) {
        QMessageBox::warning(this, tr("Fiche PDF"), erreur);
        return;
    }
    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString chemin = QFileDialog::getSaveFileName(this, tr("Exporter la fiche du projet"),
                                                  dossier + "/Devena_Projet_" + p.id() + ".pdf",
                                                  tr("Fichiers PDF (*.pdf)"));
    if (chemin.isEmpty())
        return;
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive))
        chemin += ".pdf";

    if (ecrirePdfProjets({p}, chemin, false)) {
        notifier(Notification::Succes, tr("Fiche PDF exportée"),
                 tr("%1 : fiche du projet %2.").arg(QFileInfo(chemin).fileName(), p.id()),
                 p.id(), true, tr("Ouvrir"),
                 [chemin] { QDesktopServices::openUrl(QUrl::fromLocalFile(chemin)); });
    } else {
        notifier(Notification::Erreur, tr("Export impossible"),
                 tr("Le fichier %1 n'a pas pu être créé.").arg(chemin), p.id());
    }
}

void MainWindow::exporterPdf()
{
    if (m_projetsAffiches.isEmpty()) {
        notifier(Notification::Info, tr("Rien à exporter"), tr("Le tableau ne contient aucun projet."));
        return;
    }
    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString chemin = QFileDialog::getSaveFileName(this, tr("Exporter les projets en PDF"),
                                                  dossier + "/Devena_Projets.pdf",
                                                  tr("Fichiers PDF (*.pdf)"));
    if (chemin.isEmpty())
        return;
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive))
        chemin += ".pdf";

    if (ecrirePdfProjets(m_projetsAffiches, chemin, true)) {
        notifier(Notification::Succes, tr("PDF exporté"),
                 tr("%1 : liste de %2 projet(s).").arg(QFileInfo(chemin).fileName())
                     .arg(m_projetsAffiches.size()),
                 QString(), true, tr("Ouvrir"),
                 [chemin] { QDesktopServices::openUrl(QUrl::fromLocalFile(chemin)); });
    } else {
        notifier(Notification::Erreur, tr("Export impossible"),
                 tr("Le fichier %1 n'a pas pu être créé.").arg(chemin));
    }
}

bool MainWindow::ecrirePdfProjets(const QList<Projet> &projets, const QString &chemin, bool listeSeule)
{
    if (projets.isEmpty())
        return false;

    QPdfWriter pdf(chemin);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Portrait);
    pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
    pdf.setResolution(96);
    pdf.setTitle(projets.size() == 1 ? tr("Fiche projet %1").arg(projets.first().id())
                                     : tr("Projets Devena"));
    pdf.setCreator("Devena");

    QPainter p(&pdf);
    if (!p.isActive())
        return false;
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    // Couleurs du design Devena (les memes que le module Equipes)
    const QColor marine("#071421");
    const QColor carte("#102538");
    const QColor carteSombre("#0B1C2B");
    const QColor turquoise("#18D9EF");
    const QColor blanc("#FFFFFF");
    const QColor gris("#8EA0B7");
    const QColor trait("#29445A");
    const QColor rouge("#F87171");
    const QColor orange("#F59E0B");
    const QColor vert("#22C55E");

    const int W = p.viewport().width();
    const int H = p.viewport().height();
    const int marge = 45;
    const QDate aujourdHui = QDate::currentDate();

    // Notes des projets (une seule requete pour tout le document)
    QHash<QString, ScoreProjet> notes;
    for (const ScoreProjet &s : classementProjets())
        notes.insert(s.id, s);

    int page = 0;
    // listeSeule : page(s) de liste uniquement ; sinon : une fiche par projet
    const int nbPages = listeSeule ? 1 + (projets.size() - 1) / 18 : projets.size();

    auto police = [&p](int taille, int graisse = QFont::Normal) {
        p.setFont(QFont("Segoe UI", taille, graisse));
    };

    // En tete commun : DEVENA, sous titre, titre de la page, trait turquoise
    auto entete = [&](const QString &titrePage) {
        p.fillRect(0, 0, W, H, marine);
        p.setPen(turquoise);
        police(27, QFont::Bold);
        p.drawText(QRect(marge, 45, W - 2 * marge, 55), Qt::AlignLeft | Qt::AlignVCenter, "DEVENA");
        p.setPen(gris);
        police(9);
        p.drawText(QRect(marge, 100, W - 2 * marge, 25), Qt::AlignLeft, "SMART HACKATHON MANAGEMENT");
        p.drawText(QRect(marge, 100, W - 2 * marge, 25), Qt::AlignRight,
                   tr("Exporté le %1").arg(aujourdHui.toString("dd/MM/yyyy")));
        p.setPen(turquoise);
        police(11, QFont::Bold);
        p.drawText(QRect(marge, 145, W - 2 * marge, 35), Qt::AlignCenter, titrePage);
        p.setPen(QPen(turquoise, 2));
        p.drawLine(marge, 190, W - marge, 190);
    };

    // Pied de page commun
    auto pied = [&]() {
        ++page;
        const int y = H - 65;
        p.setPen(QPen(trait, 1));
        p.drawLine(marge, y, W - marge, y);
        p.setPen(turquoise);
        police(10, QFont::Bold);
        p.drawText(QRect(marge, y + 10, W - 2 * marge, 20), Qt::AlignCenter, "DEVENA");
        p.setPen(gris);
        police(8);
        p.drawText(QRect(marge, y + 30, W - 2 * marge, 18), Qt::AlignCenter, "HACK TODAY / BUILD TOMORROW");
        p.drawText(QRect(marge, y + 30, W - 2 * marge, 18), Qt::AlignRight,
                   tr("Page %1 / %2").arg(page).arg(nbPages));
    };

    auto nouvellePage = [&]() {
        if (page > 0)
            pdf.newPage();
    };

    // ===== Page(s) de liste =====
    if (listeSeule) {
        const QStringList entetes = {tr("ID"), tr("Titre"), tr("Thématique"), tr("Statut"),
                                     tr("Création"), tr("Limite"), tr("Note")};
        const QList<int> largeurs = {60, 210, 105, 100, 85, 85, 59};   // total = W - 2 * marge
        const int hauteurLigne = 38;
        int i = 0;
        while (i < projets.size()) {
            nouvellePage();
            entete(tr("LISTE DES PROJETS"));
            p.setPen(gris);
            police(9);
            p.drawText(QRect(marge, 200, W - 2 * marge, 22), Qt::AlignLeft,
                       tr("Nombre de projets : %1").arg(projets.size()));

            int y = 230;
            p.setPen(Qt::NoPen);
            p.setBrush(carte);
            p.drawRoundedRect(QRect(marge, y, W - 2 * marge, 42), 6, 6);
            p.setPen(turquoise);
            police(9, QFont::Bold);
            int x = marge;
            for (int c = 0; c < entetes.size(); ++c) {
                p.drawText(QRect(x + 6, y, largeurs[c] - 12, 42), Qt::AlignVCenter | Qt::AlignLeft, entetes[c]);
                x += largeurs[c];
            }
            y += 42;

            for (int n = 0; n < 18 && i < projets.size(); ++n, ++i) {
                const Projet &pr = projets[i];
                p.setPen(Qt::NoPen);
                p.setBrush(i % 2 == 0 ? carte : carteSombre);
                p.drawRect(marge, y, W - 2 * marge, hauteurLigne);

                const QStringList valeurs = {
                    pr.id(), pr.titre(), ouTiret(pr.thematique()), ouTiret(pr.statut()),
                    dateOuTiret(pr.dateCreation()), dateOuTiret(pr.dateLimite()),
                    notes.contains(pr.id()) ? note1(notes.value(pr.id()).moyenne) : QStringLiteral("-")};
                x = marge;
                for (int c = 0; c < valeurs.size(); ++c) {
                    p.setPen(c == 0 || c == 6 ? turquoise : blanc);
                    police(8, c == 0 || c == 6 ? QFont::Bold : QFont::Normal);
                    p.drawText(QRect(x + 6, y, largeurs[c] - 12, hauteurLigne), Qt::AlignVCenter | Qt::AlignLeft,
                               p.fontMetrics().elidedText(valeurs[c], Qt::ElideRight, largeurs[c] - 12));
                    x += largeurs[c];
                }
                p.setPen(QPen(trait, 1));
                p.drawLine(marge, y + hauteurLigne, W - marge, y + hauteurLigne);
                y += hauteurLigne;
            }
            pied();
        }
    }

    // ===== Une fiche par projet =====
    for (const Projet &pr : projets) {
        if (listeSeule)
            break;
        nouvellePage();
        entete(tr("FICHE PROJET"));

        // Titre et identifiant
        p.setPen(blanc);
        police(21, QFont::Bold);
        p.drawText(QRect(marge, 215, W - 2 * marge, 45), Qt::AlignCenter,
                   p.fontMetrics().elidedText(pr.titre(), Qt::ElideRight, W - 2 * marge));
        p.setPen(turquoise);
        police(11, QFont::DemiBold);
        p.drawText(QRect(marge, 262, W - 2 * marge, 25), Qt::AlignCenter, tr("ID : %1").arg(pr.id()));

        // Echeance en clair
        QString echeance;
        QColor couleurEcheance = gris;
        if (pr.statut().startsWith("Termin") || pr.statut().startsWith("Annul")) {
            echeance = tr("Projet clôturé");
        } else if (pr.dateLimite().isValid()) {
            const int jours = aujourdHui.daysTo(pr.dateLimite());
            if (jours < 0) {
                echeance = tr("Dépassée de %1 jour(s)").arg(-jours);
                couleurEcheance = rouge;
            } else if (jours == 0) {
                echeance = tr("Aujourd'hui");
                couleurEcheance = orange;
            } else {
                echeance = tr("Dans %1 jour(s)").arg(jours);
                couleurEcheance = jours <= 7 ? orange : vert;
            }
        }

        // 4 cartes : thematique, statut, date de creation, date limite
        const int ecart = 14;
        const int largeurCarte = (W - 2 * marge - ecart) / 2;
        const int hauteurCarte = 88;
        const QStringList titres = {tr("THÉMATIQUE"), tr("STATUT"), tr("DATE DE CRÉATION"), tr("DATE LIMITE")};
        const QStringList valeurs = {ouTiret(pr.thematique()), ouTiret(pr.statut()),
                                     dateOuTiret(pr.dateCreation()), dateOuTiret(pr.dateLimite())};
        for (int i = 0; i < 4; ++i) {
            const int x = marge + (i % 2) * (largeurCarte + ecart);
            const int y = 310 + (i / 2) * (hauteurCarte + ecart);
            p.setPen(Qt::NoPen);
            p.setBrush(carte);
            p.drawRoundedRect(QRect(x, y, largeurCarte, hauteurCarte), 10, 10);
            p.setPen(gris);
            police(9, QFont::Bold);
            p.drawText(QRect(x + 15, y + 12, largeurCarte - 30, 22), Qt::AlignLeft, titres[i]);
            p.setPen(blanc);
            police(18, QFont::Bold);
            p.drawText(QRect(x + 15, y + 36, largeurCarte - 30, 35), Qt::AlignLeft | Qt::AlignVCenter, valeurs[i]);
            if (i == 3 && !echeance.isEmpty()) {
                p.setPen(couleurEcheance);
                police(9, QFont::Bold);
                p.drawText(QRect(x + 15, y + 36, largeurCarte - 30, 35), Qt::AlignRight | Qt::AlignVCenter, echeance);
            }
        }

        // Informations : description, equipes, evaluation
        int y = 535;
        p.setPen(turquoise);
        police(12, QFont::Bold);
        p.drawText(QRect(marge, y, W - 2 * marge, 30), Qt::AlignLeft, tr("INFORMATIONS DU PROJET"));
        y += 40;

        const QStringList equipes = equipesDuProjet(pr.id());
        QString evaluation = tr("Pas encore évalué");
        if (notes.contains(pr.id())) {
            const ScoreProjet s = notes.value(pr.id());
            evaluation = tr("Moyenne %1 / 20  ·  %2 évaluation(s)  ·  meilleure note %3")
                             .arg(note1(s.moyenne)).arg(s.nbEvaluations).arg(note1(s.meilleure));
        }
        const QList<QPair<QString, QString>> blocs = {
            {tr("Description"), ouTiret(pr.description())},
            {tr("Équipes"), equipes.isEmpty() ? tr("Aucune équipe rattachée") : equipes.join("  ·  ")},
            {tr("Évaluation"), evaluation}};

        for (const auto &bloc : blocs) {
            police(10);
            const QRect zoneTexte(60, 0, W - 120, 200);
            const int hauteurTexte = qMin(150, p.boundingRect(zoneTexte, Qt::TextWordWrap, bloc.second).height());
            const int hauteurBloc = 34 + hauteurTexte + 10;
            if (y + hauteurBloc > H - 80)
                break;   // la page est pleine : on n'ecrit pas par dessus le pied de page
            p.setPen(Qt::NoPen);
            p.setBrush(carte);
            p.drawRoundedRect(QRect(marge, y, W - 2 * marge, hauteurBloc), 8, 8);
            p.setPen(gris);
            police(9, QFont::Bold);
            p.drawText(QRect(60, y + 8, W - 120, 20), Qt::AlignLeft, bloc.first);
            p.setPen(blanc);
            police(10);
            p.drawText(QRect(60, y + 30, W - 120, hauteurTexte), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop,
                       bloc.second);
            y += hauteurBloc + 10;
        }

        pied();
    }

    return p.end();
}

// ==========================================================
// ===== Statistiques : organigramme de 4 cartes =====
// ==========================================================
// 1. PROJETS : total, projets evalues / non evalues, moyenne generale
// 2. ECHEANCES : projets non termines ranges par date limite
// 3. THEMATIQUES PRINCIPALES : les 5 plus frequentes
// 4. REPARTITION PAR STATUT : donut dessine avec QPainter + legende
// Les cartes ne sont reconstruites que si les donnees changent (pas a chaque lettre
// tapee dans la recherche) ; a chaque reconstruction, barres et donut se remplissent.

// Couleur de chaque statut (donut, legende)
static QColor couleurStatut(const QString &statut)
{
    if (statut.startsWith("En cours"))
        return QColor("#18D9EF");
    if (statut.startsWith("En attente"))
        return QColor("#F59E0B");
    if (statut.startsWith("Termin"))
        return QColor("#22C55E");
    if (statut.startsWith("Annul"))
        return QColor("#A879F5");
    return QColor("#64748B");
}

static bool estFini(const Projet &p)
{
    return p.statut().startsWith("Termin") || p.statut().startsWith("Annul");
}

void MainWindow::mettreAJourStatistiques(bool forcer)
{
    QString erreur;
    const QList<Projet> tous = Projet::lister(QString(), &erreur);
    const QList<ScoreProjet> scores = classementProjets();

    // Signature des donnees : si rien n'a change, on garde les cartes telles quelles
    QStringList morceaux;
    for (const Projet &p : tous)
        morceaux << p.id() + ";" + p.statut() + ";" + p.thematique() + ";"
                        + p.dateLimite().toString(Qt::ISODate);
    for (const ScoreProjet &s : scores)
        morceaux << s.id + ";" + QString::number(s.moyenne);
    morceaux << QDate::currentDate().toString(Qt::ISODate);
    const QString signature = morceaux.join('|');
    if (!forcer && signature == m_signatureStats)
        return;
    m_signatureStats = signature;

    // ===== Calculs =====
    const QDate aujourdHui = QDate::currentDate();
    const int total = tous.size();

    QSet<QString> idsEvalues;
    double sommeMoyennes = 0;
    for (const ScoreProjet &s : scores) {
        idsEvalues.insert(s.id);
        sommeMoyennes += s.moyenne;
    }
    int nbEvalues = 0;
    for (const Projet &p : tous)
        if (idsEvalues.contains(p.id()))
            ++nbEvalues;

    // Echeances des projets encore en cours de realisation
    int enRetard = 0, semaine = 0, mois = 0, plusTard = 0, sansDate = 0;
    for (const Projet &p : tous) {
        if (estFini(p))
            continue;
        if (!p.dateLimite().isValid()) {
            ++sansDate;
            continue;
        }
        const int jours = aujourdHui.daysTo(p.dateLimite());
        if (jours < 0)
            ++enRetard;
        else if (jours <= 7)
            ++semaine;
        else if (jours <= 30)
            ++mois;
        else
            ++plusTard;
    }

    // Thematiques, de la plus frequente a la moins frequente
    QMap<QString, int> parThematique;
    for (const Projet &p : tous)
        if (!p.thematique().trimmed().isEmpty())
            parThematique[p.thematique().trimmed()]++;
    QList<QPair<QString, int>> thematiques;
    for (auto it = parThematique.cbegin(); it != parThematique.cend(); ++it)
        thematiques.append({it.key(), it.value()});
    std::stable_sort(thematiques.begin(), thematiques.end(),
                     [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                         return a.second > b.second;
                     });

    // Statuts, dans l'ordre habituel puis les autres
    QMap<QString, int> parStatut;
    for (const Projet &p : tous)
        parStatut[p.statut().trimmed().isEmpty() ? tr("Sans statut") : p.statut().trimmed()]++;
    m_partsDonut.clear();
    for (const QString &s : {QString("En cours"), QString("En attente"),
                             QString("Terminé"), QString("Annulé")}) {
        if (parStatut.contains(s)) {
            m_partsDonut.append({s, parStatut.value(s)});
            parStatut.remove(s);
        }
    }
    for (auto it = parStatut.cbegin(); it != parStatut.cend(); ++it)
        m_partsDonut.append({it.key(), it.value()});

    // ===== Construction des cartes =====
    QWidget *zone = ui->organigramme;
    zone->setFixedHeight(214);   // hauteur des cartes : pas d'espace vide au dessus
    qDeleteAll(zone->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    delete zone->layout();

    const Couleurs c = couleurs();
    auto *principal = new QHBoxLayout(zone);
    principal->setContentsMargins(0, 0, 0, 0);
    principal->setSpacing(12);

    auto creerCarte = [&](const QString &titre, int largeur) {
        auto *carte = new QFrame(zone);
        carte->setObjectName("carteOrga");
        carte->setFixedHeight(214);
        carte->setMinimumWidth(150);   // sur un petit ecran la carte se resserre
        auto *l = new QVBoxLayout(carte);
        l->setSizeConstraint(QLayout::SetNoConstraint);
        l->setContentsMargins(14, 12, 14, 12);
        l->setSpacing(5);
        auto *t = new QLabel(titre, carte);
        t->setObjectName("titreOrga");
        t->setWordWrap(true);
        l->addWidget(t);
        auto *trait = new QFrame(carte);
        trait->setObjectName("traitOrga");
        trait->setFixedHeight(1);
        l->addWidget(trait);
        principal->addWidget(carte, largeur);
        return l;
    };

    // Ligne "libelle  barre  valeur" ; la barre se remplit dans animerBarres()
    auto ajouterBarre = [&](QVBoxLayout *l, const QString &nom, int valeur, int maximum,
                            const QColor &couleur) {
        auto *ligne = new QWidget;
        auto *h = new QHBoxLayout(ligne);
        h->setContentsMargins(0, 1, 0, 1);
        h->setSpacing(8);
        auto *libelle = new QLabel(nom, ligne);
        libelle->setObjectName("libelleOrga");
        libelle->setFixedWidth(84);
        auto *barre = new QProgressBar(ligne);
        barre->setObjectName("barreOrga");
        barre->setRange(0, qMax(1, maximum) * 100);
        barre->setValue(0);
        barre->setTextVisible(false);
        barre->setFixedHeight(7);
        barre->setMinimumWidth(30);   // les cartes peuvent se resserrer sur un petit ecran
        barre->setProperty("cible", valeur * 100);
        barre->setStyleSheet(QString("QProgressBar { background: %1; border: none; border-radius: 3px; }"
                                     "QProgressBar::chunk { background: %2; border-radius: 3px; }")
                                 .arg(c.badge.name(), couleur.name()));
        auto *nombre = new QLabel(QString::number(valeur), ligne);
        nombre->setObjectName("valeurOrga");
        nombre->setFixedWidth(26);
        nombre->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        h->addWidget(libelle);
        h->addWidget(barre, 1);
        h->addWidget(nombre);
        l->addWidget(ligne);
    };

    // Carte 1 : PROJETS
    QVBoxLayout *l1 = creerCarte(tr("PROJETS"), 10);
    auto *grandTotal = new QLabel(QStringLiteral("0"));
    grandTotal->setObjectName("grandTotal");
    grandTotal->setAlignment(Qt::AlignCenter);
    l1->addWidget(grandTotal);
    ajouterBarre(l1, tr("Évalués"), nbEvalues, total, c.accent);
    ajouterBarre(l1, tr("Non évalués"), total - nbEvalues, total, QColor("#64748B"));
    auto *moyenne = new QLabel(scores.isEmpty()
                                   ? tr("Aucune note pour l'instant")
                                   : tr("Moyenne générale : %1 / 20")
                                         .arg(QLocale(QLocale::French).toString(sommeMoyennes / scores.size(), 'f', 1)));
    moyenne->setObjectName("detailOrga");
    moyenne->setAlignment(Qt::AlignCenter);
    l1->addWidget(moyenne);
    l1->addStretch();

    // Carte 2 : ECHEANCES
    QVBoxLayout *l2 = creerCarte(tr("ÉCHÉANCES (NON TERMINÉS)"), 10);
    const int maxEcheance = qMax(1, qMax(qMax(enRetard, semaine), qMax(qMax(mois, plusTard), sansDate)));
    ajouterBarre(l2, tr("En retard"), enRetard, maxEcheance, c.danger);
    ajouterBarre(l2, tr("≤ 7 jours"), semaine, maxEcheance, QColor("#F59E0B"));
    ajouterBarre(l2, tr("8 à 30 jours"), mois, maxEcheance, c.accent);
    ajouterBarre(l2, tr("Plus de 30 j"), plusTard, maxEcheance, c.accent);
    ajouterBarre(l2, tr("Sans date"), sansDate, maxEcheance, QColor("#64748B"));
    l2->addStretch();

    // Carte 3 : THEMATIQUES PRINCIPALES
    QVBoxLayout *l3 = creerCarte(tr("THÉMATIQUES PRINCIPALES"), 14);
    const int maxThematique = thematiques.isEmpty() ? 1 : thematiques.first().second;
    for (int i = 0; i < qMin(5, int(thematiques.size())); ++i)
        ajouterBarre(l3, thematiques[i].first, thematiques[i].second, maxThematique, c.accent);
    if (thematiques.size() > 5) {
        auto *autres = new QLabel(tr("+ %1 autres thématiques").arg(thematiques.size() - 5));
        autres->setObjectName("detailOrga");
        l3->addWidget(autres);
    }
    if (thematiques.isEmpty()) {
        auto *vide = new QLabel(tr("Aucune thématique disponible"));
        vide->setObjectName("detailOrga");
        l3->addWidget(vide);
    }
    l3->addStretch();

    // Carte 4 : REPARTITION PAR STATUT (donut + legende)
    QVBoxLayout *l4 = creerCarte(tr("RÉPARTITION PAR STATUT"), 10);
    auto *ligneDonut = new QHBoxLayout;
    ligneDonut->setSpacing(10);
    auto *donut = new QWidget;
    donut->setObjectName("donutStatut");
    donut->setFixedSize(118, 118);
    donut->installEventFilter(this);   // dessine par dessinerDonut()
    ligneDonut->addWidget(donut, 0, Qt::AlignVCenter);
    auto *legende = new QVBoxLayout;
    legende->setSpacing(6);
    legende->addStretch();
    for (const auto &part : std::as_const(m_partsDonut)) {
        auto *ligne = new QLabel(QString("● %1 : %2").arg(part.first).arg(part.second));
        ligne->setStyleSheet(QString("color: %1; font-size: 12px;").arg(couleurStatut(part.first).name()));
        legende->addWidget(ligne);
    }
    if (m_partsDonut.isEmpty()) {
        auto *vide = new QLabel(tr("Aucun projet"));
        vide->setObjectName("detailOrga");
        legende->addWidget(vide);
    }
    legende->addStretch();
    ligneDonut->addLayout(legende, 1);
    l4->addLayout(ligneDonut, 1);

    animerCompteur(grandTotal, total);
    animerBarres();
}

// Donut : un arc par statut, proportionnel au nombre de projets ; le total au centre.
// m_progressionDonut (0 a 1) limite la part dessinee pendant l'animation.
void MainWindow::dessinerDonut(QWidget *zone)
{
    const Couleurs c = couleurs();
    QPainter p(zone);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF cercle = QRectF(zone->rect()).adjusted(12, 12, -12, -12);
    QPen trait;
    trait.setWidthF(13);
    trait.setCapStyle(Qt::FlatCap);

    // Anneau de fond
    trait.setColor(c.badge);
    p.setPen(trait);
    p.drawArc(cercle, 0, 360 * 16);

    int total = 0;
    for (const auto &part : std::as_const(m_partsDonut))
        total += part.second;

    if (total > 0) {
        qreal depart = 90;                              // en haut
        qreal restant = 360 * m_progressionDonut;       // angle deja "deroule"
        for (const auto &part : std::as_const(m_partsDonut)) {
            const qreal angle = 360.0 * part.second / total;
            const qreal dessine = qMin(angle, restant);
            if (dessine <= 0)
                break;
            trait.setColor(couleurStatut(part.first));
            p.setPen(trait);
            p.drawArc(cercle, int(depart * 16), int(-dessine * 16));   // sens horaire
            depart -= angle;
            restant -= dessine;
        }
    }

    QFont police = zone->font();
    police.setPixelSize(22);
    police.setBold(true);
    p.setFont(police);
    p.setPen(c.texte);
    p.drawText(zone->rect(), Qt::AlignCenter, QString::number(total));
}

// Les barres de l'organigramme se remplissent l'une apres l'autre, le donut se deroule
void MainWindow::animerBarres()
{
    const QList<QProgressBar *> barres =
        ui->organigramme->findChildren<QProgressBar *>("barreOrga");
    for (int i = 0; i < barres.size(); ++i) {
        QProgressBar *barre = barres[i];
        auto *remplissage = new QPropertyAnimation(barre, "value");
        remplissage->setDuration(900);
        remplissage->setStartValue(0);
        remplissage->setEndValue(barre->property("cible").toInt());
        remplissage->setEasingCurve(QEasingCurve::OutCubic);
        auto *groupe = new QSequentialAnimationGroup(barre);
        groupe->addPause(60 * i);
        groupe->addAnimation(remplissage);
        groupe->start(QAbstractAnimation::DeleteWhenStopped);
    }

    QWidget *donut = ui->organigramme->findChild<QWidget *>("donutStatut");
    if (!donut)
        return;
    auto *deroule = new QVariantAnimation(donut);
    deroule->setDuration(1100);
    deroule->setStartValue(0.0);
    deroule->setEndValue(1.0);
    deroule->setEasingCurve(QEasingCurve::InOutCubic);
    connect(deroule, &QVariantAnimation::valueChanged, donut, [this, donut](const QVariant &v) {
        m_progressionDonut = v.toReal();
        donut->update();
    });
    m_progressionDonut = 0.0;
    deroule->start(QAbstractAnimation::DeleteWhenStopped);
}

// ==========================================================
// ===== Vue Kanban : glisser deposer pour changer le statut =====
// ==========================================================
// Deux vues des memes projets (meme recherche, meme tri) : le tableau et le Kanban.
// Le Kanban a une colonne par statut. On attrape une carte a la souris, on la depose
// dans une autre colonne : changerStatut() appelle Projet::modifier() (UPDATE dans Oracle),
// l'action peut etre annulee (bouton de la notification ou Ctrl+Z).
// Le glisser deposer est fait a la main avec QDrag / QMimeData :
//   carte  : MouseButtonPress puis MouseMove -> QDrag (image de la carte)
//   colonne: DragEnter (la colonne s'allume), DragLeave, Drop (changement de statut)

static const char *MIME_PROJET = "application/x-devena-projet";

void MainWindow::construireKanban()
{
    const Couleurs c = couleurs();

    // ===== Boutons Tableau / Kanban, sur la ligne du titre "Tableau des projets" =====
    const int position = ui->layoutContenu->indexOf(ui->labelTableau);
    ui->layoutContenu->removeWidget(ui->labelTableau);
    auto *ligneTitre = new QHBoxLayout;
    ligneTitre->setSpacing(6);
    ligneTitre->addWidget(ui->labelTableau);
    ligneTitre->addStretch();
    auto *aide = new QLabel(tr("Glissez une carte vers une autre colonne pour changer son statut"), this);
    aide->setObjectName("aideKanban");
    aide->setProperty("role", "sousTitre");
    ligneTitre->addWidget(aide);
    ligneTitre->addSpacing(10);
    auto *boutonTableau = new QPushButton(tr("☰  Tableau"), this);
    auto *boutonKanban = new QPushButton(tr("▦  Kanban"), this);
    auto *groupe = new QButtonGroup(this);
    for (QPushButton *b : {boutonTableau, boutonKanban}) {
        b->setCheckable(true);
        b->setProperty("role", "vue");
        b->setCursor(Qt::PointingHandCursor);
        groupe->addButton(b);
        ligneTitre->addWidget(b);
    }
    boutonTableau->setObjectName("boutonVueTableau");
    boutonKanban->setObjectName("boutonVueKanban");
    ui->layoutContenu->insertLayout(position, ligneTitre);

    // ===== Les deux vues dans un QStackedWidget, a la place du tableau =====
    const int positionTableau = ui->layoutContenu->indexOf(ui->tableProjets);
    ui->layoutContenu->removeWidget(ui->tableProjets);
    m_vues = new QStackedWidget(this);
    m_vues->addWidget(ui->tableProjets);

    m_kanban = new QWidget;
    m_kanban->setMinimumHeight(360);
    auto *lKanban = new QHBoxLayout(m_kanban);
    lKanban->setContentsMargins(0, 0, 0, 0);
    lKanban->setSpacing(12);

    for (const QString &statut : STATUTS) {
        auto *colonne = new QFrame(m_kanban);
        colonne->setObjectName("colonneKanban");
        colonne->setProperty("statut", statut);
        colonne->setProperty("survol", false);
        colonne->setAcceptDrops(true);
        colonne->installEventFilter(this);
        colonne->setMinimumWidth(150);

        auto *lColonne = new QVBoxLayout(colonne);
        lColonne->setContentsMargins(10, 12, 10, 10);
        lColonne->setSpacing(8);

        // En tete : pastille de couleur, nom du statut, nombre de projets
        auto *entete = new QHBoxLayout;
        auto *nom = new QLabel(QString("●  %1").arg(statut), colonne);
        nom->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: bold;")
                               .arg(couleurStatut(statut).name()));
        auto *compte = new QLabel(QStringLiteral("0"), colonne);
        compte->setObjectName("compteKanban");
        compte->setAlignment(Qt::AlignCenter);
        compte->setMinimumWidth(28);
        entete->addWidget(nom);
        entete->addStretch();
        entete->addWidget(compte);
        lColonne->addLayout(entete);

        auto *trait = new QFrame(colonne);
        trait->setFixedHeight(2);
        trait->setStyleSheet(QString("background: %1; border: none; border-radius: 1px;")
                                 .arg(couleurStatut(statut).name()));
        lColonne->addWidget(trait);

        // Liste des cartes, avec defilement vertical
        auto *defilement = new QScrollArea(colonne);
        defilement->setWidgetResizable(true);
        defilement->setFrameShape(QFrame::NoFrame);
        defilement->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        auto *contenu = new QWidget;
        contenu->setObjectName("contenuKanban");
        // Selecteur par nom : sans lui, "transparent" s'appliquerait aussi a toutes les cartes
        contenu->setStyleSheet("#contenuKanban { background: transparent; }");
        auto *cartes = new QVBoxLayout(contenu);
        cartes->setContentsMargins(0, 2, 4, 2);
        cartes->setSpacing(8);
        cartes->addStretch();
        defilement->setWidget(contenu);
        lColonne->addWidget(defilement, 1);

        m_colonnesKanban << cartes;
        lKanban->addWidget(colonne, 1);
    }

    m_vues->addWidget(m_kanban);
    ui->layoutContenu->insertWidget(positionTableau, m_vues, 1);

    connect(boutonTableau, &QPushButton::clicked, this, [this] { afficherVue(false); });
    connect(boutonKanban, &QPushButton::clicked, this, [this] { afficherVue(true); });

    // La derniere vue choisie est retrouvee au prochain lancement
    afficherVue(QSettings("Devena", "Devena").value("projets/vueKanban", false).toBool(), false);
    Q_UNUSED(c);
}

void MainWindow::afficherVue(bool kanban, bool animer)
{
    if (!m_vues)
        return;
    m_vues->setCurrentWidget(kanban ? m_kanban : static_cast<QWidget *>(ui->tableProjets));
    ui->labelTableau->setText(kanban ? tr("Kanban des projets") : tr("Tableau des projets"));
    if (auto *b = findChild<QPushButton *>(kanban ? "boutonVueKanban" : "boutonVueTableau"))
        b->setChecked(true);
    if (auto *aide = findChild<QLabel *>("aideKanban"))
        aide->setVisible(kanban);
    QSettings("Devena", "Devena").setValue("projets/vueKanban", kanban);

    // Petite animation a chaque changement de vue
    if (!animer)
        return;
    animerPage(m_vues->currentWidget());
    if (!kanban)
        animerEntreeLignes();
}

// Une carte par projet affiche (meme filtre de recherche et meme ordre que le tableau)
void MainWindow::remplirKanban()
{
    if (m_colonnesKanban.isEmpty())
        return;
    const Couleurs c = couleurs();
    const QDate aujourdHui = QDate::currentDate();

    QHash<QString, double> notes;
    for (const ScoreProjet &s : classementProjets())
        notes.insert(s.id, s.moyenne);

    // Vide les colonnes (le dernier element de chaque colonne est l'espace extensible)
    for (QVBoxLayout *cartes : std::as_const(m_colonnesKanban)) {
        while (cartes->count() > 1) {
            QLayoutItem *element = cartes->takeAt(0);
            if (QWidget *w = element->widget()) {
                w->setObjectName(QString());   // l'ancienne carte n'est plus retrouvee par son nom
                w->hide();
                w->deleteLater();
            }
            delete element;
        }
    }

    QVector<int> nombres(STATUTS.size(), 0);
    for (const Projet &p : std::as_const(m_projetsAffiches)) {
        int colonne = STATUTS.indexOf(p.statut());
        if (colonne < 0)
            colonne = 0;   // statut vide ou inconnu : colonne "En attente"
        ++nombres[colonne];

        auto *carte = new QFrame;
        carte->setObjectName("carteKanban");
        carte->setProperty("idProjet", p.id());
        carte->setCursor(Qt::OpenHandCursor);
        carte->setToolTip(tr("Glisser pour changer le statut, double clic pour ouvrir"));
        carte->installEventFilter(this);

        auto *ligne = new QHBoxLayout(carte);
        ligne->setContentsMargins(0, 0, 10, 0);
        ligne->setSpacing(10);
        auto *bande = new QFrame(carte);   // bande de la couleur du statut, a gauche
        bande->setFixedWidth(4);
        bande->setStyleSheet(QString("background: %1; border: none; border-radius: 2px;")
                                 .arg(couleurStatut(STATUTS.value(colonne)).name()));
        ligne->addWidget(bande);

        auto *textes = new QVBoxLayout;
        textes->setContentsMargins(0, 9, 0, 9);
        textes->setSpacing(4);

        auto *haut = new QHBoxLayout;
        auto *id = new QLabel(p.id(), carte);
        id->setStyleSheet(QString("color: %1; font-weight: bold;").arg(c.accent.name()));
        haut->addWidget(id);
        haut->addStretch();
        if (notes.contains(p.id())) {
            auto *note = new QLabel(QString("★ %1").arg(note1(notes.value(p.id()))), carte);
            note->setStyleSheet("color: #F59E0B; font-weight: bold;");
            haut->addWidget(note);
        }
        textes->addLayout(haut);

        auto *titre = new QLabel(p.titre(), carte);
        titre->setWordWrap(true);
        titre->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: bold;").arg(c.texte.name()));
        textes->addWidget(titre);

        auto *bas = new QHBoxLayout;
        auto *theme = new QLabel(ouTiret(p.thematique()), carte);
        theme->setProperty("role", "badge");
        bas->addWidget(theme);
        bas->addStretch();
        QString echeance = dateOuTiret(p.dateLimite());
        QColor couleurDate = c.discret;
        const bool cloture = p.statut().startsWith("Termin") || p.statut().startsWith("Annul");
        if (p.dateLimite().isValid() && !cloture) {
            const int jours = aujourdHui.daysTo(p.dateLimite());
            if (jours < 0) {
                echeance = tr("⚠ %1").arg(echeance);
                couleurDate = c.danger;
            } else if (jours <= 7) {
                couleurDate = QColor("#F59E0B");
            }
        }
        auto *date = new QLabel(echeance, carte);
        date->setStyleSheet(QString("color: %1; font-size: 12px;").arg(couleurDate.name()));
        bas->addWidget(date);
        textes->addLayout(bas);

        ligne->addLayout(textes, 1);

        QVBoxLayout *cartes = m_colonnesKanban[colonne];
        cartes->insertWidget(cartes->count() - 1, carte);
    }

    // Nombre de projets en tete de chaque colonne
    const QList<QFrame *> colonnes = m_kanban->findChildren<QFrame *>("colonneKanban");
    for (QFrame *colonne : colonnes) {
        const int i = STATUTS.indexOf(colonne->property("statut").toString());
        if (auto *compte = colonne->findChild<QLabel *>("compteKanban"))
            compte->setText(QString::number(i >= 0 ? nombres[i] : 0));
    }
}

// Changement de statut par glisser deposer : UPDATE via Projet::modifier(), annulable
void MainWindow::changerStatut(const QString &id, const QString &statut)
{
    Projet avant;
    QString erreur;
    if (!Projet::charger(id, avant, &erreur)) {
        QMessageBox::warning(this, tr("Kanban"), erreur);
        return;
    }
    if (avant.statut() == statut)
        return;   // deposee dans sa propre colonne : rien a faire

    Projet apres = avant;
    apres.setStatut(statut);
    if (!apres.modifier(&erreur)) {
        QMessageBox::critical(this, tr("Kanban"), tr("Échec du changement de statut.\n%1").arg(erreur));
        notifier(Notification::Erreur, tr("Statut non modifié"), erreur, id, false);
        return;
    }

    empiler({Action::Modification, avant, apres});
    rafraichir();   // tableau, Kanban et statistiques

    // La carte s'illumine dans sa nouvelle colonne (juste apres son affichage)
    QTimer::singleShot(30, this, [this, id] {
    for (QFrame *carte : m_kanban->findChildren<QFrame *>("carteKanban")) {
        if (carte->property("idProjet").toString() != id)
            continue;
        carte->setProperty("nouvelle", true);
        carte->style()->unpolish(carte);
        carte->style()->polish(carte);
        auto *effet = new QGraphicsOpacityEffect(carte);
        carte->setGraphicsEffect(effet);
        auto *fondu = new QPropertyAnimation(effet, "opacity", carte);
        fondu->setDuration(500);
        fondu->setStartValue(0.2);
        fondu->setEndValue(1.0);
        fondu->setEasingCurve(QEasingCurve::OutCubic);
        connect(fondu, &QAbstractAnimation::finished, carte, [carte] { carte->setGraphicsEffect(nullptr); });
        fondu->start(QAbstractAnimation::DeleteWhenStopped);
        QPointer<QFrame> garde(carte);
        QTimer::singleShot(1400, this, [garde] {
            if (!garde)
                return;
            garde->setProperty("nouvelle", false);
            garde->style()->unpolish(garde);
            garde->style()->polish(garde);
        });
        if (auto *defilement = qobject_cast<QScrollArea *>(carte->parentWidget()->parentWidget()->parentWidget()))
            defilement->ensureWidgetVisible(carte);
    }
    });

    notifier(statut.startsWith("Termin") ? Notification::Succes : Notification::Info,
             tr("Statut modifié"),
             tr("%1 « %2 » : %3 → %4").arg(id, avant.titre(), ouTiret(avant.statut()), statut),
             id, true, tr("Annuler"), actionAnnuler());
}

// Glisser deposer : appele par eventFilter() pour les cartes et les colonnes du Kanban
bool MainWindow::gererKanban(QObject *objet, QEvent *evenement)
{
    // ===== Carte : on la saisit puis on la deplace =====
    if (objet->objectName() == "carteKanban") {
        auto *carte = static_cast<QFrame *>(objet);
        if (evenement->type() == QEvent::MouseButtonPress) {
            auto *souris = static_cast<QMouseEvent *>(evenement);
            if (souris->button() == Qt::LeftButton)
                carte->setProperty("pointDepart", souris->position().toPoint());
            return false;
        }
        if (evenement->type() == QEvent::MouseButtonDblClick) {
            const QString id = carte->property("idProjet").toString();
            QTimer::singleShot(0, this, [this, id] { voirProjet(id); });
            return true;
        }
        if (evenement->type() == QEvent::MouseMove) {
            auto *souris = static_cast<QMouseEvent *>(evenement);
            const QVariant depart = carte->property("pointDepart");
            if (!(souris->buttons() & Qt::LeftButton) || !depart.isValid())
                return false;
            const QPoint point = depart.toPoint();
            if ((souris->position().toPoint() - point).manhattanLength() < QApplication::startDragDistance())
                return false;
            carte->setProperty("pointDepart", QVariant());

            // Image de la carte qui suit la souris (legerement transparente)
            const QPixmap capture = carte->grab();
            QPixmap image(capture.size());
            image.setDevicePixelRatio(capture.devicePixelRatio());
            image.fill(Qt::transparent);
            {
                QPainter peintre(&image);
                peintre.setOpacity(0.9);
                peintre.drawPixmap(0, 0, capture);
            }

            auto *donnees = new QMimeData;
            donnees->setData(MIME_PROJET, carte->property("idProjet").toString().toUtf8());
            auto *glisser = new QDrag(carte);
            glisser->setMimeData(donnees);
            glisser->setPixmap(image);
            glisser->setHotSpot(point);

            // Pendant le deplacement, la carte d'origine reste en transparence
            auto *effet = new QGraphicsOpacityEffect(carte);
            effet->setOpacity(0.35);
            carte->setGraphicsEffect(effet);
            QPointer<QFrame> garde(carte);
            glisser->exec(Qt::MoveAction);
            if (garde)
                garde->setGraphicsEffect(nullptr);
            return true;
        }
        return false;
    }

    // ===== Colonne : elle s'allume au survol et recoit la carte =====
    if (objet->objectName() == "colonneKanban") {
        auto *colonne = static_cast<QFrame *>(objet);
        auto allumer = [colonne](bool oui) {
            colonne->setProperty("survol", oui);
            colonne->style()->unpolish(colonne);
            colonne->style()->polish(colonne);
        };
        switch (evenement->type()) {
        case QEvent::DragEnter: {
            auto *e = static_cast<QDragEnterEvent *>(evenement);
            if (!e->mimeData()->hasFormat(MIME_PROJET))
                return false;
            e->acceptProposedAction();
            allumer(true);
            return true;
        }
        case QEvent::DragMove:
            static_cast<QDragMoveEvent *>(evenement)->acceptProposedAction();
            return true;
        case QEvent::DragLeave:
            allumer(false);
            return true;
        case QEvent::Drop: {
            auto *e = static_cast<QDropEvent *>(evenement);
            allumer(false);
            if (!e->mimeData()->hasFormat(MIME_PROJET))
                return false;
            e->acceptProposedAction();
            const QString id = QString::fromUtf8(e->mimeData()->data(MIME_PROJET));
            const QString statut = colonne->property("statut").toString();
            // Applique apres la fin du glisser deposer : les cartes seront reconstruites
            QTimer::singleShot(0, this, [this, id, statut] { changerStatut(id, statut); });
            return true;
        }
        default:
            return false;
        }
    }
    return false;
}

// ==========================================================
// ===== Systeme de notifications =====
// ==========================================================
// notifier() est le point d'entree unique : il range la notification dans l'historique
// (sauvegarde entre deux lancements), met a jour le compteur de la cloche et, si demande,
// affiche un toast. Les toasts s'empilent en haut a droite, ont une barre de temps,
// se mettent en pause au survol et peuvent porter un bouton d'action ("Annuler", "Ouvrir").
// La cloche ouvre un panneau lateral avec tout l'historique, filtrable.

static const int LARGEUR_TOAST = 370;
static const int LARGEUR_PANNEAU = 410;
static const int MAX_TOASTS = 4;
static const int MAX_HISTORIQUE = 100;

QColor MainWindow::couleurType(Notification::Type type, const Couleurs &c)
{
    switch (type) {
    case Notification::Succes:      return c.accent;
    case Notification::Info:        return QColor("#60a5fa");
    case Notification::Alerte:      return QColor("#f59e0b");
    case Notification::Erreur:      return c.danger;
    case Notification::Suppression: return c.danger;
    }
    return c.accent;
}

QString MainWindow::iconeType(Notification::Type type)
{
    switch (type) {
    case Notification::Succes:      return QStringLiteral("✓");
    case Notification::Info:        return QStringLiteral("ℹ");
    case Notification::Alerte:      return QStringLiteral("⚠");
    case Notification::Erreur:      return QStringLiteral("✕");
    case Notification::Suppression: return QStringLiteral("🗑");
    }
    return QStringLiteral("•");
}

// "a l'instant", "il y a 5 min", "il y a 2 h", "hier a 14:05", "12/11/2026 a 09:30"
QString MainWindow::tempsRelatif(const QDateTime &date)
{
    const QDateTime maintenant = QDateTime::currentDateTime();
    const qint64 secondes = date.secsTo(maintenant);
    if (secondes < 60)
        return tr("à l'instant");
    if (secondes < 3600)
        return tr("il y a %1 min").arg(secondes / 60);
    if (date.date() == maintenant.date())
        return tr("il y a %1 h").arg(secondes / 3600);
    if (date.date().daysTo(maintenant.date()) == 1)
        return tr("hier à %1").arg(date.toString("HH:mm"));
    return date.toString(tr("dd/MM/yyyy 'à' HH:mm"));
}

// Feuille de style du panneau et des toasts, aux couleurs du theme courant
QString MainWindow::styleNotifications() const
{
    const Couleurs c = couleurs();
    QString qss = R"(
#panneauNotifs { background: @carte; border-left: 1px solid @bordure; }
#panneauNotifs QLabel { background: transparent; border: none; }
#titrePanneau { font-size: 15pt; font-weight: 700; }
#resumePanneau { color: @discret; }
#carteNotif { background: @carte; border: 1px solid @bordure; border-radius: 8px; }
#carteNotif[nonLue="true"] { background: @badge; }
#carteNotif:hover { border-color: @accent; }
#tempsNotif { color: @discret; font-size: 8pt; }
#messageNotif { color: @discret; }
QPushButton#lienPanneau { background: transparent; border: none; color: @accent; padding: 4px 6px; }
QPushButton#lienPanneau:hover { text-decoration: underline; }
QPushButton#fermerPanneau { background: transparent; border: none; font-size: 13pt; color: @discret; }
QPushButton#fermerPanneau:hover { color: @texte; }
)";
    qss.replace("@carte", c.carte.name())
       .replace("@bordure", c.bordure.name())
       .replace("@discret", c.discret.name())
       .replace("@accent", c.accent.name())
       .replace("@badge", c.badge.name())
       .replace("@texte", c.texte.name());
    return qss;
}

// La cloche (avec son compteur rouge) dans l'en tete, et le panneau lateral cache
void MainWindow::creerCentreNotifications()
{
    // Cloche, placee juste avant "Mode clair"
    m_boutonNotifs = new QPushButton(QStringLiteral("🔔"), this);
    m_boutonNotifs->setToolTip(tr("Notifications"));
    m_boutonNotifs->setCursor(Qt::PointingHandCursor);
    m_boutonNotifs->setFixedSize(48, 38);
    ui->layoutEntete->insertWidget(ui->layoutEntete->indexOf(ui->boutonTheme), m_boutonNotifs);
    connect(m_boutonNotifs, &QPushButton::clicked, this, [this] {
        m_panneauOuvert ? fermerPanneau() : ouvrirPanneau();
    });

    // Compteur de notifications non lues (pastille rouge sur la cloche)
    m_badgeNotifs = new QLabel(m_boutonNotifs);
    m_badgeNotifs->setAlignment(Qt::AlignCenter);
    m_badgeNotifs->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_badgeNotifs->setStyleSheet("background: #ef4444; color: white; border-radius: 9px;"
                                 " font-size: 8pt; font-weight: 700; padding: 0 4px;");
    m_badgeNotifs->hide();

    // Fond assombri : un clic dessus ferme le panneau
    m_voile = new QWidget(this);
    m_voile->setStyleSheet("background: rgba(0, 0, 0, 110);");
    m_voile->installEventFilter(this);
    m_voile->hide();

    // Panneau lateral
    m_panneau = new QFrame(this);
    m_panneau->setObjectName("panneauNotifs");
    m_panneau->setStyleSheet(styleNotifications());
    m_panneau->hide();

    auto *racine = new QVBoxLayout(m_panneau);
    racine->setContentsMargins(20, 20, 16, 16);
    racine->setSpacing(12);

    auto *ligneTitre = new QHBoxLayout;
    auto *titre = new QLabel(tr("Notifications"), m_panneau);
    titre->setObjectName("titrePanneau");
    auto *fermer = new QPushButton(QStringLiteral("✕"), m_panneau);
    fermer->setObjectName("fermerPanneau");
    fermer->setCursor(Qt::PointingHandCursor);
    connect(fermer, &QPushButton::clicked, this, &MainWindow::fermerPanneau);
    ligneTitre->addWidget(titre);
    ligneTitre->addStretch();
    ligneTitre->addWidget(fermer);
    racine->addLayout(ligneTitre);

    m_resumePanneau = new QLabel(m_panneau);
    m_resumePanneau->setObjectName("resumePanneau");
    racine->addWidget(m_resumePanneau);

    auto *ligneOutils = new QHBoxLayout;
    m_filtreNotifs = new QComboBox(m_panneau);
    m_filtreNotifs->addItems({tr("Toutes"), tr("Non lues"), tr("Succès"), tr("Infos"),
                              tr("Alertes"), tr("Erreurs"), tr("Suppressions")});
    connect(m_filtreNotifs, &QComboBox::currentIndexChanged, this, &MainWindow::remplirPanneau);
    auto *toutLu = new QPushButton(tr("Tout marquer comme lu"), m_panneau);
    toutLu->setObjectName("lienPanneau");
    toutLu->setCursor(Qt::PointingHandCursor);
    connect(toutLu, &QPushButton::clicked, this, [this] {
        for (Notification &n : m_notifications)
            n.lue = true;
        sauverNotifications();
        mettreAJourBadge();
        remplirPanneau();
    });
    auto *effacer = new QPushButton(tr("Effacer"), m_panneau);
    effacer->setObjectName("lienPanneau");
    effacer->setCursor(Qt::PointingHandCursor);
    connect(effacer, &QPushButton::clicked, this, [this] {
        if (m_notifications.isEmpty())
            return;
        if (QMessageBox::question(this, tr("Notifications"),
                                  tr("Effacer tout l'historique des notifications ?"))
            != QMessageBox::Yes)
            return;
        m_notifications.clear();
        sauverNotifications();
        mettreAJourBadge();
        remplirPanneau();
    });
    ligneOutils->addWidget(m_filtreNotifs);
    ligneOutils->addStretch();
    ligneOutils->addWidget(toutLu);
    ligneOutils->addWidget(effacer);
    racine->addLayout(ligneOutils);

    // Liste defilante des notifications
    auto *defilement = new QScrollArea(m_panneau);
    defilement->setWidgetResizable(true);
    defilement->setFrameShape(QFrame::NoFrame);
    defilement->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *contenu = new QWidget;
    contenu->setStyleSheet("background: transparent;");
    m_listeNotifs = new QVBoxLayout(contenu);
    m_listeNotifs->setContentsMargins(0, 0, 6, 0);
    m_listeNotifs->setSpacing(8);
    m_listeNotifs->addStretch();
    defilement->setWidget(contenu);
    racine->addWidget(defilement, 1);

    // Echap ferme le panneau
    auto *echap = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(echap, &QShortcut::activated, this, [this] {
        if (m_panneauOuvert)
            fermerPanneau();
    });
}

void MainWindow::notifier(Notification::Type type, const QString &titre, const QString &message,
                          const QString &idProjet, bool afficher,
                          const QString &libelleAction, const std::function<void()> &action)
{
    Notification n;
    n.type = type;
    n.titre = titre;
    n.message = message;
    n.idProjet = idProjet;
    n.date = QDateTime::currentDateTime();
    n.lue = false;

    m_notifications.prepend(n);
    while (m_notifications.size() > MAX_HISTORIQUE)
        m_notifications.removeLast();
    sauverNotifications();
    mettreAJourBadge(true);
    if (m_panneauOuvert)
        remplirPanneau();

    if (afficher)
        afficherToast(n, libelleAction, action);
}

// Bouton "Annuler" des toasts : annule l'action seulement si aucune autre n'a eu lieu depuis
std::function<void()> MainWindow::actionAnnuler()
{
    const int taille = m_historique.size();
    return [this, taille] {
        if (m_historique.size() == taille) {
            annulerDerniereAction();
        } else {
            notifier(Notification::Info, tr("Annulation impossible ici"),
                     tr("Une action plus récente a eu lieu : utilisez Ctrl+Z."));
        }
    };
}

void MainWindow::afficherToast(const Notification &n, const QString &libelleAction,
                               const std::function<void()> &action)
{
    const Couleurs c = couleurs();
    const QColor couleur = couleurType(n.type, c);

    // Pas plus de MAX_TOASTS a l'ecran : le plus ancien (en bas) s'en va
    m_toasts.removeAll(nullptr);
    while (m_toasts.size() >= MAX_TOASTS)
        fermerToast(m_toasts.last());

    auto *toast = new QFrame(this);
    toast->setObjectName("toast");
    toast->setFixedWidth(LARGEUR_TOAST);
    toast->setProperty("idProjet", n.idProjet);
    if (!n.idProjet.isEmpty())
        toast->setCursor(Qt::PointingHandCursor);
    toast->setStyleSheet(QString(
        "#toast { background: %1; border: 1px solid %2; border-left: 5px solid %3; border-radius: 8px; }"
        "#toast QLabel { background: transparent; border: none; }"
        "#toast QPushButton#actionToast { background: transparent; color: %3; border: 1px solid %3;"
        " border-radius: 6px; padding: 4px 10px; font-weight: 600; }"
        "#toast QPushButton#actionToast:hover { background: %3; color: %1; }"
        "#toast QPushButton#fermerToast { background: transparent; border: none; color: %4; padding: 2px 4px; }"
        "#toast QPushButton#fermerToast:hover { color: %5; }"
        "#toast QFrame#barreToast { background: %3; border: none; border-radius: 1px; }")
        .arg(c.carte.name(), c.bordure.name(), couleur.name(), c.discret.name(), c.texte.name()));

    auto *racine = new QVBoxLayout(toast);
    racine->setContentsMargins(0, 0, 0, 0);
    racine->setSpacing(0);

    auto *ligne = new QHBoxLayout;
    ligne->setContentsMargins(14, 12, 8, 10);
    ligne->setSpacing(12);

    auto *icone = new QLabel(iconeType(n.type), toast);
    icone->setStyleSheet(QString("color: %1; font-size: 16pt; font-weight: 700;").arg(couleur.name()));
    ligne->addWidget(icone, 0, Qt::AlignTop);

    auto *textes = new QVBoxLayout;
    textes->setSpacing(2);
    auto *titre = new QLabel(n.titre, toast);
    titre->setStyleSheet(QString("color: %1; font-size: 10.5pt; font-weight: 700;").arg(c.texte.name()));
    textes->addWidget(titre);
    if (!n.message.isEmpty()) {
        auto *message = new QLabel(n.message, toast);
        message->setWordWrap(true);
        message->setStyleSheet(QString("color: %1;").arg(c.discret.name()));
        textes->addWidget(message);
    }
    ligne->addLayout(textes, 1);

    if (!libelleAction.isEmpty() && action) {
        auto *bouton = new QPushButton(libelleAction, toast);
        bouton->setObjectName("actionToast");
        bouton->setCursor(Qt::PointingHandCursor);
        QPointer<QFrame> garde(toast);
        connect(bouton, &QPushButton::clicked, this, [this, garde, action] {
            if (garde)
                fermerToast(garde);
            action();
        });
        ligne->addWidget(bouton, 0, Qt::AlignVCenter);
    }

    auto *fermer = new QPushButton(QStringLiteral("✕"), toast);
    fermer->setObjectName("fermerToast");
    fermer->setCursor(Qt::PointingHandCursor);
    connect(fermer, &QPushButton::clicked, this, [this, toast] { fermerToast(toast); });
    ligne->addWidget(fermer, 0, Qt::AlignTop);
    racine->addLayout(ligne);

    // Barre de temps restant, en bas du toast
    auto *barre = new QFrame(toast);
    barre->setObjectName("barreToast");
    barre->setFixedHeight(3);
    barre->setFixedWidth(LARGEUR_TOAST - 6);
    racine->addWidget(barre, 0, Qt::AlignLeft);

    toast->adjustSize();
    toast->installEventFilter(this);

    // Duree de vie : plus longue pour les alertes et les erreurs
    const bool important = n.type == Notification::Alerte || n.type == Notification::Erreur;
    auto *vie = new QVariantAnimation(toast);
    vie->setObjectName("vieToast");
    vie->setDuration(important ? 8000 : 5000);
    vie->setStartValue(1.0);
    vie->setEndValue(0.0);
    connect(vie, &QVariantAnimation::valueChanged, barre, [barre](const QVariant &v) {
        barre->setFixedWidth(qMax(0, int((LARGEUR_TOAST - 6) * v.toReal())));
    });
    connect(vie, &QVariantAnimation::finished, this, [this, toast] { fermerToast(toast); });

    // Le nouveau toast arrive en haut, en glissant depuis la droite ; les autres descendent
    toast->move(width() + 10, 24);
    toast->show();
    toast->raise();
    m_toasts.prepend(toast);
    placerToasts(toast);
    vie->start();
}

// Ferme un toast : il repart vers la droite, les suivants remontent
void MainWindow::fermerToast(QFrame *toast)
{
    if (!toast || !m_toasts.contains(toast))
        return;
    m_toasts.removeAll(toast);
    if (auto *vie = toast->findChild<QVariantAnimation *>("vieToast"))
        vie->stop();

    auto *sortie = new QPropertyAnimation(toast, "pos", toast);
    sortie->setDuration(300);
    sortie->setStartValue(toast->pos());
    sortie->setEndValue(QPoint(width() + 10, toast->y()));
    sortie->setEasingCurve(QEasingCurve::InCubic);
    connect(sortie, &QAbstractAnimation::finished, toast, &QObject::deleteLater);
    sortie->start();

    placerToasts();
}

// Empile les toasts sous l'en tete (a gauche du panneau quand il est ouvert)
void MainWindow::placerToasts(QFrame *nouveau)
{
    m_toasts.removeAll(nullptr);
    const int droite = m_panneauOuvert ? width() - LARGEUR_PANNEAU : width();
    int y = 24;
    for (const QPointer<QFrame> &t : std::as_const(m_toasts)) {
        const QPoint cible(droite - LARGEUR_TOAST - 24, y);
        y += t->height() + 10;

        // Deplacement precedent encore en cours : on le remplace
        for (auto *a : t->findChildren<QPropertyAnimation *>("deplacementToast", Qt::FindDirectChildrenOnly))
            a->stop();

        auto *deplacement = new QPropertyAnimation(t, "pos", t);
        deplacement->setObjectName("deplacementToast");
        deplacement->setDuration(t == nouveau ? 550 : 280);
        deplacement->setStartValue(t->pos());
        deplacement->setEndValue(cible);
        deplacement->setEasingCurve(t == nouveau ? QEasingCurve::OutBack : QEasingCurve::OutCubic);
        deplacement->start(QAbstractAnimation::DeleteWhenStopped);
        t->raise();
    }
}

// Ouvre le panneau : fond assombri en fondu, panneau qui glisse depuis la droite
void MainWindow::ouvrirPanneau()
{
    if (m_panneauOuvert)
        return;
    m_panneauOuvert = true;
    remplirPanneau();

    m_voile->setGeometry(rect());
    auto *effet = new QGraphicsOpacityEffect(m_voile);
    m_voile->setGraphicsEffect(effet);
    m_voile->show();
    m_voile->raise();
    auto *fondu = new QPropertyAnimation(effet, "opacity", m_voile);
    fondu->setDuration(250);
    fondu->setStartValue(0.0);
    fondu->setEndValue(1.0);
    fondu->start(QAbstractAnimation::DeleteWhenStopped);

    m_panneau->setGeometry(width(), 0, LARGEUR_PANNEAU, height());
    m_panneau->show();
    m_panneau->raise();
    auto *glisse = new QPropertyAnimation(m_panneau, "pos", m_panneau);
    glisse->setDuration(350);
    glisse->setStartValue(QPoint(width(), 0));
    glisse->setEndValue(QPoint(width() - LARGEUR_PANNEAU, 0));
    glisse->setEasingCurve(QEasingCurve::OutCubic);
    glisse->start(QAbstractAnimation::DeleteWhenStopped);

    placerToasts();   // les toasts se decalent a gauche du panneau
}

// Ferme le panneau ; les notifications vues sont alors marquees comme lues
void MainWindow::fermerPanneau()
{
    if (!m_panneauOuvert)
        return;
    m_panneauOuvert = false;

    auto *glisse = new QPropertyAnimation(m_panneau, "pos", m_panneau);
    glisse->setDuration(280);
    glisse->setStartValue(m_panneau->pos());
    glisse->setEndValue(QPoint(width(), 0));
    glisse->setEasingCurve(QEasingCurve::InCubic);
    connect(glisse, &QAbstractAnimation::finished, m_panneau, [this] {
        if (!m_panneauOuvert)
            m_panneau->hide();
    });
    glisse->start(QAbstractAnimation::DeleteWhenStopped);

    if (auto *effet = qobject_cast<QGraphicsOpacityEffect *>(m_voile->graphicsEffect())) {
        auto *fondu = new QPropertyAnimation(effet, "opacity", m_voile);
        fondu->setDuration(250);
        fondu->setStartValue(effet->opacity());
        fondu->setEndValue(0.0);
        connect(fondu, &QAbstractAnimation::finished, m_voile, [this] {
            if (!m_panneauOuvert)
                m_voile->hide();
        });
        fondu->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_voile->hide();
    }

    for (Notification &n : m_notifications)
        n.lue = true;
    sauverNotifications();
    mettreAJourBadge();
    placerToasts();
}

// Reconstruit la liste du panneau selon le filtre choisi ; les cartes apparaissent en cascade
void MainWindow::remplirPanneau()
{
    if (!m_listeNotifs)
        return;

    // Vide la liste (le dernier element est l'espace extensible, on le garde)
    while (m_listeNotifs->count() > 1) {
        QLayoutItem *element = m_listeNotifs->takeAt(0);
        if (element->widget())
            element->widget()->deleteLater();
        delete element;
    }

    const Couleurs c = couleurs();
    const int filtre = m_filtreNotifs->currentIndex();
    int nonLues = 0;
    for (const Notification &n : std::as_const(m_notifications))
        if (!n.lue)
            ++nonLues;
    m_resumePanneau->setText(m_notifications.isEmpty()
                                 ? tr("Aucune notification")
                                 : tr("%1 notification(s), %2 non lue(s)")
                                       .arg(m_notifications.size()).arg(nonLues));

    int affichees = 0;
    for (int i = 0; i < m_notifications.size(); ++i) {
        const Notification &n = m_notifications[i];
        const bool garder =
            filtre == 0
            || (filtre == 1 && !n.lue)
            || (filtre == 2 && n.type == Notification::Succes)
            || (filtre == 3 && n.type == Notification::Info)
            || (filtre == 4 && n.type == Notification::Alerte)
            || (filtre == 5 && n.type == Notification::Erreur)
            || (filtre == 6 && n.type == Notification::Suppression);
        if (!garder)
            continue;

        const QColor couleur = couleurType(n.type, c);
        auto *carte = new QFrame;
        carte->setObjectName("carteNotif");
        carte->setProperty("nonLue", !n.lue);
        carte->setProperty("idProjet", n.idProjet);
        carte->setProperty("indexNotif", i);
        carte->setCursor(Qt::PointingHandCursor);
        carte->installEventFilter(this);

        auto *ligne = new QHBoxLayout(carte);
        ligne->setContentsMargins(12, 10, 12, 10);
        ligne->setSpacing(10);

        auto *icone = new QLabel(iconeType(n.type), carte);
        icone->setFixedWidth(26);
        icone->setStyleSheet(QString("color: %1; font-size: 14pt; font-weight: 700;").arg(couleur.name()));
        ligne->addWidget(icone, 0, Qt::AlignTop);

        auto *textes = new QVBoxLayout;
        textes->setSpacing(2);
        auto *titre = new QLabel(n.titre, carte);
        titre->setStyleSheet("font-weight: 700;");
        textes->addWidget(titre);
        if (!n.message.isEmpty()) {
            auto *message = new QLabel(n.message, carte);
            message->setObjectName("messageNotif");
            message->setWordWrap(true);
            textes->addWidget(message);
        }
        auto *temps = new QLabel(tempsRelatif(n.date), carte);
        temps->setObjectName("tempsNotif");
        textes->addWidget(temps);
        ligne->addLayout(textes, 1);

        if (!n.lue) {
            auto *point = new QLabel(QStringLiteral("●"), carte);
            point->setStyleSheet(QString("color: %1; font-size: 9pt;").arg(c.accent.name()));
            ligne->addWidget(point, 0, Qt::AlignTop);
        }

        m_listeNotifs->insertWidget(m_listeNotifs->count() - 1, carte);

        // Apparition en cascade (seulement les premieres, visibles a l'ecran)
        if (affichees < 10) {
            auto *effet = new QGraphicsOpacityEffect(carte);
            effet->setOpacity(0.0);
            carte->setGraphicsEffect(effet);
            auto *fondu = new QPropertyAnimation(effet, "opacity");
            fondu->setDuration(260);
            fondu->setStartValue(0.0);
            fondu->setEndValue(1.0);
            auto *groupe = new QSequentialAnimationGroup(carte);
            groupe->addPause(45 * affichees);
            groupe->addAnimation(fondu);
            connect(groupe, &QAbstractAnimation::finished, carte, [carte] {
                carte->setGraphicsEffect(nullptr);
            });
            groupe->start(QAbstractAnimation::DeleteWhenStopped);
        }
        ++affichees;
    }

    if (affichees == 0) {
        auto *vide = new QLabel(QStringLiteral("🔔\n") + tr("Rien à afficher ici"));
        vide->setAlignment(Qt::AlignCenter);
        vide->setStyleSheet(QString("color: %1; font-size: 11pt; padding: 40px;").arg(c.discret.name()));
        m_listeNotifs->insertWidget(0, vide);
    }
}

// Compteur rouge de la cloche ; avec animer = true il rebondit et la cloche tremble
void MainWindow::mettreAJourBadge(bool animer)
{
    if (!m_badgeNotifs)
        return;

    int nonLues = 0;
    for (const Notification &n : std::as_const(m_notifications))
        if (!n.lue)
            ++nonLues;

    if (nonLues == 0) {
        m_badgeNotifs->hide();
        return;
    }

    m_badgeNotifs->setText(nonLues > 99 ? QStringLiteral("99+") : QString::number(nonLues));
    const int largeur = qMax(18, m_badgeNotifs->fontMetrics().horizontalAdvance(m_badgeNotifs->text()) + 10);
    const QRect normal(m_boutonNotifs->width() - largeur - 1, 1, largeur, 18);
    m_badgeNotifs->setGeometry(normal);
    m_badgeNotifs->show();
    m_badgeNotifs->raise();

    if (!animer)
        return;

    // Le compteur grossit puis revient a sa taille (rebond)
    auto *rebond = new QPropertyAnimation(m_badgeNotifs, "geometry", m_badgeNotifs);
    rebond->setDuration(450);
    rebond->setStartValue(normal.adjusted(4, 4, -4, -4));
    rebond->setKeyValueAt(0.5, normal.adjusted(-3, -3, 3, 3));
    rebond->setEndValue(normal);
    rebond->setEasingCurve(QEasingCurve::OutBack);
    rebond->start(QAbstractAnimation::DeleteWhenStopped);

    // La cloche tremble de gauche a droite
    for (auto *a : m_boutonNotifs->findChildren<QPropertyAnimation *>("tremblement", Qt::FindDirectChildrenOnly))
        a->stop();
    const QPoint base = m_boutonNotifs->pos();
    auto *tremble = new QPropertyAnimation(m_boutonNotifs, "pos", m_boutonNotifs);
    tremble->setObjectName("tremblement");
    tremble->setDuration(500);
    tremble->setStartValue(base);
    tremble->setKeyValueAt(0.15, base + QPoint(-4, 0));
    tremble->setKeyValueAt(0.35, base + QPoint(4, 0));
    tremble->setKeyValueAt(0.55, base + QPoint(-3, 0));
    tremble->setKeyValueAt(0.75, base + QPoint(2, 0));
    tremble->setEndValue(base);
    tremble->start(QAbstractAnimation::DeleteWhenStopped);
}

// Affiche la page Projets, retrouve le projet (en vidant la recherche si besoin) et l'illumine
void MainWindow::allerAuProjet(const QString &id)
{
    if (!Projet::existe(id)) {
        notifier(Notification::Info, tr("Projet introuvable"),
                 tr("Le projet %1 n'existe plus.").arg(id));
        return;
    }

    ui->boutonProjets->setChecked(true);
    if (ui->stackedWidget->currentWidget() != ui->pageProjets) {
        ui->stackedWidget->setCurrentWidget(ui->pageProjets);
        animerPage(ui->pageProjets);
    }

    bool affiche = false;
    for (const Projet &p : std::as_const(m_projetsAffiches))
        if (p.id() == id)
            affiche = true;
    if (!affiche)
        ui->lineEditRecherche->clear();   // relance rafraichir() : tous les projets

    if (m_vues && m_vues->currentWidget() != ui->tableProjets)
        afficherVue(false, false);
    ui->scrollArea->ensureWidgetVisible(ui->tableProjets);
    animerLigne(id);
}

// Alertes d'echeances : projets non termines dont la date limite est depassee (rouge)
// ou tombe dans les 3 prochains jours (orange). Chaque alerte n'est donnee qu'une fois par jour.
void MainWindow::verifierEcheances()
{
    QString erreur;
    const QList<Projet> projets = Projet::lister(QString(), &erreur);
    if (!erreur.isEmpty())
        return;

    QSettings reglages("Devena", "Devena");
    const QDate aujourdHui = QDate::currentDate();
    QStringList dejaSignalees;
    if (reglages.value("echeances/jour").toDate() == aujourdHui)
        dejaSignalees = reglages.value("echeances/signalees").toStringList();

    QList<Notification> nouvelles;
    for (const Projet &p : projets) {
        if (p.statut().startsWith("Termin") || p.statut().startsWith("Annul") || !p.dateLimite().isValid())
            continue;

        const int jours = aujourdHui.daysTo(p.dateLimite());
        if (jours > 3)
            continue;

        const QString cle = QString("%1:%2:%3").arg(p.id(), jours < 0 ? "retard" : "proche",
                                                    p.dateLimite().toString(Qt::ISODate));
        if (dejaSignalees.contains(cle))
            continue;
        dejaSignalees << cle;

        Notification n;
        n.idProjet = p.id();
        n.date = QDateTime::currentDateTime();
        if (jours < 0) {
            n.type = Notification::Erreur;
            n.titre = tr("Échéance dépassée : %1").arg(p.id());
            n.message = tr("« %1 » devait être terminé le %2 (%3 jour(s) de retard).")
                            .arg(p.titre(), p.dateLimite().toString("dd/MM/yyyy")).arg(-jours);
        } else {
            n.type = Notification::Alerte;
            n.titre = tr("Échéance proche : %1").arg(p.id());
            n.message = jours == 0
                            ? tr("« %1 » doit être terminé aujourd'hui.").arg(p.titre())
                            : tr("« %1 » doit être terminé dans %2 jour(s), le %3.")
                                  .arg(p.titre()).arg(jours).arg(p.dateLimite().toString("dd/MM/yyyy"));
        }
        nouvelles << n;
    }

    reglages.setValue("echeances/jour", aujourdHui);
    reglages.setValue("echeances/signalees", dejaSignalees);
    if (nouvelles.isEmpty())
        return;

    // Toutes vont dans l'historique ; a l'ecran : une par une si peu nombreuses, sinon un resume
    for (const Notification &n : std::as_const(nouvelles))
        notifier(n.type, n.titre, n.message, n.idProjet, false);

    if (nouvelles.size() <= 3) {
        for (int i = 0; i < nouvelles.size(); ++i) {
            const Notification n = nouvelles[i];
            QTimer::singleShot(400 * i, this, [this, n] { afficherToast(n); });
        }
    } else {
        Notification resume;
        resume.type = Notification::Alerte;
        resume.titre = tr("%1 échéances à surveiller").arg(nouvelles.size());
        resume.message = tr("Des projets sont en retard ou presque. Détails dans les notifications.");
        resume.date = QDateTime::currentDateTime();
        afficherToast(resume, tr("Voir"), [this] { ouvrirPanneau(); });
    }
}

// Historique enregistre avec QSettings (registre Windows) : il survit a la fermeture
void MainWindow::sauverNotifications() const
{
    QVariantList liste;
    for (const Notification &n : m_notifications) {
        QVariantMap m;
        m["type"] = int(n.type);
        m["titre"] = n.titre;
        m["message"] = n.message;
        m["idProjet"] = n.idProjet;
        m["date"] = n.date;
        m["lue"] = n.lue;
        liste << m;
    }
    QSettings("Devena", "Devena").setValue("notifications/historique", liste);
}

void MainWindow::chargerNotifications()
{
    m_notifications.clear();
    const QVariantList liste = QSettings("Devena", "Devena").value("notifications/historique").toList();
    for (const QVariant &v : liste) {
        const QVariantMap m = v.toMap();
        Notification n;
        n.type = Notification::Type(qBound(0, m["type"].toInt(), int(Notification::Suppression)));
        n.titre = m["titre"].toString();
        n.message = m["message"].toString();
        n.idProjet = m["idProjet"].toString();
        n.date = m["date"].toDateTime();
        n.lue = m["lue"].toBool();
        m_notifications << n;
    }
    mettreAJourBadge();
}

// Fenetre redimensionnee : le panneau, le fond assombri et les toasts suivent
void MainWindow::resizeEvent(QResizeEvent *evenement)
{
    QMainWindow::resizeEvent(evenement);
    if (m_voile && m_voile->isVisible())
        m_voile->setGeometry(rect());
    if (m_panneau && m_panneau->isVisible())
        m_panneau->setGeometry(m_panneauOuvert ? width() - LARGEUR_PANNEAU : width(), 0,
                               LARGEUR_PANNEAU, height());
    placerToasts();
}

// ==========================================================
// ===== Theme : mode sombre / mode clair =====
// ==========================================================

void MainWindow::basculerTheme()
{
    m_sombre = !m_sombre;
    appliquerTheme();
    remplirTableau(m_projetsAffiches);   // recolore les ID
}

void MainWindow::appliquerTheme()
{
    qApp->setStyleSheet(feuilleDeStyle());
    ui->boutonTheme->setText(m_sombre ? tr("☼  Mode clair") : tr("☾  Mode sombre"));
    mettreAJourStatistiques(true);   // couleurs des barres et du donut
    if (auto *halo = qobject_cast<QGraphicsDropShadowEffect *>(ui->boutonAjouter->graphicsEffect()))
        halo->setColor(couleurs().accent);
    if (m_panneau) {
        m_panneau->setStyleSheet(styleNotifications());
        if (m_panneauOuvert)
            remplirPanneau();
    }
}

MainWindow::Couleurs MainWindow::couleurs() const
{
    // Memes couleurs que le module Equipes : fond, panneau, carte, bordure, texte,
    // texte discret, accent, texte sur accent, fond d'etiquette, danger
    if (m_sombre)
        return {QColor("#050B14"), QColor("#030914"), QColor("#091522"), QColor("#243A52"),
                QColor("#EAF2FF"), QColor("#8EA0B7"), QColor("#18D9EF"), QColor("#03212A"),
                QColor("#10394B"), QColor("#F87171")};
    return {QColor("#F4F7FB"), QColor("#FFFFFF"), QColor("#FFFFFF"), QColor("#C8D3E0"),
            QColor("#182433"), QColor("#64748B"), QColor("#087F91"), QColor("#FFFFFF"),
            QColor("#DDF7FA"), QColor("#DC2626")};
}

// Feuille de style Qt (QSS) : les mots @xxx sont remplaces par les couleurs du theme
QString MainWindow::feuilleDeStyle() const
{
    const Couleurs c = couleurs();

    QString qss = R"(
QMainWindow, QDialog, QMessageBox, #contenuProjets, QStackedWidget > QWidget {
    background: @fond;
}
QWidget { color: @texte; font-family: "Segoe UI"; font-size: 13px; }
QLabel { background: transparent; }
QToolTip { background: @carte; color: @texte; border: 1px solid @bordure; }

/* Barre laterale */
#sidebar { background: @panneau; border-right: 1px solid @bordure; }
#labelLogo { color: @accent; font-size: 27px; font-weight: bold; }
#labelSlogan, #labelPied { color: @discret; font-size: 9pt; }
QPushButton[role="menu"] {
    text-align: left; padding: 12px; background: transparent;
    border: 1px solid transparent; border-radius: 8px; color: @discret; font-weight: normal;
}
QPushButton[role="menu"]:hover { color: @texte; }
QPushButton[role="menu"]:checked { background: @badge; border: 1px solid @accent; color: @accent; font-weight: bold; }

/* Textes */
#labelTitrePage { font-size: 30px; font-weight: bold; }
QLabel[role="sousTitre"] { color: @discret; }
QLabel[role="section"] { font-size: 19px; font-weight: bold; }
QLabel[role="valeurStat"] { color: @accent; font-size: 18pt; }
QLabel[role="titreDialogue"] { color: @accent; font-size: 20px; font-weight: bold; }
QLabel[role="gras"] { font-size: 16px; font-weight: bold; }
QLabel[role="etiquette"] { color: @accent; font-weight: bold; }
QLabel[role="badge"] {
    background: @badge; color: @accent; border-radius: 6px; padding: 3px 9px;
}

/* Boutons */
QPushButton {
    background: @carte; border: 1px solid @bordure; border-radius: 8px;
    padding: 8px 12px; font-weight: bold;
}
QPushButton:hover { border-color: @accent; }
QPushButton[role="primaire"] {
    background: @accent; color: @accentTexte; border: 1px solid @accent; font-weight: bold;
}
QPushButton[role="lien"] { background: transparent; border: none; color: @discret; }
QPushButton[role="lien"]:hover { color: @accent; }
QPushButton[role="lien"]:disabled { color: @bordure; }
QToolButton[role="action"], QToolButton[role="danger"] {
    background: transparent; border: 1px solid transparent; border-radius: 6px; color: @texte;
    font-family: "Segoe UI Emoji", "Segoe UI Symbol", "Segoe UI"; font-size: 16px; padding: 0;
}
QToolButton[role="action"]:hover { background: @badge; border-color: @accent; color: @accent; }
QToolButton[role="danger"]:hover { background: @badge; border-color: @danger; color: @danger; }

/* Champs */
QLineEdit, QPlainTextEdit, QComboBox, QDateEdit {
    background: @panneau; border: 1px solid @bordure; border-radius: 8px; padding: 9px;
    font-size: 14px;
    selection-background-color: @accent; selection-color: @accentTexte;
}
QLineEdit:focus, QPlainTextEdit:focus, QComboBox:focus, QDateEdit:focus { border-color: @accent; }
QLineEdit:read-only { color: @discret; }
QComboBox QAbstractItemView {
    background: @carte; border: 1px solid @bordure;
    selection-background-color: @badge; selection-color: @texte;
}
QCalendarWidget QWidget { background: @carte; color: @texte; }
QCalendarWidget QToolButton { background: transparent; color: @texte; }
QCalendarWidget QAbstractItemView:enabled {
    selection-background-color: @accent; selection-color: @accentTexte;
}

/* Cartes et tableau */
QFrame[role="carte"] { background: @carte; border: 1px solid @bordure; border-radius: 8px; }
QFrame[role="carte"] QLabel { border: none; }
QTableWidget {
    background: @carte; border: 1px solid @bordure; border-radius: 8px;
    gridline-color: transparent; outline: none;
}
QTableWidget::item { border-bottom: 1px solid @bordure; padding: 4px 8px; }
QTableWidget::item:selected { background: @badge; color: @texte; }
QHeaderView::section {
    background: @carte; color: @accent; border: none; border-bottom: 1px solid @bordure;
    padding: 11px 8px; font-weight: bold;
}
QHeaderView::section:hover { color: @texte; }
QTableCornerButton::section { background: @carte; border: none; }

/* Kanban */
#colonneKanban { background: @carte; border: 1px solid @bordure; border-radius: 10px; }
#colonneKanban[survol="true"] { background: @badge; border: 2px dashed @accent; }
#carteKanban { background: @panneau; border: 1px solid @bordure; border-radius: 8px; }
#carteKanban:hover { border-color: @accent; }
#carteKanban[nouvelle="true"] { background: @badge; border: 2px solid @accent; }
#compteKanban { background: @badge; color: @accent; border-radius: 9px; padding: 1px 8px; font-weight: bold; }
QPushButton[role="vue"] { padding: 6px 12px; }
QPushButton[role="vue"]:checked { background: @accent; color: @accentTexte; border-color: @accent; }

/* Organigramme des statistiques */
#carteOrga { background: @carte; border: 1px solid @bordure; border-radius: 9px; }
#titreOrga { color: @accent; font-size: 12px; font-weight: bold; }
#traitOrga { background: @bordure; border: none; }
#libelleOrga { color: @texte; font-size: 11px; }
#valeurOrga { color: @accent; font-size: 12px; font-weight: bold; }
#grandTotal { color: @accent; font-size: 30px; font-weight: bold; }
#detailOrga { color: @discret; font-size: 11px; }

/* Defilement */
QScrollArea { border: none; background: transparent; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: @bordure; border-radius: 5px; min-height: 30px; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }
QScrollBar::handle:horizontal { background: @bordure; border-radius: 5px; min-width: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
)";

    qss.replace("@fond", c.fond.name())
       .replace("@panneau", c.panneau.name())
       .replace("@carte", c.carte.name())
       .replace("@bordure", c.bordure.name())
       .replace("@texte", c.texte.name())
       .replace("@discret", c.discret.name())
       .replace("@accentTexte", c.accentTexte.name())   // avant @accent
       .replace("@accent", c.accent.name())
       .replace("@badge", c.badge.name())
       .replace("@danger", c.danger.name());
    return qss;
}
