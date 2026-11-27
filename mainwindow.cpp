#include "mainwindow.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QStringList>
#include <QFont>
#include <QColor>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QPointF>
#include <QtMath>
#include <QVector>
#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <QBrush>
#include <QDateEdit>
#include <QDate>
#include <QDebug>
#include <QScrollBar>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDateTime>
#include <QFormLayout>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QMap>
#include <QSet>
#include <QLayout>
#include <QLayoutItem>
#include <algorithm>
#include <functional>
#include <QScrollArea>
#include <QProgressBar>
#include <QPainter>
#include <QProgressBar>
#include <QImage>
#include <QByteArray>
#include <QVector>
/* =====================================================
   STYLE DARK
   ===================================================== */

static const char *STYLE_DARK = R"(
QWidget {
    background: #050B14;
    color: #EAF2FF;
    font-family: "Segoe UI";
    font-size: 13px;
}

QMainWindow {
    background: #050B14;
}

QPushButton {
    background: #0B1726;
    color: #DCE7F4;
    border: 1px solid #243A52;
    border-radius: 8px;
    padding: 8px 12px;
    font-weight: bold;
}

QPushButton:hover {
    background: #10283D;
    border-color: #18D9EF;
}

QPushButton#primary {
    background: #18D9EF;
    color: #03212A;
    border-color: #18D9EF;
}

QLineEdit,
QComboBox {
    background: #091522;
    color: white;
    border: 1px solid #243A52;
    border-radius: 8px;
    padding: 9px;
    font-family: "Segoe UI";
    font-size: 14px;
}

QLineEdit:focus,
QComboBox:focus {
    border: 1px solid #18D9EF;
}

QComboBox QAbstractItemView {
    background: #091522;
    color: white;
    selection-background-color: #10394B;
    selection-color: white;
}

QTableWidget {
    background: #081421;
    alternate-background-color: #0B1828;
    border: 1px solid #1A3046;
    gridline-color: #15283B;
    border-radius: 10px;
}

QTableWidget::item {
    padding: 8px;
}

QTableWidget::item:selected {
    background: #0C3042;
}

QHeaderView::section {
    background: #091522;
    color: #18D9EF;
    border: 0;
    padding: 11px;
    font-weight: bold;
}

QFrame#statCard {
    background: #091522;
    border: 1px solid #1C3045;
    border-radius: 10px;
}

QPushButton#iconButton {
    font-size: 17px;
    padding: 2px;
}
)";

/* =====================================================
   STYLE LIGHT
   ===================================================== */

static const char *STYLE_LIGHT = R"(
QWidget {
    background: #F4F7FB;
    color: #182433;
    font-family: "Segoe UI";
    font-size: 13px;
}

QMainWindow {
    background: #F4F7FB;
}

QPushButton {
    background: #FFFFFF;
    color: #243447;
    border: 1px solid #C8D3E0;
    border-radius: 8px;
    padding: 8px 12px;
    font-weight: bold;
}

QPushButton:hover {
    background: #EAFBFD;
    border-color: #18BFD4;
}

QPushButton#primary {
    background: #18D9EF;
    color: #03212A;
    border-color: #18D9EF;
}

// Style des champs de saisie
QLineEdit,
QComboBox {
    background: #FFFFFF;
    color: #182433;
    border: 1px solid #C8D3E0;
    border-radius: 8px;
    padding: 9px;
    font-family: "Segoe UI";
    font-size: 14px;
}

QLineEdit:focus,
QComboBox:focus {
    border: 1px solid #18BFD4;
}

QComboBox QAbstractItemView {
    background: #FFFFFF;
    color: #182433;
    selection-background-color: #DDF7FA;
    selection-color: #182433;
}
QTableWidget {
    background: #FFFFFF;
    alternate-background-color: #F1F6FA;
    color: #182433;
    border: 1px solid #D3DDE8;
    gridline-color: #DCE4EC;
    border-radius: 10px;
}

QTableWidget::item {
    padding: 8px;
}

QTableWidget::item:selected {
    background: #DDF7FA;
    color: #182433;
}

QHeaderView::section {
    background: #EAF1F7;
    color: #087F91;
    border: 0;
    padding: 11px;
    font-weight: bold;
}

QFrame#statCard {
    background: #FFFFFF;
    border: 1px solid #D3DDE8;
    border-radius: 10px;
}

QPushButton#iconButton {
    font-size: 17px;
    padding: 2px;
}
)";

/* =====================================================
   SCORE
   ===================================================== */

double scoreEquipe(const Equipe &e, const Participant &p)
{
    if (e.pleine())
        return -1000000.0;

    double score = (6 - e.taille()) * 6.0;

    int h = e.hommes();
    int f = e.femmes();

    if (p.sexe.compare("Homme", Qt::CaseInsensitive) == 0)
        ++h;

    if (p.sexe.compare("Femme", Qt::CaseInsensitive) == 0)
        ++f;

    int diffSexe = qAbs(h - f);

    if (diffSexe == 0)
        score += 40;
    else if (diffSexe == 1)
        score += 30;
    else if (diffSexe == 2)
        score += 15;
    else
        score -= 20;

    if (e.aCompetence(p.competence))
        score -= 10;
    else
        score += 35;

    if (!e.membres().isEmpty())
    {
        double diffAge = qAbs(e.ageMoyen() - p.age);

        if (diffAge <= 2)
            score += 25;
        else if (diffAge <= 5)
            score += 15;
        else
            score -= diffAge;
    }

    return score;
}

/* =====================================================
   CONSTRUCTEUR
   ===================================================== */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_table(nullptr),
    m_recherche(nullptr),
    m_filtre(nullptr),
    m_notification(nullptr),
    m_courbe(nullptr),
    m_totalEquipes(nullptr),
    m_totalMembres(nullptr),
    m_totalHommes(nullptr),
    m_totalFemmes(nullptr),
    m_side(nullptr),
    m_modeClair(false)
{
    resize(1400, 900);

    setWindowTitle("Devena - Gestion des equipes");

    setStyleSheet(STYLE_DARK);

    creerInterface();

    rafraichir();
}

MainWindow::~MainWindow()
{
}
/* =====================================================
   INTERFACE
   ===================================================== */

void MainWindow::creerInterface()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    /* ================= SIDEBAR ================= */

    m_side = new QWidget;
    m_side->setFixedWidth(220);

    m_side->setStyleSheet(
        "QWidget{"
        "background:#030914;"
        "border-right:1px solid #17283A;"
        "}"
        "QPushButton{"
        "text-align:left;"
        "border:0;"
        "background:transparent;"
        "color:#718198;"
        "padding:12px;"
        "}"
        "QPushButton#active{"
        "background:#092A3D;"
        "color:#18D9EF;"
        "border:1px solid #18D9EF;"
        "}"
        );

    QVBoxLayout *sl = new QVBoxLayout(m_side);
    sl->setContentsMargins(15, 22, 15, 18);

    QLabel *logo = new QLabel("◆ Devena");
    logo->setObjectName("logoLabel");
    logo->setStyleSheet(
        "color:#18D9EF;"
        "font-size:27px;"
        "font-weight:bold;"
        );

    sl->addWidget(logo);

    QLabel *slogan = new QLabel("Connecter les talents,\nconstruire l'avenir");
    slogan->setObjectName("sloganLabel");
    slogan->setStyleSheet("color:#718198;");

    sl->addWidget(slogan);
    sl->addSpacing(22);

    QStringList menu;
    menu
        << "Tableau de bord"
        << "Employes"
        << "Participants"
        << "Equipe"
        << "Projet"
        << "Evaluation"
        << "Parametre";

    for (int i = 0; i < menu.size(); ++i)
    {
        QPushButton *b = new QPushButton(menu[i]);

        if (i == 3)
        {
            b->setObjectName("active");
        }
        else
        {
            b->setEnabled(false);
        }

        sl->addWidget(b);
    }

    sl->addStretch();

    QLabel *footer = new QLabel("Hack today\nBuild tomorrow");
    footer->setObjectName("footerLabel");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color:#3A4D65;");

    sl->addWidget(footer);
    root->addWidget(m_side);

    /* ================= CORPS ================= */

    QWidget *body = new QWidget;
    QVBoxLayout *v = new QVBoxLayout(body);
    v->setContentsMargins(25, 20, 25, 18);

    /* ================= HEADER ================= */

    QHBoxLayout *header = new QHBoxLayout;
    QVBoxLayout *titles = new QVBoxLayout;

    QLabel *title = new QLabel("Equipes");
    title->setObjectName("titleLabel");
    title->setStyleSheet(
        "font-size:30px;"
        "font-weight:bold;"
        "color:white;"
        );

    QLabel *subtitle = new QLabel("Gestion moderne et intelligente des equipes");
    subtitle->setObjectName("subtitleLabel");
    subtitle->setStyleSheet("color:#7D8BA3;");

    titles->addWidget(title);
    titles->addWidget(subtitle);

    header->addLayout(titles);
    header->addStretch();

    QLabel *user = new QLabel("Alaa\nParticipant");
    user->setObjectName("userLabel");

    header->addWidget(user);
    v->addLayout(header);

    /* ================= ACTIONS (Boutons 3la l'imin) ================= */

    QHBoxLayout *actions = new QHBoxLayout;

    QPushButton *addTeam = new QPushButton("+ Ajouter equipe");
    QPushButton *automatic = new QPushButton("⚡ Affectation automatique");
    QPushButton *exportPDF = new QPushButton("📄 Exporter PDF");
    QPushButton *lightMode = new QPushButton("☀ Mode clair");

    addTeam->setObjectName("primary");
    automatic->setObjectName("primary");

    exportPDF->setMinimumHeight(38);
    lightMode->setMinimumHeight(38);

    actions->addStretch();
    actions->addWidget(addTeam);
    actions->addWidget(automatic);
    actions->addWidget(exportPDF);
    actions->addWidget(lightMode);

    v->addLayout(actions);

    connect(addTeam, &QPushButton::clicked, this, [this]() { ajouterEquipe(); });
    connect(automatic, &QPushButton::clicked, this, [this]() { affecterAutomatiquement(); });
    connect(exportPDF, &QPushButton::clicked, this, [this]() { exporterPDF(); });
    connect(lightMode, &QPushButton::clicked, this, [this]() { changerModeClair(); });

    /* ================= NOTIFICATION ================= */

    m_notification = new QLabel;
    m_notification->setAlignment(Qt::AlignCenter);
    m_notification->hide();

    v->addWidget(m_notification);

    /* ================= RECHERCHE ================= */

    QHBoxLayout *filters = new QHBoxLayout;

    m_recherche = new QLineEdit;
    m_recherche->setPlaceholderText("🔎 Rechercher une equipe...");

    m_filtre = new QComboBox;
    m_filtre->addItems(
        QStringList()
        << "Toutes les equipes"
        << "Equipe avec membres"
        << "Equipe vide"
        << "Equipe complete"
        );
    // ================= TRI =================

    m_tri = new QComboBox;
    m_tri->addItem("Trier par ID");
    m_tri->addItem("Trier par Nom");
    m_tri->addItem("Trier par âge moyen");

    m_ordreTri = new QPushButton("↑ Croissant");
    m_ordreTri->setCursor(Qt::PointingHandCursor);

    // Ajouter les widgets dans la même ligne
    filters->addWidget(m_recherche, 1);
    filters->addWidget(m_filtre);
    filters->addWidget(m_tri);
    filters->addWidget(m_ordreTri);

    v->addLayout(filters);

    // Connexion recherche
    connect(m_recherche, &QLineEdit::textChanged, this,
            [this](const QString &) {
                filtrer();
            });

    // Connexion filtre
    connect(m_filtre, &QComboBox::currentTextChanged, this,
            [this](const QString &) {
                filtrer();
            });

    // Connexion du tri
    connect(m_tri, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) {
                trierEquipes();
            });

    // Connexion Croissant / Décroissant
    connect(m_ordreTri, &QPushButton::clicked, this, [this]() {

        m_triCroissant = !m_triCroissant;

        m_ordreTri->setText(
            m_triCroissant ? "↑ Croissant" : "↓ Décroissant"
            );

        trierEquipes();
    });

    /* ================= TABLE ================= */

    QLabel *tableTitle = new QLabel("Tableau des equipes");
    tableTitle->setObjectName("tableTitleLabel");
    tableTitle->setStyleSheet(
        "font-size:19px;"
        "font-weight:bold;"
        "color:white;"
        );

    v->addWidget(tableTitle);
    m_table = new QTableWidget;
    m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table->setColumnCount(9);

    m_table->setHorizontalHeaderLabels(
        QStringList()
        << "ID Equipe"
        << "Nom Equipe"
        << "Nb_membre"
        << "Capacite_max"
        << "Sexe"
        << "Age"
        << "Competence"
        << "Date_creation"
        << "Actions"
        );
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setColumnWidth(7, 120); // Date de creation
    m_table->setColumnWidth(8, 300); // Actions
    m_table->setMinimumHeight(220);
    m_table->setMaximumHeight(280);
    m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    v->addWidget(m_table);
    /* ================= ORGANIGRAMME ================= */

    QLabel *statsTitle = new QLabel("Organigramme des equipes");
    statsTitle->setObjectName("statsTitleLabel");
    statsTitle->setStyleSheet(
        "font-size:20px;"
        "font-weight:bold;"
        "color:white;"
        );

    v->addWidget(statsTitle);

    m_organigramme = new QWidget;
    m_organigramme->setMinimumHeight(250);
    m_organigramme->setMaximumHeight(280);

    QVBoxLayout *orgLayout = new QVBoxLayout(m_organigramme);
    orgLayout->setContentsMargins(5, 5, 5, 5);

    QScrollArea *orgScroll = new QScrollArea;
    orgScroll->setWidgetResizable(true);
    orgScroll->setFrameShape(QFrame::NoFrame);
    orgScroll->setWidget(m_organigramme);
    orgScroll->setMinimumHeight(260);
    orgScroll->setMaximumHeight(280);
    v->addWidget(orgScroll, 1);

    root->addWidget(body, 1);
}

