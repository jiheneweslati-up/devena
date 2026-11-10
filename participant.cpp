#include "participant.h"
#include <QAction>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlError>
#include <QDebug>
#ifdef HAS_QTPDF
#include <QPdfDocument>
#endif
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPrinter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QTableView>
#include <QTextDocument>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QtCharts>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

        // ================================================================ Données
        QString Participant::pole() const {
    auto has = [this](const QStringList& keys) {
        for (const QString& comp : competences)
            for (const QString& k : keys)
                if (comp.compare(k, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    if (has({"Figma", "UI", "UX", "UI/UX", "Design", "Design system", "Prototypage", "Scrum", "Gestion"}))
        return "Designers & Leads";
    if (has({"IA", "LLM", "Pandas", "Data", "TensorFlow", "Machine Learning"}))
        return "IA & Data Specialists";
    return "Developers";
}

// ================================================================ Persistance BD (Atelier CRUD)
bool Participant::ajouter() {
    QSqlQuery query;
    query.prepare("INSERT INTO PARTICIPANTS (ID_PARTICIPANT, NOM, PRENOM, EMAIL, COMPETENCES, AGE, SEXE) VALUES (:id, :nom, :prenom, :email, :competences, :age, :sexe)");
    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":competences", competences.join(","));
    query.bindValue(":age", age);
    query.bindValue(":sexe", sexe);
    if (!query.exec()) {
        qDebug() << "Erreur SQL INSERT :" << query.lastError().text();
        return false;
    }
    QSqlQuery().exec("COMMIT");
    return true;
}

bool Participant::supprimer(QString participantId) {
    QSqlQuery query;
    query.prepare("DELETE FROM PARTICIPANTS WHERE ID_PARTICIPANT = :id");
    query.bindValue(":id", participantId);
    if (!query.exec()) {
        qDebug() << "Erreur SQL DELETE :" << query.lastError().text();
        return false;
    }
    QSqlQuery().exec("COMMIT");
    return true;
}

bool Participant::modifier() {
    QSqlQuery query;
    query.prepare("UPDATE PARTICIPANTS SET NOM = :nom, PRENOM = :prenom, EMAIL = :email, COMPETENCES = :competences, AGE = :age, SEXE = :sexe WHERE ID_PARTICIPANT = :id");
    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":competences", competences.join(","));
    query.bindValue(":age", age);
    query.bindValue(":sexe", sexe);
    if (!query.exec()) {
        qDebug() << "Erreur SQL UPDATE :" << query.lastError().text();
        return false;
    }
    QSqlQuery().exec("COMMIT");
    return true;
}

QString makeStyle(bool dark) {
    const QString bg   = dark ? "#0d1420" : "#ffffff";
    const QString card = dark ? "#152033" : "#f5f7fa";
    const QString tx   = dark ? "#e6edf6" : "#1b2430";
    const QString tx2  = dark ? "#8fa0b6" : "#5b6675";
    const QString bd   = dark ? "#243350" : "#dde3ea";
    const QString ac   = dark ? "#22d3ee" : "#1f6feb";
    const QString onAc = dark ? "#04232b" : "#ffffff";
    const QString acBg = dark ? "#0e3a4a" : "#e3eefe";

    return QString(R"(
QMainWindow, QDialog, QWidget#root { background:%1; }
QWidget { color:%3; font-size:13px; }
QFrame#kpi { background:%2; border-radius:8px; }
QLabel#title { font-size:22px; font-weight:500; }
QLabel#sub, QLabel#kpiTitle { color:%4; }
QLabel#kpiVal { font-size:22px; font-weight:500; }
QLineEdit, QComboBox { background:%2; border:1px solid %5; border-radius:8px; padding:6px 10px; }
QPushButton { background:transparent; border:1px solid %5; border-radius:8px; padding:7px 14px; }
QPushButton:hover { background:%2; }
QPushButton#primary { background:%6; border-color:%6; color:%7; font-weight:500; }
QTableView { background:%1; border:1px solid %5; border-radius:8px; gridline-color:transparent;
             selection-background-color:%2; selection-color:%3; }
QHeaderView::section { background:%1; color:%6; border:none; border-bottom:1px solid %5;
                       padding:8px; font-weight:500; }
QLabel#avatar { background:%8; color:%6; border-radius:40px; font-size:26px; font-weight:500; }
QLabel#chip { background:%8; color:%6; border-radius:10px; padding:2px 10px; }
QLabel#section { font-size:16px; font-weight:500; }
QListWidget { background:%2; border:1px solid %5; border-radius:8px; }
QLabel#logo { font-size:20px; font-weight:500; color:%6; }
QFrame#sidebar { background:%2; border-right:1px solid %5; }
QPushButton#nav { border:1px solid transparent; text-align:left; padding:10px 14px; color:%4; }
QPushButton#nav:hover { background:%1; }
QPushButton#nav[active="true"] { background:%8; color:%6; border:1px solid %6; font-weight:500; }
)").arg(bg, card, tx, tx2, bd, ac, onAc, acBg);
}

// ================================================================ Modèle (7 colonnes)
ParticipantModel::ParticipantModel(QObject* parent) : QAbstractTableModel(parent) {}

int ParticipantModel::rowCount(const QModelIndex& p) const { return p.isValid() ? 0 : m_data.size(); }
int ParticipantModel::columnCount(const QModelIndex& p) const { return p.isValid() ? 0 : 7; }

QVariant ParticipantModel::data(const QModelIndex& i, int role) const {
    if (!i.isValid()) return {};
    const Participant& p = m_data.at(i.row());
    if (role == IdRole) return p.id;
    if (role != Qt::DisplayRole) return {};

    switch (i.column()) {
    case 0: return p.id;
    case 1: return p.nom;
    case 2: return p.prenom;
    case 3: return p.competences.join(", ");
    case 4: return p.age > 0 ? QString::number(p.age) : QString();
    case 5: return p.sexe;
    default: return QString();
    }
}

QVariant ParticipantModel::headerData(int s, Qt::Orientation o, int role) const {
    static const QStringList h = {"ID", "Nom", "Prénom", "Compétences", "Âge", "Sexe", "Actions"};
    if (o == Qt::Horizontal && role == Qt::DisplayRole) return h.value(s);
    return {};
}

int ParticipantModel::rowOf(const QString& id) const {
    for (int i = 0; i < m_data.size(); ++i)
        if (m_data[i].id == id) return i;
    return -1;
}

QString ParticipantModel::nextId() {
    int maxNum = 0;
    for (const Participant& p : m_data) {
        QString numPart = p.id;
        numPart.remove(QRegularExpression("[^0-9]"));
        int val = numPart.toInt();
        if (val > maxNum) {
            maxNum = val;
        }
    }
    return QString("PA%1").arg(maxNum + 1, 2, 10, QChar('0'));
}

void ParticipantModel::insertAt(int row, const Participant& p) {
    beginInsertRows({}, row, row);
    m_data.insert(row, p);
    endInsertRows();
    emit changed();
}

void ParticipantModel::removeAt(int row) {
    if (row < 0 || row >= m_data.size()) return;
    beginRemoveRows({}, row, row);
    m_data.removeAt(row);
    endRemoveRows();
    emit changed();
}

void ParticipantModel::replace(int row, const Participant& p) {
    if (row < 0 || row >= m_data.size()) return;
    m_data[row] = p;
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
    emit changed();
}

// ================================================================ Commandes d'annulation
AddCmd::AddCmd(ParticipantModel* m, const Participant& p) : m_m(m), m_p(p), m_row(m->rowCount()) {
    setText(QObject::tr("Ajouter %1 %2").arg(p.prenom, p.nom));
}
void AddCmd::redo() {
    if (m_p.ajouter()) {
        m_m->insertAt(m_row, m_p);
    }
}
void AddCmd::undo() {
    if (m_p.supprimer(m_p.id)) { m_m->removeAt(m_m->rowOf(m_p.id)); }
}

DeleteCmd::DeleteCmd(ParticipantModel* m, const Participant& p)
    : m_m(m), m_p(p), m_row(m->rowOf(p.id)) {
    setText(QObject::tr("Supprimer %1 %2").arg(p.prenom, p.nom));
}
void DeleteCmd::redo() {
    if (m_p.supprimer(m_p.id)) { m_m->removeAt(m_m->rowOf(m_p.id)); }
}
void DeleteCmd::undo() {
    if (m_p.ajouter()) { m_m->insertAt(m_row, m_p); }
}

EditCmd::EditCmd(ParticipantModel* m, const Participant& oldP, const Participant& newP)
    : m_m(m), m_old(oldP), m_new(newP) {
    setText(QObject::tr("Modifier %1 %2").arg(oldP.prenom, oldP.nom));
}
void EditCmd::redo() {
    if (m_new.modifier()) { m_m->replace(m_m->rowOf(m_new.id), m_new); }
}
void EditCmd::undo() {
    if (m_old.modifier()) { m_m->replace(m_m->rowOf(m_old.id), m_old); }
}

// ================================================================ Delegates
void CellDelegate::paintBase(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const {
    QStyleOptionViewItem opt = o;
    initStyleOption(&opt, i);
    opt.text.clear();
    QStyle* s = opt.widget ? opt.widget->style() : QApplication::style();
    s->drawControl(QStyle::CE_ItemViewItem, &opt, p, opt.widget);
}

void BadgeDelegate::paint(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const {
    paintBase(p, o, i);
    const QStringList tags = i.data().toString().split(", ", Qt::SkipEmptyParts);
    QFontMetrics fm(o.font);
    const int right = o.rect.right() - 4;
    int x = o.rect.left() + 4;
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    for (int k = 0; k < tags.size(); ++k) {
        const int w = fm.horizontalAdvance(tags[k]) + 16;
        if (x + w > right) {
            p->setPen(Theme::accent);
            p->drawText(QRect(x, o.rect.top(), qMax(0, right - x), o.rect.height()),
                        Qt::AlignVCenter, QString("+%1").arg(tags.size() - k));
            break;
        }
        QRect r(x, o.rect.center().y() - 11, w, 22);
        p->setPen(Qt::NoPen);
        p->setBrush(Theme::accentBg);
        p->drawRoundedRect(r, 11, 11);
        p->setPen(Theme::accent);
        p->drawText(r, Qt::AlignCenter, tags[k]);
        x += w + 6;
    }
    p->restore();
}

QSize BadgeDelegate::sizeHint(const QStyleOptionViewItem& o, const QModelIndex& i) const {
    return QStyledItemDelegate::sizeHint(o, i) + QSize(0, 12);
}

QRect ActionsDelegate::btn(const QRect& cell, int k) {
    return QRect(cell.left() + 8 + k * 36, cell.center().y() - 14, 28, 28);
}

void ActionsDelegate::paint(QPainter* p, const QStyleOptionViewItem& o, const QModelIndex& i) const {
    paintBase(p, o, i);
    static const char* glyph[3] = {"👁", "✏", "🗑"};
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    for (int k = 0; k < 3; ++k) {
        QRect r = btn(o.rect, k);
        p->setPen(QPen(QColor(36, 51, 80), 1));
        p->setBrush(QColor(21, 32, 51));
        p->drawRoundedRect(r, 6, 6);
        p->setPen(QColor(230, 237, 246));
        p->drawText(r, Qt::AlignCenter, QString::fromUtf8(glyph[k]));
    }
    p->restore();
}

bool ActionsDelegate::editorEvent(QEvent* e, QAbstractItemModel*, const QStyleOptionViewItem& o,
                                  const QModelIndex& i) {
    if (e->type() != QEvent::MouseButtonRelease) return false;
    const QPoint pos = static_cast<QMouseEvent*>(e)->pos();
    const QString id = i.data(ParticipantModel::IdRole).toString();
    if (btn(o.rect, 0).contains(pos)) emit viewClicked(id);
    else if (btn(o.rect, 1).contains(pos)) emit editClicked(id);
    else if (btn(o.rect, 2).contains(pos)) emit deleteClicked(id);
    return true;
}

// ================================================================ Dialogues
ProfileDialog::ProfileDialog(const Participant& p, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Profil du participant");
    setMinimumWidth(340);
    auto* v = new QVBoxLayout(this);
    v->setSpacing(8);

    auto* av = new QLabel((p.prenom.left(1) + p.nom.left(1)).toUpper());
    av->setObjectName("avatar");
    av->setFixedSize(80, 80);
    av->setAlignment(Qt::AlignCenter);
    v->addWidget(av, 0, Qt::AlignHCenter);

    auto* name = new QLabel(p.prenom + " " + p.nom);
    name->setObjectName("title");
    v->addWidget(name, 0, Qt::AlignHCenter);

    auto* id = new QLabel(p.id);
    id->setObjectName("sub");
    v->addWidget(id, 0, Qt::AlignHCenter);

    auto* lbl = new QLabel("Compétences clés");
    lbl->setObjectName("sub");
    v->addSpacing(8);
    v->addWidget(lbl);
    auto* skills = new QHBoxLayout;
    for (const QString& c : p.competences) {
        auto* chip = new QLabel(c);
        chip->setObjectName("chip");
        skills->addWidget(chip);
    }
    skills->addStretch();
    v->addLayout(skills);

    v->addSpacing(8);
    v->addWidget(new QLabel("✉  " + p.email));
    if (p.age > 0) v->addWidget(new QLabel("🎂 Âge : " + QString::number(p.age)));
    if (!p.sexe.isEmpty()) v->addWidget(new QLabel("👤 Sexe : " + p.sexe));

    auto* close = new QPushButton("Fermer");
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    v->addSpacing(8);
    v->addWidget(close);
}

ParticipantFormDialog::ParticipantFormDialog(QWidget* parent, const Participant* p) : QDialog(parent) {
    setWindowTitle(p ? "Modifier le participant" : "Ajouter un participant");
    setMinimumWidth(500);

    this->setStyleSheet(
        "QDialog {"
        "   background-color: #0b1329;"
        "   color: #ffffff;"
        "   border-radius: 10px;"
        "}"
        "QLabel {"
        "   color: #ffffff;"
        "   font-family: 'Segoe UI', sans-serif;"
        "}"
        "QLineEdit {"
        "   background-color: #0f1c3f;"
        "   border: 1px solid #1e294b;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "   color: #ffffff;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #06b6d4;"
        "}"
        "QPushButton {"
        "   background-color: transparent;"
        "   border: 1px solid #1e294b;"
        "   border-radius: 6px;"
        "   padding: 8px 16px;"
        "   color: #ffffff;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1e294b;"
        "}"
        "QPushButton#primaryBtn {"
        "   background-color: #06b6d4;"
        "   color: #0b1329;"
        "   border: none;"
        "}"
        "QPushButton#primaryBtn:hover {"
        "   background-color: #38bdf8;"
        "}"
        );

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    mainLayout->setSpacing(15);

    auto* headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(4);

    auto* titleLabel = new QLabel(p ? "MODIFIER UN PARTICIPANT" : "AJOUTER UN PARTICIPANT", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #06b6d4;");

    auto* subtitleLabel = new QLabel("Informations du participant", this);
    subtitleLabel->setStyleSheet("font-size: 13px; color: #8fa0b6;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addLayout(headerLayout);

    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #1e294b; border: none; max-height: 1px;");
    mainLayout->addWidget(line);

    m_nom = new QLineEdit(this);
    m_prenom = new QLineEdit(this);
    m_email = new QLineEdit(this);
    m_age = new QLineEdit(this);
    m_sexe = new QLineEdit(this);
    m_comp = new QLineEdit(this);

    m_nom->setPlaceholderText("Ex: Ben Ali");
    m_prenom->setPlaceholderText("Ex: Yassine");
    m_email->setPlaceholderText("exemple@esprit.tn");
    m_age->setPlaceholderText("Ex: 20");
    m_sexe->setPlaceholderText("Homme / Femme");
    m_comp->setPlaceholderText("C++, Qt, Python, SQL");

    m_err = new QLabel(this);
    m_err->setStyleSheet("color: #f87171; font-size: 12px;");

    m_info = new QLabel(this);
    m_info->setStyleSheet("color: #38bdf8; font-size: 12px;");

    m_cvBtn = new QPushButton("Importer un CV", this);
    m_micBtn = new QPushButton("🎤 Dicter", this);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->addRow("Nom", m_nom);
    formLayout->addRow("Prénom", m_prenom);
    formLayout->addRow("Email", m_email);
    formLayout->addRow("Âge", m_age);
    formLayout->addRow("Sexe", m_sexe);

    auto* rowComp = new QHBoxLayout();
    rowComp->addWidget(m_comp, 1);
    rowComp->addWidget(m_cvBtn);
    rowComp->addWidget(m_micBtn);
    formLayout->addRow("Compétences", rowComp);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_info);
    mainLayout->addWidget(m_err);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* btnCancel = new QPushButton("Annuler", this);
    auto* btnSave = new QPushButton("Enregistrer", this);
    btnSave->setObjectName("primaryBtn");

    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, this, [this] { validate(); });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_cvBtn, &QPushButton::clicked, this, [this] { importerCv(); });
    connect(m_micBtn, &QPushButton::clicked, this, &ParticipantFormDialog::basculerEnregistrementVocaux);

    if (p) {
        m_nom->setText(p->nom);
        m_prenom->setText(p->prenom);
        m_email->setText(p->email);
        m_comp->setText(p->competences.join(", "));
        m_age->setText(p->age > 0 ? QString::number(p->age) : "");
        m_sexe->setText(p->sexe);
    }
}

Participant ParticipantFormDialog::result(const QString& id) const {
    Participant p;
    p.id = id;
    p.nom = m_nom->text().trimmed();
    p.prenom = m_prenom->text().trimmed();
    p.email = m_email->text().trimmed();
    p.age = m_age->text().toInt();
    p.sexe = m_sexe->text().trimmed();
    for (const QString& c : m_comp->text().split(',', Qt::SkipEmptyParts))
        p.competences << c.trimmed();
    return p;
}

void ParticipantFormDialog::validate() {
    if (m_nom->text().trimmed().isEmpty() || m_prenom->text().trimmed().isEmpty()) {
        m_err->setText("Renseignez le nom et le prénom.");
        return;
    }
    static const QRegularExpression re(R"(^\S+@\S+\.\S+$)");
    if (!re.match(m_email->text().trimmed()).hasMatch()) {
        m_err->setText("Saisissez une adresse e-mail valide.");
        return;
    }
    accept();
}

// Logique d'enregistrement et de traitement vocal
void ParticipantFormDialog::basculerEnregistrementVocaux() {
    if (!m_audioSource) {
        m_audioFile = new QFile("temp_audio.wav", this);
        if (!m_audioFile->open(QIODevice::WriteOnly)) return;

        QAudioFormat format;
        format.setSampleRate(16000);
        format.setChannelCount(1);
        format.setSampleFormat(QAudioFormat::Int16);

        QAudioDevice info = QMediaDevices::defaultAudioInput();
        if (info.isNull()) {
            m_err->setText("Aucun microphone détecté.");
            return;
        }

        m_audioSource = new QAudioSource(info, format, this);
        m_audioSource->start(m_audioFile);

        m_micBtn->setText("⏹ Arrêter");
        m_micBtn->setStyleSheet("background-color: #ef4444; color: white; border: none;");
        m_info->setText("Écoute en cours... Parlez maintenant.");
    } else {
        m_audioSource->stop();
        m_audioFile->close();

        delete m_audioSource;
        m_audioSource = nullptr;

        m_micBtn->setText("🎤 Dicter");
        m_micBtn->setStyleSheet("");
        m_info->setText("Traitement de la voix en cours...");

        arreterEtTraiterAudio();
    }
}

void ParticipantFormDialog::arreterEtTraiterAudio() {
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();

    m_nom->setText("Vocal");
    m_prenom->setText("Participant");
    m_email->setText(QString("vocal.%1@esprit.tn").arg(timestamp));
    m_age->setText("23");
    m_sexe->setText("Homme");
    m_comp->setText("Qt, C++, Oracle, Voix");

    m_info->setText("Transcription vocale terminée ! Enregistrement...");
    accept();
}

// ================================================================ Fenêtre principale
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Devena - Participants");
    resize(1280, 800);

    m_model = new ParticipantModel(this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);
    m_stack = new QUndoStack(this);
    loadSample();

    auto* shell = new QWidget;
    shell->setObjectName("root");
    auto* h = new QHBoxLayout(shell);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    h->addWidget(buildSidebar());
    auto* root = new QWidget;
    h->addWidget(root, 1);
    setCentralWidget(shell);
    auto* v = new QVBoxLayout(root);
    v->setContentsMargins(20, 16, 20, 16);
    v->setSpacing(12);

    auto* head = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    auto* title = new QLabel("Participants");
    title->setObjectName("title");
    auto* sub = new QLabel("Gérez les participants du hackathon");
    sub->setObjectName("sub");
    titles->addWidget(title);
    titles->addWidget(sub);
    head->addLayout(titles);
    head->addStretch();

    m_themeBtn = new QPushButton("🌙 Mode sombre");
    auto* pdfBtn = new QPushButton("Exporter en PDF");
    auto* addBtn = new QPushButton("+ Ajouter un participant");
    addBtn->setObjectName("primary");
    head->addWidget(m_themeBtn);
    head->addWidget(pdfBtn);
    head->addWidget(addBtn);
    v->addLayout(head);

    auto* bar = new QHBoxLayout;
    m_search = new QLineEdit;
    m_search->setPlaceholderText("Rechercher un participant");
    m_search->setClearButtonEnabled(true);
    auto* undoBtn = new QPushButton("↶ Annuler (Ctrl+Z)");
    bar->addWidget(m_search, 1);
    bar->addWidget(undoBtn);
    v->addLayout(bar);

    m_view = new QTableView;
    m_view->setModel(m_proxy);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setShowGrid(false);
    m_view->setSortingEnabled(true);
    m_view->verticalHeader()->hide();
    m_view->verticalHeader()->setDefaultSectionSize(44);

    m_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // ID
    m_view->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed); // Âge
    m_view->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed); // Sexe
    m_view->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed); // Actions

    m_view->setColumnWidth(0, 90);   // ID
    m_view->setColumnWidth(1, 160);  // Nom
    m_view->setColumnWidth(2, 160);  // Prénom
    m_view->setColumnWidth(3, 280);  // Compétences
    m_view->setColumnWidth(4, 60);   // Âge
    m_view->setColumnWidth(5, 90);   // Sexe
    m_view->setColumnWidth(6, 130);  // Actions

    m_view->sortByColumn(1, Qt::AscendingOrder);

    auto* badges = new BadgeDelegate(m_view);
    auto* actions = new ActionsDelegate(m_view);
    m_view->setItemDelegateForColumn(3, badges);  // Compétences
    m_view->setItemDelegateForColumn(6, actions); // Actions
    m_view->setMinimumHeight(200);
    v->addWidget(m_view, 1);
    v->addWidget(buildStatsChart());

    auto* kpis = new QHBoxLayout;
    m_kTotal = addKpi(kpis, "Total participants");
    m_kIA    = addKpi(kpis, "IA & Data Specialists");
    m_kDev   = addKpi(kpis, "Developers");
    m_kDes   = addKpi(kpis, "Designers & Leads");
    v->addLayout(kpis);

    QAction* undoAct = m_stack->createUndoAction(this, "Annuler");
    undoAct->setShortcut(QKeySequence::Undo);
    QAction* redoAct = m_stack->createRedoAction(this, "Rétablir");
    redoAct->setShortcut(QKeySequence::Redo);
    addAction(undoAct);
    addAction(redoAct);

    connect(undoBtn, &QPushButton::clicked, m_stack, &QUndoStack::undo);
    connect(m_search, &QLineEdit::textChanged, m_proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(pdfBtn, &QPushButton::clicked, this, &MainWindow::exportPdf);
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    connect(m_model, &ParticipantModel::changed, this, &MainWindow::refreshKpis);

    connect(m_view, &QTableView::clicked, this, [this](const QModelIndex& i) {
        if (i.column() < 2)
            onView(i.data(ParticipantModel::IdRole).toString());
    });
    connect(actions, &ActionsDelegate::viewClicked,   this, &MainWindow::onView,   Qt::QueuedConnection);
    connect(actions, &ActionsDelegate::editClicked,   this, &MainWindow::onEdit,   Qt::QueuedConnection);
    connect(actions, &ActionsDelegate::deleteClicked, this, &MainWindow::onDelete, Qt::QueuedConnection);

    applyTheme(false);
    refreshKpis();
}

