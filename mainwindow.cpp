#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <optional>

#include "qrcodegen.hpp"   // bibliotheque QR Code generator (Nayuki, licence MIT)

#include <QApplication>
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
#include <QClipboard>
#include <QDesktopServices>
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

    construireMenu();
    configurerTableau();

    connect(ui->boutonAjouter,     &QPushButton::clicked, this, &MainWindow::ajouterProjet);
    connect(ui->boutonExporterPdf, &QPushButton::clicked, this, &MainWindow::exporterPdf);
    connect(ui->boutonTheme,       &QPushButton::clicked, this, &MainWindow::basculerTheme);
    connect(ui->boutonAnnuler,     &QPushButton::clicked, this, &MainWindow::annulerDerniereAction);
    connect(ui->lineEditRecherche, &QLineEdit::textChanged, this, &MainWindow::rafraichir);

    auto *raccourci = new QShortcut(QKeySequence::Undo, this);
    connect(raccourci, &QShortcut::activated, this, &MainWindow::annulerDerniereAction);
    ui->boutonAnnuler->setEnabled(false);

    // La courbe des statistiques est dessinee par eventFilter()
    ui->graphique->installEventFilter(this);

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
    t->setHorizontalHeaderLabels({tr("ID Projet"), tr("Titre"), tr("Description"),
                                  tr("Thématique"), tr("Statut"), tr("Date de création"),
                                  tr("Date limite"), tr("Actions")});
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
    h->setSectionResizeMode(ColDescription, QHeaderView::Stretch);
    const int largeurs[] = {95, 150, 0, 120, 130, 140, 120, 165};
    for (int c = 0; c < NbColonnes; ++c)
        if (c != ColDescription)
            t->setColumnWidth(c, largeurs[c]);

    // Double clic sur une ligne : afficher les informations
    connect(t, &QTableWidget::cellDoubleClicked, this, [this](int ligne, int) {
        if (ligne >= 0 && ligne < m_projetsAffiches.size())
            voirProjet(m_projetsAffiches[ligne].id());
    });
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

        // Boutons d'action : voir, modifier, supprimer
        auto *actions = new QWidget;
        auto *la = new QHBoxLayout(actions);
        la->setContentsMargins(4, 0, 10, 0);
        la->setSpacing(6);
        la->addStretch();

        const QString id = p.id();
        auto creer = [actions, la](const QString &icone, const QString &aide, const char *role) {
            auto *b = new QToolButton(actions);
            b->setText(icone);
            b->setToolTip(aide);
            b->setProperty("role", role);
            b->setCursor(Qt::PointingHandCursor);
            la->addWidget(b);
            return b;
        };
        connect(creer(QStringLiteral("👁"), tr("Voir"), "action"), &QToolButton::clicked,
                this, [this, id] { voirProjet(id); });
        connect(creer(QStringLiteral("✎"), tr("Modifier"), "action"), &QToolButton::clicked,
                this, [this, id] { modifierProjet(id); });
        connect(creer(QStringLiteral("▦"), tr("QR code et flyer"), "action"), &QToolButton::clicked,
                this, [this, id] { afficherQRProjet(id); });
        connect(creer(QStringLiteral("🗑"), tr("Supprimer"), "danger"), &QToolButton::clicked,
                this, [this, id] { supprimerProjet(id); });

        t->setCellWidget(i, ColActions, actions);
    }
}

