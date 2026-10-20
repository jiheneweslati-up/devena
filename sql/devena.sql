-- =====================================================================
-- Devena : schéma Oracle conforme au MCD + données de test
-- Utilisateur Oracle : devena    Source ODBC : devena
--
-- Le fichier est en UTF-8 :
--   SQL*Plus      : set NLS_LANG=AMERICAN_AMERICA.AL32UTF8
--                   sqlplus devena/2006@XE @database/devena.sql
--   SQL Developer : Outils > Préférences > Environnement > Encodage = UTF-8,
--                   puis « Exécuter le script » (F5)
--
-- Attention : le script supprime puis recrée les 5 tables.
-- =====================================================================

SET DEFINE OFF
WHENEVER SQLERROR EXIT SQL.SQLCODE ROLLBACK

BEGIN
    FOR t IN (SELECT table_name FROM user_tables
              WHERE table_name IN ('PARTICIPANTS', 'EMPLOYES', 'EQUIPES',
                                   'EVALUATION', 'EVALUTION', 'PROJETS')) LOOP
        EXECUTE IMMEDIATE 'DROP TABLE ' || t.table_name || ' CASCADE CONSTRAINTS PURGE';
    END LOOP;
END;
/

-- ---------------------------------------------------------------------
-- Tables
-- ---------------------------------------------------------------------

CREATE TABLE projets (
    id_projet    VARCHAR2(30)        CONSTRAINT projets_pk PRIMARY KEY,
    titre        VARCHAR2(80 CHAR)   NOT NULL,
    description  VARCHAR2(400 CHAR)  NOT NULL,
    thematique   VARCHAR2(30 CHAR)   NOT NULL,
    note_finale  NUMBER(4,1)         NOT NULL,
    CONSTRAINT ck_projets_thematique CHECK (thematique IN ('IA', 'Web', 'IoT')),
    CONSTRAINT ck_projets_note       CHECK (note_finale BETWEEN 0 AND 20)
);

CREATE TABLE evaluation (
    id_evaluation VARCHAR2(30)        CONSTRAINT evaluation_pk PRIMARY KEY,
    note          NUMBER(4,1)         NOT NULL,
    remarque      VARCHAR2(200 CHAR),
    CONSTRAINT ck_evaluation_note CHECK (note BETWEEN 0 AND 20)
);

-- Équipes (1,1) : chaque équipe développe un seul projet et a une seule évaluation.
CREATE TABLE equipes (
    id_equipe     VARCHAR2(30)        CONSTRAINT equipes_pk PRIMARY KEY,
    nom_equipe    VARCHAR2(50 CHAR)   NOT NULL,
    nb_membres    NUMBER              NOT NULL,
    capacite_max  NUMBER              DEFAULT 6 NOT NULL,
    sexe          VARCHAR2(20)        NOT NULL,
    age           NUMBER              NOT NULL,
    competences   VARCHAR2(200 CHAR),
    id_projet     VARCHAR2(30)        NOT NULL,
    id_evaluation VARCHAR2(30)        NOT NULL,
    CONSTRAINT ck_equipes_capacite   CHECK (capacite_max BETWEEN 1 AND 6),
    CONSTRAINT ck_equipes_membres    CHECK (nb_membres BETWEEN 1 AND capacite_max),
    CONSTRAINT fk_equipes_projet     FOREIGN KEY (id_projet)     REFERENCES projets (id_projet),
    CONSTRAINT fk_equipes_evaluation FOREIGN KEY (id_evaluation) REFERENCES evaluation (id_evaluation)
);

-- Participants (1,1) : chaque participant appartient à une seule équipe.
CREATE TABLE participants (
    id_participant VARCHAR2(30)        CONSTRAINT participants_pk PRIMARY KEY,
    nom            VARCHAR2(30 CHAR)   NOT NULL,
    prenom         VARCHAR2(30 CHAR)   NOT NULL,
    email          VARCHAR2(80 CHAR)   NOT NULL,
    competences    VARCHAR2(100 CHAR)  NOT NULL,
    id_equipe      VARCHAR2(30)        NOT NULL,
    CONSTRAINT uq_participants_email  UNIQUE (email),
    CONSTRAINT fk_participants_equipe FOREIGN KEY (id_equipe) REFERENCES equipes (id_equipe)
);