/* =====================================================
   IDS
   ===================================================== */

int MainWindow::prochainIdEquipe() const
{
    int id = 0;
    for (const Equipe &e : m_equipes)
        id = qMax(id, e.id());
    return id + 1;
}

int MainWindow::prochainIdParticipant() const
{
    int id = 0;
    for (const Participant &p : m_participants)
        id = qMax(id, p.id);
    return id + 1;
}

/* =====================================================
   MEILLEURE EQUIPE
   ===================================================== */

int MainWindow::meilleureEquipe(const Participant &p) const
{
    int bestIndex = -1;
    double bestScore = -1000000.0;

    for (int i = 0; i < m_equipes.size(); ++i)
    {
        double score = scoreEquipe(m_equipes[i], p);
        if (score > bestScore)
        {
            bestScore = score;
            bestIndex = i;
        }
    }

    return bestIndex;
}

/* =====================================================
   AFFICHER NOTIFICATION
   ===================================================== */

void MainWindow::afficherNotification(const QString &texte, bool succes)
{
    m_notification->setText(texte);
    m_notification->setStyleSheet(
        succes
            ? "background:#0A3A30;"
              "color:#67F0B5;"
              "border:1px solid #1BAA7A;"
              "border-radius:8px;"
              "padding:7px;"
              "font-weight:bold;"

            : "background:#421F28;"
              "color:#FF8C99;"
              "border:1px solid #C44858;"
              "border-radius:8px;"
              "padding:7px;"
              "font-weight:bold;"
        );

    m_notification->show();
    QTimer::singleShot(4000, m_notification, &QLabel::hide);
}

/* =====================================================
   AFFECTATION
   ===================================================== */

void MainWindow::affecterAutomatiquement(){
    for (Equipe &e : m_equipes)
        e.vider();

    for (const Participant &p : m_participants)
    {
        int i = meilleureEquipe(p);
        if (i >= 0)
            m_equipes[i].ajouter(p);
    }

    rafraichir();
    afficherNotification("Affectation automatique terminee.");
}
// =====================================================
// POPUP MODIFIER EQUIPE : FORMULAIRE VERTICAL
// =====================================================

