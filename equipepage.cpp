#include "equipepage.h"

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
EquipePage::EquipePage(QWidget *parent)
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

EquipePage::~EquipePage()
{
}
/* =====================================================
   INTERFACE
   ===================================================== */

void EquipePage::creerInterface()
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
        else if (i != 6)   // tout sauf "Parametre"
        {
            // Integration : ces modules existent dans l'application
            const QString module = menu[i];
            b->setCursor(Qt::PointingHandCursor);
            connect(b, &QPushButton::clicked, this, [this, module]() {
                emit naviguer(module);
            });
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

    filters->addWidget(m_recherche, 1);
    filters->addWidget(m_filtre);

    v->addLayout(filters);

    connect(m_recherche, &QLineEdit::textChanged, this, [this](const QString &) { filtrer(); });
    connect(m_filtre, &QComboBox::currentTextChanged, this, [this](const QString &) { filtrer(); });

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
    v->addWidget(m_table, 1);

    /* ================= STATISTIQUES ================= */

    QLabel *statsTitle = new QLabel("Statistiques");
    statsTitle->setObjectName("statsTitleLabel");
    statsTitle->setStyleSheet(
        "font-size:20px;"
        "font-weight:bold;"
        "color:white;"
        );

    v->addWidget(statsTitle);

    m_courbe = new QLabel;
    m_courbe->setMinimumHeight(165);
    m_courbe->setAlignment(Qt::AlignCenter);

    v->addWidget(m_courbe);

    QHBoxLayout *stats = new QHBoxLayout;

    auto statCard = [](const QString &name, QLabel **value) {
        QFrame *box = new QFrame;
        box->setObjectName("statCard");

        QVBoxLayout *l = new QVBoxLayout(box);

        QLabel *n = new QLabel(name);
        n->setStyleSheet("color:#7D8BA3;");

        *value = new QLabel("0");
        (*value)->setStyleSheet(
            "color:#18D9EF;"
            "font-size:21px;"
            "font-weight:bold;"
            );

        l->addWidget(n);
        l->addWidget(*value);

        return box;
    };

    stats->addWidget(statCard("Equipes", &m_totalEquipes));
    stats->addWidget(statCard("Membres", &m_totalMembres));
    stats->addWidget(statCard("Hommes", &m_totalHommes));
    stats->addWidget(statCard("Femmes", &m_totalFemmes));

    v->addLayout(stats);
    root->addWidget(body, 1);
}




/* =====================================================
   IDS
   ===================================================== */

int EquipePage::prochainIdEquipe() const
{
    int id = 0;
    for (const Equipe &e : m_equipes)
        id = qMax(id, e.id());
    return id + 1;
}

int EquipePage::prochainIdParticipant() const
{
    int id = 0;
    for (const Participant &p : m_participants)
        id = qMax(id, p.id);
    return id + 1;
}

/* =====================================================
   MEILLEURE EQUIPE
   ===================================================== */

int EquipePage::meilleureEquipe(const Participant &p) const
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

void EquipePage::afficherNotification(const QString &texte, bool succes)
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