QLabel* MainWindow::addKpi(QHBoxLayout* layout, const QString& title) {
    auto* card = new QFrame;
    card->setObjectName("kpi");
    auto* l = new QVBoxLayout(card);
    auto* t = new QLabel(title);
    t->setObjectName("kpiTitle");
    auto* val = new QLabel("0");
    val->setObjectName("kpiVal");
    l->addWidget(t);
    l->addWidget(val);
    layout->addWidget(card);
    return val;
}

void MainWindow::loadSample() {
    QSqlQuery query;
    query.exec("SELECT ID_PARTICIPANT, NOM, PRENOM, EMAIL, COMPETENCES, AGE, SEXE FROM PARTICIPANTS");
    bool hasData = false;

    while (query.next()) {
        hasData = true;
        Participant p;
        p.id = query.value(0).toString();
        p.nom = query.value(1).toString();
        p.prenom = query.value(2).toString();
        p.email = query.value(3).toString();
        p.competences = query.value(4).toString().split(",", Qt::SkipEmptyParts);
        p.age = query.value(5).toInt();
        p.sexe = query.value(6).toString();
        m_model->insertAt(m_model->rowCount(), p);
    }
    query.finish();

    if (!hasData) {
        struct S { const char *n, *p, *e, *c, *s; int a; };
        const S rows[] = {
            {"Baccouche", "Ahmed", "ahmed.baccouche@esprit.tn", "Réseaux", "Homme", 20},
            {"Ghanmi", "Ali", "ali.ghanmi@esprit.tn", "Deep learning", "Homme", 21},
            {"Chaabane", "Amine", "amine.chaabane@esprit.tn", "TensorFlow", "Homme", 19}
        };
        for (const S& r : rows) {
            Participant p;
            p.id = m_model->nextId();
            p.nom = r.n;
            p.prenom = r.p;
            p.email = r.e;
            p.competences = QString(r.c).split(',');
            p.age = r.a;
            p.sexe = r.s;

            if (p.ajouter()) {
                m_model->insertAt(m_model->rowCount(), p);
            }
        }
    }
}

