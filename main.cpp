#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <memory>

#define TIE 1
#define CLOSE_WIN 2
#define WIN 3
#define MASSACRE 5
#define MAX_POINTS 10000
#define INVALID_TURNIER_POINTS -1
auto rng = std::default_random_engine{ std::random_device{}() };

struct Player {
    std::string name;
    int p_spieler;
    int p_turnier;
    int new_points;
};

struct Range{
    int low1,high1;
    int low2,high2;
    int output;
};

void is_numeric(std::string& s) {
    for (size_t i = 0; i < s.size(); i++) {
        if (!(s[i] >= '0' && s[i] <= '9')) {
            throw std::invalid_argument("[FEHLER!] Ungueltige Eingabe");
        }
    }
}

class Table {
private:
    Player* p1 = nullptr;
    Player* p2 = nullptr;
    int delta = 0;
    bool p1_is_winner;

    const std::vector<Range> ranges = {
        {0,149,1,1999,TIE},
        {150,299,1,1999,CLOSE_WIN},
        {300,599,1,1999,WIN},
        {600,MAX_POINTS,1,1999,MASSACRE},
        {0,299,2000,2999,TIE},
        {300,599,2000,2999,CLOSE_WIN},
        {600,1199,2000,2999,WIN},
        {1200,MAX_POINTS,2000,2999,MASSACRE},
        {0,499,3000,3999,TIE},
        {500,899,3000,3999,CLOSE_WIN},
        {900,1799,3000,3999,WIN},
        {900,1799,3000,3999,WIN},
        {1800,MAX_POINTS,3000,3999,MASSACRE}
        //TO BE EXPANDED





    };

    int get_turnier_points(int delta) {
        if (delta < 0 || delta > MAX_POINTS){return INVALID_TURNIER_POINTS;}
        for (const auto& r : ranges) {
            if ((delta >= r.low1 && delta <= r.high1) && (this->points_per_page >= r.low2 && this->points_per_page <= r.high2)){
                return r.output;
            }
                
        }
        return INVALID_TURNIER_POINTS;
    }
    
public:
    bool done = false;
    int points_per_page = 0;
    Table(int points_per_page) {
        this->points_per_page = points_per_page;
    }

    void set_p1(Player* p) { p1 = p; }
    void set_p2(Player* p) { p2 = p; }

    Player* get_p1() { return p1; }
    Player* get_p2() { return p2; }

    Player* get_p_idx(int idx) {
        if (idx == 0) return p1;
        if (idx == 1) return p2;
        return nullptr;
    }

    void calc_turnier_points() {
        int output = get_turnier_points(delta);
        std::cout << "Delta: " << delta << ", Turnierpunkte: " << output << std::endl;
        if(output == INVALID_TURNIER_POINTS){
            std::cout << "[FEHLER!] Ungueltige Eingabe!" << std::endl;
            return; //Sollte nicht passieren
        }

        if(output == TIE){
            p1->p_turnier += TIE;
            p2->p_turnier += TIE;
        }else if(output == CLOSE_WIN || output == WIN || output == MASSACRE){
            if(p1_is_winner){
                p1->p_turnier += output;
            }else{
                p2->p_turnier += output;
            }
        }

    }

    void calc_delta() {
        delta = std::abs(p1->new_points - p2->new_points);

        p1_is_winner = p1->new_points > p2->new_points;
        calc_turnier_points();
    }
};

bool cmp(const std::unique_ptr<Player>& a, const std::unique_ptr<Player>& b) { 
    if(a->p_turnier == b->p_turnier){
        return a->p_spieler > b->p_spieler;
    }
    return a->p_turnier > b->p_turnier;
 }



class Tournament {
private:
    std::vector<std::unique_ptr<Player>> players;
    std::vector<std::unique_ptr<Table>> tables;
    int points_per_page = 0;

public:
    Tournament() = default;

    void init_players() {
        int n_players = 0;
        std::string n_players_str;
        while (true) {
            try{
                std::cout << "Wie viele Spieler nehmen Teil?: ";
                std::getline(std::cin, n_players_str);
                is_numeric(n_players_str);
                n_players = std::stoi(n_players_str);
            }catch (std::exception& e) {
                if (std::string(e.what()) == "stoi") {
                    std::cout << "[FEHLER!] Ungueltige Eingabe!" << std::endl;
                    continue;
                }
                std::cout << e.what() << std::endl;
                continue;
            }
            if (n_players % 2 != 0) {
                std::cout << "[FEHLER!] Spieleranzahl muss gerade sein!" << std::endl;
            }
            else { break; }
        }
        for (int i = 0; i < n_players; i++) {
            auto p = std::make_unique<Player>();
            std::string name;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            while (true) {
                std::cout << "Spielername von Spieler " << i + 1 << ": ";
                std::getline(std::cin, name);
                if (name.empty()) {
                    std::cout << "[FEHLER!] Spielername darf nicht leer sein!" << std::endl;
                }
                else {
                    break;
                }
            }
            p->name = name;
            p->p_turnier = 0;
            p->p_spieler = 0;
            players.push_back(std::move(p));
        }
        // Herausforderung hinzufuegen
    }
    

    void init_tables() {
        while(true){
            std::string points;
            std::cout << "Anzahl der Punkte pro Seite: ";

            try{
                std::getline(std::cin, points);
                is_numeric(points);
                points_per_page = std::stoi(points);
                break;
            }catch(std::exception& e){
                if (std::string(e.what()) == "stoi") {
                    std::cout << "[FEHLER!] Ungueltige Eingabe!" << std::endl;
                    continue;
                }
                std::cout << e.what() << std::endl;
                continue;

            }
        }
        for (size_t i = 0; i < players.size() / 2; i++) {
            auto t = std::make_unique<Table>(points_per_page);
            tables.push_back(std::move(t));
        }
    }