void EquipePage::affecterAutomatiquement()
{
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
bool ouvrirPopupGestionEquipe(
    QWidget *parent,
    QTableWidget *tablePrincipal,
    const Equipe *equipe,
    int idEquipe,
    int prochainId,
    QString &nomEquipe,
    QVector<Participant> &membres,
    bool lectureSeule = false)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(equipe ? "Modifier equipe" : "Ajouter equipe");
    dialog.resize(1070, 650);
    dialog.setMinimumSize(1070, 650);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *titre = new QLabel(
        equipe ? "MODIFIER UNE EQUIPE" : "AJOUTER UNE EQUIPE"
        );
    titre->setStyleSheet(
        "font-size:20px; font-weight:bold; color:#18D9EF;"
        );
    layout->addWidget(titre);

    // ===== TABLEAU EQUIPE : MEME COLONNES QUE L'INTERFACE PRINCIPALE =====

    QLabel *titreEquipe = new QLabel("Informations de l'equipe");
    titreEquipe->setStyleSheet("font-size:16px; font-weight:bold;");
    layout->addWidget(titreEquipe);


    QTableWidget *tableEquipe = new QTableWidget(1, 10);
    tableEquipe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    tableEquipe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableEquipe->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableEquipe->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableEquipe->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList entetes;

    // Récupération des titres des 8 premières colonnes
    for (int c = 0; c < 8; ++c)
    {
        if (c == 7)
            entetes << "Date_creation";
        else
            entetes << tablePrincipal->horizontalHeaderItem(c)->text();
    }
    // Ajout des clés étrangères
    entetes << "ID Projet";
    entetes << "ID Evaluation";
    tableEquipe->setHorizontalHeaderLabels(entetes);

    tableEquipe->setStyleSheet(tablePrincipal->styleSheet());
    tableEquipe->setAlternatingRowColors(true);
    tableEquipe->verticalHeader()->setVisible(false);
    tableEquipe->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Cacher definitivement la scrollbar horizontale
    tableEquipe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Adapter les 10 colonnes a la largeur disponible
    tableEquipe->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    // Désactivation de la barre de défilement horizontale
    tableEquipe->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QString id = "E" + QString::number(idEquipe);
    QString nom = equipe ? equipe->nom() : "";
    QStringList valeursEquipe = {
        id,
        nom,
        "0",
        "6",
        "0 H / 0 F",
        "-",
        "-",
        QDate::currentDate().toString("dd/MM/yyyy"),
        QString::number(equipe ? equipe->idProjet() : 0),
        QString::number(equipe ? equipe->idEvaluation() : 0)
    };
    for (int c = 0; c < 10; ++c)
    {
        QTableWidgetItem *item =
            new QTableWidgetItem(valeursEquipe[c]);

        // Ajouter : Nom et Date modifiables
        // Modifier : Nom uniquement
        // Voir : aucun champ modifiable

        bool modifiable =
            !lectureSeule &&
            (c == 1 || (equipe == nullptr && c == 7));

        if (modifiable)
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        else
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);

        tableEquipe->setItem(0, c, item);
    }
    tableEquipe->setRowHeight(0, 48);
    QLineEdit *champNomEquipe = new QLineEdit();
    champNomEquipe->setPlaceholderText("Nom de l'equipe");
    champNomEquipe->setText(nom);

    // ===== AFFICHAGE DU TABLEAU EQUIPE =====
    tableEquipe->horizontalScrollBar()->hide();
      layout->addWidget(tableEquipe);


    champNomEquipe->hide();
    // ===== TABLEAU DES MEMBRES =====

    QLabel *titreMembres = new QLabel("Membres de l'equipe (maximum 6)");
    titreMembres->setStyleSheet("font-size:16px; font-weight:bold;");
    layout->addWidget(titreMembres);
 titreMembres->hide();

    // ===== TABLEAU DES MEMBRES =====

    int nbColonnes = 6;

    QTableWidget *tableMembres = new QTableWidget(0, nbColonnes);
    tableMembres->setEditTriggers(
        equipe ? QAbstractItemView::NoEditTriggers
               : QAbstractItemView::CurrentChanged |
                     QAbstractItemView::AnyKeyPressed
        );

    tableMembres->setHorizontalHeaderLabels({
        "ID", "Nom", "Prenom", "Sexe", "Age", "Competence"
    });


    // Répartition automatique de la largeur des colonnes
    tableMembres->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableMembres->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tableMembres->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    tableMembres->setMinimumHeight(180);
    tableMembres->setMaximumHeight(300);

    tableMembres->setAlternatingRowColors(false);

    tableMembres->setStyleSheet(
        "QTableWidget {"
        "   color: white;"
        "   background-color: #07111D;"
        "   font-size: 14px;"
        "   gridline-color: #183044;"
        "}"
        "QTableWidget::item {"
        "   color: white;"
        "   background-color: #07111D;"
        "   padding: 8px;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #07111D;"
        "   color: white;"
        "}"
        // Style des champs de saisie
        "QLineEdit {"
        "   color: white;"
        "   background-color: #07111D;"
        "   font-size: 14px;"
        "   font-family: Segoe UI;"
        "   padding: 6px;"
        "   border: none;"
        "   border-radius: 0px;"
        "   selection-background-color: #087E91;"
        "   selection-color: white;"
        "}"
        "QComboBox {"
        "   color: white;"
        "   background-color: #07111D;"
        "   border: 1px solid #183044;"
        "   border-radius: 7px;"
        "   padding: 6px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   color: white;"
        "   background-color: #07111D;"
        "   selection-background-color: #10394B;"
        "   selection-color: white;"
        "}"
        );


    tableMembres->setAlternatingRowColors(true);
    tableMembres->verticalHeader()->setVisible(false);

    tableMembres->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableMembres->setSelectionMode(QAbstractItemView::SingleSelection);


    tableMembres->setWordWrap(false);
    tableMembres->setTextElideMode(Qt::ElideNone);


    // ===== DIMENSIONS DU TABLEAU =====

    tableMembres->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Fixed
        );

    tableMembres->setColumnWidth(0, 65);   // ID
    tableMembres->setColumnWidth(1, 145);  // Nom
    tableMembres->setColumnWidth(2, 145);  // Prenom
    tableMembres->setColumnWidth(3, 100);  // Sexe
    tableMembres->setColumnWidth(4, 80);   // Age
    tableMembres->setColumnWidth(5, 175);  // Competence

    tableMembres->verticalHeader()->setDefaultSectionSize(42);
    tableMembres->horizontalHeader()->setMinimumHeight(42);

    tableMembres->setMinimumHeight(180);
    tableMembres->setMaximumHeight(300);

    tableMembres->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    tableMembres->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    tableMembres->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    if (equipe)
        membres = equipe->membres();
    else
        membres.clear();

    // ===== AJOUTER UNE LIGNE MEMBRE =====

    auto ajouterLigne = [&](const Participant *p)
    {
        int ligne = tableMembres->rowCount();
        tableMembres->insertRow(ligne);

        int nouvelId = p ? p->id : prochainId++;

        QStringList valeurs;
        valeurs << QString::number(nouvelId);

        if (p)
        {
            valeurs << p->nom
                    << p->prenom
                    << p->sexe
                    << QString::number(p->age)
                    << p->competence;
        }
        else
        {
            valeurs << "" << "" << "" << "" << "";
        }

        for (int c = 0; c < 6; ++c)
        {
            QTableWidgetItem *item =
                new QTableWidgetItem(valeurs[c]);

            item->setForeground(QBrush(Qt::white));

            if (c == 0)
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            else
                item->setFlags(item->flags() | Qt::ItemIsEditable);

            if (c == 3)
            {
                QString sexe = item ? item->text().trimmed() : "";

                // Création de la liste déroulante du sexe
                QComboBox *sexeCombo = new QComboBox();

                sexeCombo->addItem("Homme");
                sexeCombo->addItem("Femme");

                // Définition des dimensions de la liste
                sexeCombo->setMinimumWidth(170);
                sexeCombo->setMinimumHeight(32);
                sexeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

                // Sélection de la valeur existante
                int index = sexeCombo->findText(sexe, Qt::MatchFixedString);
                sexeCombo->setCurrentIndex(index >= 0 ? index : 0);

                tableMembres->setCellWidget(ligne, c, sexeCombo);

                delete item;
            }
            else
            {
                tableMembres->setItem(ligne, c, item);
            }
        }

        tableMembres->setRowHeight(ligne, 42);

        // Adapter les colonnes a la largeur du tableau
        tableMembres->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch
            );
    };

    // Chargement des membres de l'equipe
    if (equipe)
        membres = equipe->membres();
    else
        membres.clear();

    // Affichage des membres dans le tableau
    for (const Participant &p : membres)
        ajouterLigne(&p);
    // Adaptation automatique des colonnes
    tableMembres->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Affichage du tableau
    layout->addWidget(tableMembres, 1);
    tableMembres->hide();
    // ===== ACTUALISER LES INFORMATIONS DU TABLEAU EQUIPE =====

    auto actualiserApercu = [&]()
    {
        int hommes = 0;
        int femmes = 0;
        int totalAge = 0;
        int nbAges = 0;
        QStringList competences;

        for (int i = 0; i < tableMembres->rowCount(); ++i)
        {
            // Sexe = colonne 3 (ComboBox)
            QComboBox *sexeCombo =
                qobject_cast<QComboBox*>(tableMembres->cellWidget(i, 3));

            QString sexe = sexeCombo
                               ? sexeCombo->currentText().trimmed()
                               : "";

            // Age = colonne 4
            bool ok = false;

            int age = tableMembres->item(i, 4)
                          ? tableMembres->item(i, 4)->text().toInt(&ok)
                          : 0;

            if (ok)
            {
                totalAge += age;
                ++nbAges;
            }

            // Competence = colonne 5
            QString competence = tableMembres->item(i, 5)
                                     ? tableMembres->item(i, 5)->text().trimmed()
                                     : "";

            if (!competence.isEmpty() &&
                !competences.contains(competence, Qt::CaseInsensitive))
            {
                competences.append(competence);
            }
        }

        tableEquipe->item(0, 1)->setText(
            tableEquipe->item(0, 1)->text().trimmed()
            );

        tableEquipe->item(0, 2)->setText(
            QString::number(tableMembres->rowCount())
            );

        tableEquipe->item(0, 3)->setText("6");

        tableEquipe->item(0, 4)->setText(
            QString::number(hommes) + " H / " +
            QString::number(femmes) + " F"
            );

        tableEquipe->item(0, 5)->setText(
            nbAges > 0
                ? QString::number(double(totalAge) / nbAges, 'f', 1)
                : "-"
            );

        tableEquipe->item(0, 6)->setText(
            competences.isEmpty() ? "-" : competences.join(", ")
            );
    };

    QObject::connect(
        tableMembres,
        &QTableWidget::itemChanged,
        &dialog,
        [&](QTableWidgetItem *item)
        {
            actualiserApercu();

            if (item)
                tableMembres->resizeColumnToContents(item->column());
        }
        );

    actualiserApercu();


    // ===== ENREGISTRER / ANNULER =====

    QHBoxLayout *boutons = new QHBoxLayout();

    QPushButton *enregistrer = new QPushButton("Enregistrer");
    QPushButton *annuler = new QPushButton("Annuler");

    enregistrer->setObjectName("primary");

    // Entrer Enregistrer
    enregistrer->setDefault(true);
    enregistrer->setAutoDefault(true);

    // EscapeAnnuler
    annuler->setAutoDefault(false);

    boutons->addStretch();
    boutons->addWidget(enregistrer);
    boutons->addWidget(annuler);

    layout->addLayout(boutons);

    QObject::connect(enregistrer, &QPushButton::clicked, &dialog, [&]()
                     {
                         QString nom = tableEquipe->item(0, 1)->text().trimmed();
                         if (nom.isEmpty())
                         {
                             QMessageBox::warning(
                                 &dialog, "Erreur",
                                 "Le nom de l'equipe est obligatoire."
                                 );
                             return;
                         }
                         // Modification : changer uniquement le nom de l'equipe
                         if (equipe)
                         {
                             nomEquipe = nom;
                             membres = equipe->membres();
                             dialog.accept();
                             return;
                         }

                         QVector<Participant> nouveauxMembres;

                         for (int i = 0; i < tableMembres->rowCount(); ++i)
                         {
                             // ID
                             if (!tableMembres->item(i, 0))
                             {
                                 QMessageBox::warning(&dialog, "Erreur", "ID invalide.");
                                 return;
                             }

                             int idMembre = tableMembres->item(i, 0)->text().toInt();

                             // Nom
                             QString nomMembre = tableMembres->item(i, 1)
                                                     ? tableMembres->item(i, 1)->text().trimmed()
                                                     : "";

                             // Prenom
                             QString prenom = tableMembres->item(i, 2)
                                                  ? tableMembres->item(i, 2)->text().trimmed()
                                                  : "";

                             // Sexe : récupération depuis ComboBox
                             QComboBox *sexeCombo =
                                 qobject_cast<QComboBox*>(tableMembres->cellWidget(i, 3));

                             QString sexe = sexeCombo
                                                ? sexeCombo->currentText().trimmed()
                                                : "";

                             // Age
                             QString ageText = tableMembres->item(i, 4)
                                                   ? tableMembres->item(i, 4)->text().trimmed()
                                                   : "";

                             // Competence
                             QString competence = tableMembres->item(i, 5)
                                                      ? tableMembres->item(i, 5)->text().trimmed()
                                                      : "";

                             bool ageValide = false;
                             int age = ageText.toInt(&ageValide);

                             if (nomMembre.isEmpty() ||
                                 prenom.isEmpty() ||
                                 competence.isEmpty() ||
                                 !ageValide ||
                                 age < 16 || age > 80 ||
                                 (sexe != "Homme" && sexe != "Femme"))

                             {
                                 QMessageBox::warning(
                                     &dialog,
                                     "Erreur",
                                     "Verifiez les informations du membre."
                                     );
                                 return;
                             }

                             Participant nouveau;

                             nouveau.id = idMembre;
                             nouveau.nom = nomMembre;
                             nouveau.prenom = prenom;
                             nouveau.sexe = sexe;
                             nouveau.age = age;
                             nouveau.competence = competence;

                             nouveauxMembres.append(nouveau);
                         }

                         nomEquipe = nom;
                         membres = nouveauxMembres;
                         dialog.accept();
                     });

    QObject::connect(annuler, &QPushButton::clicked,
                     &dialog, &QDialog::reject);
    if (lectureSeule)
    {
        tableEquipe->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableMembres->setEditTriggers(QAbstractItemView::NoEditTriggers);

        enregistrer->setText("Fermer");
        enregistrer->setObjectName("");
        annuler->hide();
    }

    return dialog.exec() == QDialog::Accepted;
}
/* =====================================================
   AJOUT EQUIPE
   ===================================================== */

