#ifndef EVALUATION_H
#define EVALUATION_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QList>
#include <QPair>
#include <QSqlQuery>
#include <QSqlQueryModel>

// ---------------------------------------------------------------------------
//  Classe purement C++ (MODÈLE) associée à la table EVALUATION
//  (base commune de l'équipe : database/devena.sql)
//
//  EVALUATION (ID_EVALUATION, NOTE, REMARQUE, DATE_EVALUATION, NOM, TYPE)
//  EQUIPES    (..., #ID_EVALUATION, #ID_PROJET)  -> une évaluation note plusieurs équipes
//  EMPLOYES   (ID_EMPLOYE, NOM, PRENOM, MOT_DE_PASSE, POSTE)
//  GERER      (#ID_EMPLOYE, #ID_EVALUATION, ID_GERER) -> employés évaluateurs
//
//  Toutes les requêtes SQL du module sont ici (architecture Modèle-Vue),
//  et ce sont toutes des requêtes préparées (protection contre l'injection SQL).
// ---------------------------------------------------------------------------

// Statistiques calculées en base (métier « statistiques »)
struct StatistiquesEvaluation
{
    int    total           = 0;
    int    totalCeMois     = 0;
    int    equipesEvaluees = 0;
    int    totalEquipes    = 0;
    int    evaluateurs     = 0;
    int    totalEmployes   = 0;
    double moyenne         = 0.0;
    int    tranches[4]     = {0, 0, 0, 0};   // ≥16 | 14–16 | 10–14 | <10
    QList<QPair<QString, double>> noteParEquipe;
};

// Ligne résumée (dernières évaluations, export PDF)
struct ResumeEvaluation
{
    QString id;
    QString nom;
    QString type;
    double  note = 0.0;
    QString remarque;
    QDate   date;
    int     nbEquipes = 0;
};

class Evaluation
{
    QString id;               // ID_EVALUATION  (ex. « EV05 »)
    double  note;             // NOTE           (0 à 20)
    QString remarque;         // REMARQUE       (obligatoire, 200 caractères max)
    QDate   dateEvaluation;   // DATE_EVALUATION
    QString nom;              // NOM            (ex. « Jury IA », 20 caractères max)
    QString type;             // TYPE           (ex. « IA », « Web », « IoT »)

public:
    // ----- Constructeurs -----
    Evaluation();
    Evaluation(const QString &id, double note, const QString &remarque, const QDate &dateEvaluation,
               const QString &nom, const QString &type);

    // ----- Getters -----
    QString getId()             const { return id; }
    double  getNote()           const { return note; }
    QString getRemarque()       const { return remarque; }
    QDate   getDateEvaluation() const { return dateEvaluation; }
    QString getNom()            const { return nom; }
    QString getType()           const { return type; }

    // ----- Setters -----
    void setId(const QString &v)          { id = v; }
    void setNote(double v)                { note = v; }
    void setRemarque(const QString &v)    { remarque = v; }
    void setDateEvaluation(const QDate &v){ dateEvaluation = v; }
    void setNom(const QString &v)         { nom = v; }
    void setType(const QString &v)        { type = v; }

    // ----- Fonctionnalités de base (CRUD) -----
    bool ajouter();                                   // Create (identifiant EVxx généré)
    QSqlQueryModel *afficher(const QString &recherche = QString(),
                             int tri = 0, const QDate &depuis = QDate());   // Read
    bool modifier();                                  // Update (utilise l'attribut id)
    bool supprimer(const QString &id);                // Delete
    bool chargerParId(const QString &id);             // remplit l'objet depuis la base

    // ----- Contrôles métier -----
    bool existeDeja() const;                          // même nom + même type + même date
    static int nbEquipesLiees(const QString &id);     // équipes qui référencent l'évaluation
    static int nbEmployesLies(const QString &id);     // employés liés (association GERER)
    static QString prochainId();                      // EV01, EV02... -> EV05

    // ----- Fonctionnalités avancées -----
    QSqlQueryModel *classementEquipes();              // podium des équipes (note reçue)
    QSqlQueryModel *alertes();                        // évaluations avec une note < 10
    StatistiquesEvaluation statistiques();
    QList<ResumeEvaluation> dernieres(int nombre);
    QList<ResumeEvaluation> lister(const QString &recherche, int tri, const QDate &depuis);

    // ----- Liste pour le champ « Type » du formulaire -----
    static QStringList listeTypes();

    static QString derniereErreur();

    // Colonnes renvoyées par afficher()
    enum Colonne { COL_ID = 0, COL_NOM, COL_TYPE, COL_NOTE, COL_REMARQUE, COL_DATE, COL_EQUIPES };

    // Règles de saisie (mêmes limites que la base)
    static constexpr double NOTE_MAX         = 20.0;
    static constexpr int    LONGUEUR_NOM     = 20;   // VARCHAR2(20 BYTE)
    static constexpr int    LONGUEUR_TYPE    = 20;   // VARCHAR2(20 BYTE)
    static constexpr int    REMARQUE_MIN     = 3;
    static constexpr int    REMARQUE_MAX     = 200;
};


// ===========================================================================
// Modèle d'affichage (QSqlQueryModel mis en forme pour le QTableView)
// ---------------------------------------------------------------------------
//   - colonne « note »  : étoiles + valeur /20, en rouge si note < 10 (alerte)
//   - colonne « date »  : format jj/mm/aaaa (Oracle renvoie un QDateTime)
//   - colonne « rang »  : médailles 🥇🥈🥉 pour le podium
//   - valeur vide (NULL) : affichée « — »
//  Aucune requête ici : le modèle est rempli par la classe Evaluation.
// ===========================================================================
class EvaluationModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    explicit EvaluationModel(QObject *parent = nullptr);

    void setColonneNote(int c) { m_colNote = c; }
    void setColonneDate(int c) { m_colDate = c; }
    void setColonneRang(int c) { m_colRang = c; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    static constexpr double SEUIL_ALERTE = 10.0;      // note sur 20
    static QString etoiles(double note);              // 5 étoiles pour une note sur 20

private:
    int m_colNote = -1;
    int m_colDate = -1;
    int m_colRang = -1;
};

#endif // EVALUATION_H