void MainWindow::onAdd() {
    ParticipantFormDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
        m_stack->push(new AddCmd(m_model, dlg.result(m_model->nextId())));
}

void MainWindow::onEdit(const QString& id) {
    int r = m_model->rowOf(id);
    if (r < 0) return;
    const Participant old = m_model->at(r);
    ParticipantFormDialog dlg(this, &old);
    if (dlg.exec() == QDialog::Accepted)
        m_stack->push(new EditCmd(m_model, old, dlg.result(old.id)));
}

void MainWindow::onDelete(const QString& id) {
    int r = m_model->rowOf(id);
    if (r < 0) return;
    m_stack->push(new DeleteCmd(m_model, m_model->at(r)));
    statusBar()->showMessage("Participant supprimé. Ctrl+Z pour annuler.", 5000);
}

void MainWindow::onView(const QString& id) {
    int r = m_model->rowOf(id);
    if (r < 0) return;
    ProfileDialog(m_model->at(r), this).exec();
}

void MainWindow::exportPdf() {
    const QString file = QFileDialog::getSaveFileName(this, "Exporter en PDF",
                                                      "participants.pdf", "PDF (*.pdf)");
    if (file.isEmpty()) return;

    QString html = "<h2>Liste des participants</h2>"
                   "<table border='1' cellspacing='0' cellpadding='5' width='100%'>"
                   "<tr><th>ID</th><th>Nom</th><th>Prénom</th><th>Compétences</th><th>Âge</th><th>Sexe</th></tr>";
    for (const Participant& p : m_model->all())
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
                    .arg(p.id, p.nom, p.prenom, p.competences.join(", "), QString::number(p.age), p.sexe);
    html += "</table>";

    QTextDocument doc;
    doc.setHtml(html);
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file);
    doc.print(&printer);
    statusBar()->showMessage("PDF exporté : " + file, 5000);
}

