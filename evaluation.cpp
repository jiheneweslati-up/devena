#include "evaluation.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QObject>
#include <QRegularExpression>
#include <QColor>
#include <QDateTime>
#include <QBrush>
#include <utility>
#include <cmath>


// ===========================================================================
//  Outils internes
// ===========================================================================
namespace {

QString g_erreur;

bool executer(QSqlQuery &q)
{
    if (!q.exec()) {
        g_erreur = q.lastError().text();
        return false;
    }
    g_erreur.clear();
    return true;
}

// Partie commune des requêtes d'affichage
// (le nombre d'équipes évaluées vient de la clé étrangère EQUIPES.ID_EVALUATION)
const char *SELECT_LISTE =
    "SELECT ev.ID_EVALUATION, ev.NOM, ev.TYPE, ev.NOTE, ev.REMARQUE, ev.DATE_EVALUATION, "
    "       (SELECT COUNT(*) FROM EQUIPES eq WHERE eq.ID_EVALUATION = ev.ID_EVALUATION) AS NB_EQUIPES "
    "FROM EVALUATION ev ";

// Les dates vides (NULL) sont placées à la fin, aussi bien sous Oracle que sous SQLite
const char *ORDRE_RECENTES =
    " ORDER BY CASE WHEN ev.DATE_EVALUATION IS NULL THEN 1 ELSE 0 END, "
    "          ev.DATE_EVALUATION DESC, ev.ID_EVALUATION DESC";

// Tri : seules ces valeurs sont possibles (jamais de texte saisi dans un ORDER BY)
QString clauseTri(int tri)
{
    switch (tri) {
    case 1:  return " ORDER BY ev.NOTE DESC, ev.ID_EVALUATION";
    case 2:  return " ORDER BY ev.NOTE ASC, ev.ID_EVALUATION";
    case 3:  return " ORDER BY CASE WHEN ev.NOM IS NULL THEN 1 ELSE 0 END, ev.NOM ASC, ev.ID_EVALUATION";
    default: return ORDRE_RECENTES;                                  // plus récentes
    }
}

// Recherche multicritère (identifiant, nom, type, remarque) + période
QSqlQuery requeteListe(const QString &recherche, int tri, const QDate &depuis)
{
    QString sql = QString(SELECT_LISTE) +
                  "WHERE (UPPER(ev.ID_EVALUATION) LIKE :m1 "
                  "    OR UPPER(ev.NOM)           LIKE :m2 "
                  "    OR UPPER(ev.TYPE)          LIKE :m3 "
                  "    OR UPPER(ev.REMARQUE)      LIKE :m4) ";
    if (depuis.isValid())
        sql += "  AND ev.DATE_EVALUATION >= :depuis ";
    sql += clauseTri(tri);

    QSqlQuery q;
    q.prepare(sql);
    const QString motif = "%" + recherche.trimmed().toUpper() + "%";
    q.bindValue(":m1", motif);
    q.bindValue(":m2", motif);
    q.bindValue(":m3", motif);
    q.bindValue(":m4", motif);
    if (depuis.isValid())
        q.bindValue(":depuis", depuis);
    return q;
}

void entetesListe(QSqlQueryModel *model)
{
    model->setHeaderData(Evaluation::COL_ID,       Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(Evaluation::COL_NOM,      Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(Evaluation::COL_TYPE,     Qt::Horizontal, QObject::tr("Type d'évaluation"));
    model->setHeaderData(Evaluation::COL_NOTE,     Qt::Horizontal, QObject::tr("Note / 20"));
    model->setHeaderData(Evaluation::COL_REMARQUE, Qt::Horizontal, QObject::tr("Remarque"));
    model->setHeaderData(Evaluation::COL_DATE,     Qt::Horizontal, QObject::tr("Date d'évaluation"));
    model->setHeaderData(Evaluation::COL_EQUIPES,  Qt::Horizontal, QObject::tr("Équipes"));
}

QDate lireDate(const QVariant &v)
{
    if (v.isNull())
        return QDate();
    return v.toDate().isValid() ? v.toDate() : v.toDateTime().date();
}

QList<ResumeEvaluation> lireResumes(QSqlQuery &q, int maximum = -1)
{
    QList<ResumeEvaluation> liste;
    while (q.next() && (maximum < 0 || liste.size() < maximum)) {
        ResumeEvaluation r;
        r.id        = q.value(0).toString();
        r.nom       = q.value(1).toString();
        r.type      = q.value(2).toString();
        r.note      = q.value(3).toDouble();
        r.remarque  = q.value(4).toString();
        r.date      = lireDate(q.value(5));
        r.nbEquipes = q.value(6).toInt();
        liste.append(r);
    }
    return liste;
}

int compter(const QString &sql, const QString &id)
{
    QSqlQuery q;
    q.prepare(sql);
    q.bindValue(":id", id);
    if (!executer(q) || !q.next())
        return 0;
    return q.value(0).toInt();
}

} // namespace

// ===========================================================================
//  Constructeurs
// ===========================================================================
Evaluation::Evaluation()
    : note(0.0), dateEvaluation(QDate::currentDate())
{
}

Evaluation::Evaluation(const QString &id, double note, const QString &remarque,
                       const QDate &dateEvaluation, const QString &nom, const QString &type)
{
    this->id             = id;
    this->note           = note;
    this->remarque       = remarque;
    this->dateEvaluation = dateEvaluation;
    this->nom            = nom;
    this->type           = type;
}

QString Evaluation::derniereErreur()
{
    return g_erreur;
}

// ===========================================================================
//  Identifiant suivant : EV01, EV02, EV03, EV04  ->  EV05
// ===========================================================================
QString Evaluation::prochainId()
{
    static const QRegularExpression format("^EV(\\d+)$", QRegularExpression::CaseInsensitiveOption);

    int max = 0;
    QSqlQuery query;
    query.prepare("SELECT ID_EVALUATION FROM EVALUATION");
    if (executer(query)) {
        while (query.next()) {
            const QRegularExpressionMatch m = format.match(query.value(0).toString().trimmed());
            if (m.hasMatch())
                max = qMax(max, m.captured(1).toInt());
        }
    }
    return QString("EV%1").arg(max + 1, 2, 10, QChar('0'));
}

// ===========================================================================
//  CREATE
// ===========================================================================
bool Evaluation::ajouter()
{
    if (id.trimmed().isEmpty())
        id = prochainId();

    QSqlQuery query;
    query.prepare("INSERT INTO EVALUATION (ID_EVALUATION, NOTE, REMARQUE, DATE_EVALUATION, NOM, TYPE) "
                  "VALUES (:id, :note, :remarque, :date, :nom, :type)");

    // Création des variables liées
    query.bindValue(":id",       id);
    query.bindValue(":note",     note);
    query.bindValue(":remarque", remarque);
    query.bindValue(":date",     dateEvaluation);
    query.bindValue(":nom",      nom);
    query.bindValue(":type",     type);

    return executer(query);
}

// ===========================================================================
//  READ
// ===========================================================================
QSqlQueryModel *Evaluation::afficher(const QString &recherche, int tri, const QDate &depuis)
{
    auto *model = new EvaluationModel();
    model->setColonneNote(COL_NOTE);
    model->setColonneDate(COL_DATE);

    QSqlQuery query = requeteListe(recherche, tri, depuis);
    executer(query);
    model->setQuery(std::move(query));
    entetesListe(model);
    return model;
}

QList<ResumeEvaluation> Evaluation::lister(const QString &recherche, int tri, const QDate &depuis)
{
    QSqlQuery query = requeteListe(recherche, tri, depuis);
    if (!executer(query))
        return {};
    return lireResumes(query);
}

bool Evaluation::chargerParId(const QString &idRecherche)
{
    QSqlQuery query;
    query.prepare("SELECT NOTE, REMARQUE, DATE_EVALUATION, NOM, TYPE "
                  "FROM EVALUATION WHERE ID_EVALUATION = :id");
    query.bindValue(":id", idRecherche);
    if (!executer(query) || !query.next())
        return false;

    id             = idRecherche;
    note           = query.value(0).toDouble();
    remarque       = query.value(1).toString();
    dateEvaluation = lireDate(query.value(2));
    nom            = query.value(3).toString();
    type           = query.value(4).toString();
    return true;
}

// ===========================================================================
//  UPDATE
// ===========================================================================
bool Evaluation::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE EVALUATION SET NOTE = :note, REMARQUE = :remarque, "
                  "DATE_EVALUATION = :date, NOM = :nom, TYPE = :type "
                  "WHERE ID_EVALUATION = :id");
    query.bindValue(":note",     note);
    query.bindValue(":remarque", remarque);
    query.bindValue(":date",     dateEvaluation);
    query.bindValue(":nom",      nom);
    query.bindValue(":type",     type);
    query.bindValue(":id",       id);