void EquipePage::ajouterEquipe()
{


    QString nom;
    QVector<Participant> membres;

    if (!ouvrirPopupGestionEquipe(
            this,
            m_table,
            nullptr,
            prochainIdEquipe(),
            prochainIdParticipant(),
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

void EquipePage::modifierEquipe(int index)
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
            membres))
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
   SUPPRIMER
   ===================================================== */

void EquipePage::supprimerEquipe(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    const Equipe &e = m_equipes[index];

    // ================= DIALOG =================

    QDialog dialog(this);
    dialog.setWindowTitle("Suppression de l'equipe");
    dialog.resize(1070, 650);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *titre = new QLabel("SUPPRESSION DE L'EQUIPE");

    titre->setStyleSheet(
        "font-size:20px;"
        "font-weight:bold;"
        "color:#18D9EF;"
        );

    layout->addWidget(titre);

    // ================= TABLEAU =================

    QTableWidget *table = new QTableWidget(1, 10);

    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList entetes;

    for (int c = 0; c < 8; ++c)
    {
        if (c == 7)
            entetes << "Date_creation";
        else if (m_table && m_table->horizontalHeaderItem(c))
            entetes << m_table->horizontalHeaderItem(c)->text();
        else
            entetes << QString("Colonne %1").arg(c + 1);
    }

    entetes << "ID Projet";
    entetes << "ID Evaluation";

    table->setHorizontalHeaderLabels(entetes);
    table->setStyleSheet(m_table->styleSheet());

    QVector<Participant> membres = e.membres();

    int hommes = 0;
    int femmes = 0;
    int sommeAge = 0;

    QStringList competences;

    for (const Participant &p : membres)
    {
        QString sexe = p.sexe.trimmed().toLower();

        if (sexe == "homme" || sexe == "h")
            hommes++;
        else if (sexe == "femme" || sexe == "f")
            femmes++;

        sommeAge += p.age;

        QString comp = p.competence.trimmed();

        if (!comp.isEmpty() && comp != "-" &&
            !competences.contains(comp))
        {
            competences << comp;
        }
    }

    QString sexeText = QString("%1 H / %2 F")
                           .arg(hommes)
                           .arg(femmes);

    QString ageText = membres.isEmpty()
                          ? "-"
                          : QString::number(
                                qRound((double)sommeAge / membres.size())
                                );

    QString competenceText = competences.isEmpty()
                                 ? "-"
                                 : competences.join(", ");

    QString dateText = "-";

    if (m_table && index < m_table->rowCount() &&
        m_table->item(index, 7))
    {
        dateText = m_table->item(index, 7)->text();
    }

    QStringList valeurs = {
        "E" + QString::number(e.id()),
        e.nom(),
        QString::number(e.taille()),
        "6",
        sexeText,
        ageText,
        competenceText,
        dateText,
        QString::number(e.idProjet()),
        QString::number(e.idEvaluation())
    };

    for (int c = 0; c < 10; ++c)
    {
        QTableWidgetItem *item = new QTableWidgetItem(valeurs[c]);

        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

        table->setItem(0, c, item);
    }

    table->setRowHeight(0, 48);

    table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents
        );

    table->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Interactive
        );

    table->setColumnWidth(1, 180);

    table->verticalHeader()->setVisible(false);

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    layout->addWidget(table);

    // ================= BOUTONS =================

    QHBoxLayout *boutons = new QHBoxLayout();

    QPushButton *btnSupprimer = new QPushButton("Supprimer");
    QPushButton *btnAnnuler = new QPushButton("Annuler");

    btnSupprimer->setStyleSheet(
        "background-color:#18D9EF;"
        "color:#050B14;"
        "padding:10px 20px;"
        "border-radius:7px;"
        "font-weight:bold;"
        "border:none;"
        );

    btnAnnuler->setStyleSheet(
        "background-color:#34495E;"
        "color:white;"
        "padding:10px 20px;"
        "border-radius:7px;"
        );

    boutons->addStretch();
    boutons->addWidget(btnSupprimer);
    boutons->addWidget(btnAnnuler);

    layout->addLayout(boutons);

    // ================= ANNULER =================

    connect(btnAnnuler, &QPushButton::clicked,
            &dialog, &QDialog::reject);

    // ================= SUPPRIMER =================

    connect(btnSupprimer, &QPushButton::clicked,
            &dialog, [this, index, &dialog]()
            {
                if (index < 0 || index >= m_equipes.size())
                    return;

                if (m_equipes.size() == 1 && !m_participants.isEmpty())
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


            });

    dialog.exec();
}