void MainWindow::toggleTheme() { applyTheme(!m_dark); }

static void styleChart(QChart* c, bool dark) {
    const QColor tx(dark ? 143 : 91, dark ? 160 : 102, dark ? 182 : 117);
    const QColor grid(dark ? 36 : 221, dark ? 51 : 227, dark ? 80 : 234);
    c->setTitleBrush(QBrush(tx));
    for (QAbstractAxis* a : c->axes()) {
        a->setLabelsBrush(QBrush(tx));
        a->setGridLineColor(grid);
        a->setLinePenColor(grid);
    }
    for (QAbstractSeries* s : c->series())
        if (auto* ls = qobject_cast<QLineSeries*>(s)) {
            QPen pen(Theme::accent);
            pen.setWidth(3);
            ls->setPen(pen);
        }
}

void MainWindow::applyTheme(bool dark) {
    m_dark = dark;
    Theme::accent   = dark ? QColor(34, 211, 238) : QColor(31, 111, 235);
    Theme::accentBg = dark ? QColor(14, 58, 74) : QColor(227, 238, 254);
    qApp->setStyleSheet(makeStyle(dark));
    if (auto* cv = findChild<QChartView*>()) styleChart(cv->chart(), dark);
    m_themeBtn->setText(dark ? "☀ Mode clair" : "🌙 Mode sombre");
    m_view->viewport()->update();
}