    if (!executer(query))
        return false;
    if (query.numRowsAffected() == 0) {
        g_erreur = QObject::tr("Aucune évaluation ne correspond à l'identifiant %1.").arg(id);
        return false;
    }
    return true;
}

// ===========================================================================
//  DELETE
//  EQUIPES et GERER référencent EVALUATION (clés étrangères) :
//  on vérifie d'abord qu'aucune ligne n'en dépend, sinon Oracle refuse
//  la suppression (ORA-02292).
// ===========================================================================
bool Evaluation::supprimer(const QString &idASupprimer)
{
    const int equipes  = nbEquipesLiees(idASupprimer);
    const int employes = nbEmployesLies(idASupprimer);
    if (equipes > 0 || employes > 0) {
        g_erreur = QObject::tr("L'évaluation %1 est encore utilisée par %2 équipe(s) et %3 employé(s).\n"
                               "Rattachez-les d'abord à une autre évaluation.")
                       .arg(idASupprimer).arg(equipes).arg(employes);
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM EVALUATION WHERE ID_EVALUATION = :id");
    query.bindValue(":id", idASupprimer);

    if (!executer(query))
        return false;
    if (query.numRowsAffected() == 0) {
        g_erreur = QObject::tr("Aucune évaluation ne correspond à l'identifiant %1.").arg(idASupprimer);
        return false;
    }
    return true;
}

// ===========================================================================
//  Contrôles métier
// ===========================================================================
// Pas deux évaluations avec le même nom, le même type et la même date
bool Evaluation::existeDeja() const
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM EVALUATION "
                  "WHERE UPPER(NOM) = UPPER(:nom) AND UPPER(TYPE) = UPPER(:type) "
                  "AND DATE_EVALUATION = :date AND ID_EVALUATION <> :id");
    query.bindValue(":nom",  nom);
    query.bindValue(":type", type);
    query.bindValue(":date", dateEvaluation);
    query.bindValue(":id",   id.isEmpty() ? QString("-") : id);
    if (!executer(query) || !query.next())
        return false;
    return query.value(0).toInt() > 0;
}

