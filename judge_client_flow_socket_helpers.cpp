#include "judge_client_flow_socket_helpers.h"

#include "header/static_var.h"
#include "library/judge_lib.h"

void init_websocket_and_bundle(const std::string &judger_id) {
    initWebSocketConnection("localhost", 5100);
    std::string mutable_id = judger_id;
    bundle.setJudgerId(mutable_id);
}