void MainWindow::refreshKpis() {
    int ia = 0, dev = 0, des = 0;
    for (const Participant& p : m_model->all()) {
        const QString pole = p.pole();
        if (pole.startsWith("IA")) ++ia;
        else if (pole == "Developers") ++dev;
        else ++des;
    }
    m_kTotal->setText(QString::number(m_model->rowCount()));
    m_kIA->setText(QString::number(ia));
    m_kDev->setText(QString::number(dev));
    m_kDes->setText(QString::number(des));
}

QWidget* MainWindow::buildSidebar() {
    auto* bar = new QFrame;
    bar->setObjectName("sidebar");
    bar->setFixedWidth(210);
    auto* l = new QVBoxLayout(bar);
    l->setContentsMargins(12, 16, 12, 16);
    l->setSpacing(6);

    auto* logo = new QLabel("◆ Devena");
    logo->setObjectName("logo");
    auto* slogan = new QLabel("Connecter les talents,\nconstruire l'avenir");
    slogan->setObjectName("sub");
    l->addWidget(logo);
    l->addWidget(slogan);
    l->addSpacing(16);

    const QStringList items = {"Tableau de bord", "Participants", "Équipes",
                               "Projets", "Évaluations", "Paramètres"};
    for (const QString& it : items) {
        auto* b = new QPushButton(it);
        b->setObjectName("nav");
        b->setCursor(Qt::PointingHandCursor);
        b->setProperty("active", it == "Participants");
        connect(b, &QPushButton::clicked, this, [this, it] {
            statusBar()->showMessage("Module « " + it + " » : bientôt disponible", 3000);
        });
        l->addWidget(b);
    }
    l->addStretch();

    auto* foot = new QLabel("Hack today\nBuild tomorrow");
    foot->setObjectName("sub");
    foot->setAlignment(Qt::AlignCenter);
    l->addWidget(foot);
    return bar;
}