int Evaluation::nbEquipesLiees(const QString &id)
{
    return compter("SELECT COUNT(*) FROM EQUIPES WHERE ID_EVALUATION = :id", id);
}

int Evaluation::nbEmployesLies(const QString &id)
{
    // Les employés sont rattachés aux évaluations par l'association GERER
    return compter("SELECT COUNT(*) FROM GERER WHERE ID_EVALUATION = :id", id);
}

// ===========================================================================
//  Fonctionnalités avancées
// ===========================================================================
QSqlQueryModel *Evaluation::classementEquipes()
{
    auto *model = new EvaluationModel();
    model->setColonneRang(0);
    model->setColonneNote(2);

    QSqlQuery query;
    query.prepare("SELECT ROW_NUMBER() OVER (ORDER BY ev.NOTE DESC, eq.NOM_EQUIPE) AS RANG, "
                  "       eq.NOM_EQUIPE, ev.NOTE, ev.ID_EVALUATION "
                  "FROM EQUIPES eq JOIN EVALUATION ev ON ev.ID_EVALUATION = eq.ID_EVALUATION "
                  "ORDER BY RANG");
    executer(query);
    model->setQuery(std::move(query));
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Rang"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Équipe"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Note / 20"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Évaluation"));
    return model;
}

QSqlQueryModel *Evaluation::alertes()
{
    auto *model = new EvaluationModel();
    model->setColonneNote(COL_NOTE);
    model->setColonneDate(COL_DATE);

    QSqlQuery query;
    query.prepare(QString(SELECT_LISTE) + "WHERE ev.NOTE < :seuil ORDER BY ev.NOTE ASC, ev.ID_EVALUATION");
    query.bindValue(":seuil", EvaluationModel::SEUIL_ALERTE);
    executer(query);
    model->setQuery(std::move(query));
    entetesListe(model);
    return model;
}

