from parse_test import *
from pycparser.c_ast import *

generator = c_generator.CGenerator()


def get_extern_while_body(ast):
    for ext in ast.ext:
        if isinstance(ext, FuncDef) and ext.decl.name == "main":
            amain_body = ext
            if amain_body is not None:
                for operation in amain_body.body:
                    if isinstance(operation, While):
                        return operation.stmt


def take_code_between_lines(lca, line1, line2):
    for operation in lca:
        if operation.coord == line1:
            print line1
        if operation.coord == line2:
            print line2
            break
        if isinstance(operation, Compound):
            take_code_between_lines(lca, line1, line2)


def get_labels(filename, labelname):
    labels = []
    lab = labelname + "="
    with open(filename) as f:
        lines = f.readlines()
    for each in lines:
        if labelname in each:
            aux = each.replace(" ", "")
            if "\n" in aux:
                aux = aux.replace("\n", "")
            aux = aux.replace(lab, "")
            aux = aux.replace(";", "")
            if aux not in labels and "old" not in aux:
                labels.append(aux)

    return labels


def get_paths_trees(ast, labels, labelname):
    nodes_dict = {}
    trees_dict = {}
    paths_dict = {}

    for i in xrange(len(labels)):
        label1 = labels[i]
        nodes_aux = []
        trees_aux = []

        for j in xrange(len(labels)):
            if j > i:
                label2 = labels[j]
                cop = copy.deepcopy(ast)

                # trees_aux.append(cop)

                # index = trees_aux.index(cop)

                nodes1 = get_label(cop, labelname, label1)
                nodes2 = get_label(cop, labelname, label2)

                # node1 = nodes1[0]
                # node2 = nodes2[0]
                for x in xrange(len(nodes1)):
                    node1 = nodes1[x]
                    for k in xrange(len(nodes2)):
                        node2 = nodes2[k]
                        cop = copy.deepcopy(ast)
                        trees_aux.append(cop)

                        aux = (node1, node2)
                        nodes_aux.append(aux)

        nodes_dict[label1] = nodes_aux
        trees_dict[label1] = trees_aux

    for i in xrange(len(labels)):
        paths = []
        label1 = labels[i]
        trees = trees_dict[label1]
        nodes = nodes_dict[label1]

        for k in xrange(len(nodes)):
            label2 = nodes[k][1].rvalue.name
            # print label1, label2
            prune_tree(get_extern_while_body(trees[k]), nodes[k][0], nodes[k][1], [], [])
            # modificare primeste nodul, nu string
            path = find_all_paths_to_label(trees[k], nodes[k][0], nodes[k][1])
            paths.append(path)

            # j += 1
        paths_dict[label1] = paths
        # print len(paths)

    return trees_dict, paths_dict


def remove_bad_paths(labels, paths_dict, labelname):
    for x in labels:
        paths = paths_dict[x]
        for i in xrange(len(paths)):
            to_remove = []
            for j in xrange(len(paths[i])):
                num = 0

                for k in xrange(len(paths[i][j])):
                    aux = paths[i][j][k]
                    if isinstance(aux, Assignment) and aux.lvalue.name == labelname:
                        num += 1
                if num > 2:
                    to_remove.append(paths[i][j])
            for x in to_remove:
                if x in paths[i]:
                    paths[i].remove(x)


def print_code(trees_dict, paths_dict, labels):
    for x in labels:
        trees = trees_dict[x]
        paths = paths_dict[x]
        for i in xrange(len(trees)):
            # print type(paths[i]), type(paths[i][0])
            generate_c_code_from_paths(paths[i], trees[i])


def add_assign_in_tree(tree, path, add_in_path):
    # list = (tree.block_items)
    to_add = []
    # list = tree.children()
    if tree is not None:  # nu am idee unde ajunge aici pe None
        for i, item in enumerate(tree.block_items):
            if isinstance(item, If):
                if isinstance(item.cond, BinaryOp):

                    if isinstance(item.cond.left, StructRef):
                        old_var = "old_" + item.cond.left.name.name

                        assign = Assignment("=", ID(old_var), ID(item.cond.left.name.name), item.coord)

                        # item.cond.left.name.name = old_var

                        aux = (item, assign)
                        add_in_path.append(aux)
                        to_add.append((i + len(to_add), assign))
                        # pun + len(to_add) pentru ca daca adaug prima data un assign la pozitia 5, practic pozitia
                        # se incrementeaza
                    else:
                        old_var = "old_" + item.cond.left.name
                        assign = Assignment("=", ID(old_var), ID(item.cond.left.name), item.coord)

                        # item.cond.left.name = old_var

                        aux = (item, assign)
                        add_in_path.append(aux)
                        to_add.append((i + len(to_add), assign))

                else:

                    old_var = "old_" + item.cond.name
                    # print "aaaa", type(item.cond.name)
                    assign = Assignment("=", ID(old_var), ID(item.cond.name), item.coord)

                    # item.cond.name = old_var

                    aux = (item, assign)
                    add_in_path.append(aux)

                    to_add.append((i + len(to_add), assign))

                add_assign_in_tree(tree.block_items[i].iftrue, path, add_in_path)
                if tree.block_items[i].iffalse is not None:
                    add_assign_in_tree(tree.block_items[i].iffalse, path, add_in_path)

    for index, element in to_add:
        tree.block_items.insert(index, element)

    return add_in_path


def add_assign_in_path(path, to_add):
    for i in xrange(len(path)):
        for k in to_add:
            if k[0] in path[i]:
                index = path[i].index(k[0])
                path[i].insert(index, k[1])


def add_ghost_assign(trees_dict, paths_dict, labels):
    for label in labels:
        trees = trees_dict[label]
        paths = paths_dict[label]

        for i in xrange(len(paths)):
            to_add_in_path = add_assign_in_tree(get_extern_while_body(trees[i]), paths[i], [])
            add_assign_in_path(paths[i], to_add_in_path)


def take_code_from_file(ast, filename, labelname):
    labels = get_labels(filename, labelname)

    trees_dict, paths_dict = get_paths_trees(ast, labels, labelname)

    remove_bad_paths(labels, paths_dict, labelname)
    # add_ghost_assign(trees_dict, paths_dict, labels)

    print_code(trees_dict,paths_dict,labels)