/* =====================================================
   VOIR EQUIPE
   ===================================================== */


void EquipePage::voirEquipe(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    const Equipe &e = m_equipes[index];

    QDialog dialog(this);
    dialog.setWindowTitle("Informations de l'equipe");
    dialog.resize(1070, 650);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *titre = new QLabel("INFORMATIONS DE L'EQUIPE");
    titre->setStyleSheet(
        "font-size:20px;"
        "font-weight:bold;"
        "color:#18D9EF;"
        );

    layout->addWidget(titre);

    // Tableau avec les memes colonnes que Modifier
    QTableWidget *table = new QTableWidget(1, 10);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList entetes;

    for (int c = 0; c < 8; ++c)
    {
        if (c == 7)
            entetes << "Date_creation";
        else
            entetes << m_table->horizontalHeaderItem(c)->text();
    }

    entetes << "ID Projet";
    entetes << "ID Evaluation";

    table->setHorizontalHeaderLabels(entetes);

    table->setStyleSheet(m_table->styleSheet());
    QVector<Participant> membres = e.membres();

    int hommes = 0;
    int femmes = 0;
    int sommeAge = 0;

    QStringList competences;

    for (const Participant &p : membres)
    {
        QString sexe = p.sexe.trimmed().toLower();

        if (sexe == "homme" || sexe == "h")
            hommes++;
        else if (sexe == "femme" || sexe == "f")
            femmes++;

        sommeAge += p.age;

        QString comp = p.competence.trimmed();

        if (!comp.isEmpty() && comp != "-" &&
            !competences.contains(comp))
        {
            competences << comp;
        }
    }

    QString sexeText = QString("%1 H / %2 F")
                           .arg(hommes)
                           .arg(femmes);

    QString ageText = membres.isEmpty()
                          ? "-"
                          : QString::number(
                                qRound((double)sommeAge / membres.size())
                                );

    QString competenceText = competences.isEmpty()
                                 ? "-"
                                 : competences.join(", ");

    QString dateText = "-";

    if (m_table && index < m_table->rowCount() &&
        m_table->item(index, 7))
    {
        dateText = m_table->item(index, 7)->text();

    }
   table->setColumnWidth(7, 150);
   table->setHorizontalHeaderLabels(entetes);

    QStringList valeurs = {
        "E" + QString::number(e.id()),
        e.nom(),
        QString::number(e.taille()),
        "6",
        sexeText,
        ageText,
        competenceText,
        dateText,
        QString::number(e.idProjet()),
        QString::number(e.idEvaluation())
    };

    for (int c = 0; c < 10; ++c)
    {
        QTableWidgetItem *item =
            new QTableWidgetItem(valeurs[c]);

        item->setFlags(
            item->flags() & ~Qt::ItemIsEditable
            );

        table->setItem(0, c, item);
    }

    table->setRowHeight(0, 48);

    table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents
        );

    table->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Interactive
        );

    table->setColumnWidth(1, 180);

    table->verticalHeader()->setVisible(false);

    table->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    table->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    layout->addWidget(table);

    QPushButton *fermer = new QPushButton("Fermer");

    layout->addWidget(fermer, 0, Qt::AlignRight);

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