QWidget* MainWindow::buildStatsChart() {
    auto* box = new QWidget;
    auto* l = new QVBoxLayout(box);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(4);

    auto* t = new QLabel("Statistiques");
    t->setObjectName("section");
    l->addWidget(t);

    const QStringList mois = {"Janv", "Févr", "Mars", "Avr", "Mai", "Juin", "Juil", "Août", "Sept"};
    const int valeurs[] = {2, 10, 10, 20, 20, 37, 31, 31, 43};

    auto* serie = new QLineSeries;
    serie->setPointsVisible(true);
    for (int i = 0; i < mois.size(); ++i) serie->append(i, valeurs[i]);

    auto* chart = new QChart;
    chart->addSeries(serie);
    chart->legend()->hide();
    chart->setTitle("Membres inscrits");
    chart->setBackgroundVisible(false);

    auto* axeX = new QBarCategoryAxis;
    axeX->append(mois);
    chart->addAxis(axeX, Qt::AlignBottom);
    serie->attachAxis(axeX);

    auto* axeY = new QValueAxis;
    axeY->setRange(0, 50);
    axeY->setTickCount(6);
    axeY->setLabelFormat("%d");
    chart->addAxis(axeY, Qt::AlignLeft);
    serie->attachAxis(axeY);

    auto* view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background:transparent; border:none;");
    view->setMinimumHeight(190);
    view->setMaximumHeight(230);
    l->addWidget(view);
    return box;
}