bool ouvrirPopupGestionEquipe(
    QWidget *parent,
    QTableWidget *tablePrincipal,
    const Equipe *equipe,
    int idEquipe,
    int prochainId,
    QString &nomEquipe,
    QVector<Participant> &membres,
    bool lectureSeule)
{
    Q_UNUSED(prochainId);

    QDialog dialog(parent);

    dialog.setWindowTitle("Modifier equipe");
    dialog.setFixedSize(780, 690);

    dialog.setStyleSheet(
        "QDialog {"
        " background-color: #050B14;"
        "}"
        "QLabel {"
        " color: #C7D3E0;"
        " font-family: 'Segoe UI';"
        " font-size: 13px;"
        "}"
        "QLabel#titrePrincipal {"
        " color: #18D9EF;"
        " font-size: 21px;"
        " font-weight: bold;"
        "}"
        "QLabel#sousTitre {"
        " color: #8193A8;"
        " font-size: 12px;"
        "}"
        "QLineEdit {"
        " color: white;"
        " background-color: #07111D;"
        " border: 1px solid #1C3448;"
        " border-radius: 7px;"
        " padding: 8px 10px;"
        " font-size: 13px;"
        " min-height: 20px;"
        "}"
        "QLineEdit:focus {"
        " border: 1px solid #18D9EF;"
        "}"
        "QLineEdit:read-only {"
        " color: #91A3B8;"
        " background-color: #091522;"
        " border: 1px solid #142A3C;"
        "}"
        "QPushButton {"
        " padding: 10px 24px;"
        " border-radius: 7px;"
        " font-size: 13px;"
        " font-weight: bold;"
        "}"
        "QPushButton#primary {"
        " background-color: #18D9EF;"
        " color: #050B14;"
        " border: none;"
        "}"
        "QPushButton#primary:hover {"
        " background-color: #45E6F5;"
        "}"
        "QPushButton#secondary {"
        " background-color: #142334;"
        " color: #D5E0EC;"
        " border: 1px solid #263E53;"
        "}"
        "QPushButton#secondary:hover {"
        " background-color: #20364A;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    layout->setContentsMargins(35, 25, 35, 25);
    layout->setSpacing(12);

    // ===== TITRE =====

    QLabel *titre = new QLabel("MODIFIER UNE EQUIPE");
    titre->setObjectName("titrePrincipal");

    QLabel *sousTitre = new QLabel(
        "Modifiez les informations de votre equipe."
        );

    sousTitre->setObjectName("sousTitre");

    layout->addWidget(titre);
    layout->addWidget(sousTitre);

    // ===== INFORMATIONS DE L'EQUIPE =====

    QLabel *titreInfos = new QLabel(
        "Informations de l'equipe"
        );

    titreInfos->setStyleSheet(
        "font-size:15px;"
        "font-weight:bold;"
        "color:#18D9EF;"
        );

    layout->addSpacing(5);
    layout->addWidget(titreInfos);

    // ===== RECHERCHER LA LIGNE DE L'EQUIPE =====

    int ligneEquipe = -1;

    if (tablePrincipal)
    {
        QString idRecherche =
            "E" + QString::number(idEquipe);

        for (int i = 0; i < tablePrincipal->rowCount(); ++i)
        {
            QTableWidgetItem *item =
                tablePrincipal->item(i, 0);

            if (item &&
                item->text().trimmed().compare(
                    idRecherche,
                    Qt::CaseInsensitive
                    ) == 0)
            {
                ligneEquipe = i;
                break;
            }
        }
    }

    // Récupération des valeurs du tableau principal

    auto valeurColonne = [&](int colonne,
                             const QString &valeurDefaut)
    {
        if (ligneEquipe >= 0 &&
            tablePrincipal &&
            tablePrincipal->item(ligneEquipe, colonne))
        {
            QString valeur =
                tablePrincipal->item(ligneEquipe, colonne)
                    ->text().trimmed();

            if (!valeur.isEmpty())
                return valeur;
        }

        return valeurDefaut;
    };

    // ===== CREATION DES CHAMPS =====

    auto creerChamp = [](const QString &valeur,
                         bool modifiable = false)
    {
        QLineEdit *champ = new QLineEdit(valeur);

        champ->setReadOnly(!modifiable);
        champ->setMinimumHeight(36);

        return champ;
    };

    QLineEdit *champID = creerChamp(
        "E" + QString::number(idEquipe)
        );

    QLineEdit *champNom = creerChamp(
        equipe ? equipe->nom() : "",
        equipe != nullptr && !lectureSeule
        );

    champNom->setPlaceholderText(
        "Nom de l'equipe"
        );

    QLineEdit *champMembres = creerChamp(
        valeurColonne(
            2,
            equipe ? QString::number(equipe->taille()) : "0"
            )
        );

    QLineEdit *champCapacite = creerChamp("6");

    QLineEdit *champSexe = creerChamp(
        valeurColonne(
            4,
            equipe
                ? QString("%1 H / %2 F")
                      .arg(equipe->hommes())
                      .arg(equipe->femmes())
                : "-"
            )
        );

    QLineEdit *champAge = creerChamp(
        valeurColonne(
            5,
            equipe
                ? QString::number(equipe->ageMoyen(), 'f', 1)
                : "-"
            )
        );

    QLineEdit *champCompetences = creerChamp(
        valeurColonne(6, "-")
        );

    QLineEdit *champDate = creerChamp(
        valeurColonne(7, "-")
        );

    QLineEdit *champProjet = creerChamp(
        equipe
            ? QString::number(equipe->idProjet())
            : "-"
        );

    QLineEdit *champEvaluation = creerChamp(
        equipe
            ? QString::number(equipe->idEvaluation())
            : "-"
        );

    // ===== DISPOSITION VERTICALE =====

    QFormLayout *form = new QFormLayout();

    form->setHorizontalSpacing(25);
    form->setVerticalSpacing(9);

    form->setLabelAlignment(
        Qt::AlignLeft | Qt::AlignVCenter
        );

    form->setFieldGrowthPolicy(
        QFormLayout::ExpandingFieldsGrow
        );

    form->addRow("ID Equipe", champID);
    form->addRow("Nom Equipe", champNom);
    form->addRow("Nombre de membres", champMembres);
    form->addRow("Capacite maximale", champCapacite);
    form->addRow("Repartition des sexes", champSexe);
    form->addRow("Age moyen", champAge);
    form->addRow("Competences", champCompetences);
    form->addRow("Date de creation", champDate);
    form->addRow("ID Projet", champProjet);
    form->addRow("ID Evaluation", champEvaluation);

    layout->addLayout(form, 1);

    // ===== BOUTONS =====

    QHBoxLayout *boutons = new QHBoxLayout();

    QPushButton *enregistrer =
        new QPushButton(lectureSeule ? "Fermer" : "Enregistrer");

    QPushButton *annuler = new QPushButton("Annuler");

    enregistrer->setObjectName("primary");
    annuler->setObjectName("secondary");

    enregistrer->setDefault(true);
    enregistrer->setAutoDefault(true);
    annuler->setAutoDefault(false);

    boutons->addStretch();
    boutons->addWidget(enregistrer);
    boutons->addWidget(annuler);

    layout->addLayout(boutons);

    // ===== VALIDATION =====

    QObject::connect(
        enregistrer,
        &QPushButton::clicked,
        &dialog,
        [&]()
        {
            if (lectureSeule)
            {
                dialog.accept();
                return;
            }

            QString nom = champNom->text().trimmed();

            if (nom.isEmpty())
            {
                QMessageBox::warning(
                    &dialog,
                    "Erreur",
                    "Le nom de l'equipe est obligatoire."
                    );

                champNom->setFocus();
                return;
            }

            nomEquipe = nom;

            if (equipe)
                membres = equipe->membres();
            else
                membres.clear();

            dialog.accept();
        }
        );

    // ===== ANNULER =====

    QObject::connect(
        annuler,
        &QPushButton::clicked,
        &dialog,
        &QDialog::reject
        );

    if (lectureSeule)
        annuler->hide();

    champNom->setFocus();

    return dialog.exec() == QDialog::Accepted;
}
// =====================================================
// POPUP AJOUT EQUIPE : FORMULAIRE VERTICAL
// =====================================================

bool ouvrirPopupAjoutEquipe(
    QWidget *parent,
    int idEquipe,
    QString &nomEquipe,
    QVector<Participant> &membres)
{
    QDialog dialog(parent);

    dialog.setWindowTitle("Ajouter equipe");
    dialog.setFixedSize(780, 690);

    dialog.setStyleSheet(
        "QDialog {"
        " background-color: #050B14;"
        "}"
        "QLabel {"
        " color: #C7D3E0;"
        " font-family: 'Segoe UI';"
        " font-size: 13px;"
        "}"
        "QLabel#titrePrincipal {"
        " color: #18D9EF;"
        " font-size: 21px;"
        " font-weight: bold;"
        "}"
        "QLabel#sousTitre {"
        " color: #8193A8;"
        " font-size: 12px;"
        "}"
        "QLineEdit {"
        " color: white;"
        " background-color: #07111D;"
        " border: 1px solid #1C3448;"
        " border-radius: 7px;"
        " padding: 8px 10px;"
        " font-size: 13px;"
        " min-height: 20px;"
        "}"
        "QLineEdit:focus {"
        " border: 1px solid #18D9EF;"
        "}"
        "QLineEdit[readOnly=\"true\"] {"
        " color: #91A3B8;"
        " background-color: #091522;"
        " border: 1px solid #142A3C;"
        "}"
        "QPushButton {"
        " padding: 10px 24px;"
        " border-radius: 7px;"
        " font-size: 13px;"
        " font-weight: bold;"
        "}"
        "QPushButton#primary {"
        " background-color: #18D9EF;"
        " color: #050B14;"
        " border: none;"
        "}"
        "QPushButton#primary:hover {"
        " background-color: #45E6F5;"
        "}"
        "QPushButton#secondary {"
        " background-color: #142334;"
        " color: #D5E0EC;"
        " border: 1px solid #263E53;"
        "}"
        "QPushButton#secondary:hover {"
        " background-color: #20364A;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    layout->setContentsMargins(35, 25, 35, 25);
    layout->setSpacing(12);

    // ===== TITRE =====

    QLabel *titre = new QLabel("AJOUTER UNE EQUIPE");
    titre->setObjectName("titrePrincipal");

    QLabel *sousTitre = new QLabel(
        "Remplissez les informations de votre equipe."
        );

    sousTitre->setObjectName("sousTitre");

    layout->addWidget(titre);
    layout->addWidget(sousTitre);

    // ===== INFORMATIONS DE L'EQUIPE =====

    QLabel *titreInfos = new QLabel("Informations de l'equipe");
    titreInfos->setStyleSheet(
        "font-size:15px;"
        "font-weight:bold;"
        "color:#18D9EF;"
        );

    layout->addSpacing(5);
    layout->addWidget(titreInfos);

    // Création des champs du formulaire

    auto creerChamp = [](const QString &valeur, bool modifiable = false)
    {
        QLineEdit *champ = new QLineEdit(valeur);
        champ->setReadOnly(!modifiable);
        champ->setMinimumHeight(36);
        return champ;
    };

    QLineEdit *champID = creerChamp(
        "E" + QString::number(idEquipe)
        );

    QLineEdit *champNom = creerChamp("", true);

    champNom->setPlaceholderText("Saisir le nom de l'equipe");

    QLineEdit *champMembres = creerChamp("0");

    QLineEdit *champCapacite = creerChamp("6");

    QLineEdit *champSexe = creerChamp("0 H / 0 F");

    QLineEdit *champAge = creerChamp("-");

    QLineEdit *champCompetences = creerChamp("-");

    QLineEdit *champDate = creerChamp(
        QDate::currentDate().toString("dd/MM/yyyy")
        );

    QLineEdit *champProjet = creerChamp("-");

    QLineEdit *champEvaluation = creerChamp("-");

    // ===== DISPOSITION VERTICALE =====

    QFormLayout *form = new QFormLayout();

    form->setHorizontalSpacing(25);
    form->setVerticalSpacing(9);

    form->setLabelAlignment(
        Qt::AlignLeft | Qt::AlignVCenter
        );

    form->setFieldGrowthPolicy(
        QFormLayout::ExpandingFieldsGrow
        );

    form->addRow("ID Equipe", champID);
    form->addRow("Nom Equipe", champNom);
    form->addRow("Nombre de membres", champMembres);
    form->addRow("Capacite maximale", champCapacite);
    form->addRow("Repartition des sexes", champSexe);
    form->addRow("Age moyen", champAge);
    form->addRow("Competences", champCompetences);
    form->addRow("Date de creation", champDate);
    form->addRow("ID Projet", champProjet);
    form->addRow("ID Evaluation", champEvaluation);

    layout->addLayout(form, 1);

    // ===== BOUTONS =====

    QHBoxLayout *boutons = new QHBoxLayout();

    QPushButton *ajouter = new QPushButton("Ajouter");
    QPushButton *annuler = new QPushButton("Annuler");

    ajouter->setObjectName("primary");
    annuler->setObjectName("secondary");

    ajouter->setDefault(true);
    ajouter->setAutoDefault(true);
    annuler->setAutoDefault(false);

    boutons->addStretch();
    boutons->addWidget(ajouter);
    boutons->addWidget(annuler);

    layout->addLayout(boutons);

    // ===== VALIDATION =====

    QObject::connect(
        ajouter,
        &QPushButton::clicked,
        &dialog,
        [&]()
        {
            QString nom = champNom->text().trimmed();

            if (nom.isEmpty())
            {
                QMessageBox::warning(
                    &dialog,
                    "Erreur",
                    "Le nom de l'equipe est obligatoire."
                    );

                champNom->setFocus();
                return;
            }

            nomEquipe = nom;

            // Les membres seront affectés automatiquement par la suite.
            membres.clear();

            dialog.accept();
        }
        );

    QObject::connect(
        annuler,
        &QPushButton::clicked,
        &dialog,
        &QDialog::reject
        );

    champNom->setFocus();

    return dialog.exec() == QDialog::Accepted;
}
/* =====================================================
   AJOUT EQUIPE
   ===================================================== */

void MainWindow::ajouterEquipe()
{


    QString nom;
    QVector<Participant> membres;

    if (!ouvrirPopupAjoutEquipe(
            this,
            prochainIdEquipe(),
            nom,
            membres))
        return;

    for (const Equipe &e : m_equipes)
    {
        if (e.nom().compare(nom, Qt::CaseInsensitive) == 0)
        {
            QMessageBox::warning(
                this, "Erreur",
                "Cette equipe existe deja."
                );
            return;
        }
    }

    // Instanciation d'un objet Equipe
    Equipe nouvelleEquipe(prochainIdEquipe(), nom);

    for (const Participant &p : membres)
    {
        nouvelleEquipe.ajouter(p);
    }

    // Appel de la méthode ajouter() du Modèle
    bool test = nouvelleEquipe.ajouter();

    if (test)
    {
        for (const Participant &p : membres)
            m_participants.append(p);

        rafraichir();

        afficherNotification("Equipe ajoutee : " + nom);
    }
    else
    {
        QMessageBox::critical(
            this,
            "Erreur",
            "Ajout non effectue."
            );
    }
}

/* =====================================================
   MODIFIER
   ===================================================== */

void MainWindow::modifierEquipe(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    QString nom;
    QVector<Participant> membres;

    if (!ouvrirPopupGestionEquipe(
            this,
            m_table,
            &m_equipes[index],
            m_equipes[index].id(),
            prochainIdParticipant(),
            nom,
            membres,
            false))
        return;
    for (int i = 0; i < m_equipes.size(); ++i)
    {
        if (i != index &&
            m_equipes[i].nom().compare(nom, Qt::CaseInsensitive) == 0)
        {
            QMessageBox::warning(
                this, "Erreur",
                "Cette equipe existe deja."
                );
            return;
        }
    }


    // Préparation de l'objet Equipe
    Etmp = Equipe(m_equipes[index].id(), nom);

    // Appel de la méthode modifier() du Modèle
    bool test = Etmp.modifier();

    if (test)
    {

        rafraichir();

        afficherNotification("Equipe modifiee.");
    }
    else
    {
        QMessageBox::critical(
            this,
            "Erreur",
            "Modification non effectuee."
            );
    }
}


/* =====================================================
   SUPPRIMER EQUIPE
   FORMULAIRE VERTICAL
   ===================================================== */

void MainWindow::supprimerEquipe(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    const Equipe &e = m_equipes[index];
    // ===== RECUPERATION DES DONNEES DU TABLEAU PRINCIPAL =====

    auto lireValeur = [&](int colonne, const QString &defaut)
    {
        if (m_table &&
            index >= 0 &&
            index < m_table->rowCount() &&
            m_table->item(index, colonne))
        {
            QString valeur =
                m_table->item(index, colonne)->text().trimmed();

            if (!valeur.isEmpty())
                return valeur;
        }

        return defaut;
    };

    QString idText = lireValeur(
        0, "E" + QString::number(e.id())
        );

    QString nomText = lireValeur(1, e.nom());

    QString nbMembresText = lireValeur(2, "-");

    QString capaciteText = lireValeur(3, "6");

    QString sexeText = lireValeur(4, "-");

    QString ageText = lireValeur(5, "-");

    QString competenceText = lireValeur(6, "-");

    QString dateText = lireValeur(7, "-");

    // ===== DIALOG =====

    QDialog dialog(this);

    dialog.setWindowTitle("Suppression de l'equipe");
    dialog.setFixedSize(780, 690);

    dialog.setStyleSheet(
        "QDialog {"
        " background-color: #050B14;"
        "}"
        "QLabel {"
        " color: #C7D3E0;"
        " font-family: 'Segoe UI';"
        " font-size: 13px;"
        "}"
        "QLabel#titrePrincipal {"
        " color: #18D9EF;"
        " font-size: 21px;"
        " font-weight: bold;"
        "}"
        "QLabel#sousTitre {"
        " color: #8193A8;"
        " font-size: 12px;"
        "}"
        "QLineEdit {"
        " color: #91A3B8;"
        " background-color: #091522;"
        " border: 1px solid #142A3C;"
        " border-radius: 7px;"
        " padding: 8px 10px;"
        " font-size: 13px;"
        " min-height: 20px;"
        "}"
        "QPushButton {"
        " padding: 10px 24px;"
        " border-radius: 7px;"
        " font-size: 13px;"
        " font-weight: bold;"
        "}"
        "QPushButton#primary {"
        " background-color: #18D9EF;"
        " color: #050B14;"
        " border: none;"
        "}"
        "QPushButton#primary:hover {"
        " background-color: #45E6F5;"
        "}"
        "QPushButton#secondary {"
        " background-color: #142334;"
        " color: #D5E0EC;"
        " border: 1px solid #263E53;"
        "}"
        "QPushButton#secondary:hover {"
        " background-color: #20364A;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    layout->setContentsMargins(35, 25, 35, 25);
    layout->setSpacing(12);

    // ===== TITRE =====

    QLabel *titre = new QLabel("SUPPRESSION DE L'EQUIPE");
    titre->setObjectName("titrePrincipal");

    QLabel *sousTitre = new QLabel(
        "Verification des informations avant suppression."
        );

    sousTitre->setObjectName("sousTitre");

    layout->addWidget(titre);
    layout->addWidget(sousTitre);

    // ===== INFORMATIONS DE L'EQUIPE =====

    QLabel *titreInfos = new QLabel(
        "Informations de l'equipe"
        );

    titreInfos->setStyleSheet(
        "font-size:15px;"
        "font-weight:bold;"
        "color:#18D9EF;"
        );

    layout->addSpacing(5);
    layout->addWidget(titreInfos);

    // ===== CREATION DES CHAMPS =====

    auto creerChamp = [](const QString &valeur)
    {
        QLineEdit *champ = new QLineEdit(valeur);

        champ->setReadOnly(true);
        champ->setMinimumHeight(36);

        return champ;
    };

    // Recuperation directe des valeurs du tableau principal

    QLineEdit *champID = creerChamp(
        lireValeur(0, "E" + QString::number(e.id()))
        );

    QLineEdit *champNom = creerChamp(
        lireValeur(1, e.nom())
        );

    QLineEdit *champMembres = creerChamp(
        lireValeur(2, "0")
        );

    QLineEdit *champCapacite = creerChamp(
        lireValeur(3, "6")
        );

    QLineEdit *champSexe = creerChamp(
        lireValeur(4, "-")
        );

    QLineEdit *champAge = creerChamp(
        lireValeur(5, "-")
        );

    QLineEdit *champCompetences = creerChamp(
        lireValeur(6, "-")
        );

    QLineEdit *champDate = creerChamp(
        lireValeur(7, "-")
        );

    // Ces deux informations ne sont pas affichees
    // dans les colonnes actuelles du tableau principal.

    QLineEdit *champProjet = creerChamp(
        QString::number(e.idProjet())
        );

    QLineEdit *champEvaluation = creerChamp(
        QString::number(e.idEvaluation())
        );

    // ===== DISPOSITION VERTICALE =====

    QFormLayout *form = new QFormLayout();

    form->setHorizontalSpacing(25);
    form->setVerticalSpacing(9);

    form->setLabelAlignment(
        Qt::AlignLeft | Qt::AlignVCenter
        );

    form->setFieldGrowthPolicy(
        QFormLayout::ExpandingFieldsGrow
        );

    form->addRow("ID Equipe", champID);
    form->addRow("Nom Equipe", champNom);
    form->addRow("Nombre de membres", champMembres);
    form->addRow("Capacite maximale", champCapacite);
    form->addRow("Repartition des sexes", champSexe);
    form->addRow("Age moyen", champAge);
    form->addRow("Competences", champCompetences);
    form->addRow("Date de creation", champDate);
    form->addRow("ID Projet", champProjet);
    form->addRow("ID Evaluation", champEvaluation);

    layout->addLayout(form, 1);

    // ===== BOUTONS =====

    QHBoxLayout *boutons = new QHBoxLayout();

    QPushButton *btnSupprimer = new QPushButton("Supprimer");
    QPushButton *btnAnnuler = new QPushButton("Annuler");

    btnSupprimer->setObjectName("primary");
    btnAnnuler->setObjectName("secondary");

    boutons->addStretch();
    boutons->addWidget(btnSupprimer);
    boutons->addWidget(btnAnnuler);

    layout->addLayout(boutons);

    // ===== ANNULER =====

    connect(
        btnAnnuler,
        &QPushButton::clicked,
        &dialog,
        &QDialog::reject
        );

    // ===== SUPPRIMER =====

    connect(
        btnSupprimer,
        &QPushButton::clicked,
        &dialog,
        [this, index, &dialog]()
        {
            if (index < 0 || index >= m_equipes.size())
                return;

            if (m_equipes.size() == 1 &&
                !m_participants.isEmpty())
            {
                QMessageBox::warning(
                    &dialog,
                    "Impossible",
                    "Gardez au moins une equipe."
                    );

                return;
            }

            // Appel de la méthode supprimer() du Modèle

            int idEquipe = m_equipes[index].id();

            bool test = Etmp.supprimer(idEquipe);

            if (test)
            {
                rafraichir();

                afficherNotification("Equipe supprimee.");

                dialog.accept();
            }
            else
            {
                QMessageBox::critical(
                    &dialog,
                    "Erreur",
                    "Suppression non effectuee."
                    );
            }
        }
        );

    dialog.exec();
}
/* =====================================================
   VOIR EQUIPE
   FORMULAIRE VERTICAL
   ===================================================== */

void MainWindow::voirEquipe(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    const Equipe &e = m_equipes[index];

    // ===== DIALOG =====

    QDialog dialog(this);

    dialog.setWindowTitle("Informations de l'equipe");
    dialog.setFixedSize(780, 690);

    dialog.setStyleSheet(
        "QDialog {"
        " background-color: #050B14;"
        "}"
        "QLabel {"
        " color: #C7D3E0;"
        " font-family: 'Segoe UI';"
        " font-size: 13px;"
        "}"
        "QLabel#titrePrincipal {"
        " color: #18D9EF;"
        " font-size: 21px;"
        " font-weight: bold;"
        "}"
        "QLabel#sousTitre {"
        " color: #8193A8;"
        " font-size: 12px;"
        "}"
        "QLineEdit {"
        " color: #91A3B8;"
        " background-color: #091522;"
        " border: 1px solid #142A3C;"
        " border-radius: 7px;"
        " padding: 8px 10px;"
        " font-size: 13px;"
        " min-height: 20px;"
        "}"
        "QPushButton {"
        " padding: 10px 24px;"
        " border-radius: 7px;"
        " font-size: 13px;"
        " font-weight: bold;"
        "}"
        "QPushButton#primary {"
        " background-color: #18D9EF;"
        " color: #050B14;"
        " border: none;"
        "}"
        "QPushButton#primary:hover {"
        " background-color: #45E6F5;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    layout->setContentsMargins(35, 25, 35, 25);
    layout->setSpacing(12);

    // ===== TITRE =====

    QLabel *titre = new QLabel("INFORMATIONS DE L'EQUIPE");

    titre->setObjectName("titrePrincipal");

    QLabel *sousTitre = new QLabel(
        "Consultez les informations de votre equipe."
        );

    sousTitre->setObjectName("sousTitre");

    layout->addWidget(titre);
    layout->addWidget(sousTitre);

    // ===== INFORMATIONS DE L'EQUIPE =====

    QLabel *titreInfos = new QLabel(
        "Informations de l'equipe"
        );

    titreInfos->setStyleSheet(
        "font-size:15px;"
        "font-weight:bold;"
        "color:#18D9EF;"
        );

    layout->addSpacing(5);
    layout->addWidget(titreInfos);

    // ===== RECUPERATION DES DONNEES DU TABLEAU PRINCIPAL =====

    int ligneEquipe = -1;

    QString idRecherche = "E" + QString::number(e.id());

    for (int i = 0; m_table && i < m_table->rowCount(); ++i)
    {
        QTableWidgetItem *item = m_table->item(i, 0);

        if (item && item->text().trimmed().compare(
                        idRecherche,
                        Qt::CaseInsensitive) == 0)
        {
            ligneEquipe = i;
            break;
        }
    }

    auto lireValeur = [&](int colonne, const QString &defaut)
    {
        if (ligneEquipe >= 0 &&
            m_table->item(ligneEquipe, colonne))
        {
            QString valeur =
                m_table->item(ligneEquipe, colonne)->text().trimmed();

            if (!valeur.isEmpty())
                return valeur;
        }

        return defaut;
    };

    // ===== CREATION DES CHAMPS =====

    auto creerChamp = [](const QString &valeur)
    {
        QLineEdit *champ = new QLineEdit(valeur);
        champ->setReadOnly(true);
        champ->setMinimumHeight(36);
        return champ;
    };

    QLineEdit *champID = creerChamp(idRecherche);

    QLineEdit *champNom = creerChamp(
        lireValeur(1, e.nom())
        );

    QLineEdit *champMembres = creerChamp(
        lireValeur(2, "0")
        );

    QLineEdit *champCapacite = creerChamp(
        lireValeur(3, "6")
        );

    QLineEdit *champSexe = creerChamp(
        lireValeur(4, "-")
        );

    QLineEdit *champAge = creerChamp(
        lireValeur(5, "-")
        );

    QLineEdit *champCompetences = creerChamp(
        lireValeur(6, "-")
        );

    QLineEdit *champDate = creerChamp(
        lireValeur(7, "-")
        );

    QLineEdit *champProjet = creerChamp(
        QString::number(e.idProjet())
        );

    QLineEdit *champEvaluation = creerChamp(
        QString::number(e.idEvaluation())
        );

    // ===== DISPOSITION VERTICALE =====

    QFormLayout *form = new QFormLayout();

    form->setHorizontalSpacing(25);
    form->setVerticalSpacing(9);

    form->setLabelAlignment(
        Qt::AlignLeft | Qt::AlignVCenter
        );

    form->setFieldGrowthPolicy(
        QFormLayout::ExpandingFieldsGrow
        );

    form->addRow("ID Equipe", champID);
    form->addRow("Nom Equipe", champNom);
    form->addRow("Nombre de membres", champMembres);
    form->addRow("Capacite maximale", champCapacite);
    form->addRow("Repartition des sexes", champSexe);
    form->addRow("Age moyen", champAge);
    form->addRow("Competences", champCompetences);
    form->addRow("Date de creation", champDate);
    form->addRow("ID Projet", champProjet);
    form->addRow("ID Evaluation", champEvaluation);

    layout->addLayout(form, 1);

    // ===== BOUTON FERMER =====

    QHBoxLayout *boutons = new QHBoxLayout();

    QPushButton *fermer = new QPushButton("Fermer");

    fermer->setObjectName("primary");

    boutons->addStretch();
    boutons->addWidget(fermer);

    layout->addLayout(boutons);

    // ===== FERMER =====

    connect(
        fermer,
        &QPushButton::clicked,
        &dialog,
        &QDialog::accept
        );

    dialog.exec();
}
/* =====================================================
   QR
   ===================================================== */
QImage MainWindow::creerQR(const QString &code) const
{
    const int n = 21;
    const int scale = 8;
    const int marge = 4;

    QByteArray contenu = code.toUtf8();

    // Version 1-L : maximum 17 octets
    if (contenu.size() > 17)
        return QImage();

    QVector<QVector<int>> modules(
        n, QVector<int>(n, 0)
        );

    QVector<QVector<bool>> reserved(
        n, QVector<bool>(n, false)
        );

    // Fonction pour placer un module
    auto placer = [&](int x, int y, bool noir)
    {
        if (x >= 0 && x < n && y >= 0 && y < n)
        {
            modules[y][x] = noir ? 1 : 0;
            reserved[y][x] = true;
        }
    };

    // Finder patterns et leurs marges blanches
    auto finder = [&](int ox, int oy)
    {
        for (int dy = -1; dy <= 7; ++dy)
        {
            for (int dx = -1; dx <= 7; ++dx)
            {
                int x = ox + dx;
                int y = oy + dy;

                if (x < 0 || x >= n || y < 0 || y >= n)
                    continue;

                bool noir = false;

                if (dx >= 0 && dx <= 6 &&
                    dy >= 0 && dy <= 6)
                {
                    noir =
                        dx == 0 || dx == 6 ||
                        dy == 0 || dy == 6 ||
                        (dx >= 2 && dx <= 4 &&
                         dy >= 2 && dy <= 4);
                }

                placer(x, y, noir);
            }
        }
    };

    finder(0, 0);
    finder(n - 7, 0);
    finder(0, n - 7);

    // Timing patterns
    for (int i = 8; i < n - 8; ++i)
    {
        placer(i, 6, i % 2 == 0);
        placer(6, i, i % 2 == 0);
    }

    // Encodage Byte Mode
    QVector<int> bits;

    auto ajouterBits = [&](int valeur, int longueur)
    {
        for (int i = longueur - 1; i >= 0; --i)
            bits.append((valeur >> i) & 1);
    };

    ajouterBits(4, 4); // Mode Byte
    ajouterBits(contenu.size(), 8);

    for (unsigned char c : contenu)
        ajouterBits(c, 8);

    // Terminator
    ajouterBits(0, qMin(4, 152 - bits.size()));

    // Completer jusqu'au prochain octet
    while (bits.size() % 8 != 0)
        bits.append(0);

    // Transformer les bits en octets
    QVector<int> data;

    for (int i = 0; i < bits.size(); i += 8)
    {
        int valeur = 0;

        for (int j = 0; j < 8; ++j)
            valeur = (valeur << 1) | bits[i + j];

        data.append(valeur);
    }

    // Remplissage jusqu'a 19 octets
    int compteur = 0;

    while (data.size() < 19)
    {
        data.append(compteur % 2 == 0 ? 0xEC : 0x11);
        compteur++;
    }

    // Multiplication dans GF(256)
    auto multiplier = [](int x, int y)
    {
        int resultat = 0;

        for (int i = 0; i < 8; ++i)
        {
            if (y & 1)
                resultat ^= x;

            bool haut = x & 0x80;

            x = (x << 1) & 0xFF;

            if (haut)
                x ^= 0x1D;

            y >>= 1;
        }

        return resultat;
    };

    // Generateur Reed-Solomon de degre 7
    QVector<int> generateur(7, 0);
    generateur[6] = 1;

    int racine = 1;

    for (int i = 0; i < 7; ++i)
    {
        for (int j = 0; j < 7; ++j)
        {
            generateur[j] =
                multiplier(generateur[j], racine);

            if (j + 1 < 7)
                generateur[j] ^= generateur[j + 1];
        }

        racine = multiplier(racine, 2);
    }

    // Calcul des 7 octets de correction
    QVector<int> correction(7, 0);

    for (int valeur : data)
    {
        int facteur = valeur ^ correction[0];

        for (int j = 0; j < 6; ++j)
            correction[j] = correction[j + 1];

        correction[6] = 0;

        for (int j = 0; j < 7; ++j)
            correction[j] ^= multiplier(
                generateur[j], facteur
                );
    }

    // Construire la sequence complete
    QVector<int> tousOctets = data;

    for (int valeur : correction)
        tousOctets.append(valeur);

    QVector<int> donneesBits;

    for (int valeur : tousOctets)
        ajouterBits(valeur, 8);

    // Les bits sont deja dans bits :
    // reconstruire la sequence data + correction
    donneesBits.clear();

    for (int valeur : tousOctets)
    {
        for (int i = 7; i >= 0; --i)
            donneesBits.append((valeur >> i) & 1);
    }

    // Reserving format information
    for (int i = 0; i <= 5; ++i)
        reserved[ i ][8] = true;

    reserved[7][8] = true;
    reserved[8][8] = true;
    reserved[8][7] = true;

    for (int i = 9; i < 15; ++i)
        reserved[8][14 - i] = true;

    for (int i = 0; i < 8; ++i)
        reserved[8][n - 1 - i] = true;

    for (int i = 8; i < 15; ++i)
        reserved[n - 15 + i][8] = true;

    // Dark module
    placer(8, 13, true);

    // Placement des donnees
    int indexBit = 0;
    bool versHaut = true;

    for (int droite = n - 1; droite >= 1; droite -= 2)
    {
        if (droite == 6)
            droite--;

        for (int v = 0; v < n; ++v)
        {
            int y = versHaut ? n - 1 - v : v;

            for (int j = 0; j < 2; ++j)
            {
                int x = droite - j;

                if (reserved[y][x])
                    continue;

                int valeur = 0;

                if (indexBit < donneesBits.size())
                    valeur = donneesBits[indexBit++];

                // Mask 0
                if ((x + y) % 2 == 0)
                    valeur ^= 1;

                modules[y][x] = valeur;
            }
        }

        versHaut = !versHaut;
    }

    // Format information : niveau L, masque 0
    int format = 8 << 10;
    int reste = format;

    for (int i = 14; i >= 10; --i)
    {
        if ((reste >> i) & 1)
            reste ^= 0x537 << (i - 10);
    }

    format = (format | reste) ^ 0x5412;

    // Placer les informations de format
    for (int i = 0; i <= 5; ++i)
        modules[i][8] = (format >> i) & 1;

    modules[7][8] = (format >> 6) & 1;
    modules[8][8] = (format >> 7) & 1;
    modules[8][7] = (format >> 8) & 1;

    for (int i = 9; i < 15; ++i)
        modules[8][14 - i] = (format >> i) & 1;

    for (int i = 0; i < 8; ++i)
        modules[8][n - 1 - i] = (format >> i) & 1;

    for (int i = 8; i < 15; ++i)
        modules[n - 15 + i][8] = (format >> i) & 1;

    // Image finale
    int dimension = (n + marge * 2) * scale;

    QImage image(
        dimension,
        dimension,
        QImage::Format_RGB32
        );

    image.fill(Qt::white);

    QPainter p(&image);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);

    for (int y = 0; y < n; ++y)
    {
        for (int x = 0; x < n; ++x)
        {
            if (modules[y][x])
            {
                p.drawRect(
                    (x + marge) * scale,
                    (y + marge) * scale,
                    scale,
                    scale
                    );
            }
        }
    }

    p.end();

    return image;
}
/* =====================================================
   AFFICHER QR
   ===================================================== */
void MainWindow::afficherQR(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    const Equipe &equipe = m_equipes[index];

    QString code = "DEVE-EQ-" + QString::number(equipe.id());
    QImage image = creerQR(code);

    QDialog dialog(this);
    dialog.setWindowTitle("QR Code - Equipe");
    dialog.setFixedSize(700, 550);

    dialog.setStyleSheet(
        "QDialog {"
        " background-color: #050B14;"
        " border: 1px solid #18D9EF;"
        "}"
        "QLabel {"
        " color: #F4F7FB;"
        " background: transparent;"
        "}"
        "QPushButton {"
        " background-color: #0A1726;"
        " color: #F4F7FB;"
        " border: 1px solid #203B52;"
        " border-radius: 8px;"
        " padding: 11px;"
        " font-size: 13px;"
        " font-weight: bold;"
        "}"
        "QPushButton:hover {"
        " background-color: #102A3D;"
        " border: 1px solid #18D9EF;"
        "}"
        );

    QVBoxLayout *v = new QVBoxLayout(&dialog);
    v->setContentsMargins(25, 20, 25, 20);
    v->setSpacing(12);

    // Nom de l'equipe
    QLabel *title = new QLabel(equipe.nom());
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "color: #18D9EF;"
        "font-size: 23px;"
        "font-weight: bold;"
        );

    // Identifiant de l'equipe
    QLabel *idLabel = new QLabel("ID : " + QString::number(equipe.id()));
    idLabel->setAlignment(Qt::AlignCenter);
    idLabel->setStyleSheet(
        "color: #18D9EF;"
        "font-size: 14px;"
        "border: 1px solid #18D9EF;"
        "border-radius: 12px;"
        "padding: 5px 15px;"
        );

    // Image QR
    QLabel *img = new QLabel;
    img->setAlignment(Qt::AlignCenter);
    img->setFixedSize(230, 230);
    img->setStyleSheet(
        "background-color: white;"
        "border: 2px solid #18D9EF;"
        "border-radius: 8px;"
        "padding: 5px;"
        );

    img->setPixmap(
        QPixmap::fromImage(image).scaled(
            210, 210,
            Qt::KeepAspectRatio,
            Qt::FastTransformation
            )
        );

    // Texte explicatif
    QLabel *description = new QLabel(
        "Scannez pour identifier cette equipe"
        );
    description->setAlignment(Qt::AlignCenter);
    description->setStyleSheet(
        "color: #8FA9C1;"
        "font-size: 12px;"
        );

    // Boutons
    QPushButton *save = new QPushButton("Enregistrer PNG");
    save->setStyleSheet(
        "QPushButton {"
        " background-color: #18D9EF;"
        " color: #050B14;"
        " border: none;"
        " border-radius: 8px;"
        " padding: 11px;"
        " font-size: 13px;"
        " font-weight: bold;"
        "}"
        "QPushButton:hover {"
        " background-color: #12BBD0;"
        "}"
        );
    QPushButton *close = new QPushButton("Fermer");

    save->setMinimumHeight(42);
    close->setMinimumHeight(42);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->setSpacing(12);
    buttons->addWidget(save);
    buttons->addWidget(close);

    v->addWidget(title);
    v->addWidget(idLabel, 0, Qt::AlignCenter);
    v->addStretch();
    v->addWidget(img, 0, Qt::AlignCenter);
    v->addWidget(description);
    v->addStretch();
    v->addLayout(buttons);

    // Enregistrer le QR
    connect(save, &QPushButton::clicked, [&]() {
        QString file = QFileDialog::getSaveFileName(
            &dialog,
            "Enregistrer QR",
            "qr_equipe.png",
            "PNG (*.png)"
            );

        if (!file.isEmpty())
            image.save(file, "PNG");
    });


    connect(close, &QPushButton::clicked,
            &dialog, &QDialog::accept);

    dialog.exec();
}


/* =====================================================
   BADGE - DESIGN DEVENA
   ===================================================== */

QImage MainWindow::creerBadge(const Equipe &e,
                              const QString &membres,
                              const QString &hommes,
                              const QString &femmes) const
{
    QImage image(1200, 700, QImage::Format_RGB32);
    image.fill(QColor("#050B14"));

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    const QColor cyan("#18D9EF");
    const QColor cyanSoft(24, 217, 239, 35);
    const QColor white("#F4F7FB");
    const QColor grey("#A9C1D5");

    // Cadre principal
    p.setPen(QPen(cyan, 4));
    p.setBrush(QColor("#071522"));
    p.drawRoundedRect(30, 30, 1140, 640, 35, 35);

    // Logo DEVENA
    p.setPen(cyan);
    p.setFont(QFont("Segoe UI", 48, QFont::Bold));
    p.drawText(100, 125, "◆ Devena");

    // Nom equipe centré dans la colonne gauche
    p.setPen(white);
    QFont nomFont("Segoe UI", 46, QFont::Bold);
    p.setFont(nomFont);
    p.drawText(QRect(100, 165, 590, 75),
               Qt::AlignCenter,
               e.nom());

    // Separation
    p.setPen(QPen(cyan, 2));
    p.drawLine(100, 268, 690, 268);

    // Informations de l'equipe (incluant ID : E...)
    p.setFont(QFont("Segoe UI", 24));

    const int iconX = 122;
    const int textX = 152;
    const int startY = 332;
    const int stepY = 56;

    // 0. ID equipe
    p.setPen(QPen(cyan, 2.2));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(iconX - 10, startY - 22, 20, 16, 3, 3);
    p.drawLine(iconX - 6, startY - 17, iconX + 6, startY - 17);
    p.drawLine(iconX - 6, startY - 12, iconX + 2, startY - 12);
    p.setPen(cyan);
    p.drawText(textX, startY, "ID : E" + QString::number(e.id()));

    // 1. Membres
    const int yMembres = startY + stepY;
    p.setPen(QPen(cyan, 2.2));
    p.drawEllipse(QPoint(iconX - 5, yMembres - 14), 5, 5);
    p.drawArc(iconX - 14, yMembres - 8, 18, 12, 0, 180 * 16);
    p.drawEllipse(QPoint(iconX + 5, yMembres - 13), 4, 4);
    p.drawArc(iconX - 2, yMembres - 7, 15, 10, 0, 180 * 16);
    p.setPen(grey);
    p.drawText(textX, yMembres, "Membres : " + membres + " / 6");

    // 2. Hommes
    const int yHommes = startY + stepY * 2;
    p.setPen(QPen(cyan, 2.2));
    p.drawEllipse(QPoint(iconX - 6, yHommes - 9), 6, 6);
    p.drawLine(iconX - 2, yHommes - 13, iconX + 6, yHommes - 21);
    p.drawLine(iconX + 1, yHommes - 21, iconX + 6, yHommes - 21);
    p.drawLine(iconX + 6, yHommes - 21, iconX + 6, yHommes - 16);
    p.setPen(grey);
    p.drawText(textX, yHommes, "Hommes : " + hommes);

    // 3. Femmes
    const int yFemmes = startY + stepY * 3;
    p.setPen(QPen(cyan, 2.2));
    p.drawEllipse(QPoint(iconX, yFemmes - 15), 6, 6);
    p.drawLine(iconX, yFemmes - 9, iconX, yFemmes + 1);
    p.drawLine(iconX - 5, yFemmes - 3, iconX + 5, yFemmes - 3);
    p.setPen(grey);
    p.drawText(textX, yFemmes, "Femmes : " + femmes);

    // 4. Age moyen
    const int yAge = startY + stepY * 4;
    p.setPen(QPen(cyan, 2.2));
    p.drawRoundedRect(iconX - 11, yAge - 22, 22, 18, 4, 4);
    p.drawEllipse(QPoint(iconX - 4, yAge - 15), 2.5, 2.5);
    p.drawArc(iconX - 8, yAge - 12, 8, 6, 0, 180 * 16);
    p.drawLine(iconX + 2, yAge - 16, iconX + 8, yAge - 16);
    p.drawLine(iconX + 2, yAge - 12, iconX + 8, yAge - 12);
    p.drawLine(iconX + 2, yAge - 8, iconX + 7, yAge - 8);
    p.setPen(grey);
    p.drawText(textX, yAge,
               "Age moyen : " +
                   QString::number(e.ageMoyen(), 'f', 1));

    // QR Code identique a la fenetre QR
    QString code = "DEVE-EQ-" + QString::number(e.id());
    QImage qr = creerQR(code);

    if (qr.isNull()) {
        p.end();
        return image;
    }

    p.setPen(QPen(cyanSoft, 8));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(751, 171, 368, 368, 18, 18);

    p.setPen(QPen(cyan, 4));
    p.setBrush(Qt::white);
    p.drawRoundedRect(755, 175, 360, 360, 15, 15);

    qr = qr.scaled(320, 320,
                   Qt::KeepAspectRatio,
                   Qt::FastTransformation);
    p.drawImage(775, 195, qr);

    // Slogan DEVENA
    p.setPen(cyan);
    QFont sloganFont("Segoe UI", 17, QFont::Bold);
    sloganFont.setItalic(true);
    p.setFont(sloganFont);
    p.drawText(QRect(100, 624, 1000, 35),
               Qt::AlignCenter,
               "Connecter les talents, construire l'avenir");

    p.end();
    return image;
}
/* =====================================================
   AFFICHER BADGE
   ===================================================== */

void MainWindow::afficherBadge(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    const Equipe &equipe = m_equipes[index];
    int ligne = index;

    QString membres = m_table->item(ligne, 2)->text();
    QString sexe = m_table->item(ligne, 4)->text();

    int nbHommes = 0;
    int nbFemmes = 0;

    QRegularExpression regexH("(\\d+)\\s*H",
                              QRegularExpression::CaseInsensitiveOption);

    QRegularExpression regexF("(\\d+)\\s*F",
                              QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch mh = regexH.match(sexe);
    QRegularExpressionMatch mf = regexF.match(sexe);

    if (mh.hasMatch())
        nbHommes = mh.captured(1).toInt();

    if (mf.hasMatch())
        nbFemmes = mf.captured(1).toInt();

    QString hommes = QString::number(nbHommes);
    QString femmes = QString::number(nbFemmes);
    QImage image = creerBadge(equipe, membres, hommes, femmes);

    QDialog dialog(this);
    dialog.setWindowTitle("Badge - " + equipe.nom());
    dialog.setFixedSize(950, 650);

    dialog.setStyleSheet(
        "QDialog {"
        " background-color: #050B14;"
        " border: 1px solid #18D9EF;"
        "}"
        "QLabel {"
        " color: #F4F7FB;"
        " background: transparent;"
        "}"
        "QPushButton {"
        " background-color: #0A1726;"
        " color: #F4F7FB;"
        " border: 1px solid #203B52;"
        " border-radius: 8px;"
        " padding: 12px;"
        " font-size: 14px;"
        " font-weight: bold;"
        "}"
        "QPushButton:hover {"
        " background-color: #102A3D;"
        " border: 1px solid #18D9EF;"
        "}"
        );

    QVBoxLayout *v = new QVBoxLayout(&dialog);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(15);

    // Image Badge
    QLabel *img = new QLabel;
    img->setAlignment(Qt::AlignCenter);

    img->setPixmap(
        QPixmap::fromImage(image).scaled(
            940, 548,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            )
        );

    // Boutons
    QPushButton *save = new QPushButton("Enregistrer PNG");
    QPushButton *close = new QPushButton("Fermer");

    save->setMinimumHeight(48);
    close->setMinimumHeight(48);

    save->setSizePolicy(QSizePolicy::Expanding,
                        QSizePolicy::Fixed);

    close->setSizePolicy(QSizePolicy::Expanding,
                         QSizePolicy::Fixed);

    // Bouton bleu DEVENA
    save->setStyleSheet(
        "QPushButton {"
        " background-color: #18D9EF;"
        " color: #050B14;"
        " border: none;"
        " border-radius: 8px;"
        " font-size: 14px;"
        " font-weight: bold;"
        "}"
        "QPushButton:hover {"
        " background-color: #12BBD0;"
        "}"
        );

    // Layout horizontal pour les boutons
    QHBoxLayout *hButtons = new QHBoxLayout;
    hButtons->setSpacing(15);

    hButtons->addWidget(save);
    hButtons->addWidget(close);

    // Ajouter les widgets
    v->addWidget(img, 1);
    v->addLayout(hButtons);

    // Enregistrer Badge PNG
    connect(save, &QPushButton::clicked, [&]() {

        QString file = QFileDialog::getSaveFileName(
            &dialog,
            "Enregistrer le badge",
            "badge_equipe.png",
            "PNG (*.png)"
            );

        if (!file.isEmpty())
            image.save(file, "PNG");
    });

    connect(close, &QPushButton::clicked,
            &dialog, &QDialog::accept);

    dialog.exec();
}

/* =====================================================
   AFFICHER EQUIPES
   ===================================================== */

void MainWindow::afficherEquipes()
{
    // Appel de la méthode afficher() du Modèle
    QSqlQueryModel *model = Etmp.afficher();

    if (!model)
    {
        QMessageBox::warning(
            this, "Erreur",
            "Impossible de charger les equipes."
            );
        return;
    }

    if (model->lastError().isValid())
    {
        QMessageBox::warning(
            this, "Erreur SQL",
            model->lastError().text()
            );

        delete model;
        return;
    }

    m_table->clearContents();
    m_table->setRowCount(model->rowCount());

    m_equipes.clear();

    for (int i = 0; i < model->rowCount(); ++i)
    {
        // 1. ID Equipe
        QString idBD =
            model->data(model->index(i, 0)).toString().trimmed();

        QString idTexte = idBD;

        if (idTexte.startsWith("e", Qt::CaseInsensitive))
            idTexte.remove(0, 1);

        bool ok = false;
        int id = idTexte.toInt(&ok);

        if (!ok)
        {
            QMessageBox::warning(
                this, "Erreur",
                "ID Equipe invalide : " + idBD
                );

            m_table->setRowCount(0);
            m_equipes.clear();
            delete model;
            return;
        }

        // 2. Nom Equipe
        QString nom =
            model->data(model->index(i, 1)).toString();

        // Création de l'objet Equipe
        Equipe e(id, nom);

        // 3. Age moyen
        e.setAgeMoyen(
            model->data(model->index(i, 5)).toDouble()
            );

        m_equipes.append(e);

        // 4. Nombre de membres
        QString nbMembres =
            model->data(model->index(i, 2)).toString();

        // 5. Capacité maximale
        QString capacite =
            model->data(model->index(i, 3)).toString();

        // 6. Sexe
        QString sexe =
            model->data(model->index(i, 4)).toString();

        // 7. Age moyen
        QString age =
            model->data(model->index(i, 5)).toString();

        // 8. Compétences
        QString competences =
            model->data(model->index(i, 6)).toString();

        // 9. Date de création
        QVariant dateValue =
            model->data(model->index(i, 7));

        QDate date = dateValue.toDateTime().date();

        if (!date.isValid())
            date = dateValue.toDate();

        QString dateText = date.isValid()
                               ? date.toString("dd/MM/yyyy")
                               : dateValue.toString();

        // Remplissage du tableau principal
        QStringList valeurs = {
            idBD.toUpper(),
            nom,
            nbMembres,
            capacite,
            sexe,
            age,
            competences,
            dateText
        };

        for (int c = 0; c < 8; ++c)
        {
            m_table->setItem(
                i, c,
                new QTableWidgetItem(valeurs[c])

                );
            // Masquer les barres de defilement
            m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }

        // ================= ACTIONS =================

        QWidget *actions = new QWidget;
        QHBoxLayout *a = new QHBoxLayout(actions);

        a->setContentsMargins(2, 2, 2, 2);
        a->setSpacing(6);

        QPushButton *eye = new QPushButton("👁");
        QPushButton *edit = new QPushButton("✎");
        QPushButton *trash = new QPushButton("🗑");
        QPushButton *qr = new QPushButton("▣");
        QPushButton *badge = new QPushButton("★");
        QPushButton *pdf = new QPushButton("📄"); // Nouveau bouton

        eye->setObjectName("iconButton");
        edit->setObjectName("iconButton");
        trash->setObjectName("iconButton");
        qr->setObjectName("iconButton");
        badge->setObjectName("iconButton");
        pdf->setObjectName("iconButton");

        eye->setFixedSize(34, 30);
        edit->setFixedSize(34, 30);
        trash->setFixedSize(34, 30);
        qr->setFixedSize(34, 30);
        badge->setFixedSize(34, 30);
        pdf->setFixedSize(34, 30);

        eye->setToolTip("Voir les membres");
        edit->setToolTip("Modifier l'equipe");
        trash->setToolTip("Supprimer l'equipe");
        qr->setToolTip("Afficher le QR");
        badge->setToolTip("Afficher le badge");
        pdf->setToolTip("Exporter la fiche PDF de l'equipe");

        a->addWidget(eye);
        a->addWidget(edit);
        a->addWidget(trash);
        a->addWidget(qr);
        a->addWidget(badge);
        a->addWidget(pdf); // Nouveau bouton

        m_table->setCellWidget(i, 8, actions);
        m_table->setRowHeight(i, 46);

        connect(eye, &QPushButton::clicked,
                [this, i]() { voirEquipe(i); });

        connect(edit, &QPushButton::clicked,
                [this, i]() { modifierEquipe(i); });

        connect(trash, &QPushButton::clicked,
                [this, i]() { supprimerEquipe(i); });

        connect(qr, &QPushButton::clicked,
                [this, i]() { afficherQR(i); });

        connect(badge, &QPushButton::clicked,
                [this, i]() { afficherBadge(i); });

        connect(pdf, &QPushButton::clicked,
                [this, i]() { exporterEquipePDF(i); });

    } // Fin du for

    delete model;

} // Fin de afficherEquipes()
/* =====================================================
   COURBE (Corrigée : à 0.0 s'il n'y a pas de membres)
   ===================================================== */

QPixmap MainWindow::creerCourbe() const
{
    const int W = 1000;
    const int H = 170;

    QPixmap pix(W, H);
    pix.fill(m_modeClair ? QColor("#FFFFFF") : QColor("#081421"));

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(m_modeClair ? QColor("#D5DEE8") : QColor("#162A3E"));

    // Lignes horizontales
    for (int y = 30; y < 145; y += 28)
    {
        p.drawLine(45, y, W - 25, y);
    }

    p.setPen(m_modeClair ? QColor("#526579") : QColor("#8EA0B7"));
    p.drawText(45, 18, "Membres inscrits");

    p.drawText(5, 138, "0.0");
    p.drawText(5, 110, "12.5");
    p.drawText(5, 82,  "37.5");
    p.drawText(5, 54,  "50.0");

    QStringList mois = {"Fév", "Mar", "Avr", "Mai", "Juin", "Juil", "Août", "Sept"};

    // Calcule le nombre total actuel de membres inscrits dans toutes les équipes

    int totalMembresActuels = 0;

    QSqlQuery query;

    if (query.exec(
            "SELECT NVL(SUM(NB_MEMBRES), 0) FROM EQUIPES")
        && query.next())
    {
        totalMembresActuels = query.value(0).toInt();
    }


    QVector<QPointF> points;
    double startX = 65;
    double endX = W - 40;
    double stepX = (endX - startX) / (mois.size() - 1);

    QVector<double> valeursY;

    if (totalMembresActuels == 0)
    {
        // Si aucun membre n'est présent (ou équipes vides), la courbe reste plate à 0.0
        valeursY = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    }
    else
    {
        // Si des membres sont affectés, la courbe progresse pour atteindre le total actuel
        double step = double(totalMembresActuels) / 7.0;
        for (int i = 0; i < mois.size(); ++i)
        {
            valeursY.append(i * step);
        }
    }

    for (int i = 0; i < mois.size(); ++i) {
        double x = startX + i * stepX;
        double y = 138 - (valeursY[i] / 50.0) * 90.0;
        points.append(QPointF(x, y));
    }

    p.setPen(QPen(QColor("#18BFD4"), 2.5));

    for (int i = 1; i < points.size(); ++i)
    {
        p.drawLine(points[i - 1], points[i]);
    }

    for (int i = 0; i < points.size(); ++i)
    {
        p.setPen(m_modeClair ? QColor("#526579") : QColor("#8EA0B7"));
        p.drawText(points[i].x() - 10, 162, mois[i]);
    }

    p.end();
    return pix;
}

class DonutStatistique : public QWidget
{
public:
    DonutStatistique(int hommes, int femmes, QWidget *parent = nullptr)
        : QWidget(parent), m_hommes(hommes), m_femmes(femmes)
    {
        setFixedSize(105, 105);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QRectF cercle(13, 13, 79, 79);

        int total = m_hommes + m_femmes;

        QPen pen;
        pen.setWidth(12);
        pen.setCapStyle(Qt::FlatCap);

        pen.setColor(QColor("#20384D"));
        p.setPen(pen);
        p.drawArc(cercle, 0, 360 * 16);

        if (total > 0)
        {
            int angleHommes = qRound(
                360.0 * m_hommes / total);

            pen.setColor(QColor("#18D9EF"));
            p.setPen(pen);
            p.drawArc(
                cercle,
                90 * 16,
                -angleHommes * 16
                );

            pen.setColor(QColor("#A879F5"));
            p.setPen(pen);
            p.drawArc(
                cercle,
                (90 - angleHommes) * 16,
                -(360 - angleHommes) * 16
                );
        }

        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI", 13, QFont::Bold));

        p.drawText(
            rect(),
            Qt::AlignCenter,
            QString::number(total)
            );
    }

private:
    int m_hommes;
    int m_femmes;
};



/* =====================================================
   STATISTIQUES - ORGANIGRAMME PROFESSIONNEL
   ===================================================== */


void MainWindow::afficherOrganigramme()
{
    if (!m_organigramme)
        return;
   bool modeClair = m_modeClair;

    QString couleurCarte = modeClair ? "#FFFFFF" : "#0B1929";
    QString couleurTexte = modeClair ? "#172B40" : "#EAF2FA";
    QString couleurBordure = modeClair ? "#D5DCE5" : "#24445B";
    QString couleurLigne = modeClair ? "#D5DCE5" : "#24445B";
    QString couleurBarreFond = modeClair ? "#E3EAF1" : "#20384D";

    int totalEquipes = 0;
    int totalMembres = 0;
    int hommes = 0;
    int femmes = 0;

    QMap<QString, int> ages;
    ages["18 - 25 ans"] = 0;
    ages["26 - 30 ans"] = 0;
    ages["31 - 35 ans"] = 0;
    ages["36 ans et plus"] = 0;
    ages["Non precise"] = 0;

    // Competences normalisees pour eviter les doublons
    QMap<QString, int> competences;
    QMap<QString, QString> nomsCompetences;

    QSqlQuery query;

    if (!query.exec(
            "SELECT SEXE, AGE, COMPETENCES, NB_MEMBRES "
            "FROM EQUIPES"))
    {
        qDebug() << query.lastError().text();
        return;
    }

    QRegularExpression regexH(
        "(\\d+)\\s*H",
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpression regexF(
        "(\\d+)\\s*F",
        QRegularExpression::CaseInsensitiveOption);

    while (query.next())
    {
        totalEquipes++;
        totalMembres += query.value(3).toInt();

        // Calcul des hommes et des femmes
        QString sexe = query.value(0).toString();

        auto mh = regexH.match(sexe);
        auto mf = regexF.match(sexe);

        if (mh.hasMatch())
            hommes += mh.captured(1).toInt();

        if (mf.hasMatch())
            femmes += mf.captured(1).toInt();

        // Repartition par age
        int age = query.value(1).toInt();

        if (age >= 18 && age <= 25)
            ages["18 - 25 ans"]++;
        else if (age >= 26 && age <= 30)
            ages["26 - 30 ans"]++;
        else if (age >= 31 && age <= 35)
            ages["31 - 35 ans"]++;
        else if (age >= 36)
            ages["36 ans et plus"]++;
        else
            ages["Non precise"]++;

        // Recuperation des competences
        QString texte = query.value(2).toString();

        QStringList liste = texte.split(
            QRegularExpression("[,;]"),
            Qt::SkipEmptyParts);

        QSet<QString> dejaCompte;

        for (QString competence : liste)
        {
            competence = competence.trimmed();

            QString cle = competence.toLower();

            if (!cle.isEmpty() &&
                cle != "-" &&
                !dejaCompte.contains(cle))
            {
                competences[cle]++;

                if (!nomsCompetences.contains(cle))
                    nomsCompetences[cle] = competence;

                dejaCompte.insert(cle);
            }
        }
    }

    // Nettoyage de l'ancien organigramme
    std::function<void(QLayout*)> viderLayout =
        [&](QLayout *layout)
    {
        if (!layout)
            return;

        while (QLayoutItem *item = layout->takeAt(0))
        {
            if (item->widget())
                delete item->widget();

            if (item->layout())
                viderLayout(item->layout());

            delete item;
        }
    };

    QLayout *ancienLayout = m_organigramme->layout();

    if (ancienLayout)
    {
        viderLayout(ancienLayout);
        delete ancienLayout;
    }

    // Style general des cartes
    QString styleCarte =
        "QFrame {"
        "background-color:" + couleurCarte + ";"
                         "border:1px solid " + couleurBordure + ";"
                           "border-radius:9px;"
                           "}";
    QVBoxLayout *principal =
        new QVBoxLayout(m_organigramme);

    principal->setContentsMargins(5, 4, 5, 4);
    principal->setSpacing(5);

    // Entete de l'organigramme
    QHBoxLayout *entete = new QHBoxLayout;
    principal->addLayout(entete);

    // Fonction de creation des cartes
    auto creerCarte =
        [&](const QString &nom) -> QPair<QFrame*, QVBoxLayout*>
    {
        QFrame *carte = new QFrame;
        carte->setStyleSheet(styleCarte);
        carte->setMinimumWidth(0);
       carte->setMinimumHeight(230);

        QVBoxLayout *l = new QVBoxLayout(carte);
       l->setContentsMargins(11, 9, 11, 9);
       l->setSpacing(8);

       // Titre de la carte
       QLabel *titreCarte = new QLabel(nom);

       titreCarte->setFixedHeight(22);
       titreCarte->setAlignment(
           Qt::AlignLeft | Qt::AlignVCenter
           );

       titreCarte->setStyleSheet(
           QString(
               "color:%1;"
               "font-family:Segoe UI;"
               "font-size:13px;"
               "font-weight:%2;"
               "background:transparent;"
               "border:none;"
               )
               .arg(modeClair ? "#172B40" : "#18D9EF")
               .arg(modeClair ? "normal" : "bold")
           );

       l->addWidget(titreCarte);

       // Ligne horizontale sous le titre
       QFrame *ligne = new QFrame;

       ligne->setFixedHeight(1);
       ligne->setSizePolicy(
           QSizePolicy::Expanding,
           QSizePolicy::Fixed
           );

       ligne->setStyleSheet(
           "background-color:#24445B;"
           "border:none;"
           );

       l->addWidget(ligne);

        return qMakePair(carte, l);
    };

    // Fonction de creation des barres
    auto ajouterBarre =
        [&](QVBoxLayout *layout,
            const QString &nom,
            int valeur,
            int maximum)
    {
            QWidget *ligne = new QWidget;

            ligne->setAttribute(Qt::WA_StyledBackground, true);

            ligne->setMinimumHeight(28);
            ligne->setMaximumHeight(34);

            ligne->setSizePolicy(
                QSizePolicy::Expanding,
                QSizePolicy::Fixed
                );

            ligne->setStyleSheet(
                "background:transparent;"
                "border:none;"
                );
        QHBoxLayout *h = new QHBoxLayout(ligne);
        h->setContentsMargins(0, 2, 0, 2);
        h->setSpacing(5);

        QLabel *label = new QLabel(nom);
        label->setMinimumWidth(85);
        label->setMaximumWidth(115);

        label->setStyleSheet(
            QString(
                "color:%1;"
                "font-size:10px;"
                "font-weight:500;"
                "background:transparent;"
                "border:none;"
                ).arg(couleurTexte)
            );
        label->setWordWrap(true);

        QProgressBar *barre = new QProgressBar;
        barre->setRange(0, qMax(1, maximum));
        barre->setValue(valeur);
        barre->setTextVisible(false);
        barre->setFixedHeight(9);

        barre->setStyleSheet(
            "QProgressBar {"
           "background:" + couleurBarreFond + ";"
            "border:none;"
            "border-radius:4px;"
            "}"
            "QProgressBar::chunk {"
            "background:#18D9EF;"
            "border-radius:4px;"
            "}"
            );

        QLabel *nombre = new QLabel(
            QString::number(valeur));

        nombre->setFixedWidth(20);
        nombre->setAlignment(Qt::AlignRight);

        nombre->setStyleSheet(
            "color:#18D9EF;"
            "font-size:12px;"
            "font-weight:bold;"
            "background:transparent;"
            "border:none;"
            );

        h->addWidget(label);
        h->addWidget(barre, 1);
        h->addWidget(nombre);

        layout->addWidget(ligne);
    };

    // =================================
    // CARTE 1 : MEMBRES
    // =================================

    auto membres = creerCarte("MEMBRES");

    QLabel *grandTotal = new QLabel(
        QString::number(totalMembres));

    grandTotal->setAlignment(Qt::AlignCenter);

    grandTotal->setStyleSheet(
        "color:#18D9EF;"
        "font-size:29px;"
        "font-weight:bold;"
        "background:transparent;"
        "border:none;"
        );

    // Centrer les statistiques des membres
    membres.second->addStretch(1);

    membres.second->addWidget(
        grandTotal,
        0,
        Qt::AlignCenter
        );

    membres.second->addSpacing(12);

    ajouterBarre(
        membres.second,
        "Hommes",
        hommes,
        qMax(1, hommes + femmes)
        );

    ajouterBarre(
        membres.second,
        "Femmes",
        femmes,
        qMax(1, hommes + femmes)
        );

    membres.second->addStretch(1);

    // =================================
    // CARTE 2 : REPARTITION PAR AGE
    // =================================

    auto ageCarte = creerCarte("REPARTITION PAR AGE");

    QStringList ordreAges = {
        "18 - 25 ans",
        "26 - 30 ans",
        "31 - 35 ans",
        "36 ans et plus",
        "Non precise"
    };

    int maxAge = 1;

    for (const QString &nom : ordreAges)
        maxAge = qMax(maxAge, ages.value(nom));

    for (const QString &nom : ordreAges)
    {
        ajouterBarre(
            ageCarte.second,
            nom,
            ages.value(nom),
            maxAge
            );
    }

    // =================================
    // CARTE 3 : COMPETENCES
    // =================================

    QList<QPair<QString, int>> classement;

    for (auto it = competences.cbegin();
         it != competences.cend(); ++it)
    {
        classement.append(
            qMakePair(it.key(), it.value()));
    }

    std::sort(
        classement.begin(),
        classement.end(),
        [](const QPair<QString, int> &a,
           const QPair<QString, int> &b)
        {
            return a.second > b.second;
        });

    auto competenceCarte =
        creerCarte("COMPETENCES PRINCIPALES");

    int maxCompetence = 1;

    if (!classement.isEmpty())
        maxCompetence = classement.first().second;

    // Zone de defilement reservee aux competences
    QScrollArea *scrollCompetences = new QScrollArea;

    scrollCompetences->setWidgetResizable(true);
    scrollCompetences->setFrameShape(QFrame::NoFrame);
    scrollCompetences->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    scrollCompetences->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);

    scrollCompetences->setStyleSheet(
        "QScrollArea {"
        "background:transparent;"
        "border:none;"
        "}"
        "QScrollArea QWidget {"
        "background:transparent;"
        "}"
        "QScrollBar:vertical {"
        "background:#14283A;"
        "width:5px;"
        "margin:2px 0px 2px 0px;"
        "border:none;"
        "border-radius:2px;"
        "}"
        "QScrollBar::handle:vertical {"
        "background:#18D9EF;"
        "min-height:20px;"
        "border-radius:2px;"
        "}"
        "QScrollBar::add-line:vertical,"
        "QScrollBar::sub-line:vertical {"
        "height:0px;"
        "}"
        "QScrollBar::add-page:vertical,"
        "QScrollBar::sub-page:vertical {"
        "background:transparent;"
        "}"
        );

    // Contenu interieur du scroll
    QWidget *contenuCompetences = new QWidget;

    QVBoxLayout *layoutCompetences =
        new QVBoxLayout(contenuCompetences);

    layoutCompetences->setContentsMargins(2, 3, 5, 3);
    layoutCompetences->setSpacing(7);

    // Afficher toutes les competences
    for (int i = 0; i < classement.size(); ++i)
    {
        QString nomCompetence =
            nomsCompetences.value(
                classement[i].first,
                classement[i].first);

        ajouterBarre(
            layoutCompetences,
            nomCompetence,
            classement[i].second,
            maxCompetence
            );
    }

    if (classement.isEmpty())
    {
        QLabel *vide = new QLabel(
            "Aucune competence disponible");

        vide->setStyleSheet(
            "color:#8DA5BA;"
            "background:transparent;"
            "border:none;"
            "font-size:10px;"
            );

        layoutCompetences->addWidget(vide);
    }

    layoutCompetences->addStretch();

    scrollCompetences->setWidget(contenuCompetences);

    competenceCarte.second->addWidget(
        scrollCompetences, 1);

    // =================================
    // CARTE 4 : REPARTITION PAR SEXE
    // =================================

    auto sexeCarte = creerCarte("REPARTITION PAR SEXE");

    QHBoxLayout *sexeLayout = new QHBoxLayout;
    sexeLayout->setSpacing(5);

    DonutStatistique *donut =
        new DonutStatistique(hommes, femmes);
    donut->setAttribute(Qt::WA_TranslucentBackground);
    donut->setAutoFillBackground(false);

    donut->setStyleSheet(
        "background:transparent;"
        "border:none;"
        );

    sexeLayout->addWidget(donut, 0, Qt::AlignCenter);

    QVBoxLayout *legende = new QVBoxLayout;
    legende->setSpacing(8);

    QLabel *hommesLabel = new QLabel(
        QString("● Hommes : %1").arg(hommes));

    hommesLabel->setStyleSheet(
        "color:#18D9EF;"
        "font-size:10px;"
        "background:transparent;"
        "border:none;"
        );

    QLabel *femmesLabel = new QLabel(
        QString("● Femmes : %1").arg(femmes));

    femmesLabel->setStyleSheet(
        "color:#A879F5;"
        "font-size:10px;"
        "background:transparent;"
        "border:none;"
        );

    legende->addWidget(hommesLabel);
    legende->addWidget(femmesLabel);
    legende->addStretch();

    sexeLayout->addLayout(legende);

    sexeCarte.second->addLayout(sexeLayout);

    // =================================
    // ALIGNEMENT DES QUATRE CARTES
    // =================================

    QHBoxLayout *cartes = new QHBoxLayout;
    cartes->setSpacing(8);

    cartes->addWidget(membres.first, 1);
    cartes->addWidget(ageCarte.first, 1);
    cartes->addWidget(competenceCarte.first, 1.5);
    cartes->addWidget(sexeCarte.first, 1);

    principal->addLayout(cartes);
}


    /* =====================================================
   EXPORT PDF : TABLEAU DES EQUIPES - DESIGN DEVENA
   ===================================================== */

    void MainWindow::exporterPDF()
{
    QString file = QFileDialog::getSaveFileName(
        this,
        "Exporter les equipes en PDF",
        "Devena_Equipes.pdf",
        "PDF (*.pdf)"
        );

    if (file.isEmpty())
        return;

    QPdfWriter pdf(file);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Landscape);
    pdf.setResolution(96);

    QPainter painter(&pdf);

    if (!painter.isActive())
    {
        QMessageBox::warning(
            this, "Erreur",
            "Impossible de creer le PDF."
            );
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    const QColor navy("#071421");
    const QColor navyCard("#102538");
    const QColor turquoise("#18D9EF");
    const QColor white("#FFFFFF");
    const QColor grey("#8EA0B7");
    const QColor lineColor("#29445A");

    const int W = painter.viewport().width();
    const int H = painter.viewport().height();

    const int margin = 35;
    const int availableWidth = W - 2 * margin;

    const int headerHeight = 45;
    const int rowHeight = 38;

    // Recuperer les colonnes sauf Actions
    QList<int> columns;

    for (int c = 0; c < m_table->columnCount(); c++)
    {
        QString header = m_table->horizontalHeaderItem(c)
        ? m_table->horizontalHeaderItem(c)->text()
        : "";

        if (header.compare("Actions", Qt::CaseInsensitive) != 0)
            columns.append(c);
    }

    if (columns.isEmpty())
    {
        painter.end();
        return;
    }

    // Calcul des largeurs
    int totalOriginalWidth = 0;

    for (int c : columns)
        totalOriginalWidth += m_table->columnWidth(c);

    if (totalOriginalWidth <= 0)
    {
        painter.end();
        return;
    }

    QVector<int> widths;

    int usedWidth = 0;

    for (int i = 0; i < columns.size(); i++)
    {
        int width;

        if (i == columns.size() - 1)
        {
            width = availableWidth - usedWidth;
        }
        else
        {
            width = int(
                double(m_table->columnWidth(columns[i]))
                * availableWidth / totalOriginalWidth
                );
        }

        widths.append(width);
        usedWidth += width;
    }

    // Fonction d'affichage de l'en-tete
    auto dessinerHeader = [&]()
    {
        painter.fillRect(0, 0, W, H, navy);

        // Logo texte
        painter.setPen(turquoise);
        painter.setFont(QFont("Segoe UI", 25, QFont::Bold));

        painter.drawText(
            QRect(margin, 25, availableWidth, 42),
            Qt::AlignLeft | Qt::AlignVCenter,
            "DEVENA"
            );

        painter.setPen(grey);
        painter.setFont(QFont("Segoe UI", 9));

        painter.drawText(
            QRect(margin, 68, availableWidth, 22),
            Qt::AlignLeft,
            "SMART HACKATHON MANAGEMENT"
            );

        painter.setPen(turquoise);
        painter.setFont(QFont("Segoe UI", 15, QFont::Bold));

        painter.drawText(
            QRect(margin, 105, availableWidth, 30),
            Qt::AlignCenter,
            "LISTE DES EQUIPES"
            );

        painter.setPen(QPen(turquoise, 2));
        painter.drawLine(
            margin, 145,
            W - margin, 145
            );

        painter.setPen(grey);
        painter.setFont(QFont("Segoe UI", 9));

        painter.drawText(
            QRect(margin, 155, availableWidth, 22),
            Qt::AlignLeft,
            "Nombre total d'equipes : "
                + QString::number(m_table->rowCount())
            );

        // En-tete du tableau
        int y = 190;
        int x = margin;

        painter.setPen(Qt::NoPen);
        painter.setBrush(navyCard);

        painter.drawRoundedRect(
            QRect(margin, y, availableWidth, headerHeight),
            6, 6
            );

        painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
        painter.setPen(turquoise);

        for (int i = 0; i < columns.size(); i++)
        {
            int c = columns[i];

            QString header = m_table->horizontalHeaderItem(c)
                                 ? m_table->horizontalHeaderItem(c)->text()
                                 : "";

            painter.drawText(
                QRect(x + 5, y, widths[i] - 10, headerHeight),
                Qt::AlignCenter,
                header
                );

            x += widths[i];
        }

        return y + headerHeight;
    };

    // Premiere page
    int y = dessinerHeader();

    // Lignes du tableau
    for (int r = 0; r < m_table->rowCount(); r++)
    {
        // Nouvelle page si necessaire
        if (y + rowHeight > H - 65)
        {
            // Footer
            painter.setPen(lineColor);
            painter.drawLine(
                margin, H - 48,
                W - margin, H - 48
                );

            painter.setPen(turquoise);
            painter.setFont(QFont("Segoe UI", 9, QFont::Bold));

            painter.drawText(
                QRect(margin, H - 40, availableWidth, 20),
                Qt::AlignCenter,
                "DEVENA - HACK TODAY / BUILD TOMORROW"
                );

            pdf.newPage();
            y = dessinerHeader();
        }

        int x = margin;

        // Fond alterne
        QColor rowColor = (r % 2 == 0)
                              ? QColor("#102538")
                              : QColor("#0B1C2B");

        painter.setPen(Qt::NoPen);
        painter.setBrush(rowColor);

        painter.drawRect(
            margin, y,
            availableWidth, rowHeight
            );

        // Donnees
        painter.setFont(QFont("Segoe UI", 8));
        painter.setPen(white);

        for (int i = 0; i < columns.size(); i++)
        {
            int c = columns[i];

            QTableWidgetItem *item = m_table->item(r, c);
            QString value = item ? item->text() : "-";

            // Alignement selon la colonne
            Qt::Alignment alignment = Qt::AlignCenter;

            if (c == 1 || c == 6)
                alignment = Qt::AlignVCenter | Qt::AlignLeft;

            painter.drawText(
                QRect(x + 5, y + 2, widths[i] - 10, rowHeight - 4),
                alignment,
                painter.fontMetrics().elidedText(
                    value,
                    Qt::ElideRight,
                    widths[i] - 14
                    )
                );

            x += widths[i];
        }

        // Separation des lignes
        painter.setPen(QPen(lineColor, 1));

        painter.drawLine(
            margin, y + rowHeight,
            W - margin, y + rowHeight
            );

        y += rowHeight;
    }

    // Footer derniere page
    painter.setPen(lineColor);
    painter.drawLine(
        margin, H - 48,
        W - margin, H - 48
        );

    painter.setPen(turquoise);
    painter.setFont(QFont("Segoe UI", 9, QFont::Bold));

    painter.drawText(
        QRect(margin, H - 40, availableWidth, 20),
        Qt::AlignCenter,
        "DEVENA - HACK TODAY / BUILD TOMORROW"
        );

    painter.end();

    QMessageBox::information(
        this,
        "Export PDF",
        "Le tableau des equipes a ete exporte avec succes."
        );
}

/* =====================================================
   EXPORT PDF : FICHE EQUIPE - DESIGN DEVENA
   ===================================================== */

void MainWindow::exporterEquipePDF(int index)
{
    if (index < 0 || index >= m_table->rowCount())
        return;

    auto valeur = [this, index](int col) -> QString
    {
        QTableWidgetItem *item = m_table->item(index, col);
        return item ? item->text() : "-";
    };

    QString id = valeur(0);
    QString nom = valeur(1);

    QString file = QFileDialog::getSaveFileName(
        this,
        "Exporter la fiche equipe",
        "Devena_Equipe_" + id + ".pdf",
        "PDF (*.pdf)"
        );

    if (file.isEmpty())
        return;

    QPdfWriter pdf(file);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Portrait);
    pdf.setResolution(96);

    QPainter p(&pdf);

    if (!p.isActive())
    {
        QMessageBox::warning(
            this, "Erreur",
            "Impossible de creer le PDF."
            );
        return;
    }

    const int W = p.viewport().width();
    const int H = p.viewport().height();

    const QColor navy("#071421");
    const QColor navyCard("#102538");
    const QColor turquoise("#18D9EF");
    const QColor white("#FFFFFF");
    const QColor grey("#8EA0B7");

    // Fond principal
    p.fillRect(0, 0, W, H, navy);

    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // ================= HEADER =================

    p.setPen(turquoise);
    p.setFont(QFont("Segoe UI", 27, QFont::Bold));

    p.drawText(
        QRect(45, 45, W - 90, 55),
        Qt::AlignLeft | Qt::AlignVCenter,
        "DEVENA"
        );

    p.setPen(grey);
    p.setFont(QFont("Segoe UI", 9));

    p.drawText(
        QRect(45, 100, W - 90, 25),
        Qt::AlignLeft,
        "SMART HACKATHON MANAGEMENT"
        );

    p.setPen(turquoise);
    p.setFont(QFont("Segoe UI", 11, QFont::Bold));

    p.drawText(
        QRect(45, 145, W - 90, 35),
        Qt::AlignCenter,
        "FICHE EQUIPE"
        );

    p.setPen(QPen(turquoise, 2));
    p.drawLine(45, 190, W - 45, 190);

    // ================= IDENTITE EQUIPE =================

    int y = 220;

    p.setPen(white);
    p.setFont(QFont("Segoe UI", 21, QFont::Bold));

    p.drawText(
        QRect(50, y, W - 100, 40),
        Qt::AlignCenter,
        nom
        );

    p.setPen(turquoise);
    p.setFont(QFont("Segoe UI", 11, QFont::DemiBold));

    p.drawText(
        QRect(50, y + 43, W - 100, 25),
        Qt::AlignCenter,
        "ID : " + id
        );

    // ================= CARTES STATISTIQUES =================

    int margin = 45;
    int gap = 14;
    int cardW = (W - 2 * margin - gap) / 2;
    int cardH = 85;
    int cardY = 320;

    QStringList statTitles = {
        "MEMBRES",
        "CAPACITE",
        "SEXE",
        "AGE MOYEN"
    };

    QStringList statValues = {
        valeur(2),
        valeur(3),
        valeur(4),
        valeur(5)
    };

    for (int i = 0; i < 4; ++i)
    {
        int col = i % 2;
        int row = i / 2;

        int x = margin + col * (cardW + gap);
        int cy = cardY + row * (cardH + gap);

        p.setPen(Qt::NoPen);
        p.setBrush(navyCard);

        p.drawRoundedRect(
            QRect(x, cy, cardW, cardH),
            10, 10
            );

        p.setPen(grey);
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));

        p.drawText(
            QRect(x + 15, cy + 12, cardW - 30, 22),
            Qt::AlignLeft,
            statTitles[i]
            );

        p.setPen(white);
        p.setFont(QFont("Segoe UI", 18, QFont::Bold));

        p.drawText(
            QRect(x + 15, cy + 36, cardW - 30, 35),
            Qt::AlignLeft | Qt::AlignVCenter,
            statValues[i]
            );
    }

    // ================= INFORMATIONS =================

    int infoY = 535;

    p.setPen(turquoise);
    p.setFont(QFont("Segoe UI", 12, QFont::Bold));

    p.drawText(
        QRect(50, infoY, W - 100, 30),
        Qt::AlignLeft,
        "INFORMATIONS DE L'EQUIPE"
        );

    infoY += 40;

    QStringList labels = {
        "Competences",
        "Date de creation"
    };

    QStringList values = {
        valeur(6),
        valeur(7)
    };

    for (int i = 0; i < labels.size(); ++i)
    {
        p.setPen(Qt::NoPen);
        p.setBrush(navyCard);

        p.drawRoundedRect(
            QRect(45, infoY, W - 90, 55),
            8, 8
            );

        p.setPen(grey);
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));

        p.drawText(
            QRect(60, infoY + 5, W - 120, 20),
            Qt::AlignLeft,
            labels[i]
            );

        p.setPen(white);
        p.setFont(QFont("Segoe UI", 10));

        p.drawText(
            QRect(60, infoY + 25, W - 120, 25),
            Qt::AlignLeft | Qt::AlignVCenter,
            values[i]
            );

        infoY += 65;
    }

    // ================= QR CODE =================

    QString code = "DEVE-EQ-" + id;
    QImage qrImage = creerQR(code);

    if (!qrImage.isNull())
    {
        int qrSize = 145;
        int qrX = (W - qrSize) / 2;
        int qrY = infoY + 15;

        // Cadre blanc
        p.setPen(Qt::NoPen);
        p.setBrush(white);

        p.drawRoundedRect(
            QRect(qrX - 10, qrY - 10,
                  qrSize + 20, qrSize + 20),
            8, 8
            );

        p.drawImage(
            QRect(qrX, qrY, qrSize, qrSize),
            qrImage
            );

        p.setPen(turquoise);
        p.setFont(QFont("Segoe UI", 10, QFont::Bold));

        p.drawText(
            QRect(45, qrY + qrSize + 20,
                  W - 90, 25),
            Qt::AlignCenter,
            code
            );
    }

    // ================= FOOTER =================

    int footerY = H - 65;

    p.setPen(QPen(QColor("#29445A"), 1));
    p.drawLine(45, footerY, W - 45, footerY);

    p.setPen(turquoise);
    p.setFont(QFont("Segoe UI", 10, QFont::Bold));

    p.drawText(
        QRect(45, footerY + 10, W - 90, 20),
        Qt::AlignCenter,
        "DEVENA"
        );

    p.setPen(grey);
    p.setFont(QFont("Segoe UI", 8));

    p.drawText(
        QRect(45, footerY + 30, W - 90, 18),
        Qt::AlignCenter,
        "HACK TODAY / BUILD TOMORROW"
        );

    p.end();

    QMessageBox::information(
        this,
        "Export PDF",
        "La fiche de l'equipe a ete exportee avec succes."
        );
}

