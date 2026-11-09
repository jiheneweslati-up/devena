#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    appliquerTheme();
    rafraichir();
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
    const int largeurs[] = {95, 150, 0, 120, 130, 140, 120, 130};
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
        connect(creer(QStringLiteral("🗑"), tr("Supprimer"), "danger"), &QToolButton::clicked,
                this, [this, id] { supprimerProjet(id); });

        t->setCellWidget(i, ColActions, actions);
    }
}

void MainWindow::rafraichir()
{
    QString erreur;
    const QList<Projet> projets = Projet::lister(ui->lineEditRecherche->text(), &erreur);
    if (!erreur.isEmpty())
        QMessageBox::critical(this, tr("Base de données"),
                              tr("Impossible de lire les projets.\n%1").arg(erreur));
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
        return;
    }
    empiler({Action::Ajout, Projet(), p});
    rafraichir();
    QMessageBox::information(this, tr("Ajout"), tr("Projet %1 ajouté.").arg(p.id()));
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

    QMessageBox::information(this, tr("Informations du projet"), texte);
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
        return;
    }
    empiler({Action::Modification, avant, apres});
    rafraichir();
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

    const QString question = tr("Supprimer le projet %1 « %2 » ?").arg(avant.id(), avant.titre());
    if (QMessageBox::question(this, tr("Suppression"), question) != QMessageBox::Yes)
        return;

    if (!Projet::supprimer(id, &erreur)) {
        QMessageBox::critical(this, tr("Suppression"),
                              tr("Échec de la suppression.\n%1").arg(erreur));
        return;
    }
    empiler({Action::Suppression, avant, Projet()});
    rafraichir();
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
        return;
    }
    rafraichir();
    QMessageBox::information(this, tr("Annuler"), message);
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

    QMessageBox::information(this, tr("Exporter en PDF"),
                             tr("PDF enregistré :\n%1").arg(chemin));
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
    ui->labelNbProjets->setText(QString::number(total));
    ui->labelNbEnCours->setText(QString::number(enCours));
    ui->labelNbTermines->setText(QString::number(termines));
    ui->labelNbEnRetard->setText(QString::number(enRetard));

    m_statsThematique.clear();
    if (q.exec("SELECT NVL(THEMATIQUE, '-'), COUNT(*) FROM PROJETS "
               "GROUP BY THEMATIQUE ORDER BY 1")) {
        while (q.next())
            m_statsThematique.append({q.value(0).toString(), q.value(1).toInt()});
    }
    ui->graphique->update();   // redessine la courbe
}

// Quand ui->graphique doit se redessiner, on dessine la courbe nous memes
bool MainWindow::eventFilter(QObject *objet, QEvent *evenement)
{
    if (objet == ui->graphique && evenement->type() == QEvent::Paint) {
        dessinerGraphique();
        return true;
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

    // Courbe
    QPainterPath chemin(points.first());
    for (int i = 1; i < n; ++i)
        chemin.lineTo(points[i]);
    p.setPen(QPen(c.accent, 2.5));
    p.setBrush(Qt::NoBrush);
    p.drawPath(chemin);

    // Points, valeurs au dessus, thematiques en dessous
    QFont police = ui->graphique->font();
    police.setPointSizeF(police.pointSizeF() * 0.9);
    p.setFont(police);
    for (int i = 0; i < n; ++i) {
        p.setPen(Qt::NoPen);
        p.setBrush(c.accent);
        p.drawEllipse(points[i], 5, 5);

        p.setPen(c.discret);
        p.drawText(QRectF(points[i].x() - 40, points[i].y() - 26, 80, 18),
                   Qt::AlignCenter, QString::number(m_statsThematique[i].second));
        p.drawText(QRectF(points[i].x() - 60, zone.bottom() + 6, 120, 18),
                   Qt::AlignCenter, m_statsThematique[i].first);
    }
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
