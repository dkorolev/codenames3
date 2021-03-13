#include "../current/bricks/dflags/dflags.h"
#include "../current/blocks/http/api.h"
#include "../current/blocks/html/html.h"
#include "../current/blocks/html/html_http.h"

DEFINE_uint16(port, 3000, "The local port to listen on.");
DEFINE_string(base_url, "http://localhost:3000/", "The base URL for the game.");

int main(int argc, char** argv) {
  ParseDFlags(&argc, &argv);

  auto& http_server = HTTP(current::net::BarePort(FLAGS_port));

  auto const scope = http_server.Register("/", [](Request r) {
    auto const html_scope = current::html::HTMLGeneratorHTTPResponseScope(std::move(r));
    HTML(html);
    {
      HTML(head);
      HTML(title);
      HTML(_) << "Codenames3";
    }
    HTML(body);
    auto const& params = r.url.query;
    if (params.has("p")) {
      {
        HTML(h1);
        HTML(_) << params.get("p", "Player");
        }
      {
        HTML(table);
        for (size_t i = 0; i < 5; ++i) {
          HTML(tr, height("100px"));
          for (size_t j = 0; j < 5; ++j) {
            HTML(td, width("100px").bgcolor(rand() % 4 ? "gray" : "green"));
          }
        }
      }
    } else {
      {
        HTML(h1);
        HTML(_) << "Boards";
      }
      {
        for (int i = 1; i <= 3; ++i) {
          HTML(h3);
          HTML(a, href(FLAGS_base_url + "?p=" + current::ToString(i) + "&seed=" + params.get("seed", "16081983")));
          HTML(_) << params.get("p" + current::ToString(i), "Player " + current::ToString(i));
        }
      }
    }
  });

  std::cout << "Listening on localhost:" << http_server.LocalPort() << std::endl;
  http_server.Join();
}
