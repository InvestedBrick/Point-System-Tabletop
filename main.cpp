/*
    Point System for Warhammer Fantasy Battle 6. Edition 
    Copyright (C) 2024  Julian Brecht

    reachable at julianbrecht25@gmail.com
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <ctime>

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
        std::cout << "Delta: " << delta << ", "<< output <<" Turnierpunkte an " << (output == TIE ? "beide" : p1_is_winner ?  p1->name : p2->name) <<std::endl;

    }

    void calc_delta() {
        delta = std::abs(p1->new_points - p2->new_points);

        p1_is_winner = p1->new_points > p2->new_points;
        calc_turnier_points();
    }
};

bool cmp(const std::shared_ptr<Player>& a, const std::shared_ptr<Player>& b) { 
    if(a->p_turnier == b->p_turnier){
        return a->p_spieler > b->p_spieler;
    }
    return a->p_turnier > b->p_turnier;
}



class Tournament {
private:
    std::vector<std::shared_ptr<Player>> players{};
    std::vector<std::unique_ptr<Table>> tables{};
    int points_per_page = 0;
    std::unordered_map<std::string, std::shared_ptr<Player>> player_map{};
    std::stringstream player_scores{};
    int round = 0;
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
            auto p = std::make_shared<Player>();
            std::string name;
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
            players.push_back(p);
            player_map[name] = p;
        }
        
    }
    

    void init_tables() {
        while(true){
        std::string points;
        std::cout << "Wie viele Punkte pro Seite?" << std::endl;
        std::cout << "(1) 1 - 1999\n(2) 2000 - 2999\n(3) 3000 - 3999\n" << std::endl;
            std::cout << "Auswahl: ";
            try{
                std::string str;
                std::getline(std::cin, str);
                int num = std::stoi(str);
                if(num == 1){points_per_page = 1000;break;}
                else if(num == 2){points_per_page = 2000;break;}
                else if(num == 3){points_per_page = 3000;break;}
                else{
                    throw std::invalid_argument("[FEHLER!] Eingabe ausserhalb erlaubter Werte");
                }
            }
            catch(std::exception& e){
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
                std::cout << "\n";
                std::cout << "Fuer welchen Tisch sollen die Ergebnisse eingetragen werden?: ";
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
        std::cout << "Gibt es Herausforderungen(j/n): ";
        std::string answer;
        std::cin >> answer;
        if(answer == "j"){
            
            while(true){
                try{
                    std::cout << "Wie viele Herausforderungen soll es geben?: ";
                    std::cin >> answer;
                    is_numeric(answer);
                    int i = std::stoi(answer);
                    if(i < 0){
                        throw std::invalid_argument("[FEHLER!] Eingabe negativ");
                    }
                    if(i > static_cast<int>(players.size() / 2)){
                        throw std::invalid_argument("[FEHLER!] Eingabe zu gross");
                    }
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    for(int j = 0; j < i; j++){
                        std::string name;
                        std::string name2;
                        std::cout << "Herausforderung Nr. " << j + 1 << ": " << std::endl;
                        std::cout << "Name des Herausforderers: ";
                        std::getline(std::cin, name);
                        if(name.empty()){
                            throw std::invalid_argument("[FEHLER!] Name leer");
                        }
                        if(player_map.find(name) == player_map.end()){
                            throw std::invalid_argument("[FEHLER!] Name nicht gefunden");
                        }

                        std::cout << "Name des Herausgeforderten: ";
                        std::getline(std::cin, name2);
                        if(name2.empty()){
                            throw std::invalid_argument("[FEHLER!] Name leer");
                        }
                        if(player_map.find(name2) == player_map.end()){
                            throw std::invalid_argument("[FEHLER!] Name nicht gefunden");
                        }
                        
                        players.erase(std::find(players.begin(), players.end(), player_map[name]));
                        players.erase(std::find(players.begin(), players.end(), player_map[name2]));
                        
                        
                        players.push_back(player_map[name]);
                        players.push_back(player_map[name2]);
                        
                    }    
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

        }
        std::cout << "\n";
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
        this->player_scores.str("");
        if(!this->player_scores.str().empty()){
            std::cout << "[FEHLER!] player scores nicht leer wtf" << std::endl;
        }
        std::cout << "\n----------- Plazierungen der Runde "<< ++round << " -----------\n" << std::endl;
        for (size_t i = 0; i < players.size(); i++) {
            this->player_scores << "Platz " << i + 1 << ": " << players[i]->name << " mit " << players[i]->p_turnier << " Turnierpunkten und " << players[i]->p_spieler << " Spielerpunkten" << std::endl;
        }
        std::cout << this->player_scores.str();

    }


    void sort_players() {
        std::sort(players.begin(), players.end(), cmp);
    }

    void undo_tables() {
        for (auto& table : tables) {
            table->done = false;
        }
    }
    void play_round() {
        undo_tables();
        for (size_t i = 0; i < tables.size(); i++) {
            //IMPORTANT: CHANGE THE CHECK TO IF THE PLAYERS HAVE PLAYED AGAINST EACH OTHER BEFORE IN GENERAL, NOT JUST ON THIS TABLE
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
                    std::cout << "[ACHTUNG!] Kein Tausch von Spielern moeglich" << std::endl;
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

    void export_to_txt(){
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
        std::stringstream filename;
        filename << "Tournament-Ergebnis-vom-" << now->tm_mday << "." << (now->tm_mon + 1) << "." << (now->tm_year + 1900) << "-_-" << now->tm_hour << "-" << now->tm_min;
        try{
           std::cout << filename.str();
           std::ofstream file(filename.str() + ".txt");

           file << this->player_scores.str();

           file.close();
        }catch(std::exception& e){
            std::cout << "[FEHLER!] Datei konnte nicht richtig beschrieben werden" << std::endl;
        }
    }
};

int main() {
    std::cout << "Point System for Warhammer Fantasy Battle 6. Edition\n         Copyright (C) 2024  Julian Brecht\n\n" << std::endl;
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
    
    while(true){
        std::string answer;

        std::cout << "Ergebnis speichern(j/n)?: ";
        std::cin >> answer;

        if(answer == "j"){
            tournament.export_to_txt();
            break;
        }
        if(answer == "n"){
            std::cout << "Sind Sie sich sicher, die Ergebnisse NICHT zu speichern(j/n)?:";
            std::cin >> answer;

            if(answer != "j"){
                continue;
            }
            break;
        }else{
            std::cout << "[FEHLER!] Invalide Eingabe" << std::endl;
            continue;
        }



    }
    
    return 0;
}