StatistiquesEvaluation Evaluation::statistiques()
{
    StatistiquesEvaluation s;

    QSqlQuery q1;
    q1.prepare("SELECT COUNT(*), COALESCE(AVG(NOTE), 0), "
               "       SUM(CASE WHEN NOTE >= 16 THEN 1 ELSE 0 END), "
               "       SUM(CASE WHEN NOTE >= 14 AND NOTE < 16 THEN 1 ELSE 0 END), "
               "       SUM(CASE WHEN NOTE >= 10 AND NOTE < 14 THEN 1 ELSE 0 END), "
               "       SUM(CASE WHEN NOTE < 10 THEN 1 ELSE 0 END) "
               "FROM EVALUATION");
    if (executer(q1) && q1.next()) {
        s.total   = q1.value(0).toInt();
        s.moyenne = q1.value(1).toDouble();
        for (int i = 0; i < 4; ++i)
            s.tranches[i] = q1.value(2 + i).toInt();
    }

    const QDate auj = QDate::currentDate();
    QSqlQuery q2;
    q2.prepare("SELECT COUNT(*) FROM EVALUATION WHERE DATE_EVALUATION >= :debut");
    q2.bindValue(":debut", QDate(auj.year(), auj.month(), 1));
    if (executer(q2) && q2.next())
        s.totalCeMois = q2.value(0).toInt();

    QSqlQuery q3;
    q3.prepare("SELECT COUNT(*), SUM(CASE WHEN ID_EVALUATION IS NOT NULL THEN 1 ELSE 0 END) FROM EQUIPES");
    if (executer(q3) && q3.next()) {
        s.totalEquipes    = q3.value(0).toInt();
        s.equipesEvaluees = q3.value(1).toInt();
    }

    QSqlQuery q4;
    q4.prepare("SELECT COUNT(*) FROM EMPLOYES");
    if (executer(q4) && q4.next())
        s.totalEmployes = q4.value(0).toInt();

    // Évaluateurs = employés reliés à au moins une évaluation (association GERER)
    QSqlQuery q4b;
    q4b.prepare("SELECT COUNT(DISTINCT ID_EMPLOYE) FROM GERER");
    if (executer(q4b) && q4b.next())
        s.evaluateurs = q4b.value(0).toInt();

    QSqlQuery q5;
    q5.prepare("SELECT eq.NOM_EQUIPE, ev.NOTE "
               "FROM EQUIPES eq JOIN EVALUATION ev ON ev.ID_EVALUATION = eq.ID_EVALUATION "
               "ORDER BY ev.NOTE DESC, eq.NOM_EQUIPE");
    if (executer(q5))
        while (q5.next())
            s.noteParEquipe.append({q5.value(0).toString(), q5.value(1).toDouble()});

    return s;
}