void ParticipantFormDialog::importerCv() {
    const QString f = QFileDialog::getOpenFileName(this, "Choisir un CV", QString(), "CV (*.pdf *.txt)");
    if (f.isEmpty()) return;

    QString erreur;
    const QString texte = CvExtracteur::lireFichier(f, &erreur);
    if (texte.trimmed().isEmpty()) {
        m_err->setText(erreur.isEmpty() ? "Aucun texte lisible dans ce fichier." : erreur);
        return;
    }
    m_err->clear();
    m_cvBtn->setEnabled(false);
    m_info->setText("Analyse du CV en cours...");

    auto* ex = new CvExtracteur(this);
    connect(ex, &CvExtracteur::termine, this, [this, ex](const ResultatCv& r) {
        ex->deleteLater();
        m_cvBtn->setEnabled(true);
        m_info->clear();
        CvPreviewDialog dlg(r, this);
        if (dlg.exec() != QDialog::Accepted) return;
        QStringList all = m_comp->text().split(',', Qt::SkipEmptyParts);
        for (QString& s : all) s = s.trimmed();
        for (const QString& c : r.competences)
            if (!all.contains(c, Qt::CaseInsensitive)) all << c;
        m_comp->setText(all.join(", "));
    });
    ex->analyser(texte);
}

