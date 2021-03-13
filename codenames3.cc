#include "../current/bricks/dflags/dflags.h"
#include "../current/blocks/http/api.h"
#include "../current/blocks/html/html.h"
#include "../current/blocks/html/html_http.h"

DEFINE_uint16(port, 3000, "The local port to listen on.");
DEFINE_string(base_url, "http://localhost:3000/", "The base URL for the game.");

inline std::string _ = "grey";
inline std::string B = "black";
inline std::string G = "green";

struct Coloring final {
  std::string p[3];
  int c;
};

inline Coloring colorings[] = {
  { { _, _, _ }, 19 },
  { { G, _, _ }, 1 },
  { { _, G, _ }, 1 },
  { { _, _, G }, 1 },
  { { B, G, G }, 1 },
  { { G, B, G }, 1 },
  { { G, G, B }, 1 },
};

struct GameCreator final {
  std::vector<size_t> const indexes;
  GameCreator() : indexes(GenIndexes()) {}
  static std::vector<size_t> GenIndexes() {
    size_t const N = sizeof(colorings) / sizeof(colorings[0]);
    std::map<std::multiset<std::string>, int> check;
    std::vector<size_t> indexes;
    for (size_t t = 0u; t < N; ++t) {
      auto const& e = colorings[t];
      if (!(e.c >= 0 && e.c <= 25)) {
        std::cerr << "Wrong individual counter!\n";
        std::exit(-1);
      }
      int& v = check[std::multiset<std::string>(e.p, e.p + 3)];
      if (!v) {
        v = e.c;
      } else if (v != e.c) {
        std::cerr << "Imbalance detected!\n";
        std::exit(-1);
      }
      for (int i = 0; i < e.c; ++i) {
        indexes.push_back(t);
      }
    }
    if (indexes.size() != 25u) {
      std::cerr << "The total is not 25!\n";
      std::exit(-1);
    }
    return indexes;
  }
};

struct Game final {
  std::vector<std::string> player_names;
  std::vector<size_t> permutation;
  void GeneratePermutation(std::string const&) {
    permutation = current::Singleton<GameCreator>().indexes;
    std::shuffle(std::begin(permutation), std::end(permutation), current::random::mt19937_64_tls());
  }
};

int main(int argc, char** argv) {
  ParseDFlags(&argc, &argv);

  current::Singleton<GameCreator>();

  auto& http_server = HTTP(current::net::BarePort(FLAGS_port));

  std::map<std::string, Game> games;

  auto const scope = http_server.Register("/", [&games](Request r) {
    auto const html_scope = current::html::HTMLGeneratorHTTPResponseScope(std::move(r));
    HTML(html);
    {
      HTML(head);
      HTML(title);
      HTML(_) << "Codenames3";
    }
    HTML(body);
    auto const& params = r.url.query;
    if (params.has("game")) {
      if (params.has("p") && games.find(params["game"]) != games.end()) {
        int p = current::FromString<int>(params["p"]);
        p = ((p % 3) + 2) % 3;
        Game const& game = games.at(params["game"]);
        HTML(br);
        HTML(br);
        {
          HTML(h1);
          HTML(_) << "<p align=center>" + game.player_names[p] + "</p>";
        }
        HTML(br);
        {
          HTML(table, align("center"));
          for (size_t i = 0; i < 5; ++i) {
            HTML(tr, height("100px"));
            for (size_t j = 0; j < 5; ++j) {
              HTML(td, width("100px").bgcolor(colorings[game.permutation[i * 5 + j]].p[p]));
            }
          }
        }
      } else {
        std::string const game_id = params.get("game", "");
        Game& game = games[game_id];
        if (game.permutation.empty()) {
          game.GeneratePermutation(game_id);
        }
        game.player_names = {
          params.get("p1", "Player One"),
          params.get("p2", "Player Two"),
          params.get("p3", "Player Three")
        };
        {
          HTML(h1);
          HTML(_) << "Boards";
        }
        {
          for (int i = 1; i <= 3; ++i) {
            HTML(h3);
            HTML(a, href(FLAGS_base_url + "?p=" + current::ToString(i) + "&game=" + game_id));
            HTML(_) << params.get("p" + current::ToString(i), "Player " + current::ToString(i));
          }
        }
      }
    } else {
      HTML(_) << "Need `?game=...`.\n";
    }
  });

  std::cout << "Listening on localhost:" << http_server.LocalPort() << std::endl;
  http_server.Join();
}