QImage EquipePage::creerQR(const QString &code) const
{
    const int n = 21;
    const int s = 7;

    QImage image((n + 8) * s, (n + 8) * s, QImage::Format_RGB32);
    image.fill(Qt::white);

    QPainter p(&image);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);

    int total = 0;
    for (char c : code.toUtf8())
    {
        total += static_cast<unsigned char>(c);
    }

    for (int y = 0; y < n; ++y)
    {
        for (int x = 0; x < n; ++x)
        {
            bool fixed = (x < 7 && y < 7) || (x >= n - 7 && y < 7) || (x < 7 && y >= n - 7);
            if (fixed) continue;

            int value = x * 31 + y * 17 + total;
            if (value % 5 == 0 || value % 11 == 0)
            {
                p.drawRect((x + 4) * s, (y + 4) * s, s, s);
            }
        }
    }

    auto finder = [&](int ox, int oy) {
        for (int y = 0; y < 7; ++y)
        {
            for (int x = 0; x < 7; ++x)
            {
                bool black = x == 0 || x == 6 || y == 0 || y == 6 || (x >= 2 && x <= 4 && y >= 2 && y <= 4);
                if (black)
                {
                    p.drawRect((ox + x + 4) * s, (oy + y + 4) * s, s, s);
                }
            }
        }
    };

    finder(0, 0);
    finder(n - 7, 0);
    finder(0, n - 7);

    p.end();
    return image;
}