-- Employés (1,1) : chaque employé gère une seule évaluation.
CREATE TABLE employes (
    id_employe    VARCHAR2(30)        CONSTRAINT employes_pk PRIMARY KEY,
    nom           VARCHAR2(30 CHAR)   NOT NULL,
    prenom        VARCHAR2(30 CHAR)   NOT NULL,
    specialite    VARCHAR2(50 CHAR)   NOT NULL,
    id_evaluation VARCHAR2(30)        NOT NULL,
    CONSTRAINT fk_employes_evaluation FOREIGN KEY (id_evaluation) REFERENCES evaluation (id_evaluation)
);

CREATE INDEX idx_equipes_projet      ON equipes (id_projet);
CREATE INDEX idx_equipes_evaluation  ON equipes (id_evaluation);
CREATE INDEX idx_participants_equipe ON participants (id_equipe);
CREATE INDEX idx_employes_evaluation ON employes (id_evaluation);

-- ---------------------------------------------------------------------
-- Données de test
-- ---------------------------------------------------------------------

INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P001', 'Smart City', 'Tableau de bord qui pilote l''éclairage public, le trafic et le stationnement en temps réel.', 'IA', 17.5);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P002', 'Health App', 'Application de suivi des rendez-vous médicaux et des constantes de santé des étudiants.', 'Web', 15.0);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P003', 'EduTech', 'Plateforme de quiz adaptatifs qui suit la progression des élèves en mathématiques.', 'Web', 18.0);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P004', 'GreenRoute', 'Capteurs connectés qui mesurent la qualité de l''air sur les trajets du campus.', 'IoT', 16.5);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P005', 'SecureVote', 'Détection des votes suspects pendant les délibérations du jury par apprentissage automatique.', 'IA', 14.0);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P006', 'CampusNav', 'Plan interactif du campus pour trouver les salles, les stands et les ateliers.', 'Web', 13.5);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P007', 'MediScan', 'Analyse d''images médicales pour proposer un premier diagnostic aux médecins.', 'IA', 19.0);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P008', 'AquaWatch', 'Surveillance du niveau et de la qualité de l''eau des fontaines publiques.', 'IoT', 16.0);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P009', 'FarmSense', 'Arrosage intelligent piloté par des capteurs d''humidité du sol et la météo.', 'IoT', 15.5);
INSERT INTO projets (id_projet, titre, description, thematique, note_finale) VALUES ('P010', 'JobMatch', 'Mise en relation des étudiants et des entreprises selon leurs compétences.', 'Web', 12.5);

INSERT INTO evaluation (id_evaluation, note, remarque) VALUES ('EV01', 16.5, 'Jury IA : modèles convaincants, données d''entraînement bien préparées.');
INSERT INTO evaluation (id_evaluation, note, remarque) VALUES ('EV02', 14.5, 'Jury Web : interfaces soignées, démonstrations à mieux préparer.');
INSERT INTO evaluation (id_evaluation, note, remarque) VALUES ('EV03', 15.5, 'Jury IoT : prototypes fonctionnels, documentation technique à compléter.');
INSERT INTO evaluation (id_evaluation, note, remarque) VALUES ('EV04', 13.0, 'Jury Sécurité : bonne analyse des risques, tests d''intrusion insuffisants.');

INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e1',  'equipe degital', 5, 6, 'h2/3f', 22, 'Web, UI/UX, JavaScript',            'P006', 'EV02');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e2',  'NovaLab',        4, 5, '2H/2F', 21, 'IA, Python, vision par ordinateur', 'P001', 'EV01');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e3',  'Pixel Crew',     5, 6, '3H/2F', 22, 'Web, React, Node.js',               'P002', 'EV02');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e4',  'DataMiners',     3, 4, '1H/2F', 23, 'Data science, Python, SQL',         'P007', 'EV01');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e5',  'GreenCode',      4, 5, '2H/2F', 20, 'IoT, Arduino, capteurs',            'P004', 'EV03');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e6',  'CyberShield',    4, 6, '3H/1F', 22, 'Cybersécurité, réseaux, Linux',     'P005', 'EV04');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e7',  'ByteForce',      4, 6, '3H/1F', 21, 'Java, Spring Boot, Angular',        'P010', 'EV02');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e8',  'AgriTech Squad', 3, 4, '2H/1F', 22, 'IoT, ESP32, MQTT',                  'P009', 'EV03');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e9',  'MedAI',          4, 5, '1H/3F', 23, 'IA, imagerie médicale, PyTorch',    'P007', 'EV01');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e10', 'Code Phoenix',   5, 6, '3H/2F', 21, 'IA, Qt/C++, OpenCV',                'P001', 'EV01');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e11', 'CloudNine',      3, 5, '2H/1F', 20, 'Web, Flutter, Firebase',            'P003', 'EV02');
INSERT INTO equipes (id_equipe, nom_equipe, nb_membres, capacite_max, sexe, age, competences, id_projet, id_evaluation) VALUES ('e12', 'Quantum Bits',   4, 6, '2H/2F', 22, 'IoT, Raspberry Pi, Python',         'P008', 'EV03');

INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA01', 'Ben Ali',    'Mohamed', 'mohamed.benali@esprit.tn',    'Web, JavaScript',       'e1');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA02', 'Ayari',      'Rania',   'rania.ayari@esprit.tn',       'UI/UX, Figma',          'e1');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA03', 'Sassi',      'Yasmine', 'yasmine.sassi@esprit.tn',     'React',                 'e1');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA04', 'Dridi',      'Khalil',  'khalil.dridi@esprit.tn',      'Node.js',               'e1');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA05', 'Mejri',      'Eya',     'eya.mejri@esprit.tn',         'HTML/CSS',              'e1');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA06', 'Hammami',    'Youssef', 'youssef.hammami@esprit.tn',   'Python, IA',            'e2');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA07', 'Zouari',     'Sarra',   'sarra.zouari@esprit.tn',      'Vision par ordinateur', 'e2');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA08', 'Chaabane',   'Amine',   'amine.chaabane@esprit.tn',    'TensorFlow',            'e2');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA09', 'Rekik',      'Nour',    'nour.rekik@esprit.tn',        'Data science',          'e2');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA10', 'Ferchichi',  'Omar',    'omar.ferchichi@esprit.tn',    'React',                 'e3');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA11', 'Masmoudi',   'Ines',    'ines.masmoudi@esprit.tn',     'UI/UX',                 'e3');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA12', 'Kammoun',    'Aziz',    'aziz.kammoun@esprit.tn',      'Node.js',               'e3');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA13', 'Bouazizi',   'Mariem',  'mariem.bouazizi@esprit.tn',   'Oracle SQL',            'e3');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA14', 'Jaziri',     'Hamza',   'hamza.jaziri@esprit.tn',      'Qt/C++',                'e3');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA15', 'Gharsalli',  'Salma',   'salma.gharsalli@esprit.tn',   'Python',                'e4');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA16', 'Tlili',      'Fares',   'fares.tlili@esprit.tn',       'SQL, Power BI',         'e4');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA17', 'Ben Amor',   'Chaima',  'chaima.benamor@esprit.tn',    'Machine learning',      'e4');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA18', 'Khelifi',    'Rayen',   'rayen.khelifi@esprit.tn',     'Arduino',               'e5');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA19', 'Hajji',      'Molka',   'molka.hajji@esprit.tn',       'Capteurs IoT',          'e5');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA20', 'Belhadj',    'Skander', 'skander.belhadj@esprit.tn',   'C embarqué',            'e5');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA21', 'Saidi',      'Hiba',    'hiba.saidi@esprit.tn',        'Électronique',          'e5');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA22', 'Ouni',       'Wassim',  'wassim.ouni@esprit.tn',       'Tests d''intrusion',    'e6');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA23', 'Baccouche',  'Ahmed',   'ahmed.baccouche@esprit.tn',   'Réseaux',               'e6');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA24', 'Gabsi',      'Oumayma', 'oumayma.gabsi@esprit.tn',     'Cryptographie',         'e6');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA25', 'Toumi',      'Seif',    'seif.toumi@esprit.tn',        'Linux',                 'e6');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA26', 'Riahi',      'Bilel',   'bilel.riahi@esprit.tn',       'Java, Spring Boot',     'e7');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA27', 'Nasri',      'Mehdi',   'mehdi.nasri@esprit.tn',       'Angular',               'e7');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA28', 'Chebbi',     'Emna',    'emna.chebbi@esprit.tn',       'UI/UX',                 'e7');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA29', 'Laabidi',    'Anis',    'anis.laabidi@esprit.tn',      'Oracle SQL',            'e7');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA30', 'Brahmi',     'Iheb',    'iheb.brahmi@esprit.tn',       'ESP32',                 'e8');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA31', 'Fourati',    'Aya',     'aya.fourati@esprit.tn',       'MQTT',                  'e8');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA32', 'Arfaoui',    'Malek',   'malek.arfaoui@esprit.tn',     'Capteurs',              'e8');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA33', 'Bouslama',   'Farah',   'farah.bouslama@esprit.tn',    'PyTorch',               'e9');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA34', 'Chouchane',  'Lina',    'lina.chouchane@esprit.tn',    'Imagerie médicale',     'e9');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA35', 'Mabrouk',    'Asma',    'asma.mabrouk@esprit.tn',      'Python',                'e9');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA36', 'Ghanmi',     'Ali',     'ali.ghanmi@esprit.tn',        'Deep learning',         'e9');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA37', 'Jendoubi',   'Yassine', 'yassine.jendoubi@esprit.tn',  'Qt/C++',                'e10');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA38', 'Sfaxi',      'Taha',    'taha.sfaxi@esprit.tn',        'OpenCV',                'e10');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA39', 'Hamdi',      'Rim',     'rim.hamdi@esprit.tn',         'Python',                'e10');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA40', 'Marzouki',   'Ghassen', 'ghassen.marzouki@esprit.tn',  'IA',                    'e10');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA41', 'Abidi',      'Dorra',   'dorra.abidi@esprit.tn',       'Data visualisation',    'e10');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA42', 'Kefi',       'Nidhal',  'nidhal.kefi@esprit.tn',       'Flutter',               'e11');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA43', 'Bejaoui',    'Mouna',   'mouna.bejaoui@esprit.tn',     'Firebase',              'e11');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA44', 'Lahmar',     'Zied',    'zied.lahmar@esprit.tn',       'Dart',                  'e11');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA45', 'Ben Youssef','Karim',   'karim.benyoussef@esprit.tn',  'Raspberry Pi',          'e12');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA46', 'Hachicha',   'Sirine',  'sirine.hachicha@esprit.tn',   'Python',                'e12');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA47', 'Zribi',      'Adem',    'adem.zribi@esprit.tn',        'Électronique',          'e12');
INSERT INTO participants (id_participant, nom, prenom, email, competences, id_equipe) VALUES ('PA48', 'Guesmi',     'Nesrine', 'nesrine.guesmi@esprit.tn',    'IoT',                   'e12');

