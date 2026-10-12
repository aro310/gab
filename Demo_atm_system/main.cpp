#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

static int langueActuelle = 1;

static std::string txt(const std::string& fr, const std::string& mg) {
    return langueActuelle == 2 ? mg : fr;
}

static double readAmount(const std::string& prompt) {
    double v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v) return v;
        std::cout << txt("Valeur invalide, reessayez.\n", "Tsy mety ny sanda, andramo indray.\n");
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

static std::string now() {
    std::time_t t = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

struct Transaction {
    std::string date;
    std::string type;
    double montant;
    double solde;
};

static void appendCSV(const Transaction& tx, const std::string& path) {
    std::ofstream f(path, std::ios::app);
    if (!f) return;
    f << tx.date << "," << tx.type << "," << tx.montant << "," << tx.solde << "\n";
}

static std::vector<Transaction> loadCSV(const std::string& path) {
    std::vector<Transaction> hist;
    std::ifstream f(path);
    if (!f) return hist;

    std::string line;
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        Transaction tx;
        std::string montant, solde;
        if (!std::getline(ss, tx.date,   ',')) continue;
        if (!std::getline(ss, tx.type,   ',')) continue;
        if (!std::getline(ss, montant,   ',')) continue;
        if (!std::getline(ss, solde,     ',')) continue;
        try {
            tx.montant = std::stod(montant);
            tx.solde   = std::stod(solde);
        } catch (...) { continue; }
        hist.push_back(tx);
    }
    return hist;
}

class CompteBancaire {
    std::string titulaire;
    double solde;
    std::string csvPath;
    std::vector<Transaction> historique;

    void log(const std::string& type, double montant) {
        Transaction tx{ now(), type, montant, solde };
        historique.push_back(tx);
        appendCSV(tx, csvPath);
    }

public:
    CompteBancaire(std::string nom, double initial, std::string path = "historique.csv")
        : titulaire(std::move(nom)), solde(initial), csvPath(std::move(path))
    {
        if (initial < 0.0)
            throw std::invalid_argument(txt("Solde initial negatif refuse.", "Tsy azo ekena ny ambiny voalohany ratsy."));
        historique = loadCSV(csvPath);
        std::cout << txt("Compte ouvert : ", "ny kaonty misokatra : ") << titulaire << "\n";
    }

    virtual ~CompteBancaire() = default;

    double getSolde() const { return solde; }
    const std::string& getTitulaire() const { return titulaire; }

    void deposer(double montant, const std::string& type = "DEPOT") {
        if (montant <= 0.0) { std::cout << txt("Montant invalide.\n", "Vola tsy mety.\n"); return; }
        solde += montant;
        std::cout << "+ " << montant << " Ariary  =>  " << solde << " Ariary\n";
        log(type, montant);
    }

    void retirer(double montant) {
        if (montant <= 0.0) { std::cout << txt("Montant invalide.\n", "Vola tsy mety.\n"); return; }
        if (montant > solde) {
            std::cout << txt("Solde insuffisant (", "Tsy ampy ny ambiny (")
                      << solde << txt(" Ariary).\n", " Ariary).\n");
            return;
        }
        solde -= montant;
        std::cout << "- " << montant << " Ariary  =>  " << solde << " Ariary\n";
        log("RETRAIT", montant);
    }

    void afficherHistorique() const {
        if (historique.empty()) {
            std::cout << txt("Aucune transaction enregistree.\n", "Tsy misy ara-bola voarakitra.\n");
            return;
        }
        std::cout << "\n";
        for (const auto& tx : historique)
            std::cout << tx.date << "  " << tx.type << "  "
                      << tx.montant << " Ariary  =>  " << tx.solde << " Ariary\n";
    }

    virtual void afficherType() const {
        std::cout << txt("Compte standard\n", "Kaonty mahazatra\n");
    }

    void afficher() const {
        std::cout << "\n" << txt("Titulaire : ", "Tompon-kaonty : ") << titulaire
                  << "\n" << txt("Solde     : ", "Ambiny    : ") << solde << " Ariary\n";
        afficherType();
    }
};

class CompteEpargne : public CompteBancaire {
    double taux;

public:
    CompteEpargne(std::string nom, double initial, double taux, std::string path = "historique.csv")
        : CompteBancaire(std::move(nom), initial, std::move(path)), taux(taux)
    {
        if (this->taux < 0.0)
            throw std::invalid_argument(txt("Taux negatif refuse.", "Tsy azo ekena ny tahan'ny zanabola ratsy."));
        std::cout << txt("Taux : ", "Tahan'ny zanabola : ") << this->taux << "%\n";
    }

    void appliquerInterets() {
        deposer(getSolde() * taux / 100.0, "INTERET");
    }

    void afficherType() const override {
        std::cout << txt("Compte epargne  (taux : ", "Kaonty fitahirizana  (tahan'ny zanabola : ")
                  << taux << "%)\n";
    }
};

int main() {
    int choixLangue;
    std::cout << "Choisir la langue / Safidy fiteny\n1. Francais\n2. Malagasy\n> ";

    if (!(std::cin >> choixLangue)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Langue invalide.\n";
        return 1;
    }

    langueActuelle = (choixLangue == 2) ? 2 : 1;

    try {
        CompteEpargne compte("Ahmed Benali", 50000.0, 3.5);

        int choix;
        bool actif = true;

        while (actif) {
            std::cout << "\n"
                      << txt("1. Infos",      "1. Fampahafantarana")  << "   "
                      << txt("2. Depot",      "2. Hampiditra vola")<< "   "
                      << txt("3. Retrait",    "3. Haka vola")     << "   "
                      << txt("4. Interets",   "4. Zanabola")       << "   "
                      << txt("5. Historique", "5. fifanakalozana farany")        << "   "
                      << txt("6. Quitter",    "6. Hivoaka")          << "\n> ";

            if (!(std::cin >> choix)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << txt("Entree invalide.\n", "Tsy mety ny fidirana.\n");
                continue;
            }

            switch (choix) {
                case 1: compte.afficher();                                              break;
                case 2: compte.deposer(readAmount(txt("Montant : ", "Vola : ")));      break;
                case 3: compte.retirer(readAmount(txt("Montant : ", "Vola : ")));      break;
                case 4: compte.appliquerInterets();                                     break;
                case 5: compte.afficherHistorique();                                    break;
                case 6: actif = false;                                                  break;
                default: std::cout << txt("Choix hors plage (1-6).\n", "Safidy ivelan'ny fetra (1-6).\n");
            }
        }

        std::cout << "\n" << txt("Merci d'avoir utilise notre ATM. Au revoir !", "Misaotra nampiasa ny ATM anay. Veloma !") << "\n";

    } catch (const std::exception& e) {
        std::cerr << txt("Erreur : ", "Hadisoana : ") << e.what() << "\n";
        return 1;
    }

    return 0;
}