/* =====================================================
   MODE CLAIR
   ===================================================== */

void MainWindow::changerModeClair()
{
    m_modeClair = !m_modeClair;

    if (m_modeClair)
    {
        setStyleSheet(STYLE_LIGHT);

        m_side->setStyleSheet(
            "QWidget{"
            "background:#FFFFFF;"
            "border-right:1px solid #D6E0EA;"
            "}"
            "QPushButton{"
            "text-align:left;"
            "border:0;"
            "background:transparent;"
            "color:#64748B;"
            "padding:12px;"
            "}"
            "QPushButton#active{"
            "background:#E3F8FB;"
            "color:#087F91;"
            "border:1px solid #18D9EF;"
            "}"
            );

        QLabel *logo = findChild<QLabel *>("logoLabel");
        QLabel *slogan = findChild<QLabel *>("sloganLabel");
        QLabel *footer = findChild<QLabel *>("footerLabel");
        QLabel *title = findChild<QLabel *>("titleLabel");
        QLabel *subtitle = findChild<QLabel *>("subtitleLabel");
        QLabel *tableTitle = findChild<QLabel *>("tableTitleLabel");
        QLabel *statsTitle = findChild<QLabel *>("statsTitleLabel");

        if (logo) logo->setStyleSheet("color:#087F91;font-size:27px;font-weight:bold;");
        if (slogan) slogan->setStyleSheet("color:#64748B;");
        if (footer) footer->setStyleSheet("color:#94A3B8;");
        if (title) title->setStyleSheet("font-size:30px;font-weight:bold;color:#182433;");
        if (subtitle) subtitle->setStyleSheet("color:#64748B;");
        if (tableTitle) tableTitle->setStyleSheet("font-size:19px;font-weight:bold;color:#182433;");
        if (statsTitle) statsTitle->setStyleSheet("font-size:20px;font-weight:bold;color:#182433;");
    }
    else
    {
        setStyleSheet(STYLE_DARK);

        m_side->setStyleSheet(
            "QWidget{"
            "background:#030914;"
            "border-right:1px solid #17283A;"
            "}"
            "QPushButton{"
            "text-align:left;"
            "border:0;"
            "background:transparent;"
            "color:#718198;"
            "padding:12px;"
            "}"
            "QPushButton#active{"
            "background:#092A3D;"
            "color:#18D9EF;"
            "border:1px solid #18D9EF;"
            "}"
            );

        QLabel *logo = findChild<QLabel *>("logoLabel");
        QLabel *slogan = findChild<QLabel *>("sloganLabel");
        QLabel *footer = findChild<QLabel *>("footerLabel");
        QLabel *title = findChild<QLabel *>("titleLabel");
        QLabel *subtitle = findChild<QLabel *>("subtitleLabel");
        QLabel *tableTitle = findChild<QLabel *>("tableTitleLabel");
        QLabel *statsTitle = findChild<QLabel *>("statsTitleLabel");

        if (logo) logo->setStyleSheet("color:#18D9EF;font-size:27px;font-weight:bold;");
        if (slogan) slogan->setStyleSheet("color:#718198;");
        if (footer) footer->setStyleSheet("color:#3A4D65;");
        if (title) title->setStyleSheet("font-size:30px;font-weight:bold;color:white;");
        if (subtitle) subtitle->setStyleSheet("color:#7D8BA3;");
        if (tableTitle) tableTitle->setStyleSheet("font-size:19px;font-weight:bold;color:white;");
        if (statsTitle) statsTitle->setStyleSheet("font-size:20px;font-weight:bold;color:white;");
    }

   afficherOrganigramme();
}

