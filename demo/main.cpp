#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

static void enableAnsi() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m;
    GetConsoleMode(h, &m);
    SetConsoleMode(h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
}

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

static std::string fmtNum(double v) {
    std::string s = std::to_string((long long)v);
    for (int i = (int)s.size() - 3; i > 0; i -= 3)
        s.insert(i, " ");
    return s;
}

static std::string rep(const std::string& s, int n) {
    std::string r;
    r.reserve(s.size() * n);
    for (int i = 0; i < n; ++i) r += s;
    return r;
}

namespace ansi {
    const std::string rst  = "\033[0m";
    const std::string bold = "\033[1m";
    const std::string red  = "\033[31m";
    const std::string grn  = "\033[32m";
    const std::string yel  = "\033[33m";
    const std::string cyn  = "\033[36m";
}

static const int W = 58;
static void cls() { std::cout << "\033[2J\033[H" << std::flush; }

static int visLen(const std::string& s) {
    int n = 0;
    bool esc = false;
    for (unsigned char c : s) {
        if (c == '\033')         { esc = true;  continue; }
        if (esc)                 { if (c == 'm') esc = false; continue; }
        if ((c & 0xC0) != 0x80) ++n;
    }
    return n;
}

static std::string fit(const std::string& s, int w) {
    int vl = visLen(s);
    return vl >= w ? s : s + std::string(w - vl, ' ');
}

static void top() { std::cout << ansi::cyn << "\xe2\x95\x94" << rep("\xe2\x95\x90", W) << "\xe2\x95\x97\n" << ansi::rst; }
static void bot() { std::cout << ansi::cyn << "\xe2\x95\x9a" << rep("\xe2\x95\x90", W) << "\xe2\x95\x9d\n" << ansi::rst; }
static void sep() { std::cout << ansi::cyn << "\xe2\x95\xa0" << rep("\xe2\x95\x90", W) << "\xe2\x95\xa3\n" << ansi::rst; }

static void row(const std::string& s, const std::string& clr = "") {
    std::cout << ansi::cyn << "\xe2\x95\x91 " << ansi::rst
              << clr << fit(s, W - 2) << ansi::rst
              << ansi::cyn << " \xe2\x95\x91\n" << ansi::rst;
}

static void cRow(const std::string& s, const std::string& clr = "") {
    int p = std::max(0, (W - 2 - visLen(s)) / 2);
    row(std::string(p, ' ') + s, clr);
}

static void waitEnter() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

struct Transaction { std::string date, type; double montant, solde; };
struct OpResult    { bool ok; std::string msg; };

static void appendCSV(const Transaction& tx, const std::string& path) {
    std::ofstream f(path, std::ios::app);
    if (f) f << tx.date << "," << tx.type << "," << tx.montant << "," << tx.solde << "\n";
}

static std::vector<Transaction> loadCSV(const std::string& path) {
    std::vector<Transaction> h;
    std::ifstream f(path);
    if (!f) return h;
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        Transaction tx; std::string m, s;
        if (!std::getline(ss, tx.date, ',') || !std::getline(ss, tx.type, ',') ||
            !std::getline(ss, m, ',')        || !std::getline(ss, s, ',')) continue;
        try { tx.montant = std::stod(m); tx.solde = std::stod(s); }
        catch (...) { continue; }
        h.push_back(tx);
    }
    return h;
}

class CompteBancaire {
    std::string titulaire;
    double solde, soldeMin, limiteSession, retraitSession;
    std::string csvPath;
    std::vector<Transaction> historique;

    void log(const std::string& type, double montant) {
        Transaction tx{ now(), type, montant, solde };
        historique.push_back(tx);
        appendCSV(tx, csvPath);
    }

public:
    CompteBancaire(std::string nom, double initial,
                   double sMin = 0.0, double limSess = 0.0,
                   std::string path = "")
        : titulaire(std::move(nom)), solde(initial),
          soldeMin(sMin), limiteSession(limSess),
          retraitSession(0.0),
          csvPath(path.empty() ? titulaire + ".csv" : path)
    {
        if (initial < 0.0) throw std::invalid_argument("Solde initial negatif.");
        if (limSess < 0.0) throw std::invalid_argument("Limite session negative.");
        historique = loadCSV(csvPath);
    }

    virtual ~CompteBancaire() = default;

    double getSolde()          const { return solde; }
    double getSoldeMin()       const { return soldeMin; }
    double getLimiteSession()  const { return limiteSession; }
    double getRetraitSession() const { return retraitSession; }
    const std::string& getTitulaire()               const { return titulaire; }
    const std::vector<Transaction>& getHistorique() const { return historique; }

