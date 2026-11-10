#include "appshell.h"
#include "employee.h"
#include "equipepage.h"      // module Equipes (Alaa)
#include "mainwindow.h"      // module Projets (Jihene)
#include "participant.h"     // module Participants (Nour)
#include "evaluationwindow.h" // module Evaluations (Amin)
#include "connection.h"      // Singleton de connexion Oracle/ODBC
#include "SessionManager.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QSpacerItem>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTimer>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QVariantAnimation>
#include <QShowEvent>
#include <QHideEvent>
#include <QVector>
#include <QLineF>
#include <QtMath>
#include <cmath>

namespace {
const QString theme = R"(
QWidget { background:#050B14; color:#F3F6FA; font-family:'Segoe UI'; font-size:13px; }
QFrame#panel { background:#081522; border:1px solid #1B344B; border-radius:10px; }
QFrame#loginVisual { background:#081522; border:1px solid #1B344B; border-radius:12px; }
QFrame#sidebar { background:#030A13; border-right:1px solid #1B344B; }
QFrame#topBar { background:#050B14; border:0; border-radius:0; }
QFrame#statCard { background:#081522; border:1px solid #1B344B; border-radius:10px; }
QFrame#accentCard { background:#081522; border:1px solid #1B344B; border-radius:10px; }
QLabel#logo { color:#08D5E8; font-size:22px; font-weight:700; }
QLabel#eyebrow { color:#8FA9C3; font-size:11px; font-weight:700; }
QLabel#heading { color:#F3F6FA; font-size:28px; font-weight:700; }
QLabel#subheading { color:#F3F6FA; font-size:16px; font-weight:600; }
QLabel#muted { color:#4785B8; }
QLabel#statValue { color:#08D5E8; font-size:22px; font-weight:700; }
QLabel#statTitle { color:#8FA9C3; font-size:12px; font-weight:600; }
QLabel#bigGlow { color:#08D5E8; font-size:44px; font-weight:800; }
QLineEdit { background:#081522; border:1px solid #1B344B; border-radius:9px; padding:10px 12px; color:#F3F6FA; selection-background-color:#0D2637; }
QLineEdit:focus { border:1px solid #08D5E8; background:#081522; }
QPushButton { background:#081522; border:1px solid #1B344B; border-radius:9px; padding:9px 14px; color:#F3F6FA; font-weight:600; }
QPushButton:hover { border-color:#08D5E8; background:#0D2637; }
QPushButton#primary { background:#08D5E8; color:#030A13; border:1px solid #08D5E8; font-weight:700; }
QPushButton#primary:hover { background:#38e8f5; }
QPushButton#navButton { text-align:left; background:transparent; border:1px solid transparent; color:#8FA9C3; padding:11px 14px; border-radius:9px; }
QPushButton#navButton:hover { background:#0D2637; color:#F3F6FA; border-color:#1B344B; }
QPushButton#navActive { text-align:left; background:#092B3A; border:1px solid #08D5E8; color:#08D5E8; padding:11px 14px; font-weight:700; border-radius:9px; min-width:204px; min-height:48px; }
QPushButton#smallAction { padding:8px 12px; }
QWidget#loginPage QLabel { background:transparent; }
)";

// ──────────────────────────────────────────────────────────────────────────────
// HackathonCanvas — animation de la page de login :
// réseau de nœuds connectés (les équipes du hackathon), ligne de scan,
// et une ligne de terminal qui s'écrit toute seule.
// Dessiné avec QPainter + QTimer, aucune ressource externe.
class HackathonCanvas : public QWidget {
public:
    explicit HackathonCanvas(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumHeight(190);
        auto *rng = QRandomGenerator::global();
        for (int i = 0; i < 38; ++i) {
            Node n;
            n.p = QPointF(rng->generateDouble(), rng->generateDouble());
            n.v = QPointF((rng->generateDouble() - 0.5) * 0.05,
                          (rng->generateDouble() - 0.5) * 0.05);
            n.r = (i < 6) ? 3.6 : 1.9;          // les 6 premiers sont des "hubs"
            m_nodes.append(n);
        }
        m_phrases << "> devena --init smart-hackathon"
                  << "> chargement des equipes ........ OK"
                  << "> jurys : IA | Web | IoT | Securite"
                  << "> connexion Oracle ............... OK"
                  << "> pret a innover";
        m_timer.setInterval(33);                 // ~30 images / seconde
        QObject::connect(&m_timer, &QTimer::timeout, this, [this] { avancer(); });
        m_total.start();
        m_frame.start();
    }

protected:
    void showEvent(QShowEvent *e) override { QWidget::showEvent(e); m_frame.restart(); m_timer.start(); }
    void hideEvent(QHideEvent *e) override { QWidget::hideEvent(e); m_timer.stop(); }

    void paintEvent(QPaintEvent *) override {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing, true);
        const QRectF full = rect();
        const qreal  w = full.width(), h = full.height();
        const qreal  t = m_total.elapsed() / 1000.0;
        const QColor cyan(8, 213, 232);

        // Fond : écran sombre arrondi dans le panneau
        g.fillRect(full, QColor("#081522"));
        QPainterPath ecran;
        ecran.addRoundedRect(full.adjusted(0.5, 0.5, -0.5, -0.5), 10, 10);
        g.fillPath(ecran, QColor("#050B14"));
        g.setClipPath(ecran);

        // Grille
        g.setPen(QPen(QColor(27, 52, 75, 110), 1));
        for (qreal x = 0; x < w; x += 40) g.drawLine(QPointF(x, 0), QPointF(x, h));
        for (qreal y = 0; y < h; y += 40) g.drawLine(QPointF(0, y), QPointF(w, y));

        // Liens entre nœuds proches
        const qreal portee = 130.0;
        int liens = 0;
        for (int i = 0; i < m_nodes.size(); ++i) {
            const QPointF a(m_nodes[i].p.x() * w, m_nodes[i].p.y() * h);
            for (int j = i + 1; j < m_nodes.size(); ++j) {
                const QPointF b(m_nodes[j].p.x() * w, m_nodes[j].p.y() * h);
                const qreal d = QLineF(a, b).length();
                if (d < portee) {
                    QColor c = cyan;
                    c.setAlpha(int(150 * (1.0 - d / portee)));
                    g.setPen(QPen(c, 1));
                    g.drawLine(a, b);
                    ++liens;
                }
            }
        }

        // Nœuds (les hubs pulsent)
        g.setPen(Qt::NoPen);
        for (int i = 0; i < m_nodes.size(); ++i) {
            const Node &n = m_nodes[i];
            const QPointF c(n.p.x() * w, n.p.y() * h);
            if (n.r > 3.0) {
                const qreal pulse = 0.5 + 0.5 * qSin(t * 2.2 + i);
                QColor halo = cyan;
                halo.setAlpha(int(25 + 45 * pulse));
                g.setBrush(halo);
                g.drawEllipse(c, n.r * (2.4 + pulse), n.r * (2.4 + pulse));
            }
            QColor coeur = cyan;
            coeur.setAlpha(n.r > 3.0 ? 255 : 190);
            g.setBrush(coeur);
            g.drawEllipse(c, n.r, n.r);
        }

        // Ligne de scan qui descend
        const qreal sy = std::fmod(t * 0.20, 1.0) * (h + 80) - 40;
        QLinearGradient trainee(0, sy - 70, 0, sy);
        trainee.setColorAt(0.0, QColor(8, 213, 232, 0));
        trainee.setColorAt(1.0, QColor(8, 213, 232, 34));
        g.fillRect(QRectF(0, sy - 70, w, 70), trainee);
        g.setPen(QPen(QColor(8, 213, 232, 130), 1));
        g.drawLine(QPointF(0, sy), QPointF(w, sy));

        // Repères HUD dans les coins
        g.setPen(QPen(QColor(8, 213, 232, 170), 2));
        const qreal m = 12, L = 18;
        g.drawLine(QPointF(m, m), QPointF(m + L, m));         g.drawLine(QPointF(m, m), QPointF(m, m + L));
        g.drawLine(QPointF(w - m, m), QPointF(w - m - L, m)); g.drawLine(QPointF(w - m, m), QPointF(w - m, m + L));
        g.drawLine(QPointF(m, h - m), QPointF(m + L, h - m)); g.drawLine(QPointF(m, h - m), QPointF(m, h - m - L));
        g.drawLine(QPointF(w - m, h - m), QPointF(w - m - L, h - m));
        g.drawLine(QPointF(w - m, h - m), QPointF(w - m, h - m - L));

        // Texte HUD
        QFont mono("Consolas");
        mono.setStyleHint(QFont::Monospace);
        mono.setPixelSize(11);
        mono.setBold(true);
        g.setFont(mono);
        const bool clignote = (int(t * 2) % 2) == 0;
        g.setPen(Qt::NoPen);
        g.setBrush(clignote ? QColor("#34d399") : QColor(52, 211, 153, 70));
        g.drawEllipse(QPointF(m + L + 16, m + 9), 4, 4);
        g.setPen(QColor("#8FA9C3"));
        g.drawText(QRectF(m + L + 28, m, 260, 18), Qt::AlignLeft | Qt::AlignVCenter,
                   "HACKATHON // LIVE");
        g.drawText(QRectF(w - 280 - m - L, m, 270, 18), Qt::AlignRight | Qt::AlignVCenter,
                   QString("NODES %1  LINKS %2").arg(m_nodes.size()).arg(liens));

        // Ligne de terminal animée
        mono.setPixelSize(13);
        mono.setBold(false);
        g.setFont(mono);
        const QRectF bande(m + L + 6, h - m - 30, w - 2 * (m + L + 6), 24);
        g.fillRect(bande.adjusted(-8, -2, 8, 2), QColor(5, 11, 20, 215));
        g.setPen(cyan);
        g.drawText(bande, Qt::AlignLeft | Qt::AlignVCenter,
                   m_phrases[m_phrase].left(int(m_chars)) + (clignote ? "_" : " "));
    }

private:
    struct Node { QPointF p; QPointF v; qreal r = 2.0; };

    void avancer() {
        const qreal dt = qMin<qreal>(m_frame.restart() / 1000.0, 0.1);
        for (Node &n : m_nodes) {
            n.p += n.v * dt;
            if (n.p.x() < 0.02 || n.p.x() > 0.98) { n.v.setX(-n.v.x()); n.p.setX(qBound<qreal>(0.02, n.p.x(), 0.98)); }
            if (n.p.y() < 0.04 || n.p.y() > 0.96) { n.v.setY(-n.v.y()); n.p.setY(qBound<qreal>(0.04, n.p.y(), 0.96)); }
        }
        // Machine à écrire : 26 caractères/s, pause de 1,6 s, puis phrase suivante
        const int longueur = m_phrases[m_phrase].size();
        if (m_chars < longueur) {
            m_chars += 26.0 * dt;
        } else {
            m_pause += dt;
            if (m_pause > 1.6) {
                m_pause  = 0.0;
                m_chars  = 0.0;
                m_phrase = (m_phrase + 1) % m_phrases.size();
            }
        }
        update();
    }

    QVector<Node> m_nodes;
    QStringList   m_phrases;
    int           m_phrase = 0;
    qreal         m_chars  = 0.0;
    qreal         m_pause  = 0.0;
    QTimer        m_timer;
    QElapsedTimer m_total;
    QElapsedTimer m_frame;
};

QLabel *label(const QString &text, const QString &name = QString()) {
    auto *l = new QLabel(text);
    l->setTextFormat(Qt::RichText);
    if (!name.isEmpty()) l->setObjectName(name);
    return l;
}

// Helper: create a stat card and return a pointer to its value label so it can be updated later.
struct StatCardResult { QFrame *frame; QLabel *valueLabel; };
StatCardResult statCardEx(const QString &title, const QString &value, const QString &hint) {
    auto *f = new QFrame;
    f->setObjectName("statCard");
    auto *v = new QVBoxLayout(f);
    v->setContentsMargins(20,18,20,18);
    v->setSpacing(8);
    v->addWidget(label(title.toUpper(),"statTitle"));
    auto *val = label(value,"statValue");
    v->addWidget(val);
    v->addWidget(label(hint,"muted"));
    return {f, val};
}

QPushButton *navButton(const QString &text, bool active = false) {
    auto *b = new QPushButton(text);
    b->setObjectName(active ? "navActive" : "navButton");
    b->setMinimumHeight(44);
    return b;
}
} // namespace

// ──────────────────────────────────────────────────────────────────────────────
AppShell::AppShell(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Devena — Smart Hackathon");
    resize(1440, 850);
    setMinimumSize(1120, 700);

    // --- Initialisation de la connexion Oracle/ODBC via le Singleton ---
    if (!Connection::createInstance().createConnection()) {
        qDebug() << "[Devena] Mode démo : Oracle non disponible, données locales utilisées.";
    }

    pages = new QStackedWidget(this);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(pages);

    pages->addWidget(creerLoginPage());
    pages->addWidget(creerDashboardPage());

    employePage = new EmployeWindow;
    employePage->setWindowFlags(Qt::Widget);
    connect(employePage, &EmployeWindow::navigateDashboard, this, &AppShell::afficherDashboard);
    connect(employePage, &EmployeWindow::navigateModule,    this, &AppShell::allerVers);
    pages->addWidget(employePage);

    appliquerTheme();
}

// ──────────────────────────────────────────────────────────────────────────────
QWidget *AppShell::creerLoginPage() {
    auto *page = new QWidget;
    page->setObjectName("loginPage");
    auto *outer = new QHBoxLayout(page);
    outer->setContentsMargins(38,28,38,28);
    outer->setSpacing(28);

    // Futuristic left panel
    auto *visual = new QFrame;
    visual->setObjectName("loginVisual");
    auto *visualLayout = new QVBoxLayout(visual);
    visualLayout->setContentsMargins(36,34,36,30);
    visualLayout->setSpacing(18);

    visualLayout->addWidget(label("◆ DEVENA","logo"));
    visualLayout->addWidget(label("SMART HACKATHON  /  WORKSPACE","eyebrow"));
    visualLayout->addSpacing(14);
    visualLayout->addWidget(label("Connectez les talents.<br/>Accélérez les idées.<br/><span style='color:#22e6ff'>Construisez le futur.</span>","heading"));
    visualLayout->addWidget(label("Une plateforme centralisée pour gérer les employés, les équipes et les projets de votre hackathon.","muted"));
    visualLayout->addSpacing(6);
    visualLayout->addWidget(new HackathonCanvas, 1);   // animation hackathon

    auto *orbPanel = new QFrame;
    orbPanel->setObjectName("panel");
    auto *orbLayout = new QHBoxLayout(orbPanel);
    orbLayout->setContentsMargins(20,18,20,18);
    auto *orb = label("◈","bigGlow");
    orb->setAlignment(Qt::AlignCenter);
    orbLayout->addWidget(orb);
    auto *orbText = new QVBoxLayout;
    orbText->addWidget(label("SYSTEM ONLINE","eyebrow"));
    orbText->addWidget(label("Espace sécurisé de gestion","subheading"));
    orbText->addWidget(label("Connexion Oracle ODBC active","muted"));
    orbLayout->addLayout(orbText,1);
    visualLayout->addWidget(orbPanel);

    auto *features = new QHBoxLayout;
    features->addWidget(label("◉  GESTION CENTRALISÉE","muted"));
    features->addStretch();
    features->addWidget(label("⌁  UI CYBER","muted"));
    visualLayout->addLayout(features);

    // Login form
    auto *formWrap = new QWidget;
    formWrap->setMinimumWidth(390);
    formWrap->setMaximumWidth(500);
    auto *formOuter = new QVBoxLayout(formWrap);
    formOuter->setContentsMargins(0,0,0,0);
    formOuter->addStretch();

    auto *panel = new QFrame;
    panel->setObjectName("panel");
    auto *form = new QVBoxLayout(panel);
    form->setContentsMargins(32,32,32,30);
    form->setSpacing(15);
    form->addWidget(label("AUTHENTIFICATION","eyebrow"));
    form->addWidget(label("Bon retour.","heading"));
    form->addWidget(label("Connectez-vous pour accéder à votre espace Devena.","muted"));
    form->addSpacing(8);
    form->addWidget(label("ID Employé"));
    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("ID Employé (ex: 001)");
    emailEdit->setObjectName("loginEmail");
    form->addWidget(emailEdit);
    form->addWidget(label("Mot de passe"));
    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Saisissez votre mot de passe");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setObjectName("loginPassword");
    form->addWidget(passwordEdit);
    loginError = label("");
    loginError->setStyleSheet("color:#fb7185;");
    form->addWidget(loginError);
    auto *login = new QPushButton("ACCÉDER À L'ESPACE  →");
    login->setObjectName("primary");
    login->setMinimumHeight(48);
    // Halo cyan qui pulse doucement autour du bouton
    auto *halo = new QGraphicsDropShadowEffect(login);
    halo->setOffset(0, 0);
    halo->setColor(QColor(8, 213, 232, 190));
    halo->setBlurRadius(12);
    login->setGraphicsEffect(halo);
    auto *pulse = new QVariantAnimation(login);
    pulse->setDuration(2400);
    pulse->setLoopCount(-1);
    pulse->setStartValue(10.0);
    pulse->setKeyValueAt(0.5, 36.0);
    pulse->setEndValue(10.0);
    connect(pulse, &QVariantAnimation::valueChanged, halo,
            [halo](const QVariant &v) { halo->setBlurRadius(v.toReal()); });
    pulse->start();
    form->addWidget(login);
    form->addSpacing(4);
    auto *demo = label("COMPTE DE DÉMONSTRATION","eyebrow");
    form->addWidget(demo);
    form->addWidget(label("ID: 001  /  Mot de passe: ai","muted"));
    form->addWidget(label("Les identifiants sont vérifiés en base Oracle.","muted"));

    formOuter->addWidget(panel);
    formOuter->addStretch();

    outer->addWidget(visual, 6);
    outer->addWidget(formWrap, 4);

    connect(login,        &QPushButton::clicked,       this, &AppShell::verifierConnexion);
    connect(passwordEdit, &QLineEdit::returnPressed,   this, &AppShell::verifierConnexion);
    connect(emailEdit,    &QLineEdit::returnPressed,   this, &AppShell::verifierConnexion);
    return page;
}

// ──────────────────────────────────────────────────────────────────────────────
QWidget *AppShell::creerDashboardPage() {
    auto *page = new QWidget;
    auto *root = new QHBoxLayout(page);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // Sidebar
    auto *sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(236);
    auto *side = new QVBoxLayout(sidebar);
    side->setContentsMargins(13,22,13,18);
    side->setSpacing(8);
    side->addWidget(label("◆ Devena","logo"));
    side->addWidget(label("Connecter les talents,<br/>construire l'avenir","muted"));
    side->addSpacing(18);
    auto *dashNav     = navButton("Tableau de bord", true);
    auto *employeeNav = navButton("Employés");
    side->addWidget(dashNav);
    side->addWidget(employeeNav);
    auto *participantsNav = navButton("Participants");
    side->addWidget(participantsNav);
    connect(participantsNav, &QPushButton::clicked, this, [this] { allerVers("Participants"); });
    auto *equipesNav = navButton("Équipes");
    auto *projetsNav = navButton("Projets");
    side->addWidget(equipesNav);
    side->addWidget(projetsNav);
    connect(equipesNav, &QPushButton::clicked, this, [this] { allerVers("Equipes"); });
    connect(projetsNav, &QPushButton::clicked, this, [this] { allerVers("Projets"); });
    auto *evaluationsNav = navButton("Évaluations");
    side->addWidget(evaluationsNav);
    connect(evaluationsNav, &QPushButton::clicked, this, [this] { allerVers("Evaluations"); });
    side->addWidget(navButton("Paramètres"));
    side->addStretch();
    auto *tagline = label("Hack today<br/>Build tomorrow","muted");
    tagline->setAlignment(Qt::AlignCenter);
    side->addWidget(tagline);

    auto *content = new QWidget;
    auto *body = new QVBoxLayout(content);
    body->setContentsMargins(22,18,22,18);
    body->setSpacing(16);

    auto *header = new QHBoxLayout;
    auto *titleColumn = new QVBoxLayout;
    titleColumn->setSpacing(5);
    titleColumn->addWidget(label("Tableau de bord","heading"));
    titleColumn->addWidget(label("Vue d'ensemble de votre espace Devena","muted"));
    header->addLayout(titleColumn);
    header->addStretch();
    auto *logout = new QPushButton("Déconnexion");
    logout->setObjectName("smallAction");
    header->addWidget(logout);
    body->addLayout(header);
    connect(logout, &QPushButton::clicked, this, &AppShell::deconnecter);

    // Welcome banner
    auto *welcome = new QFrame;
    welcome->setObjectName("panel");
    auto *welcomeLayout = new QVBoxLayout(welcome);
    welcomeLayout->setContentsMargins(18,16,18,16);
    welcomeLayout->setSpacing(7);
    welcomeLabel = label("Bienvenue dans votre espace Devena","subheading");
    welcomeLayout->addWidget(welcomeLabel);
    welcomeLayout->addWidget(label("Retrouvez rapidement les outils de gestion de votre hackathon.","muted"));
    body->addWidget(welcome);

    // ── Stat cards (keep pointers to update them after login) ──
    auto *stats = new QGridLayout;
    stats->setSpacing(12);

    auto c0 = statCardEx("Total employés",   "—", "Tous les profils enregistrés");
    auto c1 = statCardEx("Jury IA (EV01)",   "—", "Évaluateurs Intelligence Artificielle");
    auto c2 = statCardEx("Jury Web (EV02)",  "—", "Évaluateurs Développement Web");
    auto c3 = statCardEx("Total équipes",    "—", "Équipes enregistrées");

    m_statTotal  = c0.valueLabel;
    m_statEV01   = c1.valueLabel;
    m_statEV02   = c2.valueLabel;
    m_statEquipe = c3.valueLabel;

    stats->addWidget(c0.frame, 0, 0);
    stats->addWidget(c1.frame, 0, 1);
    stats->addWidget(c2.frame, 0, 2);
    stats->addWidget(c3.frame, 0, 3);
    body->addLayout(stats);

    auto *moduleTitle = label("Modules","subheading");
    body->addWidget(moduleTitle);
    auto *employeeCard = new QFrame;
    employeeCard->setObjectName("panel");
    auto *ec = new QHBoxLayout(employeeCard);
    ec->setContentsMargins(18,16,18,16);
    ec->setSpacing(14);
    auto *icon = label("♙");
    icon->setStyleSheet("color:#22d3ee;font-size:28px;font-weight:700;");
    icon->setMinimumWidth(36);
    icon->setAlignment(Qt::AlignCenter);
    ec->addWidget(icon);
    auto *et = new QVBoxLayout;
    et->setSpacing(5);
    et->addWidget(label("Gestion des employés","subheading"));
    et->addWidget(label("Ajouter, modifier, supprimer, rechercher et exporter les employés en PDF.","muted"));
    ec->addLayout(et,1);
    auto *openEmployees = new QPushButton("Ouvrir le module  →");
    openEmployees->setObjectName("primary");
    ec->addWidget(openEmployees);
    body->addWidget(employeeCard);
    body->addStretch();
    auto *footer = label("DEVENA  •  SMART HACKATHON","muted");
    footer->setAlignment(Qt::AlignCenter);
    body->addWidget(footer);

    root->addWidget(sidebar);
    root->addWidget(content, 1);
    connect(employeeNav,  &QPushButton::clicked, this, &AppShell::afficherEmployes);
    connect(openEmployees,&QPushButton::clicked, this, &AppShell::afficherEmployes);
    connect(dashNav,      &QPushButton::clicked, this, &AppShell::afficherDashboard);
    return page;
}

// ──────────────────────────────────────────────────────────────────────────────
void AppShell::verifierConnexion()
{
    const QString idEmploye = emailEdit->text().trimmed();
    const QString password  = passwordEdit->text();

    if (idEmploye.isEmpty() || password.isEmpty()) {
        loginError->setText("Veuillez saisir votre ID employé et votre mot de passe.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    if (!db.isOpen()) {
        loginError->setText("Base de données non disponible. Vérifiez la connexion ODBC.");
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT NOM, PRENOM, POSTE FROM EMPLOYES "
              "WHERE TRIM(ID_EMPLOYE) = TRIM(:id) "
              "AND TRIM(MOT_DE_PASSE) = TRIM(:pwd)");
    q.bindValue(":id",  idEmploye);
    q.bindValue(":pwd", password);

    if (!q.exec()) {
        loginError->setText("Erreur SQL : " + q.lastError().text());
        return;
    }

    if (q.next()) {
        const QString nom    = q.value("NOM").toString();
        const QString prenom = q.value("PRENOM").toString();
        const QString poste  = q.value("POSTE").toString();

        SessionManager::getInstance().setEmployee(idEmploye, nom, prenom, poste);

        loginError->clear();
        welcomeLabel->setText("Bienvenue, " + prenom + " " + nom +
                              " — " + SessionManager::getInstance().posteName());

        loadDashboardStats();
        pages->setCurrentIndex(1);
    } else {
        loginError->setText("Identifiants incorrects. Vérifiez votre ID et mot de passe.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void AppShell::loadDashboardStats()
{
    QSqlDatabase db = QSqlDatabase::database("devena_conn");
    if (!db.isOpen()) return;

    auto runCount = [&](const QString &sql) -> QString {
        QSqlQuery q(db);
        if (q.exec(sql) && q.next())
            return q.value(0).toString();
        return "?";
    };

    m_statTotal ->setText(runCount("SELECT COUNT(*) FROM EMPLOYES"));
    m_statEV01  ->setText(runCount("SELECT COUNT(*) FROM EMPLOYES WHERE POSTE='EV01'"));
    m_statEV02  ->setText(runCount("SELECT COUNT(*) FROM EMPLOYES WHERE POSTE='EV02'"));
    m_statEquipe->setText(runCount("SELECT COUNT(*) FROM EQUIPES"));
}

// ──────────────────────────────────────────────────────────────────────────────
// Navigation commune : chaque module garde son propre menu de gauche et
// previent AppShell quand on clique sur un autre module.
void AppShell::allerVers(const QString &module)
{
    if (module.contains("Tableau", Qt::CaseInsensitive)) { afficherDashboard(); return; }
    if (module.contains("Employ",  Qt::CaseInsensitive)) { afficherEmployes();  return; }

    if (module.contains("quipe", Qt::CaseInsensitive)) {          // Equipes (Alaa)
        if (!equipesPage) {
            equipesPage = new EquipePage;
            equipesPage->setWindowFlags(Qt::Widget);
            connect(equipesPage, &EquipePage::naviguer, this, &AppShell::allerVers);
            pages->addWidget(equipesPage);
        }
        pages->setCurrentWidget(equipesPage);
    } else if (module.contains("Projet", Qt::CaseInsensitive)) {  // Projets (Jihene)
        if (!projetsPage) {
            projetsPage = new MainWindow;
            projetsPage->setWindowFlags(Qt::Widget);
            connect(projetsPage, &MainWindow::naviguer, this, &AppShell::allerVers);
            pages->addWidget(projetsPage);
        }
        pages->setCurrentWidget(projetsPage);
    } else if (module.contains("Particip", Qt::CaseInsensitive)) {   // Participants (Nour)
        if (!participantsPage) {
            participantsPage = new ModParticipants::MainWindow;
            participantsPage->setWindowFlags(Qt::Widget);
            connect(participantsPage, &ModParticipants::MainWindow::naviguer, this, &AppShell::allerVers);
            pages->addWidget(participantsPage);
        }
        pages->setCurrentWidget(participantsPage);
    } else if (module.contains("valuation", Qt::CaseInsensitive)) {  // Evaluations (Amin)
        if (!evaluationsPage) {
            evaluationsPage = new EvaluationWindow;
            evaluationsPage->setWindowFlags(Qt::Widget);
            connect(evaluationsPage, &EvaluationWindow::naviguer, this, &AppShell::allerVers);
            pages->addWidget(evaluationsPage);
        }
        pages->setCurrentWidget(evaluationsPage);
    }
}

void AppShell::afficherEmployes()   { pages->setCurrentIndex(2); }
void AppShell::afficherDashboard()  { pages->setCurrentIndex(1); }
void AppShell::deconnecter()
{
    SessionManager::getInstance().clear();
    emailEdit->clear();
    passwordEdit->clear();
    loginError->clear();
    pages->setCurrentIndex(0);
}
void AppShell::appliquerTheme()     { qApp->setStyleSheet(theme); }