void MainWindow::rafraichir()
{
    QString erreur;
    const QList<Projet> projets = Projet::lister(ui->lineEditRecherche->text(), &erreur);
    if (!erreur.isEmpty()) {
        QMessageBox::critical(this, tr("Base de données"),
                              tr("Impossible de lire les projets.\n%1").arg(erreur));
        notifier(Notification::Erreur, tr("Lecture impossible"), erreur, QString(), false);
    }
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

bool MainWindow::ouvrirFormulaire(Projet &p, bool modification)
{
    QDialog dlg(this);
    dlg.setWindowTitle(modification ? tr("Modifier le projet") : tr("Ajouter un projet"));
    dlg.resize(1500, 440);

    // Champs
    auto *id = new QLineEdit(p.id(), &dlg);
    id->setMaxLength(30);
    id->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9_]{0,30}"), id));
    id->setReadOnly(modification);

    auto *titre = new QLineEdit(p.titre(), &dlg);
    titre->setMaxLength(80);

    auto *description = new QPlainTextEdit(p.description(), &dlg);
    description->setFixedHeight(90);

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

    // Titre et sous titre
    auto *labelTitre = new QLabel(modification ? tr("MODIFIER LE PROJET") : tr("AJOUTER UN PROJET"), &dlg);
    labelTitre->setProperty("role", "titreDialogue");
    auto *sousTitre = new QLabel(tr("Informations du projet"), &dlg);
    sousTitre->setProperty("role", "gras");

    // Cadre avec les champs alignes : etiquettes sur la ligne 0, champs sur la ligne 1
    auto *cadre = new QFrame(&dlg);
    cadre->setProperty("role", "carte");
    auto *grille = new QGridLayout(cadre);
    grille->setContentsMargins(14, 14, 14, 14);
    grille->setHorizontalSpacing(12);

    const QList<QPair<QString, QWidget *>> champs = {
        {tr("ID Projet"), id},
        {tr("Titre"), titre},
        {tr("Description"), description},
        {tr("Thématique"), thematique},
        {tr("Statut"), statut},
        {tr("Date de création"), dateCreation},
        {tr("Date limite"), dateLimite},
    };
    for (int col = 0; col < champs.size(); ++col) {
        auto *etiquette = new QLabel(champs[col].first, cadre);
        etiquette->setProperty("role", "etiquette");
        grille->addWidget(etiquette, 0, col);
        grille->addWidget(champs[col].second, 1, col, Qt::AlignTop);
    }
    grille->setColumnStretch(2, 1);   // la description prend la place restante
    grille->setRowStretch(2, 1);

    // Boutons
    auto *enregistrer = new QPushButton(tr("Enregistrer"), &dlg);
    enregistrer->setProperty("role", "primaire");
    enregistrer->setDefault(true);
    auto *annuler = new QPushButton(tr("Annuler"), &dlg);
    auto *ligneBoutons = new QHBoxLayout;
    ligneBoutons->addStretch();
    ligneBoutons->addWidget(enregistrer);
    ligneBoutons->addWidget(annuler);

    auto *racine = new QVBoxLayout(&dlg);
    racine->setContentsMargins(26, 22, 26, 20);
    racine->setSpacing(12);
    racine->addWidget(labelTitre);
    racine->addWidget(sousTitre);
    racine->addWidget(cadre, 1);
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
    if (dlg.exec() != QDialog::Accepted)
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
void MainWindow::voirProjet(const QString &id)
{
    Projet p;
    QString erreur;
    if (!Projet::charger(id, p, &erreur)) {
        QMessageBox::warning(this, tr("Informations du projet"), erreur);
        return;
    }

    const QString texte =
        tr("ID PROJET : %1\n\n").arg(p.id()) +
        tr("TITRE : %1\n\n").arg(ouTiret(p.titre())) +
        tr("DESCRIPTION :\n%1\n\n").arg(ouTiret(p.description())) +
        tr("THÉMATIQUE : %1\n\n").arg(ouTiret(p.thematique())) +
        tr("STATUT : %1\n\n").arg(ouTiret(p.statut())) +
        tr("DATE DE CRÉATION : %1\n\n").arg(dateOuTiret(p.dateCreation())) +
        tr("DATE LIMITE : %1").arg(dateOuTiret(p.dateLimite()));

    QMessageBox boite(QMessageBox::Information, tr("Informations du projet"), texte,
                      QMessageBox::Ok, this);
    animerApparition(&boite, false);
    boite.exec();
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

// ===== Export PDF : la liste affichee (avec le filtre de recherche) =====
void MainWindow::exporterPdf()
{
    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString chemin = QFileDialog::getSaveFileName(this, tr("Exporter en PDF"),
                                                  dossier + "/projets.pdf",
                                                  tr("Fichiers PDF (*.pdf)"));
    if (chemin.isEmpty())
        return;
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive))
        chemin += ".pdf";

    auto echapper = [](const QString &s) { return ouTiret(s).toHtmlEscaped(); };

    QString html =
        "<h2 style='color:#0891b2'>Devena : liste des projets</h2>"
        "<p style='color:#555'>Exporté le " + QDate::currentDate().toString("dd/MM/yyyy") +
        " · " + QString::number(m_projetsAffiches.size()) + " projet(s)</p>"
        "<table width='100%' cellspacing='0' cellpadding='5' border='1' "
        "style='border-collapse:collapse; border-color:#cbd5e1; font-size:9pt'>"
        "<tr style='background:#0891b2; color:white'>"
        "<th>ID Projet</th><th>Titre</th><th>Description</th><th>Thématique</th>"
        "<th>Statut</th><th>Date de création</th><th>Date limite</th></tr>";

    for (const Projet &p : std::as_const(m_projetsAffiches)) {
        html += "<tr><td>" + echapper(p.id()) + "</td><td>" + echapper(p.titre()) +
                "</td><td>" + echapper(p.description()) + "</td><td>" +
                echapper(p.thematique()) + "</td><td>" + echapper(p.statut()) +
                "</td><td>" + dateOuTiret(p.dateCreation()) + "</td><td>" +
                dateOuTiret(p.dateLimite()) + "</td></tr>";
    }
    html += "</table>";

    QPdfWriter pdf(chemin);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Landscape);
    pdf.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    pdf.setTitle(tr("Liste des projets"));

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&pdf);

    notifier(Notification::Succes, tr("PDF exporté"),
             tr("%1 enregistré (%2 projet(s)).").arg(QFileInfo(chemin).fileName())
                 .arg(m_projetsAffiches.size()),
             QString(), true, tr("Ouvrir"),
             [chemin] { QDesktopServices::openUrl(QUrl::fromLocalFile(chemin)); });
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

// ==========================================================
// ===== Statistiques =====
// ==========================================================