/* =====================================================
   AFFICHER QR
   ===================================================== */

void EquipePage::afficherQR(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    QString code = "DEVE-EQ-" + QString::number(m_equipes[index].id());
    QImage image = creerQR(code);

    QDialog dialog(this);
    dialog.setWindowTitle("QR - " + m_equipes[index].nom());
  dialog.setFixedSize(700, 500);

    QVBoxLayout *v = new QVBoxLayout(&dialog);

    QLabel *title = new QLabel(m_equipes[index].nom());
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#18D9EF;font-size:20px;font-weight:bold;");

    QLabel *img = new QLabel;
    img->setAlignment(Qt::AlignCenter);
    img->setPixmap(QPixmap::fromImage(image));

    QPushButton *save = new QPushButton("Enregistrer PNG");
    QPushButton *close = new QPushButton("Fermer");

    v->addWidget(title);
    v->addWidget(img);
    v->addWidget(new QLabel(code));
    v->addWidget(save);
    v->addWidget(close);

    connect(save, &QPushButton::clicked, [this, image]() {
        QString file = QFileDialog::getSaveFileName(this, "Enregistrer QR", "qr_equipe.png", "PNG (*.png)");
        if (!file.isEmpty()) image.save(file);
    });

    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

/* =====================================================
   BADGE
   ===================================================== */

QImage EquipePage::creerBadge(const Equipe &e) const
{
    QImage image(900, 500, QImage::Format_RGB32);
    image.fill(QColor("#081522"));

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(QPen(QColor("#18D9EF"), 3));
    p.drawRoundedRect(10, 10, 880, 480, 20, 20);

    p.setPen(QColor("#18D9EF"));
    p.setFont(QFont("Segoe UI", 24, QFont::Bold));
    p.drawText(40, 55, "◆ Devena");

    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 30, QFont::Bold));
    p.drawText(40, 120, e.nom());

    p.setFont(QFont("Segoe UI", 16));
    p.drawText(45, 170, "Equipe #" + QString::number(e.id()));
    p.drawText(45, 220, "Membres : " + QString::number(e.taille()) + " / 6");
    p.drawText(45, 255, "Hommes : " + QString::number(e.hommes()));
    p.drawText(45, 290, "Femmes : " + QString::number(e.femmes()));
    p.drawText(45, 325, "Age moyen : " + QString::number(e.ageMoyen(), 'f', 1));

    QImage qr = creerQR("DEVE-EQ-" + QString::number(e.id()));
    qr = qr.scaled(190, 190, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    p.drawImage(660, 100, qr);
    p.end();

    return image;
}