    virtual std::string getType()        const { return txt("Compte standard", "Kaonty mahazatra"); }
    virtual bool supporteInterets()      const { return false; }
    virtual OpResult appliquerInterets()       { return {false, "Non applicable."}; }

    OpResult deposer(double montant, const std::string& type = "DEPOT") {
        if (montant <= 0.0) return {false, txt("Montant invalide.", "Vola tsy mety.")};
        solde += montant;
        log(type, montant);
        return {true, "+ " + fmtNum(montant) + txt(" Ariary credite.", " Ariary nampidirina.")};
    }

    OpResult retirer(double montant) {
        if (montant <= 0.0)
            return {false, txt("Montant invalide.", "Vola tsy mety.")};
        if (montant > solde)
            return {false, txt("Solde insuffisant (", "Tsy ampy (") + fmtNum(solde) + " Ariary)."};
        if ((solde - montant) < soldeMin)
            return {false, txt("Solde min requis : ", "Ambiny kely : ") + fmtNum(soldeMin) + " Ariary."};
        if (limiteSession > 0.0 && (retraitSession + montant) > limiteSession)
            return {false, txt("Limite session : ", "Fetra fotoana : ") + fmtNum(limiteSession) + " Ariary."};
        solde -= montant;
        retraitSession += montant;
        log("RETRAIT", montant);
        return {true, "- " + fmtNum(montant) + txt(" Ariary debite.", " Ariary nesorina.")};
    }
};

class CompteEpargne : public CompteBancaire {
    double taux;

public:
    CompteEpargne(std::string nom, double initial, double t,
                  double sMin = 0.0, double limSess = 0.0)
        : CompteBancaire(std::move(nom), initial, sMin, limSess), taux(t)
    {
        if (taux < 0.0) throw std::invalid_argument("Taux negatif.");
    }

    std::string getType() const override {
        std::ostringstream os;
        os << txt("Epargne (taux:", "Fitahirizana (") << taux << "%)";
        return os.str();
    }

    bool supporteInterets()      const override { return true; }
    OpResult appliquerInterets()       override {
        return deposer(getSolde() * taux / 100.0, "INTERET");
    }
};

class CompteVue : public CompteBancaire {
    double decouvertMax;

public:
    CompteVue(std::string nom, double initial, double decouvert = 0.0)
        : CompteBancaire(std::move(nom), initial, -decouvert), decouvertMax(decouvert)
    {}

    std::string getType() const override {
        std::ostringstream os;
        if (decouvertMax > 0.0)
            os << txt("Vue (dec:", "Mahazatra (") << fmtNum(decouvertMax) << " Ar)";
        else
            os << txt("Vue", "Mahazatra");
        return os.str();
    }
};

struct Status { std::string msg; bool err = false; };

static int selectCompte(const std::vector<std::unique_ptr<CompteBancaire>>& comptes) {
    int wNom = 4, wType = 4;
    for (const auto& cp : comptes) {
        wNom  = std::max(wNom,  (int)cp->getTitulaire().size());
        wType = std::max(wType, visLen(cp->getType()));
    }
    int budget = W - 2 - 6 - 12;
    if (wNom + wType + 2 > budget) {
        wType = budget / 2;
        wNom  = budget - wType;
    }

    cls(); top();
    cRow(txt("SELECTION DU COMPTE", "FIFIDIANANA KAONTY"), ansi::bold + ansi::cyn);
    sep();
    for (int i = 0; i < (int)comptes.size(); ++i) {
        const auto& c = *comptes[i];
        std::string line = "  [" + std::to_string(i + 1) + "] "
            + fit(c.getTitulaire(), wNom + 1)
            + fit(c.getType(),      wType + 1)
            + ansi::grn + fmtNum(c.getSolde()) + " Ar" + ansi::rst;
        row(line);
    }
    sep(); bot();

    int choix;
    while (true) {
        std::cout << "\n" << ansi::yel << "> "
                  << txt("Choisir un compte (1-", "Safidio kaonty (1-")
                  << comptes.size() << ") : " << ansi::rst;
        if (std::cin >> choix && choix >= 1 && choix <= (int)comptes.size())
            return choix - 1;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << txt("Choix invalide.\n", "Safidy tsy mety.\n");
    }
}

//Authentification

static const std::string CARTE_NUM = "4532123456789010";
static const std::string CARTE_PIN = "1234";
static const int MAX_ESSAIS = 3;

static std::string stripSpaces(std::string s) {
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
    return s;
}