// Remplit les 4 cartes et les points de la courbe (projets par thematique)
void MainWindow::mettreAJourStatistiques()
{
    int total = 0, enCours = 0, termines = 0, enRetard = 0;
    QSqlQuery q;

    // "Termin%" evite de dependre de l'encodage du "e" accentue
    if (q.exec("SELECT COUNT(*), "
               "SUM(CASE WHEN STATUT = 'En cours' THEN 1 ELSE 0 END), "
               "SUM(CASE WHEN STATUT LIKE 'Termin%' THEN 1 ELSE 0 END), "
               "SUM(CASE WHEN DATE_LIMITE < TRUNC(SYSDATE) "
               "AND (STATUT IS NULL OR STATUT NOT LIKE 'Termin%') THEN 1 ELSE 0 END) "
               "FROM PROJETS")
        && q.next()) {
        total    = q.value(0).toInt();
        enCours  = q.value(1).toInt();
        termines = q.value(2).toInt();
        enRetard = q.value(3).toInt();
    }
    animerCompteur(ui->labelNbProjets, total);
    animerCompteur(ui->labelNbEnCours, enCours);
    animerCompteur(ui->labelNbTermines, termines);
    animerCompteur(ui->labelNbEnRetard, enRetard);

    const QList<int> chiffres = {total, enCours, termines, enRetard};
    if (chiffres != m_chiffres) {
        m_chiffres = chiffres;
        animerCartes();
    }

    const QList<QPair<QString, int>> anciennes = m_statsThematique;
    m_statsThematique.clear();
    if (q.exec("SELECT NVL(THEMATIQUE, '-'), COUNT(*) FROM PROJETS "
               "GROUP BY THEMATIQUE ORDER BY 1")) {
        while (q.next())
            m_statsThematique.append({q.value(0).toString(), q.value(1).toInt()});
    }

    // La courbe ne se redessine en anime que si les chiffres ont change
    // (pas a chaque lettre tapee dans la recherche)
    if (m_statsThematique != anciennes)
        animerGraphique();
    else
        ui->graphique->update();
}