/* =====================================================
   AFFICHER BADGE
   ===================================================== */

void EquipePage::afficherBadge(int index)
{
    if (index < 0 || index >= m_equipes.size())
        return;

    QImage image = creerBadge(m_equipes[index]);

    QDialog dialog(this);
    dialog.setWindowTitle("Badge - " + m_equipes[index].nom());
  dialog.setFixedSize(700, 500);
    QVBoxLayout *v = new QVBoxLayout(&dialog);

    QLabel *img = new QLabel;
    img->setAlignment(Qt::AlignCenter);
    img->setPixmap(QPixmap::fromImage(image).scaled(
        640, 356,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    QPushButton *save = new QPushButton("Enregistrer le badge");
    QPushButton *close = new QPushButton("Fermer");

    v->addWidget(img, 1);
    v->addWidget(save);
    v->addWidget(close);

    connect(save, &QPushButton::clicked, [this, image]() {
        QString file = QFileDialog::getSaveFileName(this, "Enregistrer le badge", "badge_equipe.png", "PNG (*.png)");
        if (!file.isEmpty()) image.save(file);
    });

    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

/* =====================================================
   AFFICHER EQUIPES
   ===================================================== */

void EquipePage::afficherEquipes()
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

        eye->setObjectName("iconButton");
        edit->setObjectName("iconButton");
        trash->setObjectName("iconButton");
        qr->setObjectName("iconButton");
        badge->setObjectName("iconButton");

        eye->setFixedSize(34, 30);
        edit->setFixedSize(34, 30);
        trash->setFixedSize(34, 30);
        qr->setFixedSize(34, 30);
        badge->setFixedSize(34, 30);

        eye->setToolTip("Voir les membres");
        edit->setToolTip("Modifier l'equipe");
        trash->setToolTip("Supprimer l'equipe");
        qr->setToolTip("Afficher le QR");
        badge->setToolTip("Afficher le badge");

        a->addWidget(eye);
        a->addWidget(edit);
        a->addWidget(trash);
        a->addWidget(qr);
        a->addWidget(badge);

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
    }

    delete model;
}


/* =====================================================
   COURBE
   ===================================================== */

/* =====================================================
   COURBE (Corrigée : à 0.0 s'il n'y a pas de membres)
   ===================================================== */

QPixmap EquipePage::creerCourbe() const
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
    for (const Equipe &e : m_equipes)
    {
        totalMembresActuels += e.taille();
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
/* =====================================================
   STATISTIQUES
   ===================================================== */

void EquipePage::afficherStatistiques()
{
    int total = 0;
    int h = 0;
    int f = 0;

    for (const Equipe &e : m_equipes)
    {
        total += e.taille();
        h += e.hommes();
        f += e.femmes();
    }

    m_totalEquipes->setText(QString::number(m_equipes.size()));
    m_totalMembres->setText(QString::number(total));
    m_totalHommes->setText(QString::number(h));
    m_totalFemmes->setText(QString::number(f));

    m_courbe->setPixmap(creerCourbe());
}

/* =====================================================
   FILTRE
   ===================================================== */

void EquipePage::filtrer()
{
    QString search = m_recherche->text().trimmed().toLower();
    QString filter = m_filtre->currentText();

    for (int i = 0; i < m_equipes.size(); ++i)
    {
        const Equipe &e = m_equipes[i];
        bool visible = search.isEmpty() || e.nom().toLower().contains(search);

        if (filter == "Equipe avec membres")
        {
            visible = visible && e.taille() > 0;
        }
        else if (filter == "Equipe vide")
        {
            visible = visible && e.taille() == 0;
        }
        else if (filter == "Equipe complete")
        {
            visible = visible && e.pleine();
        }

        m_table->setRowHidden(i, !visible);
    }
}

/* =====================================================
   EXPORT PDF
   ===================================================== */

void EquipePage::exporterPDF()
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
        QMessageBox::warning(this, "Erreur", "Impossible de creer le PDF.");
        return;
    }

    QRect page = painter.viewport();

    int left = 50;
    int top = 50;

    painter.setPen(QColor("#087F91"));
    painter.setFont(QFont("Segoe UI", 24, QFont::Bold));
    painter.drawText(left, top + 25, "◆ Devena");

    painter.setPen(Qt::black);
    painter.setFont(QFont("Segoe UI", 18, QFont::Bold));
    painter.drawText(left, top + 65, "Liste des equipes");

    painter.setFont(QFont("Segoe UI", 10));
    painter.drawText(left, top + 90, "Gestion des equipes - Hackathon");

    int y = top + 125;

    int colID = left;
    int colEquipe = left + 65;
    int colMembres = left + 300;
    int colHF = left + 405;
    int colAge = left + 500;
    int colStatut = left + 600;

    painter.setBrush(QColor("#EAF1F7"));
    painter.setPen(QColor("#087F91"));
    painter.drawRect(left, y, page.width() - 100, 35);

    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.drawText(colID + 8, y + 23, "ID");
    painter.drawText(colEquipe + 8, y + 23, "Equipe");
    painter.drawText(colMembres + 8, y + 23, "Membres");
    painter.drawText(colHF + 8, y + 23, "H / F");
    painter.drawText(colAge + 8, y + 23, "Age moyen");
    painter.drawText(colStatut + 8, y + 23, "Statut");

    y += 35;

    painter.setFont(QFont("Segoe UI", 10));
    painter.setPen(Qt::black);

    for (const Equipe &e : m_equipes)
    {
        if (y > page.height() - 100)
        {
            pdf.newPage();
            y = 70;
        }

        painter.setPen(QColor("#D5DEE8"));
        painter.drawLine(left, y + 35, page.width() - 50, y + 35);

        painter.setPen(Qt::black);
        painter.drawText(colID + 8, y + 22, "E" + QString::number(e.id()));
        painter.drawText(colEquipe + 8, y + 22, e.nom());
        painter.drawText(colMembres + 8, y + 22, QString::number(e.taille()) + " / 6");
        painter.drawText(colHF + 8, y + 22, QString::number(e.hommes()) + " / " + QString::number(e.femmes()));
        painter.drawText(colAge + 8, y + 22, QString::number(e.ageMoyen(), 'f', 1));
        painter.drawText(colStatut + 8, y + 22, e.statut());

        y += 36;

        for (const Participant &p : e.membres())
        {
            if (y > page.height() - 100)
            {
                pdf.newPage();
                y = 70;
            }

            painter.setPen(QColor("#526579"));
            QString membre = "   - " + p.prenom + " " + p.nom + " | " + p.sexe + " | " + QString::number(p.age) + " ans | " + p.competence;
            painter.drawText(colEquipe + 8, y + 18, membre);
            y += 25;
        }

        y += 10;
    }

    if (y > page.height() - 180)
    {
        pdf.newPage();
        y = 70;
    }

    int total = 0;
    int hommes = 0;
    int femmes = 0;

    for (const Equipe &e : m_equipes)
    {
        total += e.taille();
        hommes += e.hommes();
        femmes += e.femmes();
    }

    painter.setPen(QColor("#087F91"));
    painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
    painter.drawText(left, y + 30, "Statistiques");

    painter.setPen(Qt::black);
    painter.setFont(QFont("Segoe UI", 11));
    painter.drawText(left, y + 60, "Nombre d'equipes : " + QString::number(m_equipes.size()));
    painter.drawText(left, y + 85, "Nombre de membres : " + QString::number(total));
    painter.drawText(left, y + 110, "Hommes : " + QString::number(hommes));
    painter.drawText(left, y + 135, "Femmes : " + QString::number(femmes));

    painter.end();

    QMessageBox::information(this, "Export PDF", "Le fichier PDF a ete exporte avec succes.");
}

/* =====================================================
   MODE CLAIR
   ===================================================== */

void EquipePage::changerModeClair()
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

    afficherStatistiques();
}

/* =====================================================
   RAFRAICHIR
   ===================================================== */

void EquipePage::rafraichir()
{
    afficherEquipes();
    afficherStatistiques();
    filtrer();
}