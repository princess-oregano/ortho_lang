#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "../front_end/lexer.h"
#include "tree_dump.h"
#include "system.h"
#include "../log.h"

// Stream of dump output.
static FILE *DMP_STREAM = nullptr;
static char DOT_FILENAME[FILENAME_MAX] = {};
static char PNG_FILENAME[FILENAME_MAX] = {};

// Opens a graphviz file to write dump to.
static void
open_graph_dump()
{
        DIR* dir = opendir("dmp");
        if (dir) {
                closedir(dir);
        } else if (ENOENT == errno) {
                mkdir("dmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        } else {
                fprintf(stderr, "Couldn't make a directory for dumps.\n");
        }

        char filename[] = "dmp/DUMP_XXXXXX";

        DMP_STREAM = fdopen(mkstemp(filename), "w");
        setvbuf(DMP_STREAM, nullptr, _IONBF, 0);

        memcpy(DOT_FILENAME, filename, strlen(filename) + 1);
}

// Genetares .png image from given dot code.
static char *
generate_graph()
{
        memcpy(PNG_FILENAME, DOT_FILENAME, strlen(DOT_FILENAME));
        strcat(PNG_FILENAME, ".png");

        system_wformat("%s %s %s %s", "dot -Tpng",
                        DOT_FILENAME, ">", PNG_FILENAME);

        return PNG_FILENAME;
}

#define DEF_OP(NAME, SIGN) case OP_##NAME: \
                                fprintf(DMP_STREAM, \
                                        "node%d [label = \"%d\\n%s\", shape = rect]\n", \
                                        node_count, curr, SIGN); \
                                break;
static void
op_node_graph_dump(tree_t *tree, int curr, int node_count)
{
        switch (tree->nodes[curr].data.val.op) {
                #include "../operations.inc"
                default:
                        log("Invalid type encountered: %d.\n", 
                                        tree->nodes[curr].data.val.op);
                        assert(0 && "Invalid operation type encountered.");
                        break;
        }
}
#undef DEF_OP

#define DEF_KW(NAME, SIGN) case KW_##NAME: \
                                fprintf(DMP_STREAM, \
                                        "node%d [label = \"%d\\n%s\", shape = rect]\n", \
                                        node_count, curr, SIGN); \
                                break;
static void
kw_node_graph_dump(tree_t *tree, int curr, int node_count)
{
        switch (tree->nodes[curr].data.val.kw) {
                #include "../keywords.inc"
                default:
                        log("Invalid type encountered: %d.\n", 
                                        tree->nodes[curr].data.val.op);
                        assert(0 && "Invalid operation type encountered.");
                        break;
        }
}
#undef DEF_KW

// Creates a graphviz dump of node and all nide's children.
static void
node_graph_dump(tree_t *tree, int curr, int prev, const char *color)
{
        static int node_count = 1;

        // Need to add switch to print data correctly.
        switch (tree->nodes[curr].data.type) {
                case TOK_FUNC:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\n%s\", shape = rect]\n",
                                node_count, curr, tree->nodes[curr].data.val.var);
                        break;
                case TOK_BLOCK:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\nBLOCK\", shape = rect]\n",
                                node_count, curr);
                        break;
                case TOK_EMBED:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\nEMBED %d\", shape = rect]\n",
                                node_count, curr, tree->nodes[curr].data.val.em);
                        break;
                case TOK_EXP:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\nEXP\", shape = rect]\n",
                                node_count, curr);
                        break;
                case TOK_PUNC:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\n punc\", shape = rect]\n",
                                node_count, curr);
                        break;
                case TOK_POISON:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\nVOID\", shape = rect]\n",
                                node_count, curr);
                        break;
                case TOK_DECL:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\nDECL\", shape = rect]\n",
                                node_count, curr);
                        break;
                case TOK_VAR:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\n%s\", shape = rect]\n",
                                node_count, curr, tree->nodes[curr].data.val.var);
                        break;
                case TOK_NUM:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\n%lg\", shape = rect]\n",
                                node_count, curr, tree->nodes[curr].data.val.num);
                        break;
                case TOK_OP:
                        op_node_graph_dump(tree, curr, node_count);
                        break;
                case TOK_KW:
                        kw_node_graph_dump(tree, curr, node_count);
                        break;
                case TOK_EOF:
                        fprintf(DMP_STREAM,
                                "node%d [label = \"%d\\nEOF\", shape = rect]\n",
                                node_count, curr);
                        break;
                default:
                        log("Invalid type encountered: type = %d.\n",
                                        tree->nodes[curr].data.type);
                        assert(0 && "Invalid data type encountered.");
                        break;
        }

        fprintf(DMP_STREAM,
        "node%d -> node%d [arrowhead = none, color = %s]\n",
                prev, node_count++, color);

        prev = node_count - 1;

        if (tree->nodes[curr].left != -1)
                node_graph_dump(tree, tree->nodes[curr].left, prev, "crimson");

        if (tree->nodes[curr].right != -1)
                node_graph_dump(tree, tree->nodes[curr].right, prev, "cyan");
}

char *
tree_graph_dump(tree_t *tree, var_info_t var_info)
{
        open_graph_dump();

        log("TREE DUMP\n");
        log("%s[%p] %s at %s(%d)\n",
                var_info.init_var_name, tree,
                var_info.func_name, var_info.file_name,
                var_info.line);

        fprintf(DMP_STREAM,
        "digraph G {\n"
        "ranksep = 1.5\n"
        "graph [dpi = 100]\n"
        "splines = ortho\n");

        fprintf(DMP_STREAM,
        "node0[label = \"root\", shape = rect]\n");

        node_graph_dump(tree, tree->root, 0, "deeppink");

        fprintf(DMP_STREAM, "}\n");

        fclose(DMP_STREAM);

        return generate_graph();
}

