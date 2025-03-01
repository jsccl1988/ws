#include <wspp/server/session_handler.hpp>
#include <wspp/util/crypto.hpp>

using namespace std;

namespace wspp {
namespace server {
using namespace util;
string SessionHandler::generateSID() {
    string code = encodeBase64(randomBytes(32));
    std::replace_if(code.begin(), code.end(), [&](char c) {
        return !std::isalnum(c);
    }, '-');
    return code;
}
} // namespace server
} // namespace wspp