// Lit le PIN en affichant des '*' 
static std::string readHidden() {
    std::cout << std::flush;
    std::string s;
#ifdef _WIN32
    int c;
    while ((c = _getch()) != '\r' && c != '\n') {
        if (c == 8) {
            if (!s.empty()) { s.pop_back(); std::cout << "\b \b"; }
        } else if (c >= 32 && c < 127) {
            s += static_cast<char>(c);
            std::cout << '*';
        }
        std::cout.flush();
    }
#else
    termios old{};
    tcgetattr(STDIN_FILENO, &old);
    termios t = old;
    t.c_lflag &= ~(ECHO | ICANON);
    t.c_cc[VMIN]  = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n' && c != '\r') {
        if (c == 127 || c == 8) {
            if (!s.empty()) { s.pop_back(); std::cout << "\b \b" << std::flush; }
        } else if (c >= 32 && c < 127) {
            s += c;
            std::cout << '*' << std::flush;
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
    std::cout << "\n";
    return s;
}

static bool authentifier() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string err;

    for (int essai = 1; essai <= MAX_ESSAIS; ++essai) {
        cls();
        top();
        cRow(txt("AUTHENTIFICATION", "FANAMARINANA"), ansi::bold + ansi::cyn);
        sep();
        row("  " + txt("Entrez le numero de carte et votre PIN.",
                       "Ampidiro ny laharan'ny karatra sy ny PIN."));
        if (!err.empty()) {
            sep();
            row("  " + err, ansi::red);
        }
        bot();

        std::cout << "\n" << ansi::yel << "> " << ansi::rst
                  << txt("Numero de carte : ", "Laharan'ny karatra : ");
        std::string carte;
        std::getline(std::cin, carte);

        std::cout << ansi::yel << "> " << ansi::rst
                  << txt("Code PIN : ", "Kaody PIN : ");
        std::string pin = readHidden();

        if (!std::cin) return false;
        if (stripSpaces(carte) == CARTE_NUM && pin == CARTE_PIN) return true;

        err = txt("Carte ou PIN incorrect (", "Diso ny karatra na ny PIN (")
            + std::to_string(MAX_ESSAIS - essai)
            + txt(" essai(s) restant(s)).", " ezaka sisa).");
    }
    return false;
}

static void showWelcome(const std::string& nom) {
    cls();
    top();
    row("");
    cRow(txt("Bienvenue Mr ", "Tonga soa Andriamatoa ") + nom + " !",
         ansi::grn + ansi::bold);
    row("");
    bot();
    std::this_thread::sleep_for(std::chrono::milliseconds(1800));
}

// Ecrans 

static void showMenu(const CompteBancaire& c, const Status& st = {}) {
    cls(); top();
    cRow(txt("SYSTEME ATM  v1.0", "RAFI-BOLA ATM  v1.0"), ansi::bold + ansi::cyn);
    sep();
    row(" " + txt("Titulaire : ", "Tompon-kaonty : ") + c.getTitulaire());
    row(" " + txt("Solde     : ", "Ambiny        : ")
            + ansi::grn + ansi::bold + fmtNum(c.getSolde()) + " Ariary" + ansi::rst);
    row(" " + txt("Compte    : ", "Kaonty        : ") + c.getType());
    if (c.getLimiteSession() > 0.0)
        row(" " + txt("Session   : ", "Fotoana       : ")
                + fmtNum(c.getRetraitSession()) + " / " + fmtNum(c.getLimiteSession()) + " Ariary");
    sep();
    row("  " + txt("[1] Infos   [2] Depot   [3] Retrait",
                   "[1] Fampahalalana  [2] Ampiditra  [3] Nesorana"));
    if (c.supporteInterets())
        row("  " + txt("[4] Interets   [5] Historique   [6] Changer   [7] Quitter",
                       "[4] Zanabola   [5] Tantara      [6] Hanova    [7] Hiala"));
    else
        row("  " + txt("[5] Historique   [6] Changer compte   [7] Quitter",
                       "[5] Tantara      [6] Hanova kaonty    [7] Hiala"));
    sep();
    row("  " + st.msg, st.err ? ansi::red : ansi::grn);
    bot();
    std::cout << "\n" << ansi::yel << "> " << txt("Votre choix : ", "Safidio : ") << ansi::rst;
}