INSERT INTO employes (id_employe, nom, prenom, specialite, id_evaluation) VALUES ('001', 'aakari',    'mahdi', 'ai',                  'EV01');
INSERT INTO employes (id_employe, nom, prenom, specialite, id_evaluation) VALUES ('002', 'Ben Salah', 'Amira', 'Data science',        'EV01');
INSERT INTO employes (id_employe, nom, prenom, specialite, id_evaluation) VALUES ('003', 'Gharbi',    'Walid', 'Développement web',   'EV02');
INSERT INTO employes (id_employe, nom, prenom, specialite, id_evaluation) VALUES ('004', 'Trabelsi',  'Ines',  'UX design',           'EV02');
INSERT INTO employes (id_employe, nom, prenom, specialite, id_evaluation) VALUES ('005', 'Mansouri',  'Karim', 'Systèmes embarqués',  'EV03');
INSERT INTO employes (id_employe, nom, prenom, specialite, id_evaluation) VALUES ('006', 'Jebali',    'Sonia', 'Réseaux IoT',         'EV03');
INSERT INTO employes (id_employe, nom, prenom, specialite, id_evaluation) VALUES ('007', 'Haddad',    'Nizar', 'Cybersécurité',       'EV04');
INSERT INTO employes (id_employe, nom, prenom, specialite, id_evaluation) VALUES ('008', 'Karray',    'Leila', 'Sécurité réseau',     'EV04');

COMMIT;

SELECT 'PROJETS' AS table_name, COUNT(*) AS nb_lignes FROM projets
UNION ALL SELECT 'EVALUATION',   COUNT(*) FROM evaluation
UNION ALL SELECT 'EQUIPES',      COUNT(*) FROM equipes
UNION ALL SELECT 'PARTICIPANTS', COUNT(*) FROM participants
UNION ALL SELECT 'EMPLOYES',     COUNT(*) FROM employes;