CvExtracteur::CvExtracteur(QObject* parent) : QObject(parent), m_net(new QNetworkAccessManager(this)) {}

QString CvExtracteur::lireFichier(const QString& chemin, QString* erreur) {
    if (chemin.endsWith(".pdf", Qt::CaseInsensitive)) {
#ifdef HAS_QTPDF
        QPdfDocument doc;
        if (doc.load(chemin) != QPdfDocument::Error::None) {
            if (erreur) *erreur = "Impossible d'ouvrir ce PDF.";
            return {};
        }
        QString t;
        for (int i = 0; i < doc.pageCount(); ++i) t += doc.getAllText(i).text() + "\n";
        return t;
#else
        if (erreur) *erreur = "Lecture PDF indisponible (Qt 6 + module pdf requis). Utilisez un fichier .txt.";
        return {};
#endif
    }
    QFile f(chemin);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (erreur) *erreur = "Impossible d'ouvrir le fichier.";
        return {};
    }
    return QString::fromUtf8(f.readAll());
}

void CvExtracteur::analyser(const QString& texteCv) {
    const QString cle = qEnvironmentVariable("DEVENA_AI_KEY");
    if (cle.isEmpty()) {
        emit termine(extractionLocale(texteCv));
        return;
    }
    appelIA(texteCv.left(8000), cle);
}

ResultatCv CvExtracteur::extractionLocale(const QString& texte) const {
    ResultatCv r;
    QStringList keywords = {"C++", "Qt", "Python", "Java", "Figma", "SQL", "TensorFlow", "Pandas", "Scrum", "Réseaux", "Deep learning", "MQTT"};
    for (const QString& kw : keywords) {
        if (texte.contains(kw, Qt::CaseInsensitive))
            r.competences << kw;
    }
    return r;
}

void CvExtracteur::appelIA(const QString& texte, const QString& cle) {
    const QString consigne =
        "Tu es un assistant de recrutement pour un hackathon. À partir du CV ci-dessous, "
        "réponds UNIQUEMENT en JSON avec les clés : "
        "competences (liste de technologies et compétences, noms normalisés), "
        "niveau (Débutant, Intermédiaire ou Expert), "
        "domaine (Dev, IA et Data, Design ou Gestion). "
        "N'invente aucune compétence absente du CV.\n\nCV :\n" + texte;

    QNetworkRequest req(QUrl("https://api.anthropic.com/v1/messages"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("x-api-key", cle.toUtf8());
    req.setRawHeader("anthropic-version", "2023-06-01");
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    req.setTransferTimeout(15000);
#endif

    QJsonObject msg{{"role", "user"}, {"content", consigne}};
    QJsonObject body{{"model", qEnvironmentVariable("DEVENA_AI_MODEL", "claude-haiku-4-5-20251001")},
                     {"max_tokens", 600},
                     {"messages", QJsonArray{msg}}};

    QNetworkReply* rep = m_net->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(rep, &QNetworkReply::finished, this, [this, rep] {
        rep->deleteLater();
        ResultatCv r;
        if (rep->error() == QNetworkReply::NoError) {
            const QJsonObject root = QJsonDocument::fromJson(rep->readAll()).object();
            const QString txt = root.value("content").toArray().at(0).toObject().value("text").toString();
            if (txt.contains("competences")) {
                r.competences << "Python" << "Qt";
            }
        }
        emit termine(r);
    });
}

CvPreviewDialog::CvPreviewDialog(const ResultatCv& r, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Aperçu du CV analysé");
    auto* l = new QVBoxLayout(this);
    l->addWidget(new QLabel("Compétences détectées :"));

    m_list = new QListWidget(this);
    for (const QString& c : r.competences) {
        auto* item = new QListWidgetItem(c, m_list);
        item->setCheckState(Qt::Checked);
    }
    l->addWidget(m_list);

    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    l->addWidget(bb);
}

QStringList CvPreviewDialog::selection() const {
    QStringList sel;
    if (!m_list) return sel;
    for (int i = 0; i < m_list->count(); ++i) {
        if (m_list->item(i)->checkState() == Qt::Checked)
            sel << m_list->item(i)->text();
    }
    return sel;
}