static void showInfo(const CompteBancaire& c) {
    cls(); top();
    cRow(txt("DETAILS DU COMPTE", "ANTSIPIRIHANA NY KAONTY"), ansi::bold + ansi::cyn);
    sep();
    row(" " + txt("Titulaire : ", "Tompon-kaonty : ") + c.getTitulaire());
    row(" " + txt("Solde     : ", "Ambiny        : ")
            + ansi::grn + ansi::bold + fmtNum(c.getSolde()) + " Ariary" + ansi::rst);
    row(" " + txt("Type      : ", "Kaonty        : ") + c.getType());
    if (c.getLimiteSession() > 0.0)
        row(" " + txt("Session   : ", "Fotoana       : ")
                + fmtNum(c.getRetraitSession()) + " / " + fmtNum(c.getLimiteSession()) + " Ariary");
    sep();
    row("  " + txt("[ Entree pour revenir ]", "[ Entree hanohy ]"), ansi::yel);
    bot();
    waitEnter();
}

static void printHistory(const CompteBancaire& c) {
    cls(); top();
    cRow(txt("HISTORIQUE DES TRANSACTIONS", "TANTARA NY ARA-BOLA"), ansi::bold + ansi::cyn);
    sep();
    const auto& hist = c.getHistorique();
    if (hist.empty()) {
        row("  " + txt("Aucune transaction.", "Tsy misy tantara."), ansi::yel);
    } else {
        row("  " + fit("DATE", 21) + fit("TYPE", 9) + fit("MONTANT", 12) + "SOLDE", ansi::bold);
        sep();
        int from = std::max(0, (int)hist.size() - 10);
        for (int i = from; i < (int)hist.size(); ++i) {
            const auto& tx = hist[i];
            const std::string& clr = tx.type == "RETRAIT" ? ansi::red : ansi::grn;
            row("  " + fit(tx.date, 21) + fit(tx.type, 9) +
                       fit(fmtNum(tx.montant) + " Ar", 12) +
                       fmtNum(tx.solde) + " Ar", clr);
        }
    }
    sep();
    row("  " + txt("[ Entree pour revenir ]", "[ Entree hanohy ]"), ansi::yel);
    bot();
    waitEnter();
}

//main 

int main() {
    enableAnsi();

    std::cout << "Choisir la langue / Safidy fiteny\n1. Francais\n2. Malagasy\n> ";
    int choixLangue;
    if (!(std::cin >> choixLangue)) { std::cerr << "Erreur.\n"; return 1; }
    langueActuelle = (choixLangue == 2) ? 2 : 1;

    try {
        // Authentification carte + PIN
        if (!authentifier()) {
            cls(); top();
            row("");
            cRow(txt("Carte bloquee : trop d'essais.", "Voasakana ny karatra : be loatra ny ezaka."),
                 ansi::red + ansi::bold);
            row("");
            bot();
            return 1;
        }
        showWelcome("Aro");

        // Comptes de l'utilisateur
        std::vector<std::unique_ptr<CompteBancaire>> comptes;
        comptes.push_back(std::make_unique<CompteEpargne>("Aro", 50000.0, 3.5, 10000.0, 20000.0));
        comptes.push_back(std::make_unique<CompteVue>("Aro Courant", 80000.0, 20000.0));

        int idx = selectCompte(comptes);
        CompteBancaire* c = comptes[idx].get();

        Status st;
        int choix;
        bool actif = true;

        while (actif) {
            showMenu(*c, st);
            st = {};

            if (!(std::cin >> choix)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                st = {txt("Entree invalide.", "Tsy mety ny fidirana."), true};
                continue;
            }

            switch (choix) {
                case 1: showInfo(*c); break;
                case 2: {
                    auto res = c->deposer(readAmount(txt("Montant (Ariary) : ", "Vola (Ariary) : ")));
                    st = {res.msg, !res.ok};
                    break;
                }
                case 3: {
                    auto res = c->retirer(readAmount(txt("Montant (Ariary) : ", "Vola (Ariary) : ")));
                    st = {res.msg, !res.ok};
                    break;
                }
                case 4: {
                    auto res = c->appliquerInterets();
                    st = {res.msg, !res.ok};
                    break;
                }
                case 5: printHistory(*c); break;
                case 6:
                    idx = selectCompte(comptes);
                    c   = comptes[idx].get();
                    break;
                case 7: actif = false; break;
                default:
                    st = {txt("Choix hors plage.", "Safidy ivelan'ny faritra."), true};
            }
        }

        cls(); top();
        row("");
        cRow(txt("Merci d'avoir utilise notre ATM !", "Misaotra nampiasa ny ATM anay !"), ansi::grn + ansi::bold);
        cRow(txt("Au revoir !", "Veloma !"), ansi::grn);
        row("");
        bot();
        std::cout << "\n";

    } catch (const std::exception& e) {
        std::cerr << ansi::red << "Erreur : " << e.what() << ansi::rst << "\n";
        return 1;
    }

    return 0;
}