QList<ResumeEvaluation> Evaluation::dernieres(int nombre)
{
    QSqlQuery query;
    query.prepare(QString(SELECT_LISTE) + ORDRE_RECENTES);
    if (!executer(query))
        return {};
    return lireResumes(query, nombre);
}

// ===========================================================================
//  Types proposés dans le formulaire (ceux de la base + thématiques des projets)
// ===========================================================================
QStringList Evaluation::listeTypes()
{
    QStringList types = {"IA", "Web", "IoT", "Securite"};
    QSqlQuery query;
    query.prepare("SELECT DISTINCT TYPE FROM EVALUATION WHERE TYPE IS NOT NULL");
    if (executer(query))
        while (query.next()) {
            const QString t = query.value(0).toString().trimmed();
            if (!t.isEmpty() && !types.contains(t, Qt::CaseInsensitive))
                types.append(t);
        }
    return types;
}

// ===========================================================================
// Classe EvaluationModel
// ===========================================================================

EvaluationModel::EvaluationModel(QObject *parent) : QSqlQueryModel(parent)
{
}

// Note sur 20 -> 0 à 5 étoiles (4 points par étoile)
QString EvaluationModel::etoiles(double note)
{
    const int pleines = qBound(0, static_cast<int>(note / 4.0 + 0.3), 5);
    return QString::fromUtf8("★").repeated(pleines) + QString::fromUtf8("☆").repeated(5 - pleines);
}

QVariant EvaluationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QVariant brut = QSqlQueryModel::data(index, Qt::DisplayRole);
    const int col = index.column();

    // Note de la ligne (sert aussi à colorer toute la ligne en cas d'alerte)
    double noteLigne = -1;
    if (m_colNote >= 0)
        noteLigne = QSqlQueryModel::data(this->index(index.row(), m_colNote), Qt::DisplayRole).toDouble();
    const bool alerte = noteLigne >= 0 && noteLigne < SEUIL_ALERTE;

    switch (role) {
    case Qt::DisplayRole:
        if (col == m_colNote) {
            const double n = brut.toDouble();
            return etoiles(n) + "   " + QString::number(n, 'f', 1) + " / 20"
                   + (alerte ? QString::fromUtf8("  ⚠") : QString());
        }
        if (col == m_colRang) {
            const int r = brut.toInt();
            if (r == 1) return QString::fromUtf8("🥇");
            if (r == 2) return QString::fromUtf8("🥈");
            if (r == 3) return QString::fromUtf8("🥉");
            return QString::number(r);
        }
        if (brut.isNull() || brut.toString().trimmed().isEmpty())
            return QString::fromUtf8("—");                     // valeur vide dans la base
        if (col == m_colDate) {
            const QDate d = brut.toDate().isValid() ? brut.toDate() : brut.toDateTime().date();
            return d.isValid() ? d.toString("dd/MM/yyyy") : brut.toString();
        }
        if (brut.typeId() == QMetaType::Double) {
            const double v = brut.toDouble();
            return (v == std::floor(v)) ? QString::number(static_cast<qlonglong>(v))
                                        : QString::number(v, 'f', 1);
        }
        return brut;

    case Qt::ForegroundRole:
        if (col == m_colNote)
            return QBrush(alerte ? QColor("#ff8a8a") : QColor("#f5c242"));
        return QVariant();

    case Qt::BackgroundRole:
        if (alerte)
            return QBrush(QColor(110, 39, 48, 70));
        return QVariant();

    case Qt::TextAlignmentRole:
        if (col == m_colRang)
            return int(Qt::AlignCenter);
        return int(Qt::AlignLeft | Qt::AlignVCenter);

    case Qt::UserRole:          // valeur brute (pour le code)
        return brut;

    default:
        return QSqlQueryModel::data(index, role);
    }
}
