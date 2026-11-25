// cupoj-compare: standalone wrapper around the legacy judger's text checker (external/compare),
// so judge v2 keeps exactly the same AC / PE / WA semantics and diff report format.
// usage: cupoj-compare <answer_file> <user_output_file>   (run inside a dir that has data.in and user.out)
// prints the verdict code (4 AC, 5 PE, 6 WA, 10 RE) to stdout; WA/PE details are appended to ./diff.out
#include <cstdio>
#include <cstdlib>
#include "../../external/compare/CompareImpl.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <answer> <user_output> [full_diff=1]\n", argv[0]);
        return 2;
    }
    CompareImpl c;
    c.setDebug(0);
    c.setFullDiff(argc > 3 ? atoi(argv[3]) : 1);
    int r = c.compare(argv[1], argv[2]);
    printf("%d\n", r);
    return 0;
}
