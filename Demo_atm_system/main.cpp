#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

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

class CompteBancaire {
    std::string titulaire;
    double solde;

public:
    CompteBancaire(std::string nom, double initial)
        : titulaire(std::move(nom)), solde(initial)
    {
        if (initial < 0.0)
            throw std::invalid_argument(txt("Solde initial negatif refuse.", "Tsy azo ekena ny ambiny voalohany ratsy."));
        std::cout << txt("Compte ouvert : ", "ny kaonty misokatra : ") << titulaire << "\n";
    }

    virtual ~CompteBancaire() = default;

    double getSolde() const { return solde; }
    const std::string& getTitulaire() const { return titulaire; }

    void deposer(double montant) {
        if (montant <= 0.0) { std::cout << txt("Montant invalide.\n", "Vola tsy mety.\n"); return; }
        solde += montant;
        std::cout << "+ " << montant << " Ariary  =>  " << solde << " Ariary\n";
    }

    void retirer(double montant) {
        if (montant <= 0.0) { std::cout << txt("Montant invalide.\n", "Vola tsy mety.\n"); return; }
        if (montant > solde) { std::cout << txt("Solde insuffisant (", "Tsy ampy ny ambiny (") << solde << txt(" Ariary).\n", " Ariary).\n"); return; }
        solde -= montant;
        std::cout << "- " << montant << " Ariary  =>  " << solde << " Ariary\n";
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
    CompteEpargne(std::string nom, double initial, double taux)
        : CompteBancaire(std::move(nom), initial), taux(taux)
    {
        if (this->taux < 0.0)
            throw std::invalid_argument(txt("Taux negatif refuse.", "Tsy azo ekena ny tahan'ny zanabola ratsy."));
        std::cout << txt("Taux : ", "Tahan'ny zanabola : ") << this->taux << "%\n";
    }

    void appliquerInterets() {
        deposer(getSolde() * taux / 100.0);
    }

    void afficherType() const override {
        std::cout << txt("Compte epargne  (taux : ", "Kaonty fitahirizana  (tahan'ny zanabola : ") << taux << "%)\n";
    }
};

int main() {
    int choixLangue;

    std::cout << "Choisir la langue / Safidy fiteny\n";
    std::cout << "1. Francais\n";
    std::cout << "2. Malagasy\n";
    std::cout << "> ";

    if (!(std::cin >> choixLangue)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Langue invalide.\n";
        return 1;
    }

    if (choixLangue == 2) {
        langueActuelle = 2;
    } else {
        langueActuelle = 1;
    }

    try {
        CompteEpargne compte("Ahmed Benali", 50000.0, 3.5);

        int choix;
        bool actif = true;

        while (actif) {
            std::cout << "\n"
                      << txt("1. Infos", "1. Fampahalalana") << "   "
                      << txt("2. Depot", "2. Fampidiram-bola") << "   "
                      << txt("3. Retrait", "3. Fanesorana") << "   "
                      << txt("4. Interets", "4. Zanabola") << "   "
                      << txt("5. Quitter", "5. Hiala") << "\n> ";

            if (!(std::cin >> choix)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << txt("Entree invalide.\n", "Tsy mety ny fidirana.\n");
                continue;
            }

            switch (choix) {
                case 1: compte.afficher(); break;
                case 2: compte.deposer(readAmount(txt("Montant : ", "Vola : ")));  break;
                case 3: compte.retirer(readAmount(txt("Montant : ", "Vola : ")));  break;
                case 4: compte.appliquerInterets(); break;
                case 5: actif = false; break;
                default: std::cout << txt("Choix hors plage (1-5).\n", "Safidy ivelan'ny faritra (1-5).\n");
            }
        }

        std::cout << "\n" << txt("Merci d'avoir utilise notre ATM. Au revoir !", "Misaotra nampiasa ny ATM anay. Veloma !") << "\n";

    } catch (const std::exception& e) {
        std::cerr << txt("Erreur : ", "Hadisoana : ") << e.what() << "\n";
        return 1;
    }

    return 0;
}