// Quand ui->graphique doit se redessiner, on dessine la courbe nous memes
bool MainWindow::eventFilter(QObject *objet, QEvent *evenement)
{
    if (objet == ui->graphique && evenement->type() == QEvent::Paint) {
        dessinerGraphique();
        return true;
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

void MainWindow::dessinerGraphique()
{
    const Couleurs c = couleurs();
    QPainter p(ui->graphique);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF zone = ui->graphique->rect().adjusted(30, 30, -30, -30);

    // Lignes horizontales de fond
    p.setPen(QPen(c.bordure, 1));
    for (int i = 0; i <= 4; ++i) {
        const qreal y = zone.top() + zone.height() * i / 4.0;
        p.drawLine(QPointF(zone.left(), y), QPointF(zone.right(), y));
    }

    if (m_statsThematique.isEmpty())
        return;

    int maxi = 1;
    for (const auto &d : std::as_const(m_statsThematique))
        maxi = qMax(maxi, d.second);

    // Position de chaque point
    QList<QPointF> points;
    const int n = m_statsThematique.size();
    for (int i = 0; i < n; ++i) {
        const qreal x = (n == 1) ? zone.center().x()
                                 : zone.left() + zone.width() * i / (n - 1);
        const qreal y = zone.bottom() - (zone.height() * 0.8) * m_statsThematique[i].second / maxi;
        points.append(QPointF(x, y));
    }

    // Animation : seule la partie a gauche de xVisible est dessinee
    const qreal xVisible = zone.left() - 8 + (zone.width() + 16) * m_progressionCourbe;

    // Courbe (coupee a xVisible)
    QPainterPath chemin(points.first());
    for (int i = 1; i < n; ++i)
        chemin.lineTo(points[i]);
    p.save();
    p.setClipRect(QRectF(0, 0, xVisible, ui->graphique->height()));
    p.setPen(QPen(c.accent, 2.5));
    p.setBrush(Qt::NoBrush);
    p.drawPath(chemin);
    p.restore();

    // Points et valeurs (quand la courbe les atteint), thematiques toujours visibles
    QFont police = ui->graphique->font();
    police.setPointSizeF(police.pointSizeF() * 0.9);
    p.setFont(police);
    for (int i = 0; i < n; ++i) {
        if (points[i].x() <= xVisible) {
            p.setPen(Qt::NoPen);
            p.setBrush(c.accent);
            p.drawEllipse(points[i], 5, 5);

            p.setPen(c.discret);
            p.drawText(QRectF(points[i].x() - 40, points[i].y() - 26, 80, 18),
                       Qt::AlignCenter, QString::number(m_statsThematique[i].second));
        }
        p.setPen(c.discret);
        p.drawText(QRectF(points[i].x() - 60, zone.bottom() + 6, 120, 18),
                   Qt::AlignCenter, m_statsThematique[i].first);
    }
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

// La courbe des statistiques se dessine de gauche a droite.
// m_progressionCourbe passe de 0 a 1, dessinerGraphique() n'en montre que cette partie.
void MainWindow::animerGraphique()
{
    if (!m_animationCourbe) {
        m_animationCourbe = new QVariantAnimation(this);
        m_animationCourbe->setDuration(900);
        m_animationCourbe->setStartValue(0.0);
        m_animationCourbe->setEndValue(1.0);
        m_animationCourbe->setEasingCurve(QEasingCurve::InOutCubic);
        connect(m_animationCourbe, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_progressionCourbe = v.toReal();
            ui->graphique->update();
        });
    }
    m_animationCourbe->stop();
    m_progressionCourbe = 0.0;
    m_animationCourbe->start();
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

// Melange deux couleurs : t = 0 donne a, t = 1 donne b
static QColor melanger(const QColor &a, const QColor &b, qreal t)
{
    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t,
                            a.greenF() + (b.greenF() - a.greenF()) * t,
                            a.blueF() + (b.blueF() - a.blueF()) * t);
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

// Les 4 cartes de statistiques s'allument (bordure et fond couleur d'accent)
// l'une apres l'autre, puis reviennent doucement a leur couleur normale.
void MainWindow::animerCartes()
{
    const Couleurs c = couleurs();
    const QList<QFrame *> cartes = {ui->carteProjets, ui->carteEnCours,
                                    ui->carteTermines, ui->carteEnRetard};

    for (int i = 0; i < cartes.size(); ++i) {
        QFrame *carte = cartes[i];

        // Animation precedente encore en cours : on l'arrete
        for (auto *g : carte->findChildren<QSequentialAnimationGroup *>(QString(), Qt::FindDirectChildrenOnly))
            g->stop();
        carte->setStyleSheet(QString());

        auto *eclat = new QVariantAnimation;
        eclat->setDuration(1100);
        eclat->setStartValue(1.0);   // 1 = allumee, 0 = normale
        eclat->setEndValue(0.0);
        eclat->setEasingCurve(QEasingCurve::InQuad);
        connect(eclat, &QVariantAnimation::valueChanged, carte, [carte, c](const QVariant &v) {
            const qreal t = v.toReal();
            carte->setStyleSheet(QString("QFrame#%1 { background: %2; border: 1px solid %3; border-radius: 8px; }")
                                     .arg(carte->objectName(),
                                          melanger(c.carte, c.badge, t).name(),
                                          melanger(c.bordure, c.accent, t).name()));
        });
        connect(eclat, &QVariantAnimation::finished, carte, [carte] {
            carte->setStyleSheet(QString());   // retour au style normal du theme
        });

        auto *groupe = new QSequentialAnimationGroup(carte);
        groupe->addPause(140 * i);
        groupe->addAnimation(eclat);
        groupe->start(QAbstractAnimation::DeleteWhenStopped);
    }
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
// ===== QR codes et flyers =====
// ==========================================================
// Chaque projet a un QR code qui contient ses informations (texte) : scanne avec
// l'appareil photo d'un telephone, il affiche l'ID, le titre, la thematique, le statut,
// la date limite et la description. L'encodage est fait par la bibliotheque
// "QR Code generator" (qrcodegen.hpp / .cpp, licence MIT) ; le dessin est fait ici avec QPainter.

// Encode un texte en QR code (correction d'erreurs moyenne, renforcee si la place le permet)
static std::optional<qrcodegen::QrCode> encoderQR(const QString &texte)
{
    try {
        return qrcodegen::QrCode::encodeText(texte.toUtf8().constData(),
                                             qrcodegen::QrCode::Ecc::MEDIUM);
    } catch (const std::exception &) {
        return std::nullopt;   // texte trop long pour un QR code
    }
}

// Dessine le QR dans une zone carree, avec la marge claire de 4 modules qu'exige la norme
// (sans elle, les telephones le lisent mal). lignesVisibles sert a l'animation de "scan".
// chevauchement : petit debord des carres, pour eviter des traits blancs dans certains lecteurs PDF.
static void dessinerQR(QPainter &p, const qrcodegen::QrCode &qr, const QRectF &zone,
                       const QColor &encre, const QColor &fond,
                       int lignesVisibles = -1, qreal chevauchement = 0)
{
    const int n = qr.getSize();
    const qreal module = zone.width() / (n + 8);
    p.fillRect(zone, fond);
    p.setPen(Qt::NoPen);
    p.setBrush(encre);

    const int derniere = lignesVisibles < 0 ? n : qMin(n, lignesVisibles);
    for (int y = 0; y < derniere; ++y) {
        int x = 0;
        while (x < n) {
            if (!qr.getModule(x, y)) {
                ++x;
                continue;
            }
            // Regroupe les modules noirs voisins d'une meme ligne en un seul rectangle
            const int debut = x;
            while (x < n && qr.getModule(x, y))
                ++x;
            p.drawRect(QRectF(zone.left() + (debut + 4) * module, zone.top() + (y + 4) * module,
                              (x - debut) * module + chevauchement, module + chevauchement));
        }
    }
}

// Image du QR (noir sur blanc). La taille d'un module est un nombre entier de pixels :
// l'image reste nette, sans flou, quelle que soit sa taille.
static QImage creerImageQR(const qrcodegen::QrCode &qr, int tailleVoulue, int lignesVisibles = -1)
{
    const int n = qr.getSize();
    const int module = qMax(1, tailleVoulue / (n + 8));
    const int taille = module * (n + 8);
    QImage image(taille, taille, QImage::Format_RGB32);
    QPainter p(&image);
    dessinerQR(p, qr, QRectF(0, 0, taille, taille), Qt::black, Qt::white, lignesVisibles);
    return image;
}

// Texte encode dans le QR d'un projet
QString MainWindow::contenuQR(const Projet &p) const
{
    QStringList lignes;
    lignes << tr("DEVENA · Projet du hackathon")
           << tr("%1 : %2").arg(p.id(), p.titre())
           << tr("Thématique : %1").arg(ouTiret(p.thematique()))
           << tr("Statut : %1").arg(ouTiret(p.statut()))
           << tr("Date limite : %1").arg(dateOuTiret(p.dateLimite()));
    if (!p.description().isEmpty())
        lignes << QString() << p.description();
    return lignes.join('\n');
}

// Fenetre "QR code du projet" : le QR se dessine ligne par ligne (scan), puis
// on peut le copier, l'enregistrer en PNG, ou creer le flyer PDF du projet (ou de tous).
void MainWindow::afficherQRProjet(const QString &id)
{
    Projet p;
    QString erreur;
    if (!Projet::charger(id, p, &erreur)) {
        QMessageBox::warning(this, tr("QR code"), erreur);
        return;
    }
    const auto qr = encoderQR(contenuQR(p));
    if (!qr) {
        notifier(Notification::Erreur, tr("QR code impossible"),
                 tr("Les informations de %1 sont trop longues pour un QR code.").arg(id), id);
        return;
    }
    const qrcodegen::QrCode code = *qr;
    const Couleurs c = couleurs();
    const int TAILLE_APERCU = 312;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("QR code du projet %1").arg(p.id()));
    dlg.resize(860, 470);

    auto *titre = new QLabel(tr("QR CODE DU PROJET"), &dlg);
    titre->setProperty("role", "titreDialogue");
    auto *sousTitre = new QLabel(tr("À imprimer sur vos flyers : un scan avec un téléphone affiche "
                                    "les informations du projet."), &dlg);
    sousTitre->setProperty("role", "sousTitre");

    // Le QR, sur une carte blanche (un QR se lit mieux sur fond clair, meme en mode sombre)
    auto *cadreQR = new QFrame(&dlg);
    cadreQR->setObjectName("cadreQR");
    cadreQR->setStyleSheet("QFrame#cadreQR { background: white; border-radius: 12px; }");
    cadreQR->setFixedSize(TAILLE_APERCU + 28, TAILLE_APERCU + 28);
    auto *imageQR = new QLabel(cadreQR);
    imageQR->setAlignment(Qt::AlignCenter);
    auto *lQR = new QVBoxLayout(cadreQR);
    lQR->setContentsMargins(14, 14, 14, 14);
    lQR->addWidget(imageQR);

    // Informations du projet et contenu encode
    auto *cadreInfos = new QFrame(&dlg);
    cadreInfos->setProperty("role", "carte");
    auto *lInfos = new QVBoxLayout(cadreInfos);
    lInfos->setContentsMargins(18, 16, 18, 16);
    lInfos->setSpacing(6);
    auto *labelId = new QLabel(p.id(), cadreInfos);
    labelId->setStyleSheet(QString("color: %1; font-size: 13pt; font-weight: 700;").arg(c.accent.name()));
    auto *labelTitre = new QLabel(p.titre(), cadreInfos);
    labelTitre->setWordWrap(true);
    labelTitre->setStyleSheet("font-size: 15pt; font-weight: 700;");
    auto *labelDetails = new QLabel(tr("Thématique : %1   ·   Statut : %2   ·   Date limite : %3")
                                        .arg(ouTiret(p.thematique()), ouTiret(p.statut()),
                                             dateOuTiret(p.dateLimite())), cadreInfos);
    labelDetails->setProperty("role", "sousTitre");
    labelDetails->setWordWrap(true);
    auto *etiquette = new QLabel(tr("Contenu du QR code"), cadreInfos);
    etiquette->setProperty("role", "etiquette");
    auto *contenu = new QPlainTextEdit(contenuQR(p), cadreInfos);
    contenu->setReadOnly(true);
    auto *version = new QLabel(tr("QR version %1  ·  %2 × %2 modules  ·  correction d'erreurs moyenne ou plus")
                                   .arg(code.getVersion()).arg(code.getSize()), cadreInfos);
    version->setProperty("role", "sousTitre");
    lInfos->addWidget(labelId);
    lInfos->addWidget(labelTitre);
    lInfos->addWidget(labelDetails);
    lInfos->addSpacing(6);
    lInfos->addWidget(etiquette);
    lInfos->addWidget(contenu, 1);
    lInfos->addWidget(version);

    auto *ligneCentre = new QHBoxLayout;
    ligneCentre->setSpacing(18);
    ligneCentre->addWidget(cadreQR, 0, Qt::AlignTop);
    ligneCentre->addWidget(cadreInfos, 1);

    // Boutons
    auto *copier = new QPushButton(tr("Copier l'image"), &dlg);
    auto *png = new QPushButton(tr("Enregistrer en PNG"), &dlg);
    auto *tous = new QPushButton(tr("Flyers de tous les projets"), &dlg);
    auto *flyer = new QPushButton(tr("Créer le flyer (PDF)"), &dlg);
    flyer->setProperty("role", "primaire");
    flyer->setDefault(true);
    auto *fermer = new QPushButton(tr("Fermer"), &dlg);
    auto *ligneBoutons = new QHBoxLayout;
    ligneBoutons->addWidget(copier);
    ligneBoutons->addWidget(png);
    ligneBoutons->addWidget(tous);
    ligneBoutons->addStretch();
    ligneBoutons->addWidget(flyer);
    ligneBoutons->addWidget(fermer);

    auto *racine = new QVBoxLayout(&dlg);
    racine->setContentsMargins(26, 22, 26, 20);
    racine->setSpacing(12);
    racine->addWidget(titre);
    racine->addWidget(sousTitre);
    racine->addLayout(ligneCentre, 1);
    racine->addLayout(ligneBoutons);

    const QString dossier = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    connect(fermer, &QPushButton::clicked, &dlg, &QDialog::accept);

    connect(copier, &QPushButton::clicked, &dlg, [this, code, id] {
        QApplication::clipboard()->setImage(creerImageQR(code, 1024));
        notifier(Notification::Info, tr("QR code copié"),
                 tr("L'image du QR de %1 est prête à coller (Ctrl+V) dans Word, Canva…").arg(id), id);
    });

    connect(png, &QPushButton::clicked, &dlg, [this, &dlg, code, id, dossier] {
        QString chemin = QFileDialog::getSaveFileName(&dlg, tr("Enregistrer le QR code"),
                                                      dossier + "/QR_" + id + ".png",
                                                      tr("Images PNG (*.png)"));
        if (chemin.isEmpty())
            return;
        if (!chemin.endsWith(".png", Qt::CaseInsensitive))
            chemin += ".png";
        // 1200 pixels : assez grand pour une impression nette
        if (creerImageQR(code, 1200).save(chemin, "PNG")) {
            notifier(Notification::Succes, tr("QR code enregistré"),
                     tr("%1 (image 1200 pixels, prête pour l'impression).").arg(QFileInfo(chemin).fileName()),
                     id, true, tr("Ouvrir"),
                     [chemin] { QDesktopServices::openUrl(QUrl::fromLocalFile(chemin)); });
        } else {
            notifier(Notification::Erreur, tr("Enregistrement impossible"),
                     tr("Le fichier %1 n'a pas pu être écrit.").arg(chemin), id);
        }
    });

    connect(flyer, &QPushButton::clicked, &dlg, [this, &dlg, p, dossier] {
        QString chemin = QFileDialog::getSaveFileName(&dlg, tr("Créer le flyer"),
                                                      dossier + "/Flyer_" + p.id() + ".pdf",
                                                      tr("Fichiers PDF (*.pdf)"));
        if (chemin.isEmpty())
            return;
        if (!chemin.endsWith(".pdf", Qt::CaseInsensitive))
            chemin += ".pdf";
        if (exporterFlyers({p}, chemin)) {
            notifier(Notification::Succes, tr("Flyer créé"),
                     tr("%1 : flyer A4 avec QR code.").arg(QFileInfo(chemin).fileName()),
                     p.id(), true, tr("Ouvrir"),
                     [chemin] { QDesktopServices::openUrl(QUrl::fromLocalFile(chemin)); });
        } else {
            notifier(Notification::Erreur, tr("Flyer impossible"),
                     tr("Le fichier %1 n'a pas pu être créé.").arg(chemin), p.id());
        }
    });

    connect(tous, &QPushButton::clicked, &dlg, [this, &dlg, dossier] {
        QString erreurListe;
        const QList<Projet> projets = Projet::lister(QString(), &erreurListe);
        if (projets.isEmpty())
            return;
        QString chemin = QFileDialog::getSaveFileName(&dlg, tr("Flyers de tous les projets"),
                                                      dossier + "/Flyers_Devena.pdf",
                                                      tr("Fichiers PDF (*.pdf)"));
        if (chemin.isEmpty())
            return;
        if (!chemin.endsWith(".pdf", Qt::CaseInsensitive))
            chemin += ".pdf";
        if (exporterFlyers(projets, chemin)) {
            notifier(Notification::Succes, tr("Flyers créés"),
                     tr("%1 : %2 flyer(s), un par page.").arg(QFileInfo(chemin).fileName())
                         .arg(projets.size()),
                     QString(), true, tr("Ouvrir"),
                     [chemin] { QDesktopServices::openUrl(QUrl::fromLocalFile(chemin)); });
        } else {
            notifier(Notification::Erreur, tr("Flyers impossibles"),
                     tr("Le fichier %1 n'a pas pu être créé.").arg(chemin));
        }
    });

    // Animation : le QR se dessine de haut en bas, suivi d'une ligne de scan couleur d'accent
    imageQR->setPixmap(QPixmap::fromImage(creerImageQR(code, TAILLE_APERCU, 0)));
    auto *scan = new QVariantAnimation(&dlg);
    scan->setDuration(1100);
    scan->setStartValue(0);
    scan->setEndValue(code.getSize() + 1);
    scan->setEasingCurve(QEasingCurve::InOutQuad);
    const QColor accent = c.accent;
    connect(scan, &QVariantAnimation::valueChanged, imageQR, [imageQR, code, accent, TAILLE_APERCU](const QVariant &v) {
        const int lignes = v.toInt();
        QImage image = creerImageQR(code, TAILLE_APERCU, lignes);
        if (lignes <= code.getSize()) {
            const int module = image.width() / (code.getSize() + 8);
            const int y = (lignes + 4) * module;
            QPainter peintre(&image);
            peintre.setPen(QPen(accent, 3));
            peintre.drawLine(0, y, image.width(), y);
        }
        imageQR->setPixmap(QPixmap::fromImage(image));
    });
    QTimer::singleShot(250, scan, [scan] { scan->start(); });   // apres le fondu de la fenetre

    animerApparition(&dlg);
    dlg.exec();
}

// Flyers A4 (un projet par page) : bandeau Devena, thematique et statut, titre,
// description, dates, et un grand QR code au centre bas. Le QR est dessine en
// formes vectorielles : il reste net quelle que soit la taille d'impression.
bool MainWindow::exporterFlyers(const QList<Projet> &projets, const QString &chemin)
{
    if (projets.isEmpty())
        return false;

    QPdfWriter pdf(chemin);
    pdf.setResolution(300);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Portrait);
    pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
    pdf.setTitle(projets.size() == 1 ? tr("Flyer %1").arg(projets.first().id()) : tr("Flyers Devena"));
    pdf.setCreator("Devena");

    QPainter p;
    if (!p.begin(&pdf))
        return false;

    const qreal W = pdf.width();
    const qreal H = pdf.height();
    const auto mm = [&pdf](qreal v) { return v * pdf.resolution() / 25.4; };

    // Couleurs du flyer (fond blanc : economique et lisible a l'impression)
    const QColor marine("#060c16");
    const QColor cyanVif("#22d3ee");
    const QColor cyanFonce("#0e7490");
    const QColor encre("#0f172a");
    const QColor gris("#475569");
    const QColor grisClair("#94a3b8");

    QFont police("Segoe UI");
    const auto ecrire = [&](qreal taille, bool gras, const QColor &couleur) {
        police.setPointSizeF(taille);
        police.setBold(gras);
        p.setFont(police);
        p.setPen(couleur);
    };

    for (int i = 0; i < projets.size(); ++i) {
        if (i > 0)
            pdf.newPage();
        const Projet &pr = projets[i];
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::TextAntialiasing, true);

        // === Bandeau du haut ===
        p.fillRect(QRectF(0, 0, W, mm(58)), marine);
        p.fillRect(QRectF(0, mm(58), W, mm(2.5)), cyanVif);

        ecrire(30, true, cyanVif);
        p.drawText(QRectF(mm(18), mm(14), W - mm(36), mm(16)), Qt::AlignLeft | Qt::AlignVCenter,
                   QStringLiteral("◆ Devena"));
        ecrire(12, false, grisClair);
        p.drawText(QRectF(mm(18), mm(32), W - mm(36), mm(8)), Qt::AlignLeft | Qt::AlignVCenter,
                   tr("Connecter les talents, construire l'avenir"));
        ecrire(10.5, true, QColor("#cbd5e1"));
        p.drawText(QRectF(mm(18), mm(15), W - mm(36), mm(7)), Qt::AlignRight | Qt::AlignVCenter,
                   tr("PROJET DU HACKATHON"));
        ecrire(28, true, cyanVif);
        p.drawText(QRectF(mm(18), mm(23), W - mm(36), mm(16)), Qt::AlignRight | Qt::AlignVCenter, pr.id());

        // === Pastilles thematique et statut ===
        qreal y = mm(74);
        qreal x = mm(18);
        ecrire(11, true, cyanFonce);
        const QFontMetricsF mesure(police, &pdf);
        for (const QString &texte : {pr.thematique(), pr.statut()}) {
            if (texte.trimmed().isEmpty())
                continue;
            const QRectF pastille(x, y, mesure.horizontalAdvance(texte) + mm(9), mm(9));
            p.setPen(QPen(cyanFonce, mm(0.35)));
            p.setBrush(QColor("#e0f7fb"));
            p.drawRoundedRect(pastille, mm(2), mm(2));
            p.setPen(cyanFonce);
            p.drawText(pastille, Qt::AlignCenter, texte);
            x = pastille.right() + mm(4);
        }
        p.setBrush(Qt::NoBrush);
        y += mm(16);

        // === Titre ===
        ecrire(30, true, encre);
        const QRectF zoneTitre(mm(18), y, W - mm(36), mm(42));
        const QRectF titreOccupe = p.boundingRect(zoneTitre, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop,
                                                  pr.titre());
        p.drawText(zoneTitre, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, pr.titre());
        y = qMin(titreOccupe.bottom(), zoneTitre.bottom()) + mm(6);

        // Position du bloc QR (en bas) : la description s'arrete au dessus
        const qreal tailleQR = mm(72);
        const qreal basQR = H - mm(16) - mm(22);
        const QRectF zoneQR((W - tailleQR) / 2, basQR - tailleQR, tailleQR, tailleQR);
        const QRectF carteQR = zoneQR.adjusted(-mm(5), -mm(5), mm(5), mm(5));
        const qreal yDates = carteQR.top() - mm(16);

        // === Description ===
        ecrire(13, false, gris);
        p.drawText(QRectF(mm(18), y, W - mm(36), qMax<qreal>(0, yDates - mm(5) - y)),
                   Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, pr.description());

        // === Dates ===
        p.setPen(QPen(QColor("#e2e8f0"), mm(0.3)));
        p.drawLine(QPointF(mm(18), yDates - mm(2)), QPointF(W - mm(18), yDates - mm(2)));
        ecrire(12, false, gris);
        p.drawText(QRectF(mm(18), yDates, W - mm(36), mm(8)), Qt::AlignLeft | Qt::AlignVCenter,
                   tr("Créé le %1").arg(dateOuTiret(pr.dateCreation())));
        ecrire(12, true, encre);
        p.drawText(QRectF(mm(18), yDates, W - mm(36), mm(8)), Qt::AlignRight | Qt::AlignVCenter,
                   tr("Date limite : %1").arg(dateOuTiret(pr.dateLimite())));

        // === QR code ===
        p.setPen(QPen(cyanVif, mm(0.6)));
        p.setBrush(Qt::white);
        p.drawRoundedRect(carteQR, mm(4), mm(4));
        p.setBrush(Qt::NoBrush);
        const auto qr = encoderQR(contenuQR(pr));
        if (qr) {
            p.setRenderHint(QPainter::Antialiasing, false);   // bords francs : meilleure lecture
            dessinerQR(p, *qr, zoneQR, marine, Qt::white, -1, 1.0);
            p.setRenderHint(QPainter::Antialiasing, true);
        } else {
            ecrire(12, false, gris);
            p.drawText(zoneQR, Qt::AlignCenter | Qt::TextWordWrap, tr("QR code indisponible"));
        }
        ecrire(14, true, encre);
        p.drawText(QRectF(0, carteQR.bottom() + mm(4), W, mm(8)), Qt::AlignHCenter | Qt::AlignVCenter,
                   tr("Scannez pour découvrir le projet"));
        ecrire(10, false, grisClair);
        p.drawText(QRectF(0, carteQR.bottom() + mm(11), W, mm(6)), Qt::AlignHCenter | Qt::AlignVCenter,
                   tr("Visez le code avec l'appareil photo de votre téléphone"));

        // === Bandeau du bas ===
        p.fillRect(QRectF(0, H - mm(16), W, mm(16)), marine);
        ecrire(11, true, cyanVif);
        p.drawText(QRectF(0, H - mm(16), W, mm(16)), Qt::AlignCenter,
                   QStringLiteral("Hack today  ·  Build tomorrow"));
    }

    return p.end();
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
    ui->graphique->update();
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
    if (m_sombre)
        return {QColor("#060c16"), QColor("#09121f"), QColor("#0a1524"), QColor("#1b2b40"),
                QColor("#e2e8f0"), QColor("#8b97a8"), QColor("#22d3ee"), QColor("#03141b"),
                QColor("#0c2b36"), QColor("#f87171")};
    return {QColor("#f3f6fa"), QColor("#ffffff"), QColor("#ffffff"), QColor("#d5dde8"),
            QColor("#0f172a"), QColor("#64748b"), QColor("#0891b2"), QColor("#ffffff"),
            QColor("#e0f7fb"), QColor("#dc2626")};
}

// Feuille de style Qt (QSS) : les mots @xxx sont remplaces par les couleurs du theme
QString MainWindow::feuilleDeStyle() const
{
    const Couleurs c = couleurs();

    QString qss = R"(
QMainWindow, QDialog, QMessageBox, #contenuProjets, QStackedWidget > QWidget {
    background: @fond;
}
QWidget { color: @texte; font-family: "Segoe UI"; font-size: 10pt; }
QLabel { background: transparent; }
QToolTip { background: @carte; color: @texte; border: 1px solid @bordure; }

/* Barre laterale */
#sidebar { background: @panneau; border-right: 1px solid @bordure; }
#labelLogo { color: @accent; font-size: 17pt; }
#labelSlogan, #labelPied { color: @discret; font-size: 9pt; }
QPushButton[role="menu"] {
    text-align: left; padding: 11px 14px; background: transparent;
    border: 1px solid transparent; border-radius: 6px; color: @discret;
}
QPushButton[role="menu"]:hover { color: @texte; }
QPushButton[role="menu"]:checked { background: @badge; border: 1px solid @accent; color: @texte; }

/* Textes */
#labelTitrePage { font-size: 20pt; }
QLabel[role="sousTitre"] { color: @discret; }
QLabel[role="section"] { font-size: 12pt; }
QLabel[role="valeurStat"] { color: @accent; font-size: 18pt; }
QLabel[role="titreDialogue"] { color: @accent; font-size: 14pt; font-weight: 700; }
QLabel[role="gras"] { font-weight: 600; }
QLabel[role="etiquette"] { color: @accent; }
QLabel[role="badge"] {
    background: @badge; color: @accent; border-radius: 6px; padding: 3px 9px;
}

/* Boutons */
QPushButton {
    background: @carte; border: 1px solid @bordure; border-radius: 6px;
    padding: 8px 14px;
}
QPushButton:hover { border-color: @accent; }
QPushButton[role="primaire"] {
    background: @accent; color: @accentTexte; border: 1px solid @accent; font-weight: 600;
}
QPushButton[role="lien"] { background: transparent; border: none; color: @discret; }
QPushButton[role="lien"]:hover { color: @accent; }
QPushButton[role="lien"]:disabled { color: @bordure; }
QToolButton[role="action"] {
    background: transparent; border: none; color: @discret; font-size: 12pt; padding: 2px 4px;
}
QToolButton[role="action"]:hover { color: @accent; }
QToolButton[role="danger"] {
    background: transparent; border: none; color: @discret; font-size: 12pt; padding: 2px 4px;
}
QToolButton[role="danger"]:hover { color: @danger; }

/* Champs */
QLineEdit, QPlainTextEdit, QComboBox, QDateEdit {
    background: @panneau; border: 1px solid @bordure; border-radius: 6px; padding: 7px 9px;
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
    padding: 12px 8px; font-weight: 600; font-size: 9pt;
}
QTableCornerButton::section { background: @carte; border: none; }

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
