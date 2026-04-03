#include <kineflow/scene.hpp>
#include <iostream>

// ── shared state ──────────────────────────────────────────────────────────
struct State {
    int playerHp  = 100;
    int playerDmg = 15;
    int enemyHp   = 60;
    int enemyDmg  = 8;
    int turn      = 1;
};

inline static State gState{};

// ── main (all-in-one) ────────────────────────────────────────────────────
struct Main : public kine::Scene {

    // ── tags ──────────────────────────────────────────────────────────────
    struct Combat       {};
    struct PlayerAction {};
    struct EnemyTurn    { bool attacked = false; };
    struct GameOver     { bool victory  = false; };

    // ── entry point ───────────────────────────────────────────────────────
    Scene* work() {
        gState = State{};
        return enter<Main>(std::ref(gState), Combat{});
    }

    // ── combat ────────────────────────────────────────────────────────────
    Scene* work(State& s, Combat) {
        std::cout << "\n=== TURN " << s.turn++ << " ===\n";
        std::cout << "  Player: " << s.playerHp << " HP\n";
        std::cout << "  Enemy:  " << s.enemyHp  << " HP\n";

        if (s.playerHp <= 0) return enter<Main>(std::ref(s), GameOver{false});
        if (s.enemyHp  <= 0) return enter<Main>(std::ref(s), GameOver{true});

        return enter<Main>(std::ref(s), PlayerAction{});
    }

    // ── player action ─────────────────────────────────────────────────────
    Scene* work(State& s, PlayerAction) {
        int choice = 0;
        std::cout << "\n  [1] Atacar\n"
                     "  [2] Curar (+20 HP)\n"
                     "  [3] Fugir\n  > ";
        std::cin >> choice;

        if (choice == 1) return enter<Main>(std::ref(s), EnemyTurn{true});
        if (choice == 2) return enter<Main>(std::ref(s), EnemyTurn{false});
        return enter<Main>(std::ref(s), GameOver{false});
    }

    // ── enemy turn ────────────────────────────────────────────────────────
    Scene* work(State& s, EnemyTurn t) {
        int dealt = 0;

        if (t.attacked) {
            dealt = s.playerDmg + (rand() % 10) - 5;
            s.enemyHp -= dealt;
            std::cout << "\n  Player ataca! (-" << dealt << " HP)\n";
        } else {
            s.playerHp += 20;
            std::cout << "\n  Player cura! (+20 HP)\n";
        }

        dealt = s.enemyDmg + (rand() % 6) - 3;
        s.playerHp -= dealt;
        std::cout << "  Enemy ataca!  (-" << dealt << " HP)\n";

        return enter<Main>(std::ref(s), Combat{});
    }

    // ── game over ─────────────────────────────────────────────────────────
    Scene* work(State& s, GameOver go) {
        std::cout << (go.victory ? "\n*** VITORIA! ***\n"
                                 : "\n*** DERROTA ***\n");
        std::cout << "  Jogar de novo? [1=sim]  > ";
        char again; std::cin >> again;

        if (again == 1) {
            s = State{};
            return enter<Main>(std::ref(s), Combat{});
        }
        return nullptr;
    }

    void drop() {
        std::cout << "[State] hp=" << gState.playerHp << "\n";
    }
};