    void eval_tables() {
        std::string tbl;
        while (true) {
            try {
                std::cout << "Fuer welchen Tische sollen die Ergebnisse eingetragen werden?: ";
                std::getline(std::cin, tbl);
                is_numeric(tbl);
                int i = std::stoi(tbl);
                if (i > static_cast<int>(players.size() / 2) || i < 1) {
                    throw std::invalid_argument("[FEHLER!] Eingabe ausserhalb der Tischanzahl");
                }
                if (tables[i - 1]->done) {
                    throw std::invalid_argument("[FEHLER!] Tisch bereits fertig");
                }
                for (int j = 0; j < 2; j++) {
                    std::string points;
                    std::cout << "Wie viele Punkte hat " << tables[i - 1]->get_p_idx(j)->name << " erspielt?: ";
                    std::getline(std::cin, points);

                    is_numeric(points);

                    if (std::stoi(points) < 0) {
                        throw std::invalid_argument("[FEHLER!] Eingabe negativ");
                    }
                    tables[i - 1]->get_p_idx(j)->new_points = std::stoi(points);
                    tables[i - 1]->get_p_idx(j)->p_spieler += std::stoi(points);
                }

                tables[i - 1]->calc_delta();
                tables[i - 1]->done = true;

                if (std::all_of(tables.begin(), tables.end(), [](const std::unique_ptr<Table>& t) { return t->done; })) {
                    break;
                }
            }
            catch (const std::exception& e) {
                if (std::string(e.what()) == "stoi") {
                    std::cout << "[FEHLER!] Ungueltige Eingabe!" << std::endl;
                    continue;
                }
                std::cout << e.what() << std::endl;
                continue;
            }
        }
    }

    void play_first_round() {
        undo_tables();
        std::shuffle(players.begin(), players.end(), rng);

        for (size_t i = 0; i < tables.size(); i++) {
            tables[i]->set_p1(players[i * 2].get());
            tables[i]->set_p2(players[i * 2 + 1].get());
        }
        int cntr = 1;
        for (const auto& t : tables) {
            std::cout << "Am Tisch " << cntr++ << " spielen " << t->get_p1()->name << " und " << t->get_p2()->name << std::endl;
        }

        std::cout << "\n----------- Bitte Ergebnisse der Runde eintragen -----------\n" << std::endl;
        eval_tables();
    }

    void print_players() {
        std::cout << "\n----------- Spielerliste -----------\n" << std::endl;
        for (size_t i = 0; i < players.size(); i++) {
            std::cout << "Platz " << i + 1 << ": " << players[i]->name << " mit " << players[i]->p_turnier << " Turnierpunkten und " << players[i]->p_spieler << " Spielerpunkten" << std::endl;
        }
    }

    void sort_players() {
        std::sort(players.begin(), players.end(), cmp);
    }

    void undo_tables() {
        for (auto& table : tables) {
            table->done = false;
        }
    }
    /*
    a t1  -> a -> a 1.
    b t1  -> b -> d 3.
    c t2  -> d -> b 2.
    d t2  -> c -> c 4.
    */
    void play_round() {
        undo_tables();
        
        for (size_t i = 0; i < tables.size(); i++) {
            //Check if both players have already played on this table the round before
            if((players[i * 2].get() == tables[i]->get_p1() && players[i * 2 + 1].get() == tables[i]->get_p2()) || (players[i * 2].get() == tables[i]->get_p2() && players[i * 2 + 1].get() == tables[i]->get_p1())) {
                std::cout << "[ACHTUNG!] Spieler " << players[i * 2].get()->name << " und " << players[i * 2 + 1].get()->name << " haben bereits gegeneinander gespielt" << std::endl;
                //if so, test to see if a swap is possible and swap two indices after the current player[i*2]
                if(i * 2 + 2 < players.size()){
                    std::cout << "[ACHTUNG!] Spieler "<< players[i * 2 + 1].get()->name << " und " << players[i * 2 + 2].get()->name << " werden getauscht" << std::endl;
                    std::iter_swap(players.begin() + (i * 2 + 1), players.begin() + (i * 2 + 2));
                    tables[i]->set_p2(players[i * 2 + 1].get());
                    continue;
                }else{
                    std::cout << "[ACHTUNG!] Kein Tausch von Spielern möeglich" << std::endl;
                }
            }
            tables[i]->set_p1(players[i * 2].get());
            tables[i]->set_p2(players[i * 2 + 1].get());
        }
        int cntr = 1;
        std::cout << "\n----------- Tische -----------\n" << std::endl;
        for (const auto& t : tables) {
            std::cout << "Am Tisch " << cntr++ << " spielen " << t->get_p1()->name << " und " << t->get_p2()->name << std::endl;
        }

        std::cout << "\n----------- Bitte Ergebnisse der Runde eintragen -----------\n" << std::endl;
        eval_tables();
    }
};

int main() {
    
    std::cout << "Willkommen zum Turnier-Schema!\n" << std::endl;
    Tournament tournament;
    tournament.init_players();
    tournament.init_tables();
    std::cout << "\n----------- Runde 1 -----------\n" << std::endl;
    tournament.play_first_round();
    tournament.sort_players();
    tournament.print_players();
    system("pause");
    std::cout << "\n----------- Runde 2 -----------\n" << std::endl;
    tournament.play_round();
    tournament.sort_players();
    tournament.print_players();
    system("pause");
    std::cout << "\n----------- Runde 3 -----------\n" << std::endl;
    tournament.play_round();
    tournament.sort_players();
    tournament.print_players();
    system("pause");
    return 0;
}