// ==========================================
// FILTRER LES EQUIPES
// ==========================================

void MainWindow::filtrer()
{
    QString recherche = m_recherche->text().trimmed().toLower();
    QString filtre = m_filtre->currentText();

    for (int i = 0; i < m_table->rowCount(); ++i)
    {
        bool visible = true;

        QString nomEquipe = m_table->item(i, 1)
                                ? m_table->item(i, 1)->text().toLower()
                                : "";

        int nbMembres = m_table->item(i, 2)
                            ? m_table->item(i, 2)->text().toInt()
                            : 0;

        int capacite = m_table->item(i, 3)
                           ? m_table->item(i, 3)->text().toInt()
                           : 0;

        // Recherche par nom d'equipe
        if (!recherche.isEmpty() && !nomEquipe.contains(recherche))
            visible = false;

        // Filtrage selon le nombre de membres
        if (filtre == "Equipe avec membres" && nbMembres == 0)
            visible = false;

        else if (filtre == "Equipe vide" && nbMembres != 0)
            visible = false;

        else if (filtre == "Equipe complete" && nbMembres < capacite)
            visible = false;

        m_table->setRowHidden(i, !visible);
    }
}
void MainWindow::trierEquipes()
{
    if (!m_table || !m_tri)
        return;

    int colonne = 0;

    switch (m_tri->currentIndex())
    {
    case 0:
        colonne = 0; // ID
        break;

    case 1:
        colonne = 1; // Nom
        break;

    case 2:
        colonne = 5; // Age moyen
        break;
    }

    QList<int> ordre;

    for (int i = 0; i < m_table->rowCount(); ++i)
        ordre.append(i);

    std::stable_sort(ordre.begin(), ordre.end(),
                     [this, colonne](int a, int b)
                     {
                         QString va = m_table->item(a, colonne)->text();
                         QString vb = m_table->item(b, colonne)->text();

                         int r = 0;

                         if (colonne == 0)
                         {
                             r = va.mid(1).toInt() - vb.mid(1).toInt();
                         }
                         else if (colonne == 1)
                         {
                             r = QString::localeAwareCompare(va, vb);
                         }
                         else
                         {
                             r = va.toDouble() < vb.toDouble() ? -1 :
                                     va.toDouble() > vb.toDouble() ? 1 : 0;
                         }

                         return m_triCroissant ? r < 0 : r > 0;
                     });

    // Réorganiser les lignes
    m_table->setSortingEnabled(false);

    for (int i = 0; i < ordre.size(); ++i)
        m_table->verticalHeader()->moveSection(
            m_table->verticalHeader()->visualIndex(ordre[i]), i);
}
/* =====================================================
   RAFRAICHIR
   ===================================================== */
void MainWindow::rafraichir()
{
    afficherEquipes();
   afficherOrganigramme();
}