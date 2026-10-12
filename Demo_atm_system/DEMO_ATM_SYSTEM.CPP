/*
 * =============================================
 *   SYSTEME ATM - Programmation Orientee Objet
 *   Langage : C++
 *   Niveau  : Debutant
 * =============================================
 *
 *  Concepts POO couverts :
 *   1. Encapsulation  (attributs prives + getters/setters)
 *   2. Constructeur   (initialisation d'un objet)
 *   3. Destructeur    (liberation de l'objet)
 *   4. Heritage       (CompteEpargne herite de CompteBancaire)
 *   5. Polymorphisme  (methode virtuelle afficherType())
 */

#include <iostream>
#include <string>
using namespace std;

// ================================================================
// CLASSE DE BASE : CompteBancaire
// ================================================================
// Represente un compte bancaire generique.
// On utilise "private" pour proteger les donnees sensibles.

class CompteBancaire {

    // -- Attributs prives (Encapsulation) --
    // Ces donnees ne peuvent pas etre modifiees directement de l'exterieur.
private:
    string titulaire;   // Nom du proprietaire du compte
    double solde;       // Solde actuel du compte

    // -- Methodes publiques --
    // Ce sont les seules facon d'interagir avec le compte.
public:

    // --- CONSTRUCTEUR ---
    // Appele automatiquement quand on cree un objet.
    // Il initialise le nom et le solde de depart.
    CompteBancaire(string nom, double soldeInitial) {
        titulaire = nom;
        solde     = soldeInitial;
        cout << "[INFO] Compte cree pour : " << titulaire << endl;
    }

    // --- DESTRUCTEUR ---
    // Appele automatiquement quand l'objet est detruit (fin du programme).
    virtual ~CompteBancaire() {
        cout << "[INFO] Compte de " << titulaire << " ferme." << endl;
    }

    // --- GETTER : lire le solde (lecture seule) ---
    double getSolde() {
        return solde;
    }

    // --- GETTER : lire le nom du titulaire ---
    string getTitulaire() {
        return titulaire;
    }

    // --- METHODE : Deposer de l'argent ---
    void deposer(double montant) {
        if (montant <= 0) {
            cout << "[ERREUR] Le montant doit etre positif." << endl;
            return;
        }
        solde += montant;
        cout << "[OK] Depot de " << montant << " DZD effectue." << endl;
    }

    // --- METHODE : Retirer de l'argent ---
    void retirer(double montant) {
        if (montant <= 0) {
            cout << "[ERREUR] Le montant doit etre positif." << endl;
            return;
        }
        if (montant > solde) {
            cout << "[ERREUR] Solde insuffisant ! Solde actuel : " << solde << " DZD" << endl;
            return;
        }
        solde -= montant;
        cout << "[OK] Retrait de " << montant << " DZD effectue." << endl;
    }

    // --- METHODE VIRTUELLE : Afficher le type de compte ---
    // "virtual" permet au polymorphisme de fonctionner.
    // Chaque classe enfant peut redefinir cette methode.
    virtual void afficherType() {
        cout << "Type : Compte Bancaire Standard" << endl;
    }

    // --- METHODE : Afficher les informations du compte ---
    void afficherInfo() {
        cout << "================================" << endl;
        cout << "  Titulaire : " << titulaire      << endl;
        cout << "  Solde     : " << solde << " DZD" << endl;
        afficherType(); // Polymorphisme en action !
        cout << "================================" << endl;
    }
};


// ================================================================
// CLASSE ENFANT : CompteEpargne  (HERITAGE)
// ================================================================
// CompteEpargne herite de CompteBancaire.
// Elle ajoute un taux d'interet en plus.

class CompteEpargne : public CompteBancaire {

    // Attribut supplementaire propre au compte epargne
private:
    double tauxInteret; // Taux en pourcentage (ex: 3.5 pour 3.5%)

public:

    // Constructeur de la classe enfant
    // On appelle d'abord le constructeur du parent avec : CompteBancaire(nom, solde)
    CompteEpargne(string nom, double soldeInitial, double taux)
        : CompteBancaire(nom, soldeInitial) {
        tauxInteret = taux;
        cout << "[INFO] Taux d'interet : " << tauxInteret << "%" << endl;
    }

    // --- METHODE : Appliquer les interets au solde ---
    void appliquerInterets() {
        double interets = getSolde() * (tauxInteret / 100.0);
        deposer(interets);
        cout << "[OK] Interets de " << interets << " DZD ajoutes." << endl;
    }

    // --- POLYMORPHISME : On redefinit afficherType() ---
    // Cette version remplace celle du parent pour ce type de compte.
    void afficherType() override {
        cout << "Type : Compte Epargne (Taux: " << tauxInteret << "%)" << endl;
    }
};


// ================================================================
// PROGRAMME PRINCIPAL
// ================================================================
int main() {

    cout << "============================================" << endl;
    cout << "   BIENVENUE AU SYSTEME ATM - POO C++      " << endl;
    cout << "============================================" << endl;

    // Creation d'un objet CompteEpargne
    // Le constructeur est appele ici automatiquement.
    CompteEpargne monCompte("Ahmed Benali", 50000.0, 3.5);

    cout << endl;

    int choix;
    bool continuer = true;

    // Boucle principale du menu ATM
    while (continuer) {

        cout << "\n-------- MENU ATM --------" << endl;
        cout << " 1. Voir mes informations  " << endl;
        cout << " 2. Deposer de l'argent    " << endl;
        cout << " 3. Retirer de l'argent    " << endl;
        cout << " 4. Appliquer les interets " << endl;
        cout << " 5. Quitter                " << endl;
        cout << "--------------------------" << endl;
        cout << "Votre choix : ";
        cin >> choix;

        switch (choix) {

            case 1:
                // Affiche les infos + le type (polymorphisme)
                monCompte.afficherInfo();
                break;

            case 2: {
                double montant;
                cout << "Montant a deposer (DZD) : ";
                cin >> montant;
                monCompte.deposer(montant);
                break;
            }

            case 3: {
                double montant;
                cout << "Montant a retirer (DZD) : ";
                cin >> montant;
                monCompte.retirer(montant);
                break;
            }

            case 4:
                // Methode specifique a CompteEpargne (heritage)
                monCompte.appliquerInterets();
                break;

            case 5:
                continuer = false;
                cout << "\nMerci d'avoir utilise notre ATM. Au revoir !" << endl;
                break;

            default:
                cout << "[ERREUR] Choix invalide. Entrez un nombre entre 1 et 5." << endl;
        }
    }

    // Le destructeur est appele automatiquement ici (fin du main)
    return 0;
}